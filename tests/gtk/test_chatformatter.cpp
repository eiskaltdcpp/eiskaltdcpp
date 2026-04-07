/*
 * Tests for ChatFormatter — Phase 4 complex logic extraction.
 */
#include <catch2/catch_test_macros.hpp>
#include "ChatFormatter.h"
#include "GtkWulforUtil.h"  // for isMagnet, isLink, etc.

#include <set>
#include <string>
#include <vector>

using namespace gtk_chat;

// ── classifyToken ──

TEST_CASE("ChatFormatter: plain text token", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("hello", firstNick, false);
    REQUIRE(seg.type == SegmentType::TEXT);
    REQUIRE(seg.text == "hello");
    REQUIRE_FALSE(firstNick);
}

TEST_CASE("ChatFormatter: nick detection <user>", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("<Alice>", firstNick, false);
    REQUIRE(seg.type == SegmentType::NICK);
    REQUIRE(seg.extra == "Alice");
    REQUIRE(firstNick == true);
}

TEST_CASE("ChatFormatter: nick detected only once", "[gtk][chat]")
{
    bool firstNick = false;
    classifyToken("<Alice>", firstNick, false);
    REQUIRE(firstNick == true);

    // Second <...> token is plain text
    auto seg = classifyToken("<Bob>", firstNick, false);
    REQUIRE(seg.type == SegmentType::TEXT);
    REQUIRE(seg.text == "<Bob>");
}

TEST_CASE("ChatFormatter: BBCode [b]text[/b]", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("[b]bold[/b]", firstNick, false);
    REQUIRE(seg.type == SegmentType::BOLD);
    REQUIRE(seg.text == "bold");
}

TEST_CASE("ChatFormatter: BBCode [i]text[/i]", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("[i]italic[/i]", firstNick, false);
    REQUIRE(seg.type == SegmentType::ITALIC);
    REQUIRE(seg.text == "italic");
}

TEST_CASE("ChatFormatter: BBCode [u]text[/u]", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("[u]underlined[/u]", firstNick, false);
    REQUIRE(seg.type == SegmentType::UNDERLINE);
    REQUIRE(seg.text == "underlined");
}

TEST_CASE("ChatFormatter: BBCode case insensitive", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("[B]BOLD[/B]", firstNick, false);
    REQUIRE(seg.type == SegmentType::BOLD);
    REQUIRE(seg.text == "BOLD");
}

TEST_CASE("ChatFormatter: BBCode [img] with magnet", "[gtk][chat]")
{
    bool firstNick = false;
    std::string token = "[img]magnet:?xt=urn:tree:tiger:ABC&xl=100&dn=pic.jpg[/img]";
    auto seg = classifyToken(token, firstNick, false);
    REQUIRE(seg.type == SegmentType::IMAGE);
    REQUIRE_FALSE(seg.extra.empty());
}

TEST_CASE("ChatFormatter: [img] without magnet is TEXT", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("[img]not-a-magnet[/img]", firstNick, false);
    // Not a valid magnet, so not IMAGE — falls through to TEXT
    REQUIRE(seg.type == SegmentType::TEXT);
}

TEST_CASE("ChatFormatter: URL detection http", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("https://example.com", firstNick, false);
    REQUIRE(seg.type == SegmentType::URL);
    REQUIRE(seg.extra == "https://example.com");
}

TEST_CASE("ChatFormatter: hub URL detection", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("adc://hub.example.com:1234", firstNick, false);
    REQUIRE(seg.type == SegmentType::HUB_URL);
    REQUIRE(seg.extra == "adc://hub.example.com:1234");
}

TEST_CASE("ChatFormatter: magnet detection", "[gtk][chat]")
{
    bool firstNick = false;
    std::string magnet = "magnet:?xt=urn:tree:tiger:ABC&xl=100&dn=test.txt";
    auto seg = classifyToken(magnet, firstNick, false);
    REQUIRE(seg.type == SegmentType::MAGNET);
    REQUIRE(seg.extra == magnet);
}

TEST_CASE("ChatFormatter: malformed BBCode [b] without close", "[gtk][chat]")
{
    bool firstNick = false;
    auto seg = classifyToken("[b]nope", firstNick, false);
    REQUIRE(seg.type == SegmentType::TEXT);
    REQUIRE(seg.text == "[b]nope");
}

// ── parseMessage ──

TEST_CASE("ChatFormatter: parseMessage empty", "[gtk][chat]")
{
    auto segs = parseMessage("");
    REQUIRE(segs.empty());
}

TEST_CASE("ChatFormatter: parseMessage plain text", "[gtk][chat]")
{
    auto segs = parseMessage("hello world");
    // "hello" TEXT, " " TEXT, "world" TEXT
    REQUIRE(segs.size() == 3);
    REQUIRE(segs[0].type == SegmentType::TEXT);
    REQUIRE(segs[0].text == "hello");
    REQUIRE(segs[2].text == "world");
}

TEST_CASE("ChatFormatter: parseMessage with timestamp", "[gtk][chat]")
{
    ParseOptions opts;
    opts.timestamped = true;
    auto segs = parseMessage("[12:34] hello", opts);
    REQUIRE(segs.size() >= 2);
    REQUIRE(segs[0].type == SegmentType::TIMESTAMP);
    REQUIRE(segs[0].text == "[12:34] ");
}

TEST_CASE("ChatFormatter: parseMessage with nick and text", "[gtk][chat]")
{
    auto segs = parseMessage("<Alice> hello there");
    // "<Alice>" NICK, " " TEXT, "hello" TEXT, " " TEXT, "there" TEXT
    REQUIRE(segs.size() == 5);
    REQUIRE(segs[0].type == SegmentType::NICK);
    REQUIRE(segs[0].extra == "Alice");
    REQUIRE(segs[2].text == "hello");
}

TEST_CASE("ChatFormatter: parseMessage with URL and BBCode", "[gtk][chat]")
{
    auto segs = parseMessage("[b]bold[/b] https://test.com");
    REQUIRE(segs.size() == 3);
    REQUIRE(segs[0].type == SegmentType::BOLD);
    REQUIRE(segs[0].text == "bold");
    REQUIRE(segs[2].type == SegmentType::URL);
}

TEST_CASE("ChatFormatter: parseMessage preserves spacing", "[gtk][chat]")
{
    auto segs = parseMessage("a b");
    REQUIRE(segs.size() == 3);
    REQUIRE(segs[1].type == SegmentType::TEXT);
    REQUIRE(segs[1].text == " ");
}

// ── matchEmoticons ──

TEST_CASE("ChatFormatter: matchEmoticons finds triggers", "[gtk][chat]")
{
    std::set<std::string> triggers = {":)", ":-)"};
    auto segs = matchEmoticons("hi :) there", triggers);
    REQUIRE(segs.size() == 3);
    REQUIRE(segs[0].type == SegmentType::TEXT);
    REQUIRE(segs[0].text == "hi ");
    REQUIRE(segs[1].type == SegmentType::EMOTICON);
    REQUIRE(segs[1].text == ":)");
    REQUIRE(segs[2].type == SegmentType::TEXT);
    REQUIRE(segs[2].text == " there");
}

TEST_CASE("ChatFormatter: matchEmoticons no match", "[gtk][chat]")
{
    std::set<std::string> triggers = {":)"};
    auto segs = matchEmoticons("no emoticons here", triggers);
    REQUIRE(segs.size() == 1);
    REQUIRE(segs[0].type == SegmentType::TEXT);
}

TEST_CASE("ChatFormatter: matchEmoticons empty triggers", "[gtk][chat]")
{
    std::set<std::string> triggers;
    auto segs = matchEmoticons("hi :)", triggers);
    REQUIRE(segs.size() == 1);
    REQUIRE(segs[0].type == SegmentType::TEXT);
}

TEST_CASE("ChatFormatter: matchEmoticons max limit", "[gtk][chat]")
{
    std::set<std::string> triggers = {"x"};
    // String with 60 'x' chars — should cap at 48 emoticon matches
    std::string text(60, 'x');
    auto segs = matchEmoticons(text, triggers);
    int emoticonCount = 0;
    for (const auto &s : segs)
        if (s.type == SegmentType::EMOTICON) ++emoticonCount;
    REQUIRE(emoticonCount == 48);
    // Remaining 12 chars as TEXT
    bool hasTrailingText = false;
    for (const auto &s : segs)
        if (s.type == SegmentType::TEXT) hasTrailingText = true;
    REQUIRE(hasTrailingText);
}

TEST_CASE("ChatFormatter: matchEmoticons prefers longer trigger at same pos", "[gtk][chat]")
{
    std::set<std::string> triggers = {":)", ":-)"};
    auto segs = matchEmoticons(":-)", triggers);
    REQUIRE(segs.size() == 1);
    REQUIRE(segs[0].type == SegmentType::EMOTICON);
    REQUIRE(segs[0].text == ":-)");
}

// ── extractUrls / extractMagnets ──

TEST_CASE("ChatFormatter: extractUrls finds URLs", "[gtk][chat]")
{
    auto urls = extractUrls("visit https://example.com and adc://hub.test");
    REQUIRE(urls.size() == 2);
    REQUIRE(urls[0] == "https://example.com");
    REQUIRE(urls[1] == "adc://hub.test");
}

TEST_CASE("ChatFormatter: extractUrls empty on no URLs", "[gtk][chat]")
{
    auto urls = extractUrls("no links here");
    REQUIRE(urls.empty());
}

TEST_CASE("ChatFormatter: extractMagnets finds magnets", "[gtk][chat]")
{
    std::string msg = "get magnet:?xt=urn:tree:tiger:ABC&dn=file.txt now";
    auto magnets = extractMagnets(msg);
    REQUIRE(magnets.size() == 1);
}
