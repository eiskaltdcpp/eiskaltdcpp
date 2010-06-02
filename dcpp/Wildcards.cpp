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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Wildcards.h"
using namespace std;
using namespace dcpp;
int Wildcard::wildcardfit(const char *wildcard, const char *test, bool useSet)
{
    int fit = 1;

    for (; ('\000' != *wildcard) && (1 == fit) && ('\000' != *test); wildcard++)
    {
        switch (*wildcard)
        {
            case '?':
                test++;
                break;
            case '*':
                fit = asterisk(&wildcard, &test);
                // the asterisk was skipped by asterisk() but the loop will
                // increment by itself. So we have to decrement
                wildcard--;
                break;
            case '[':
                if (useSet)
                {
                    wildcard++; // leave out the opening square bracket
                    fit = set(&wildcard, &test);
                    // we don't need to decrement the wildcard as in case
                    // of asterisk because the closing ] is still there
                    break;
                } //if we're not using the set option, fall through
            default:
                fit = (int)(*wildcard == *test);
                test++;
        }
    }
    while ((*wildcard == '*') && (1 == fit))
        // here the teststring is empty otherwise you cannot
        // leave the previous loop
        wildcard++;
    return (int)((1 == fit) && ('\0' == *test) && ('\0' == *wildcard));
}

int Wildcard::wildcardfit(const wchar_t *wildcard, const wchar_t *test, bool useSet)
{
    int fit = 1;

    for (; (L'\000' != *wildcard) && (1 == fit) && (L'\000' != *test); wildcard++)
    {
        switch (*wildcard)
        {
            case L'?':
                test++;
                break;
            case L'*':
                fit = asterisk(&wildcard, &test);
                // the asterisk was skipped by asterisk() but the loop will
                // increment by itself. So we have to decrement
                wildcard--;
                break;
            case L'[':
                if (useSet)
                {
                    wildcard++; // leave out the opening square bracket
                    fit = set(&wildcard, &test);
                    // we don't need to decrement the wildcard as in case
                    // of asterisk because the closing ] is still there
                    break;
                }//if we're not using the set option, fall through
            default:
                fit = (int)(*wildcard == *test);
                test++;
        }
    }
    while ((*wildcard == L'*') && (1 == fit))
        // here the teststring is empty otherwise you cannot
        // leave the previous loop
        wildcard++;
    return (int)((1 == fit) && (L'\0' == *test) && (L'\0' == *wildcard));
}

int Wildcard::set(const char **wildcard, const char **test)
{
    int fit = 0;
    int negation = 0;
    int at_beginning = 1;

    if ('!' == **wildcard)
    {
        negation = 1;
        (*wildcard)++;
    }
    while ((']' != **wildcard) || (1 == at_beginning))
    {
        if (0 == fit)
        {
            if (('-' == **wildcard)
                    && ((*(*wildcard - 1)) < (*(*wildcard + 1)))
                    && (']' != *(*wildcard + 1))
                    && (0 == at_beginning))
            {
                if (((**test) >= (*(*wildcard - 1)))
                        && ((**test) <= (*(*wildcard + 1))))
                {
                    fit = 1;
                    (*wildcard)++;
                }
            }
            else if ((**wildcard) == (**test))
            {
                fit = 1;
            }
        }
        (*wildcard)++;
        at_beginning = 0;
    }
    if (1 == negation)
        /* change from zero to one and vice versa */
        fit = 1 - fit;
    if (1 == fit)
        (*test)++;

    return (fit);
}

int Wildcard::set(const wchar_t **wildcard, const wchar_t **test)
{
    int fit = 0;
    int negation = 0;
    int at_beginning = 1;

    if (L'!' == **wildcard)
    {
        negation = 1;
        (*wildcard)++;
    }
    while ((L']' != **wildcard) || (1 == at_beginning))
    {
        if (0 == fit)
        {
            if ((L'-' == **wildcard)
                    && ((*(*wildcard - 1)) < (*(*wildcard + 1)))
                    && (L']' != *(*wildcard + 1))
                    && (0 == at_beginning))
            {
                if (((**test) >= (*(*wildcard - 1)))
                        && ((**test) <= (*(*wildcard + 1))))
                {
                    fit = 1;
                    (*wildcard)++;
                }
            }
            else if ((**wildcard) == (**test))
            {
                fit = 1;
            }
        }
        (*wildcard)++;
        at_beginning = 0;
    }
    if (1 == negation)
        // change from zero to one and vice versa
        fit = 1 - fit;
    if (1 == fit)
        (*test)++;

    return (fit);
}

int Wildcard::asterisk(const char **wildcard, const char **test) {
    /* Warning: uses multiple returns */
    int fit = 1;

    /* erase the leading asterisk */
    (*wildcard)++;
    while (('\000' != (**test))
            && (('?' == **wildcard)
                || ('*' == **wildcard))) {
        if ('?' == **wildcard)
            (*test)++;
        (*wildcard)++;
    }
    /* Now it could be that test is empty and wildcard contains */
    /* aterisks. Then we delete them to get a proper state */
    while ('*' == (**wildcard))
        (*wildcard)++;

    if (('\0' == (**test)) && ('\0' != (**wildcard)))
        return (fit = 0);
    if (('\0' == (**test)) && ('\0' == (**wildcard)))
        return (fit = 1);
    else {
        /* Neither test nor wildcard are empty!          */
        /* the first character of wildcard isn't in [*?] */
        if (0 == wildcardfit(*wildcard, (*test))) {
            do {
                (*test)++;
                /* skip as much characters as possible in the teststring */
                /* stop if a character match occurs */
                while (((**wildcard) != (**test))
                        && ('['  != (**wildcard))
                        && ('\0' != (**test)))
                    (*test)++;
            }
            while ((('\0' != **test)) ?
                    (0 == wildcardfit(*wildcard, (*test)))
                    : (0 != (fit = 0)));
        }
        if (('\0' == **test) && ('\0' == **wildcard))
            fit = 1;
        return (fit);
    }
}

int Wildcard::asterisk(const wchar_t **wildcard, const wchar_t **test) {
    // Warning: uses multiple returns
    int fit = 1;

    // erase the leading asterisk
    (*wildcard)++;
    while ((L'\000' != (**test))
            && ((L'?' == **wildcard)
                || (L'*' == **wildcard))) {
        if (L'?' == **wildcard)
            (*test)++;
        (*wildcard)++;
    }
    // Now it could be that test is empty and wildcard contains
    // aterisks. Then we delete them to get a proper state
    while ('*' == (**wildcard))
        (*wildcard)++;

    if ((L'\0' == (**test)) && (L'\0' != (**wildcard)))
        return (fit = 0);
    if ((L'\0' == (**test)) && (L'\0' == (**wildcard)))
        return (fit = 1);
    else {
        // Neither test nor wildcard are empty!
        // the first character of wildcard isn't in [*?]
        if (0 == wildcardfit(*wildcard, (*test))) {
            do {
                (*test)++;
                // skip as much characters as possible in the teststring
                // stop if a character match occurs
                while (((**wildcard) != (**test))
                        && (L'['  != (**wildcard))
                        && (L'\0' != (**test)))
                    (*test)++;
            }
            while (((L'\0' != **test)) ?
                    (0 == wildcardfit(*wildcard, (*test)))
                    : (0 != (fit = 0)));
        }
        if ((L'\0' == **test) && (L'\0' == **wildcard))
            fit = 1;
        return (fit);
    }
}


bool Wildcard::patternMatch(const string& text, const string& pattern, bool useSet) {
    string sText = Text::toLower(text);
    string sPattern = Text::toLower(pattern);
    return (wildcardfit(sPattern.c_str(), sText.c_str(), useSet) == 1);
}

bool Wildcard::patternMatch(const wstring& text, const wstring& pattern, bool useSet) {
    wstring sText = Text::toLower(text);
    wstring sPattern = Text::toLower(pattern);
    return (wildcardfit(sPattern.c_str(), sText.c_str(), useSet) == 1);
}

bool Wildcard::patternMatch(const string& text, const string& patternlist, char delimiter, bool useSet) {
    StringTokenizer<string> st(patternlist, delimiter);
    bool bMatched = false;
    for (StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
        bMatched = patternMatch(text, *i, useSet);
        if (bMatched)
            return true;
    }
    return bMatched;
}

bool Wildcard::patternMatch(const wstring& text, const wstring& patternlist, wchar_t delimiter, bool useSet) {
    StringTokenizer<wstring> st(patternlist, delimiter);
    bool bMatched = false;
    for (WStringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
        bMatched = patternMatch(text, *i, useSet);
        if (bMatched)
            return true;
    }
    return bMatched;
}
