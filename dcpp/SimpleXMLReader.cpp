/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "stdinc.h"

#include "SimpleXMLReader.h"
#include "SimpleXML.h"

#include "Text.h"
#include "Streams.h"

namespace dcpp {

static bool isSpace(int c) {
        return c == 0x20 || c == 0x09 || c == 0x0d || c == 0x0a;
}

static bool inRange(int c, int a, int b) {
        return c >= a && c <= b;
}

static bool isNameStartChar(int c) {
    return  c == ':'
            || inRange(c, 'A', 'Z')
            || c == '_'
            || inRange(c, 'a', 'z')
            // Comment out some valid XML chars that we don't allow
            || c == '+' //NOTE: freedcpp
/*              || inRange(c, 0xC0, 0xD6)
            || inRange(c, 0xD8, 0xF6)
            || inRange(c, 0xF8, 0x2FF)
            || inRange(c, 0x370, 0x37D)
            || inRange(c, 0x37F, 0x1FFF)
            || inRange(c, 0x200C, 0x200D)
            || inRange(c, 0x2070, 0x218F)
            || inRange(c, 0x2C00, 0x2FEF)
            || inRange(c, 0x3001, 0xD7FF)
            || inRange(c, 0xF900, 0xFDCF)
            || inRange(c, 0xFDF0, 0xFFFD)
            || inRange(c, 0x10000, 0xEFFFF) */
            ;
}

static bool isNameChar(int c) {
    return isNameStartChar(c)
            || c == '-'
            || c == '.'
            || inRange(c, '0', '9')
            // Again, real XML is more permissive
/*              || c == 0xB7
            || inRange(c, 0x0300, 0x036F)
            || inRange(c, 0x203F, 0x2040) */
            ;
}

SimpleXMLReader::SimpleXMLReader(SimpleXMLReader::CallBack* callback) :
    bufPos(0), pos(0), cb(callback), state(STATE_START)
{
    elements.reserve(64);
    attribs.reserve(16);
}

void SimpleXMLReader::append(std::string& str, size_t maxLen, int c) {
    if(str.size() + 1 > maxLen) {
        error("Buffer overflow");
    }
    str.append(1, (std::string::value_type)c);
}

void SimpleXMLReader::append(std::string& str, size_t maxLen, std::string::const_iterator begin, std::string::const_iterator end) {
    if(str.size() + (end - begin) > maxLen) {
        error("Buffer overflow");
    }
    str.append(begin, end);
}

/// @todo This is cheating - we should be converting from the encoding, but since we simplify a few things
/// this is ok
int SimpleXMLReader::charAt(size_t n) const { return buf[bufPos + n]; }
void SimpleXMLReader::advancePos(size_t n) { bufPos += n; pos += n; }
string::size_type SimpleXMLReader::bufSize() const { return buf.size() - bufPos; }

bool SimpleXMLReader::error(const char* e) {
    throw SimpleXMLException(Util::toString(pos) + ": " + e);
}

const string& SimpleXMLReader::CallBack::getAttrib(StringPairList& attribs, const string& name, size_t hint) {
    hint = min(hint, attribs.size());

    StringPairIter i = find_if(attribs.begin() + hint, attribs.end(), CompareFirst<string, string>(name));
    if(i == attribs.end()) {
        i = find_if(attribs.begin(), attribs.begin() + hint, CompareFirst<string, string>(name));
        return ((i == (attribs.begin() + hint)) ? Util::emptyString : i->second);
    } else {
        return i->second;
    }
}

bool SimpleXMLReader::literal(const char* lit, size_t len, bool withSpace, ParseState newState) {
    string::size_type n = 0, nend = bufSize();
    for(; n < nend && n < len; ++n) {
        if(charAt(n) != lit[n]) {
            return false;
        }
    }

    if(n == len) {
        if(withSpace) {
            if(n == nend) {
                return true;
            }
            if(!isSpace(charAt(n))) {
                return false;
            }
            n++;
        }
        advancePos(n);
        state = newState;
    }

    return true;
}

bool SimpleXMLReader::element() {
    if(!needChars(2)) {
            return true;
    }

    int c = charAt(1);
        if(charAt(0) == '<' && isNameStartChar(c)) {
            if(elements.size() >= MAX_NESTING) {
                error("Max nesting exceeded");
            }

            state = STATE_ELEMENT_NAME;
            elements.push_back(std::string());
            append(elements.back(), MAX_NAME_SIZE, c);

            advancePos(2);

            return true;
    }

    return false;
}

bool SimpleXMLReader::elementName() {
    size_t i = 0;
    for(size_t iend = bufSize(); i < iend; ++i) {
        int c = charAt(i);

        if(isSpace(c)) {
            append(elements.back(), MAX_NAME_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);

            state = STATE_ELEMENT_ATTR;
            advancePos(i + 1);
            return true;
        } else if(c == '/') {
            append(elements.back(), MAX_NAME_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);

            state = STATE_ELEMENT_END_SIMPLE;
            advancePos(i + 1);
            return true;
        } else if(c == '>') {
            append(elements.back(), MAX_NAME_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);

            cb->startTag(elements.back(), attribs, false);
            attribs.clear();

            state = STATE_CONTENT;
            advancePos(i + 1);
            return true;
        } else if(!isNameChar(c)) {
            return false;
        }
    }

    append(elements.back(), MAX_NAME_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);
    advancePos(i);

    return true;
}

bool SimpleXMLReader::elementAttr() {
    if(!needChars(1)) {
        return true;
    }

    int c = charAt(0);
    if(isNameStartChar(c)) {
        attribs.push_back(StringPair());
        append(attribs.back().first, MAX_NAME_SIZE, c);

        state = STATE_ELEMENT_ATTR_NAME;
        advancePos(1);

        return true;
    }

    return false;
}

bool SimpleXMLReader::elementAttrName() {
    size_t i = 0;
    for(size_t iend = bufSize(); i < iend; ++i) {
        int c = charAt(i);

        if(isSpace(c)) {
            append(attribs.back().first, MAX_NAME_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);

            state = STATE_ELEMENT_ATTR_EQ;
            advancePos(i + 1);
            return true;
        } else if(c == '=') {
            append(attribs.back().first, MAX_NAME_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);

            state = STATE_ELEMENT_ATTR_VALUE;
            advancePos(i + 1);
            return true;
        } else if(!isNameChar(c)) {
            return false;
        }
    }

    append(attribs.back().first, MAX_NAME_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);
    advancePos(i);
    return true;
}

bool SimpleXMLReader::elementAttrValue() {
    size_t i = 0;
    for(size_t iend = bufSize(); i < iend; ++i) {
        int c = charAt(i);

        if((state == STATE_ELEMENT_ATTR_VALUE_APOS && c == '\'') || (state == STATE_ELEMENT_ATTR_VALUE_QUOT && c == '"')) {
            append(attribs.back().second, MAX_VALUE_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);

            if(!encoding.empty() && encoding != Text::utf8) {
                attribs.back().second = Text::toUtf8(attribs.back().second, encoding);
            }

            state = STATE_ELEMENT_ATTR;
            advancePos(i + 1);
            return true;
        } else if(c == '&') {
            append(attribs.back().second, MAX_VALUE_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);
            advancePos(i);
            return entref(attribs.back().second);
        }
    }

    append(attribs.back().second, MAX_VALUE_SIZE, buf.begin() + bufPos, buf.begin() + bufPos + i);
    advancePos(i);

    return true;
}

bool SimpleXMLReader::elementEndSimple() {
    if(!needChars(1)) {
        return true;
    }

    if(charAt(0) == '>') {
        cb->startTag(elements.back(), attribs, true);
        elements.pop_back();
        attribs.clear();

        state = STATE_CONTENT;
        advancePos(1);
        return true;
    }

    return false;
}

bool SimpleXMLReader::elementEndComplex() {
    if(!needChars(1)) {
        return true;
    }

    if(charAt(0) == '>') {
        cb->startTag(elements.back(), attribs, false);
        attribs.clear();

        state = STATE_CONTENT;
        advancePos(1);
        return true;
    }

    return false;
}

bool SimpleXMLReader::character(int character, ParseState newState) {
    if(!needChars(1)) {
        return true;
    }

    if(charAt(0) == character) {
        advancePos(1);
        state = newState;
        return true;
    }

    return false;
}

bool SimpleXMLReader::declVersionNum() {
    if(!needChars(5)) {
        return true;
    }

    const int sep = charAt(0);

    if((sep == '"' || sep == '\'') && charAt(1) == '1' && charAt(2) == '.') {
        // At least one more number
        if(!inRange(charAt(3), '0', '9')) {
                return false;
        }

        // Now an unknown number of [0-9]
        for(string::size_type n = 4, nend = bufSize(); n < nend; ++n) {
            int c = charAt(n);
            if(c == sep) {
                state = STATE_DECL_ENCODING;
                advancePos(n+1);
                return true;
            }

            if(!inRange(c, 0, 9)) {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool SimpleXMLReader::declEncodingValue() {
    while(bufSize() > 0) {
        int c = charAt(0);

        if((state == STATE_DECL_ENCODING_NAME_APOS && c == '\'') || (state == STATE_DECL_ENCODING_NAME_QUOT && c == '"')) {
            encoding = Text::toLower(encoding);
            state = STATE_DECL_STANDALONE;
            advancePos(1);
            return true;
        } else if(c == '&') {
            if(!entref(encoding)) {
                    return false;
            }
        } else {
            append(encoding, MAX_VALUE_SIZE, c);
            advancePos(1);
        }
    }

    return true;
}

bool SimpleXMLReader::comment() {
    while(bufSize() > 0) {
        int c = charAt(0);

        // TODO We shouldn't allow ---> to end a comment
        if(c == '-') {
            if(!needChars(3)) {
                return true;
            }
            if(charAt(1) == '-' && charAt(2) == '>') {
                state = STATE_CONTENT;
                advancePos(3);
                return true;
            }
        }

        advancePos(1);
    }

    return true;
}

bool SimpleXMLReader::entref(string& d) {
    if(d.size() + 1 >= MAX_VALUE_SIZE) {
        error("Buffer overflow");
    }

    if(bufSize() > 6) {
        if(charAt(1) == 'l' && charAt(2) == 't' && charAt(3) == ';') {
            d.append(1, '<');
            advancePos(4);
            return true;
        } else if(charAt(1) == 'g' && charAt(2) == 't' && charAt(3) == ';') {
            d.append(1, '>');
            advancePos(4);
            return true;
        } else if(charAt(1) == 'a' && charAt(2) == 'm' && charAt(3) == 'p' && charAt(4) == ';') {
            d.append(1, '&');
            advancePos(5);
            return true;
        } else if(charAt(1) == 'q' && charAt(2) == 'u' && charAt(3) == 'o' && charAt(4) == 't' && charAt(5) == ';') {
            d.append(1, '"');
            advancePos(6);
            return true;
        } else if(charAt(1) == 'a' && charAt(2) == 'p' && charAt(3) == 'o' && charAt(4) == 's' && charAt(5) == ';') {
            d.append(1, '\'');
            advancePos(6);
            return true;

        // Ignore &#00000 decimal and &#x0000 hex values to avoid error, they wouldn't be parsed anyway
        } else if(charAt(1) == '#' && isdigit(charAt(2)) && charAt(3) == ';') {
            advancePos(4);
            return true;
        } else if(charAt(1) == '#' && isdigit(charAt(2)) && isdigit(charAt(3)) && charAt(4) == ';') {
            advancePos(5);
            return true;
        } else if(charAt(1) == '#' && isdigit(charAt(2)) && isdigit(charAt(3)) && isdigit(charAt(4)) && charAt(5) == ';') {
            advancePos(6);
            return true;
        } else if(charAt(1) == '#' && isdigit(charAt(2)) && isdigit(charAt(3)) && isdigit(charAt(4)) && isdigit(charAt(5)) && charAt(6) == ';') {
            advancePos(7);
            return true;
        } else if(charAt(1) == '#' && isdigit(charAt(2)) && isdigit(charAt(3)) && isdigit(charAt(4)) && isdigit(charAt(5)) && isdigit(charAt(6)) && charAt(7) == ';') {
            advancePos(8);
            return true;

        } else if(charAt(1) == '#' && (charAt(2) == 'x' ||  charAt(2) == 'X') && isxdigit(charAt(3)) && charAt(4) == ';') {
            advancePos(5);
            return true;
        } else if(charAt(1) == '#' && (charAt(2) == 'x' ||  charAt(2) == 'X') && isxdigit(charAt(3)) && isxdigit(charAt(4)) && charAt(5) == ';') {
            advancePos(6);
            return true;
        } else if(charAt(1) == '#' && (charAt(2) == 'x' ||  charAt(2) == 'X') && isxdigit(charAt(3)) && isxdigit(charAt(4)) && isxdigit(charAt(5)) && charAt(6) == ';') {
            advancePos(7);
            return true;
        } else if(charAt(1) == '#' && (charAt(2) == 'x' ||  charAt(2) == 'X') && isxdigit(charAt(3)) && isxdigit(charAt(4)) && isxdigit(charAt(5)) && isxdigit(charAt(6)) && charAt(7) == ';') {
            advancePos(8);
            return true;
        }
    } else {
        return true;
    }

    return false;
}

bool SimpleXMLReader::content() {
    if(!needChars(1)) {
        return true;
    }

    int c = charAt(0);
    if(c == '<') {
        if(!value.empty()) {
            error("Mixed content not supported");
        }
        return false;
    }

    if(c == '&') {
        return entref(value);
    }

    append(value, MAX_VALUE_SIZE, c);

    advancePos(1);

    return true;
}

bool SimpleXMLReader::elementEnd() {
    if(elements.empty()) {
        return false;
    }

    const string& top = elements.back();
    if(!needChars(top.size())) {
        return true;
    }

    if(top.compare(0, top.size(), &buf[bufPos], top.size()) == 0) {
        state = STATE_ELEMENT_END_END;
        advancePos(top.size());
        return true;
    }

    return false;
}

bool SimpleXMLReader::elementEndEnd() {
    if(!needChars(1)) {
        return true;
    }

    if(charAt(0) == '>') {
        if(!encoding.empty() && encoding != Text::utf8) {
            value = Text::toUtf8(encoding);
        }
        cb->endTag(elements.back(), value);
        value.clear();
        elements.pop_back();

        state = STATE_CONTENT;
        advancePos(1);
        return true;
    }

    return false;
}

bool SimpleXMLReader::skipSpace(bool store) {
    if(!needChars(1)) {
        return true;
    }
    bool skipped = false;
    int c;
    while(needChars(1) && isSpace(c = charAt(0))) {
        if(store) {
            append(value, MAX_VALUE_SIZE, c);
        }
        advancePos();
        skipped = true;
    }

    return skipped;
}

bool SimpleXMLReader::needChars(size_t n) const {
    return bufPos + n <= buf.size();
}

#define LITN(x) x, sizeof(x)-1

void SimpleXMLReader::parse(InputStream& stream, size_t maxSize) {
    const size_t BUF_SIZE = 64*1024;
    size_t bytesRead = 0;
    do {
        size_t old = buf.size();
        buf.resize(BUF_SIZE);

        size_t n = buf.size() - old;
        size_t len = stream.read(&buf[old], n);

        if(maxSize > 0 && (bytesRead + len) > maxSize)
            error("Greater than maximum allowed size");

        if(len == 0) {
            if(elements.empty()) {
                // Fine...
                return;
            }
            error("Unexpected end of stream");
        }
        buf.resize(old + len);
        bytesRead += len;
    } while(process());
}

bool SimpleXMLReader::parse(const char* data, size_t len, bool more) {
    buf.append(data, len);
    return process();
}
bool SimpleXMLReader::spaceOrError(const char* message) {
    if(!skipSpace()) {
        error(message);
    }
    return true;
}

bool SimpleXMLReader::process() {
    ParseState oldState = state;
    string::size_type oldPos = bufPos;

    while(true) {
        switch(state) {
        case STATE_START:
                literal(LITN("\xef\xbb\xbf"), false, STATE_START)       // Byte order mark
                || literal(LITN("<?xml"), true, STATE_DECL_VERSION)
                || literal(LITN("<!--"), false, STATE_COMMENT)
                || element()
                || spaceOrError("Expecting XML declaration, element or comment");
                break;
        case STATE_DECL_VERSION:
                skipSpace()
                || literal(LITN("version"), false, STATE_DECL_VERSION_EQ)
                || spaceOrError("Expecting version");
                break;
        case STATE_DECL_VERSION_EQ:
                character('=', STATE_DECL_VERSION_NUM)
                || spaceOrError("Expecting =");
                break;
        case STATE_DECL_VERSION_NUM:
                declVersionNum()
                || spaceOrError("Expecting version number");
                break;
        case STATE_DECL_ENCODING:
                literal(LITN("encoding"), false, STATE_DECL_ENCODING_EQ)
                || literal(LITN("standalone"), false, STATE_DECL_STANDALONE_EQ)
                || literal(LITN("?>"), false, STATE_CONTENT)
                || spaceOrError("Expecting encoding | standalone | ?>");
                break;
        case STATE_DECL_ENCODING_EQ:
                character('=', STATE_DECL_ENCODING_NAME)
                || spaceOrError("Expecting =");
                break;
        case STATE_DECL_ENCODING_NAME:
                character('\'', STATE_DECL_ENCODING_NAME_APOS)
                || character('"', STATE_DECL_ENCODING_NAME_QUOT)
                || spaceOrError("Expecting encoding name start");
                break;
        case STATE_DECL_ENCODING_NAME_APOS:
        case STATE_DECL_ENCODING_NAME_QUOT:
                declEncodingValue()
                || spaceOrError("Expecting encoding value");
        case STATE_DECL_STANDALONE:
                literal(LITN("standalone"), false, STATE_DECL_STANDALONE_EQ)
                || literal(LITN("?>"), false, STATE_CONTENT)
                || spaceOrError("Expecting standalone | ?>");
                break;
        case STATE_DECL_STANDALONE_EQ:
                character('=', STATE_DECL_STANDALONE_YES)
                || spaceOrError("Expecting =");
                break;
        case STATE_DECL_STANDALONE_YES:
                literal(LITN("\"yes\""), false, STATE_DECL_END)
                || literal(LITN("'yes'"), false, STATE_DECL_END)
                || spaceOrError("Expecting standalone=yes");
                break;
        case STATE_DECL_END:
                literal(LITN("?>"), false, STATE_CONTENT)
                || spaceOrError("Expecting ?>");
                break;
        case STATE_ELEMENT_NAME:
                elementName()
                || error("Error while parsing element start");
                break;
        case STATE_ELEMENT_END_SIMPLE:
                elementEndSimple()
                || error("Expecting >");
                break;
        case STATE_ELEMENT_END:
                elementEnd()
                || spaceOrError("Expecting element end");
                break;
        case STATE_ELEMENT_END_END:
                elementEndEnd()
                || spaceOrError("Expecting >");
                break;
        case STATE_ELEMENT_ATTR:
                elementAttr()
                || elementEndComplex()
                || character('/', STATE_ELEMENT_END_SIMPLE)
                || spaceOrError("Expecting attribute | /> | >");
                break;
        case STATE_ELEMENT_ATTR_NAME:
                elementAttrName()
                || error("Expecting attribute name");
                break;
        case STATE_ELEMENT_ATTR_EQ:
                character('=', STATE_ELEMENT_ATTR_VALUE)
                || spaceOrError("Expecting attribute =");
                break;
        case STATE_ELEMENT_ATTR_VALUE:
                character('\'', STATE_ELEMENT_ATTR_VALUE_APOS)
                || character('"', STATE_ELEMENT_ATTR_VALUE_QUOT)
                || spaceOrError("Expecting attribute value start");
                break;
        case STATE_ELEMENT_ATTR_VALUE_APOS:
        case STATE_ELEMENT_ATTR_VALUE_QUOT:
                elementAttrValue()
                || error("Expecting attribute value");
                break;
        case STATE_COMMENT:
                comment()
                || error("Error while parsing comment");
                break;
        case STATE_CONTENT:
                skipSpace(true)
                || literal(LITN("<!--"), false, STATE_COMMENT)
                || element()
                || literal(LITN("</"), false, STATE_ELEMENT_END)
                || content()
                || error("Expecting content, element or comment");
                break;
        case STATE_END:
                buf.clear();
                return false;
        default:
                error("Unexpected state"); break;
        }

        if(oldState == state && oldPos == bufPos) {
            // Need more data...
            if(bufPos > 0) {
                buf.erase(buf.begin(), buf.begin() + bufPos);
                bufPos = 0;
            }
            return true;
        }

        if(state == STATE_CONTENT && state != oldState) {
            // might contain whitespace from previous unfruitful contents (that turned out to be elements / comments)
            value.clear();
        }

        oldState = state;
        oldPos = bufPos;
    }

    // should never happen
    return false;
}

}
