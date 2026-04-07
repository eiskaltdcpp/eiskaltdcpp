/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

/**
 * Widget-independent chat message parser.
 *
 * Extracts the token classification logic from Hub::applyTags_gui() into a
 * pure function that takes a message string and returns a list of typed
 * segments.  No GTK dependency.
 *
 * Segment types mirror the Tag::TypeTag / link-kind classification used by
 * the original Hub::applyTags_gui(), but expressed as a flat enum that is
 * independent of GTK text tags.
 */
namespace gtk_chat {

/// Segment types emitted by the parser.
enum class SegmentType {
    TEXT,       ///< Plain, unclassified text
    TIMESTAMP,  ///< "[HH:MM:SS] " prefix
    NICK,       ///< "<nick>" token (angle-bracketed nick)
    BOLD,       ///< [b]text[/b]
    ITALIC,     ///< [i]text[/i]
    UNDERLINE,  ///< [u]text[/u]
    IMAGE,      ///< [img]magnet-uri[/img]
    URL,        ///< http, https, www, ftp, sftp, irc, ircs, im, mailto, news
    HUB_URL,    ///< dchub, nmdcs, adc, adcs
    MAGNET,     ///< magnet:? link
    EMOTICON    ///< Matched emoticon trigger text
};

/// Nick classification — mirrors the Tag::TypeTag nick subtypes.
enum class NickType {
    NORMAL,
    MYOWN,
    OPERATOR,
    FAVORITE
};

/// A single parsed segment of a chat message.
struct ChatSegment {
    SegmentType type = SegmentType::TEXT;
    std::string text;       ///< Visible / inner text
    std::string extra;      ///< URL target, magnet URI, emoticon file, etc.
    NickType    nickType = NickType::NORMAL; ///< Only relevant when type == NICK
};

/// Options controlling the parser.
struct ParseOptions {
    bool timestamped  = false;  ///< Whether the message has a leading timestamp
    bool splitMagnets = false;  ///< Replace raw magnet URIs with "name (size)"
};

/**
 * Parse a single whitespace-delimited token and classify it.
 *
 * This is the core classification extracted from Hub::applyTags_gui().
 * It handles BBCode ([b], [i], [u], [img]), URL/hub-URL/magnet detection,
 * and nick detection (leading <nick> token).
 *
 * @param token          A single whitespace-delimited word from the message.
 * @param isFirstNick    Whether we have already seen the nick token.  Updated
 *                       by the function if this token is the nick.
 * @param splitMagnets   If true, replace magnet URI with "name (size)".
 * @return               The classified segment.
 */
ChatSegment classifyToken(const std::string &token,
                          bool &isFirstNick,
                          bool splitMagnets = false);

/**
 * Parse a complete chat message line into a list of segments.
 *
 * 1. If timestamped, the leading "[HH:MM:SS] " is emitted as TIMESTAMP.
 * 2. The remainder is split on whitespace; each token is classified via
 *    classifyToken().
 * 3. Spaces between tokens are re-inserted as TEXT segments so that
 *    concatenating all segment texts reproduces the original message.
 *
 * Emoticon matching is NOT done here — it requires the emoticon pack data
 * and is provided separately by matchEmoticons().
 */
std::vector<ChatSegment> parseMessage(const std::string &message,
                                      const ParseOptions &opts = {});

/**
 * Scan text for emoticon trigger strings and split into TEXT / EMOTICON
 * segments.
 *
 * @param text      The plain text to scan (typically the text of a TEXT segment).
 * @param triggers  The set of known emoticon trigger strings (e.g. ":)", ":-P").
 * @return          A list of TEXT and EMOTICON segments that, concatenated, equal
 *                  the input text.  Returns a single TEXT segment when no
 *                  triggers match.
 */
std::vector<ChatSegment> matchEmoticons(const std::string &text,
                                        const std::set<std::string> &triggers);

/**
 * Extract all URLs from text (convenience wrapper).
 * Splits on whitespace and returns tokens matching isLink().
 */
std::vector<std::string> extractUrls(const std::string &text);

/**
 * Extract all magnet links from text (convenience wrapper).
 */
std::vector<std::string> extractMagnets(const std::string &text);

} // namespace gtk_chat
