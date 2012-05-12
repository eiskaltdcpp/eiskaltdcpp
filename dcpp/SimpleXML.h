/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 *
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

#include "Exception.h"
#include "Streams.h"
#include "SimpleXMLReader.h"

namespace dcpp {

STANDARD_EXCEPTION(SimpleXMLException);

/**
 * A simple XML class that loads an XML-ish structure into an internal tree
 * and allows easy access to each element through a "current location".
 */
class SimpleXML : private boost::noncopyable
{
public:
    SimpleXML() : root("BOGUSROOT", Util::emptyString, NULL), current(&root), found(false) {
        resetCurrentChild();
    }
    ~SimpleXML() { }

    void addTag(const string& aName, const string& aData = Util::emptyString);
    void addTag(const string& aName, int aData) {
        addTag(aName, Util::toString(aData));
    }
    void addTag(const string& aName, int64_t aData) {
        addTag(aName, Util::toString(aData));
    }

    template<typename T>
    void addAttrib(const string& aName, const T& aData) {
        addAttrib(aName, Util::toString(aData));
    }

    void addAttrib(const string& aName, const string& aData);
    void addAttrib(const string& aName, bool aData) {
        addAttrib(aName, string(aData ? "1" : "0"));
    }

    template <typename T>
    void addChildAttrib(const string& aName, const T& aData) {
        addChildAttrib(aName, Util::toString(aData));
    }
    void addChildAttrib(const string& aName, const string& aData);
    void addChildAttrib(const string& aName, bool aData) {
        addChildAttrib(aName, string(aData ? "1" : "0"));
    }

    const string& getData() const {
        dcassert(current != NULL);
        return current->data;
    }

    void stepIn() {
        checkChildSelected();
        current = *currentChild;
        currentChild = current->children.begin();
        found = false;
    }

    void stepOut() {
        if(current == &root)
            throw SimpleXMLException("Already at lowest level");

        dcassert(current->parent != NULL);

        currentChild = find(current->parent->children.begin(), current->parent->children.end(), current);

        current = current->parent;
        found = true;
    }

    void resetCurrentChild() noexcept {
        found = false;
        dcassert(current != NULL);
        currentChild = current->children.begin();
    }

    bool findChild(const string& aName) noexcept;

    const string& getChildData() const {
        checkChildSelected();
        return (*currentChild)->data;
    }

    const string& getChildAttrib(const string& aName, const string& aDefault = Util::emptyString) {
        checkChildSelected();
        return (*currentChild)->getAttrib(aName, aDefault);
    }

    int getIntChildAttrib(const string& aName) {
        checkChildSelected();
        return Util::toInt(getChildAttrib(aName));
    }
    int64_t getLongLongChildAttrib(const string& aName) {
        checkChildSelected();
        return Util::toInt64(getChildAttrib(aName));
    }
    bool getBoolChildAttrib(const string& aName) {
        checkChildSelected();
        const string& tmp = getChildAttrib(aName);

        return !tmp.empty() && tmp[0] == '1';
    }

    void fromXML(const string& aXML);
    string toXML() { string tmp; StringOutputStream os(tmp); toXML(&os); return tmp; }
    void toXML(OutputStream* f) { if(!root.children.empty()) root.children[0]->toXML(0, f); }

    static const string& escape(const string& str, string& tmp, bool aAttrib, bool aLoading = false, const string &encoding = Text::utf8) {
        if(needsEscape(str, aAttrib, aLoading, encoding)) {
            tmp = str;
            return escape(tmp, aAttrib, aLoading, encoding);
        }
        return str;
    }
    static string& escape(string& aString, bool aAttrib, bool aLoading = false, const string &encoding = Text::utf8);
    /**
     * This is a heuristic for whether escape needs to be called or not. The results are
     * only guaranteed for false, i e sometimes true might be returned even though escape
     * was not needed...
     */
    static bool needsEscape(const string& aString, bool aAttrib, bool aLoading = false, const string &encoding = Text::utf8) {
        return Util::stricmp(encoding, Text::utf8) != 0 || (((aLoading) ? aString.find('&') : aString.find_first_of(aAttrib ? "<&>'\"" : "<&>")) != string::npos);
    }
    static const string utf8Header;
private:
    class Tag {
    public:
        typedef Tag* Ptr;
        typedef vector<Ptr> List;
        typedef List::iterator Iter;

        /**
         * A simple list of children. To find a tag, one must search the entire list.
         */
        List children;
        /**
         * Attributes of this tag. According to the XML standard the names
         * must be unique (case-sensitive). (Assuming that we have few attributes here,
         * we use a vector instead of a (hash)map to save a few bytes of memory and unnecessary
         * calls to the memory allocator...)
         */
        StringPairList attribs;

        /** Tag name */
        string name;

        /** Tag data, may be empty. */
        string data;

        /** Parent tag, for easy traversal */
        Ptr parent;

        Tag(const string& aName, const StringPairList& a, Ptr aParent) : attribs(a), name(aName), data(), parent(aParent) {
        }

        Tag(const string& aName, const string& d, Ptr aParent) : name(aName), data(d), parent(aParent) {
        }

        const string& getAttrib(const string& aName, const string& aDefault = Util::emptyString) {
            StringPairIter i = find_if(attribs.begin(), attribs.end(), CompareFirst<string,string>(aName));
            return (i == attribs.end()) ? aDefault : i->second;
        }
        void toXML(int indent, OutputStream* f);

        void appendAttribString(string& tmp);
        /** Delete all children! */
        ~Tag() {
            for(Iter i = children.begin(); i != children.end(); ++i) {
                delete *i;
            }
        }

    private:
        Tag(const Tag&);
        Tag& operator=(Tag&);
    };

    class TagReader : public SimpleXMLReader::CallBack {
    public:
        TagReader(Tag* root) : cur(root) { }
        virtual bool getData(string&) { return false; }
        virtual void startTag(const string& name, StringPairList& attribs, bool simple) {
            cur->children.push_back(new Tag(name, attribs, cur));
            if(!simple)
                cur = cur->children.back();
        }
        virtual void endTag(const string&, const string& d) {
            cur->data = d;
            if(cur->parent == NULL)
                throw SimpleXMLException("Invalid end tag");
            cur = cur->parent;
        }

        Tag* cur;
    };

    /** Bogus root tag, should have only one child! */
    Tag root;

    /** Current position */
    Tag::Ptr current;

    Tag::Iter currentChild;

    void checkChildSelected() const noexcept {
        dcassert(current != NULL);
        dcassert(currentChild != current->children.end());
    }

    bool found;
};

} // namespace dcpp
