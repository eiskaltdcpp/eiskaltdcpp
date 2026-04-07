/*
 * Additional Util tests — sanitizeUrl, cleanPathChars, checkExtension,
 * validateFileName, isPrivateIp, parseIpPort, decodeQuery,
 * toAdcFile/toNmdcFile, URL protocol checks, and Segment helpers.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "Util.h"
#include "Text.h"

using namespace dcpp;

// ===== sanitizeUrl =====

TEST_CASE("Util: sanitizeUrl strips leading/trailing spaces", "[util]") {
    std::string url = "  http://example.com  ";
    Util::sanitizeUrl(url);
    REQUIRE(url == "http://example.com");
}

TEST_CASE("Util: sanitizeUrl strips angle brackets", "[util]") {
    std::string url = "<http://example.com>";
    Util::sanitizeUrl(url);
    REQUIRE(url == "http://example.com");
}

TEST_CASE("Util: sanitizeUrl strips quotes", "[util]") {
    std::string url = "\"http://example.com\"";
    Util::sanitizeUrl(url);
    REQUIRE(url == "http://example.com");
}

TEST_CASE("Util: sanitizeUrl strips tabs and newlines", "[util]") {
    // sanitizeUrl processes each special char independently in order,
    // so \r\n at end: \r is processed before \n, leaving \r behind.
    std::string url1 = "\thttp://example.com\n";
    Util::sanitizeUrl(url1);
    REQUIRE(url1 == "http://example.com");

    std::string url2 = "\r\nhttp://example.com";
    Util::sanitizeUrl(url2);
    // \r stripped first (leading), then \n stripped (now leading)
    REQUIRE(url2 == "http://example.com");
}

TEST_CASE("Util: sanitizeUrl with mixed garbage", "[util]") {
    std::string url = " <\"http://x.com\"> ";
    Util::sanitizeUrl(url);
    REQUIRE(url == "http://x.com");
}

TEST_CASE("Util: sanitizeUrl empty string", "[util]") {
    std::string url;
    Util::sanitizeUrl(url);
    REQUIRE(url.empty());
}

TEST_CASE("Util: sanitizeUrl clean URL unchanged", "[util]") {
    std::string url = "adc://hub.example.com:411";
    Util::sanitizeUrl(url);
    REQUIRE(url == "adc://hub.example.com:411");
}

// ===== cleanPathChars =====

TEST_CASE("Util: cleanPathChars replaces dots, slashes, backslashes", "[util]") {
    REQUIRE(Util::cleanPathChars("file.name") == "file_name");
    REQUIRE(Util::cleanPathChars("path/to") == "path_to");
    REQUIRE(Util::cleanPathChars("path\\to") == "path_to");
    REQUIRE(Util::cleanPathChars("a.b/c\\d") == "a_b_c_d");
}

TEST_CASE("Util: cleanPathChars no special chars", "[util]") {
    REQUIRE(Util::cleanPathChars("simple") == "simple");
    REQUIRE(Util::cleanPathChars("") == "");
}

// ===== checkExtension =====

TEST_CASE("Util: checkExtension accepts clean extension", "[util]") {
    REQUIRE(Util::checkExtension("txt") == true);
    REQUIRE(Util::checkExtension("mp3") == true);
    // Note: dots are not in badChars, so "tar.gz" passes
    REQUIRE(Util::checkExtension("tar.gz") == true);
}

TEST_CASE("Util: checkExtension rejects control chars and spaces", "[util]") {
    REQUIRE(Util::checkExtension("tx t") == false);  // space
    REQUIRE(Util::checkExtension("t:x") == false);   // colon
    std::string withCtrl = "ab";
    withCtrl += '\x01';
    REQUIRE(Util::checkExtension(withCtrl) == false);
}

// ===== validateFileName =====

TEST_CASE("Util: validateFileName replaces bad chars", "[util]") {
    // Control char should be replaced with _
    std::string name = "file";
    name += '\x01';
    name += "name.txt";
    std::string result = Util::validateFileName(name);
    REQUIRE(result.find('\x01') == std::string::npos);
    REQUIRE(result.find('_') != std::string::npos);
}

TEST_CASE("Util: validateFileName removes ./ sequences", "[util]") {
    std::string result = Util::validateFileName("path/./file");
    REQUIRE(result.find("/./") == std::string::npos);
}

TEST_CASE("Util: validateFileName blocks ../ traversal", "[util]") {
    std::string result = Util::validateFileName("path/../secret/file");
    // The .. should be replaced with __
    REQUIRE(result.find("/../") == std::string::npos);
}

TEST_CASE("Util: validateFileName removes double slashes", "[util]") {
    std::string result = Util::validateFileName("path//to//file");
    REQUIRE(result.find("//") == std::string::npos);
}

TEST_CASE("Util: validateFileName with extra bad chars", "[util]") {
    std::string result = Util::validateFileName("file#name@here", "#@");
    REQUIRE(result.find('#') == std::string::npos);
    REQUIRE(result.find('@') == std::string::npos);
}

// ===== isPrivateIp =====

TEST_CASE("Util: isPrivateIp - 10.x.x.x", "[util]") {
    REQUIRE(Util::isPrivateIp("10.0.0.1") == true);
    REQUIRE(Util::isPrivateIp("10.255.255.255") == true);
}

TEST_CASE("Util: isPrivateIp - 127.x.x.x", "[util]") {
    REQUIRE(Util::isPrivateIp("127.0.0.1") == true);
    REQUIRE(Util::isPrivateIp("127.255.255.255") == true);
}

TEST_CASE("Util: isPrivateIp - 172.16-31.x.x", "[util]") {
    REQUIRE(Util::isPrivateIp("172.16.0.1") == true);
    REQUIRE(Util::isPrivateIp("172.31.255.255") == true);
    REQUIRE(Util::isPrivateIp("172.15.0.1") == false);
    REQUIRE(Util::isPrivateIp("172.32.0.1") == false);
}

TEST_CASE("Util: isPrivateIp - 192.168.x.x", "[util]") {
    REQUIRE(Util::isPrivateIp("192.168.0.1") == true);
    REQUIRE(Util::isPrivateIp("192.168.255.255") == true);
    REQUIRE(Util::isPrivateIp("192.167.0.1") == false);
}

TEST_CASE("Util: isPrivateIp - public addresses", "[util]") {
    REQUIRE(Util::isPrivateIp("8.8.8.8") == false);
    REQUIRE(Util::isPrivateIp("1.1.1.1") == false);
    REQUIRE(Util::isPrivateIp("203.0.113.1") == false);
}

TEST_CASE("Util: isPrivateIp - invalid IP", "[util]") {
    REQUIRE(Util::isPrivateIp("not.an.ip") == false);
    REQUIRE(Util::isPrivateIp("") == false);
}

// ===== parseIpPort =====

TEST_CASE("Util: parseIpPort splits correctly", "[util]") {
    std::string ip, port;

    Util::parseIpPort("192.168.1.1:411", ip, port);
    REQUIRE(ip == "192.168.1.1");
    REQUIRE(port == "411");
}

TEST_CASE("Util: parseIpPort no port", "[util]") {
    std::string ip, port;
    Util::parseIpPort("192.168.1.1", ip, port);
    REQUIRE(ip == "192.168.1.1");
    REQUIRE(port.empty());
}

TEST_CASE("Util: parseIpPort IPv6-like with multiple colons", "[util]") {
    // Uses rfind(':'), so last colon is the split point
    std::string ip, port;
    Util::parseIpPort("[::1]:8080", ip, port);
    REQUIRE(ip == "[::1]");
    REQUIRE(port == "8080");
}

// ===== decodeQuery =====

TEST_CASE("Util: decodeQuery basic", "[util]") {
    auto result = Util::decodeQuery("key1=val1&key2=val2");
    REQUIRE(result.size() == 2);
    REQUIRE(result["key1"] == "val1");
    REQUIRE(result["key2"] == "val2");
}

TEST_CASE("Util: decodeQuery single param", "[util]") {
    auto result = Util::decodeQuery("foo=bar");
    REQUIRE(result.size() == 1);
    REQUIRE(result["foo"] == "bar");
}

TEST_CASE("Util: decodeQuery empty string", "[util]") {
    auto result = Util::decodeQuery("");
    REQUIRE(result.empty());
}

TEST_CASE("Util: decodeQuery magnet-style", "[util]") {
    auto result = Util::decodeQuery("xt=urn:tree:tiger:ABC&xl=12345&dn=file.txt");
    REQUIRE(result["xt"] == "urn:tree:tiger:ABC");
    REQUIRE(result["xl"] == "12345");
    REQUIRE(result["dn"] == "file.txt");
}

// ===== toAdcFile / toNmdcFile =====

TEST_CASE("Util: toAdcFile prepends / and converts backslashes", "[util]") {
    REQUIRE(Util::toAdcFile("path\\to\\file.txt") == "/path/to/file.txt");
    REQUIRE(Util::toAdcFile("file.txt") == "/file.txt");
}

TEST_CASE("Util: toAdcFile special filenames pass through", "[util]") {
    REQUIRE(Util::toAdcFile("files.xml.bz2") == "files.xml.bz2");
    REQUIRE(Util::toAdcFile("files.xml") == "files.xml");
}

TEST_CASE("Util: toNmdcFile strips leading / and converts slashes", "[util]") {
    REQUIRE(Util::toNmdcFile("/path/to/file.txt") == "path\\to\\file.txt");
    REQUIRE(Util::toNmdcFile("/file.txt") == "file.txt");
}

TEST_CASE("Util: toNmdcFile empty string", "[util]") {
    REQUIRE(Util::toNmdcFile("") == Util::emptyString);
}

TEST_CASE("Util: toAdcFile/toNmdcFile round-trip", "[util]") {
    std::string orig = "Share\\Movies\\film.avi";
    std::string adc = Util::toAdcFile(orig);
    std::string nmdc = Util::toNmdcFile(adc);
    REQUIRE(nmdc == orig);
}

// ===== URL protocol checks =====

TEST_CASE("Util: isAdcUrl", "[util]") {
    REQUIRE(Util::isAdcUrl("adc://hub.example.com") == true);
    REQUIRE(Util::isAdcUrl("ADC://hub.example.com") == true);  // case insensitive
    REQUIRE(Util::isAdcUrl("adcs://hub.example.com") == false);
    REQUIRE(Util::isAdcUrl("dchub://hub.example.com") == false);
    REQUIRE(Util::isAdcUrl("") == false);
}

TEST_CASE("Util: isAdcsUrl", "[util]") {
    REQUIRE(Util::isAdcsUrl("adcs://hub.example.com") == true);
    REQUIRE(Util::isAdcsUrl("ADCS://hub.example.com") == true);
    REQUIRE(Util::isAdcsUrl("adc://hub.example.com") == false);
}

TEST_CASE("Util: isNmdcUrl", "[util]") {
    REQUIRE(Util::isNmdcUrl("dchub://hub.example.com") == true);
    REQUIRE(Util::isNmdcUrl("DCHUB://hub.example.com") == true);
    REQUIRE(Util::isNmdcUrl("adc://hub.example.com") == false);
    REQUIRE(Util::isNmdcUrl("") == false);
}

// ===== trimCopy =====

TEST_CASE("Util: trimCopy strips whitespace", "[util]") {
    REQUIRE(Util::trimCopy("  hello  ") == "hello");
    REQUIRE(Util::trimCopy("\t\nhello\r\n") == "hello");
    REQUIRE(Util::trimCopy("hello") == "hello");
}

TEST_CASE("Util: trimCopy all whitespace returns empty", "[util]") {
    REQUIRE(Util::trimCopy("   ").empty());
    REQUIRE(Util::trimCopy("").empty());
}

// ===== addBrackets =====

TEST_CASE("Util: addBrackets", "[util]") {
    REQUIRE(Util::addBrackets("test") == "<test>");
    REQUIRE(Util::addBrackets("") == "<>");
}
