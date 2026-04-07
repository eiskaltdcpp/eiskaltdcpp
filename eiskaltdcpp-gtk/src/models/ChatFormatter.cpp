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

#include "dcpp/stdinc.h"
#include "ChatFormatter.h"
#include "GtkWulforUtil.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace gtk_chat {

// ── helpers ──────────────────────────────────────────────────────────────────

namespace {

/// Case-insensitive prefix check (ASCII only — matches g_ascii_strncasecmp).
bool hasPrefixCI(const std::string &str, const char *prefix) {
    size_t plen = std::char_traits<char>::length(prefix);
    if (str.size() < plen) return false;
    for (size_t i = 0; i < plen; ++i)
        if (std::tolower(static_cast<unsigned char>(str[i])) !=
            std::tolower(static_cast<unsigned char>(prefix[i])))
            return false;
    return true;
}

/// Case-insensitive search for a suffix like "[/b]" inside str, returning
/// the position or std::string::npos.
std::string::size_type rfindCI(const std::string &str, const char *tag) {
    size_t tlen = std::char_traits<char>::length(tag);
    if (str.size() < tlen) return std::string::npos;
    for (size_t i = str.size() - tlen; ; --i) {
        bool match = true;
        for (size_t j = 0; j < tlen; ++j) {
            if (std::tolower(static_cast<unsigned char>(str[i + j])) !=
                std::tolower(static_cast<unsigned char>(tag[j]))) {
                match = false;
                break;
            }
        }
        if (match) return i;
        if (i == 0) break;
    }
    return std::string::npos;
}

/// Try to match a BBCode tag pair like [b]...[/b].
/// Returns true if matched; inner is set to the text between tags.
bool tryBBCode(const std::string &token, const char *openTag, const char *closeTag,
               std::string &inner) {
    size_t openLen = std::char_traits<char>::length(openTag);
    if (!hasPrefixCI(token, openTag)) return false;
    auto closePos = rfindCI(token, closeTag);
    if (closePos == std::string::npos || closePos < openLen)
        return false;
    inner = token.substr(openLen, closePos - openLen);
    return true;
}

} // anonymous namespace

// ── classifyToken ────────────────────────────────────────────────────────────

ChatSegment classifyToken(const std::string &token,
                          bool &isFirstNick,
                          bool splitMagnets) {
    ChatSegment seg;
    seg.text = token;

    // Nick detection — first token matching <...>
    if (!isFirstNick && token.size() >= 3 &&
        token.front() == '<' && token.back() == '>') {
        isFirstNick = true;
        seg.type = SegmentType::NICK;
        // Store just the nick name (without angle brackets) in extra
        seg.extra = token.substr(1, token.size() - 2);
        return seg;
    }

    // BBCode: [img]magnet[/img]
    std::string inner;
    if (tryBBCode(token, "[img]", "[/img]", inner)) {
        if (gtk_util::isMagnet(inner)) {
            seg.type = SegmentType::IMAGE;
            seg.extra = inner;        // magnet URI
            seg.text = inner;         // display text
            return seg;
        }
    }

    // BBCode: [b]text[/b]
    if (tryBBCode(token, "[b]", "[/b]", inner)) {
        seg.type = SegmentType::BOLD;
        seg.text = inner;
        return seg;
    }

    // BBCode: [i]text[/i]
    if (tryBBCode(token, "[i]", "[/i]", inner)) {
        seg.type = SegmentType::ITALIC;
        seg.text = inner;
        return seg;
    }

    // BBCode: [u]text[/u]
    if (tryBBCode(token, "[u]", "[/u]", inner)) {
        seg.type = SegmentType::UNDERLINE;
        seg.text = inner;
        return seg;
    }

    // URL detection (order: link → hubURL → magnet)
    if (gtk_util::isLink(token)) {
        seg.type = SegmentType::URL;
        seg.extra = token;
        return seg;
    }
    if (gtk_util::isHubURL(token)) {
        seg.type = SegmentType::HUB_URL;
        seg.extra = token;
        return seg;
    }
    if (gtk_util::isMagnet(token)) {
        seg.type = SegmentType::MAGNET;
        seg.extra = token;
        if (splitMagnets) {
            std::string line;
            if (gtk_util::splitMagnet(token, line))
                seg.text = line;       // "name (size)" display
        }
        return seg;
    }

    // Plain text
    seg.type = SegmentType::TEXT;
    return seg;
}

// ── parseMessage ─────────────────────────────────────────────────────────────

std::vector<ChatSegment> parseMessage(const std::string &message,
                                      const ParseOptions &opts) {
    std::vector<ChatSegment> segments;
    if (message.empty()) return segments;

    std::string remaining = message;

    // 1. Timestamp extraction — "[HH:MM:SS] " or "[HH:MM] "
    if (opts.timestamped && remaining.size() > 2 && remaining[0] == '[') {
        auto closePos = remaining.find(']');
        if (closePos != std::string::npos && closePos < 12) {
            // Include the bracket and following space
            size_t endPos = closePos + 1;
            if (endPos < remaining.size() && remaining[endPos] == ' ')
                ++endPos;
            ChatSegment ts;
            ts.type = SegmentType::TIMESTAMP;
            ts.text = remaining.substr(0, endPos);
            segments.push_back(std::move(ts));
            remaining = remaining.substr(endPos);
        }
    }

    // 2. Split on whitespace and classify each token
    bool firstNick = false;
    std::istringstream iss(remaining);
    std::string token;
    bool first = true;

    // We need to preserve original spacing. Walk character by character.
    size_t pos = 0;
    while (pos < remaining.size()) {
        // Skip and collect whitespace
        size_t wsStart = pos;
        while (pos < remaining.size() && (remaining[pos] == ' ' || remaining[pos] == '\t'))
            ++pos;
        if (pos > wsStart && !first) {
            ChatSegment ws;
            ws.type = SegmentType::TEXT;
            ws.text = remaining.substr(wsStart, pos - wsStart);
            segments.push_back(std::move(ws));
        } else if (pos > wsStart && first) {
            // Leading whitespace
            ChatSegment ws;
            ws.type = SegmentType::TEXT;
            ws.text = remaining.substr(wsStart, pos - wsStart);
            segments.push_back(std::move(ws));
        }
        if (pos >= remaining.size()) break;

        // Collect token (non-whitespace)
        size_t tokStart = pos;
        while (pos < remaining.size() && remaining[pos] != ' ' && remaining[pos] != '\t')
            ++pos;
        token = remaining.substr(tokStart, pos - tokStart);
        segments.push_back(classifyToken(token, firstNick, opts.splitMagnets));
        first = false;
    }

    return segments;
}

// ── matchEmoticons ───────────────────────────────────────────────────────────

std::vector<ChatSegment> matchEmoticons(const std::string &text,
                                        const std::set<std::string> &triggers) {
    std::vector<ChatSegment> result;
    if (text.empty() || triggers.empty()) {
        if (!text.empty()) {
            ChatSegment s;
            s.type = SegmentType::TEXT;
            s.text = text;
            result.push_back(std::move(s));
        }
        return result;
    }

    size_t pos = 0;
    int emoticonCount = 0;
    constexpr int MAX_EMOTICONS = 48;

    while (pos < text.size() && emoticonCount < MAX_EMOTICONS) {
        // Find the earliest trigger match starting at or after pos
        size_t bestPos = std::string::npos;
        std::string bestTrigger;

        for (const auto &trigger : triggers) {
            auto found = text.find(trigger, pos);
            if (found != std::string::npos) {
                if (found < bestPos || (found == bestPos && trigger.size() > bestTrigger.size())) {
                    bestPos = found;
                    bestTrigger = trigger;
                }
            }
        }

        if (bestPos == std::string::npos) break;

        // Emit text before the trigger
        if (bestPos > pos) {
            ChatSegment ts;
            ts.type = SegmentType::TEXT;
            ts.text = text.substr(pos, bestPos - pos);
            result.push_back(std::move(ts));
        }

        // Emit the emoticon
        ChatSegment es;
        es.type = SegmentType::EMOTICON;
        es.text = bestTrigger;
        result.push_back(std::move(es));
        ++emoticonCount;

        pos = bestPos + bestTrigger.size();
    }

    // Emit remaining text
    if (pos < text.size()) {
        ChatSegment ts;
        ts.type = SegmentType::TEXT;
        ts.text = text.substr(pos);
        result.push_back(std::move(ts));
    }

    return result;
}

// ── Convenience extractors ───────────────────────────────────────────────────

std::vector<std::string> extractUrls(const std::string &text) {
    std::vector<std::string> urls;
    std::istringstream iss(text);
    std::string token;
    while (iss >> token) {
        if (gtk_util::isLink(token) || gtk_util::isHubURL(token))
            urls.push_back(token);
    }
    return urls;
}

std::vector<std::string> extractMagnets(const std::string &text) {
    std::vector<std::string> magnets;
    std::istringstream iss(text);
    std::string token;
    while (iss >> token) {
        if (gtk_util::isMagnet(token))
            magnets.push_back(token);
    }
    return magnets;
}

} // namespace gtk_chat
