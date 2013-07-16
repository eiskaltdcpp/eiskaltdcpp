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

#pragma once

#include "typedefs.h"
#include <boost/noncopyable.hpp>

namespace dcpp {

class SimpleXMLReader {
public:
    struct CallBack : private boost::noncopyable {
        virtual ~CallBack() { }
        /** A new XML tag has been encountered.
        @param name Name of the tag.
        @param attribs List of attribute name / contents pairs representing attributes of the tag.
        Use the getAttrib function to retrieve one particular attribute.
        @param simple Whether this tag is void of any data (<example/>). */
        virtual void startTag(const std::string& name, StringPairList& attribs, bool simple) { }

        /** Contents of an XML tag have been read.
        @param data Contents of the tag.
        @note This may be called several times per tag with partial contents in mixed content
        situations, such as: <outer>Data1<inner>Data2</inner>Data3</outer> (data will be called
        once for "Data1", once for "Data2", once for "Data3"). */
        virtual void data(const std::string& data) { }

        /** Contents of an XML tag have been read.
        @param name Name of the tag. */
        virtual void endTag(const std::string& name) { }
    protected:
        static const std::string& getAttrib(StringPairList& attribs, const std::string& name, size_t hint);
    };

    SimpleXMLReader(CallBack* callback);
    virtual ~SimpleXMLReader() { }

    void parse(InputStream& is, size_t maxSize = 0);
    bool parse(const char* data, size_t len);
    bool parse(const string& str);
private:

    static const size_t MAX_NAME_SIZE = 256;
    static const size_t MAX_VALUE_SIZE = 64*1024;
    static const size_t MAX_NESTING = 32;

    enum ParseState {
        /// Start of document
        STATE_START,

        /// In <?xml declaration, expect version
        STATE_DECL_VERSION,

        /// In <?xml declaration, expect =
        STATE_DECL_VERSION_EQ,

        /// In <?xml declaration, expect version number
        STATE_DECL_VERSION_NUM,

        /// In <?xml declaration, expect encoding
        STATE_DECL_ENCODING,

        /// In <?xml declaration, expect =
        STATE_DECL_ENCODING_EQ,

        /// In <?xml declaration, expect encoding name
        STATE_DECL_ENCODING_NAME,

        STATE_DECL_ENCODING_NAME_APOS,

        STATE_DECL_ENCODING_NAME_QUOT,

        /// in <?xml declaration, expect standalone
        STATE_DECL_STANDALONE,

        /// in <?xml declaration, expect =
        STATE_DECL_STANDALONE_EQ,

        /// in <?xml declaration, expect standalone yes
        STATE_DECL_STANDALONE_YES,

        /// In <?xml declaration, expect %>
        STATE_DECL_END,

        /// In < element, expect element name
        STATE_ELEMENT_NAME,

        /// In < element, expect attribute or element end
        STATE_ELEMENT_ATTR,

        /// In < element, in attribute name
        STATE_ELEMENT_ATTR_NAME,

        /// In < element, expect %
        STATE_ELEMENT_ATTR_EQ,

        /// In < element, waiting for attribute value start
        STATE_ELEMENT_ATTR_VALUE,

        STATE_ELEMENT_ATTR_VALUE_QUOT,

        STATE_ELEMENT_ATTR_VALUE_APOS,

        STATE_ELEMENT_END_SIMPLE,

        STATE_ELEMENT_END,

        STATE_ELEMENT_END_END,

        /// In <!-- comment field
        STATE_COMMENT,

        STATE_CONTENT,

        STATE_END
    };


    std::string buf;
    std::string::size_type bufPos;
    uint64_t pos;

    StringPairList attribs;
    std::string value;

    CallBack* cb;
    std::string encoding;

    ParseState state;

    StringList elements;

    void append(std::string& str, size_t maxLen, int c);
    void append(std::string& str, size_t maxLen, std::string::const_iterator begin, std::string::const_iterator end);

    bool needChars(size_t n) const;
    int charAt(size_t n) const;
    bool skipSpace(bool store = false);
    void advancePos(size_t n = 1);
    std::string::size_type bufSize() const;

    bool literal(const char* lit, size_t len, bool withSpace, ParseState newState);
    bool character(int c, ParseState newState);

    bool declVersionNum();
    bool declEncodingValue();

    bool element();
    bool elementName();
    bool elementEnd();
    bool elementEndEnd();
    bool elementEndSimple();
    bool elementEndComplex();
    bool elementAttr();
    bool elementAttrName();
    bool elementAttrValue();

    bool comment();

    bool content();

    bool entref(std::string& d);

    bool process();
    bool spaceOrError(const char* error);

    bool error(const char* message);
};

}
