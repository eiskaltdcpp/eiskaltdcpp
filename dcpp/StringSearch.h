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

#include "Text.h"

#include "noexcept.h"

namespace dcpp {

/**
 * A class that implements a fast substring search algo suited for matching
 * one pattern against many strings (currently Quick Search, a variant of
 * Boyer-Moore. Code based on "A very fast substring search algorithm" by
 * D. Sunday).
 * @todo Perhaps find an algo suitable for matching multiple substrings.
 */
class StringSearch {
public:
    typedef vector<StringSearch> List;

    explicit StringSearch(const string& aPattern) noexcept : pattern(Text::toLower(aPattern)) {
        initDelta1();
    }
    StringSearch(const StringSearch& rhs) noexcept : pattern(rhs.pattern) {
        memcpy(delta1, rhs.delta1, sizeof(delta1));
    }
    const StringSearch& operator=(const StringSearch& rhs) {
        memcpy(delta1, rhs.delta1, sizeof(delta1));
        pattern = rhs.pattern;
        return *this;
    }
    const StringSearch& operator=(const string& rhs) {
        pattern = Text::toLower(rhs);
        initDelta1();
        return *this;
    }

    bool operator==(const StringSearch& rhs) { return pattern == rhs.pattern; }

    const string& getPattern() const { return pattern; }

    /** Match a text against the pattern */
    bool match(const string& aText) const noexcept {

        // Lower-case representation of UTF-8 string, since we no longer have that 1 char = 1 byte...
        string lower;
        Text::toLower(aText, lower);

        // uint8_t to avoid problems with signed char pointer arithmetic
        uint8_t *tx = (uint8_t*)lower.c_str();
        uint8_t *px = (uint8_t*)pattern.c_str();

        string::size_type plen = pattern.length();

        if(aText.length() < plen) {
            return false;
        }

        uint8_t *end = tx + aText.length() - plen + 1;
        while(tx < end) {
            size_t i = 0;
            for(; px[i] && (px[i] == tx[i]); ++i)
                ;       // Empty!

            if(px[i] == 0)
                return true;

            tx += delta1[tx[plen]];
        }

        return false;
    }

private:
    enum { ASIZE = 256 };
    /**
     * Delta1 shift, uint16_t because we expect all patterns to be shorter than 2^16
     * chars.
     */
    uint16_t delta1[ASIZE];
    string pattern;

    void initDelta1() {
        uint16_t x = (uint16_t)(pattern.length() + 1);
        uint16_t i;
        for(i = 0; i < ASIZE; ++i) {
            delta1[i] = x;
        }
        // x = pattern.length();
        x--;
        uint8_t* p = (uint8_t*)pattern.data();
        for(i = 0; i < x; ++i) {
            delta1[p[i]] = (uint16_t)(x - i);
        }
    }
};

} // namespace dcpp
