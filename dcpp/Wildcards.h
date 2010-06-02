// Copyright (C) 1996 - 2002 Florian Schintke
// Modified 2002 by Opera, opera@home.se
//
// This is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2, or (at your option) any later
// version.
//
// Thanks to the E.S.O. - ACS project that has done this C++ interface
// to the wildcards pttern matching algorithm

#ifndef WILDCARD_H
#define WILDCARD_H

#include "StringTokenizer.h"
#include "Util.h"
#include "Text.h"

// Implementation of the UN*X wildcards
// Supported wild-characters: '*', '?'; sets: [a-z], '!' negation
// Examples:
//       '[a-g]l*i?n' matches 'florian'
//       '[!abc]*e' matches 'smile'
//       '[-z] matches 'a'

class Wildcard
{
    public:
        // This function implements the UN*X wildcards and returns:
        // 0 - if *wildcard does not match *test
        // 1 - if *wildcard matches *test
        static int wildcardfit(const char *wildcard, const char *test, bool useSet = true);
        static int wildcardfit(const wchar_t *wildcard, const wchar_t *test, bool useSet = true);

        // Checks whether a text matches a pattern
        static bool patternMatch(const std::string& text, const std::string& pattern, bool useSet = true);
        static bool patternMatch(const std::wstring& text, const std::wstring& pattern, bool useSet = true);

        // Checks whether a text matches any pattern in a patternlist
        static bool patternMatch(const std::string& text, const std::string& patternlist, char delimiter, bool useSet = true);
        static bool patternMatch(const std::wstring& text, const std::wstring& patternlist, wchar_t delimiter, bool useSet = true);

    private:
        // Scans a set of characters and returns 0 if the set mismatches at this
        // position in the teststring and 1 if it is matching
        // wildcard is set to the closing ] and test is unmodified if mismatched
        // and otherwise the char pointer is pointing to the next character
        static int set(const char **wildcard, const char **test);
        static int set(const wchar_t **wildcard, const wchar_t **test);

        // Scans an asterisk
        static int asterisk(const char **wildcard, const char **test);
        static int asterisk(const wchar_t **wildcard, const wchar_t **test);
};

#endif
