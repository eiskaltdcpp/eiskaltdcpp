/*
 * Tests for SettingsValidation — Phase 4 complex logic extraction.
 */
#include <catch2/catch_test_macros.hpp>
#include "SettingsValidation.h"
#include <dcpp/stdinc.h>
#include <dcpp/Util.h> // PATH_SEPARATOR
#include <algorithm>

using namespace gtk_settings_validate;

namespace {
/// Convert a Unix-style hardcoded path to the platform's native separator.
inline std::string nativePath(const char* p) {
    std::string s(p);
#ifdef _WIN32
    std::replace(s.begin(), s.end(), '/', '\\');
#endif
    return s;
}
} // anon

// ── isValidPort ──

TEST_CASE("Validation: valid ports", "[gtk][validate]")
{
    REQUIRE(isValidPort(1));
    REQUIRE(isValidPort(80));
    REQUIRE(isValidPort(443));
    REQUIRE(isValidPort(65535));
}

TEST_CASE("Validation: invalid ports", "[gtk][validate]")
{
    REQUIRE_FALSE(isValidPort(0));
    REQUIRE_FALSE(isValidPort(-1));
    REQUIRE_FALSE(isValidPort(65536));
    REQUIRE_FALSE(isValidPort(100000));
}

// ── adjustTlsPort / adjustDhtPort ──

TEST_CASE("Validation: adjustTlsPort when different", "[gtk][validate]")
{
    REQUIRE(adjustTlsPort(412, 411) == 412);
}

TEST_CASE("Validation: adjustTlsPort when same", "[gtk][validate]")
{
    REQUIRE(adjustTlsPort(411, 411) == 412);
}

TEST_CASE("Validation: adjustDhtPort when different", "[gtk][validate]")
{
    REQUIRE(adjustDhtPort(6251, 6250) == 6251);
}

TEST_CASE("Validation: adjustDhtPort when same", "[gtk][validate]")
{
    REQUIRE(adjustDhtPort(6250, 6250) == 6251);
}

// ── ensureTrailingPathSep ──

TEST_CASE("Validation: path already has separator", "[gtk][validate]")
{
    REQUIRE(ensureTrailingPathSep(nativePath("/home/user/")) == nativePath("/home/user/"));
}

TEST_CASE("Validation: path missing separator", "[gtk][validate]")
{
    REQUIRE(ensureTrailingPathSep(nativePath("/home/user")) == nativePath("/home/user/"));
}

TEST_CASE("Validation: empty path unchanged", "[gtk][validate]")
{
    REQUIRE(ensureTrailingPathSep("").empty());
}

// ── validateDownloadDir ──

TEST_CASE("Validation: download dir valid", "[gtk][validate]")
{
    auto r = validateDownloadDir("/home/user/Downloads");
    REQUIRE(r.ok);
}

TEST_CASE("Validation: download dir empty invalid", "[gtk][validate]")
{
    auto r = validateDownloadDir("");
    REQUIRE_FALSE(r.ok);
    REQUIRE(r.error.find("empty") != std::string::npos);
}

// ── validateNick ──

TEST_CASE("Validation: valid nick", "[gtk][validate]")
{
    auto r = validateNick("Alice");
    REQUIRE(r.ok);
}

TEST_CASE("Validation: empty nick invalid", "[gtk][validate]")
{
    auto r = validateNick("");
    REQUIRE_FALSE(r.ok);
}

TEST_CASE("Validation: nick with leading space invalid", "[gtk][validate]")
{
    auto r = validateNick(" Alice");
    REQUIRE_FALSE(r.ok);
    REQUIRE(r.error.find("space") != std::string::npos);
}

TEST_CASE("Validation: nick with trailing space invalid", "[gtk][validate]")
{
    auto r = validateNick("Alice ");
    REQUIRE_FALSE(r.ok);
}

// ── isValidIpv4 ──

TEST_CASE("Validation: valid IPv4 addresses", "[gtk][validate]")
{
    REQUIRE(isValidIpv4("192.168.1.1"));
    REQUIRE(isValidIpv4("0.0.0.0"));
    REQUIRE(isValidIpv4("255.255.255.255"));
    REQUIRE(isValidIpv4("10.0.0.1"));
}

TEST_CASE("Validation: invalid IPv4 addresses", "[gtk][validate]")
{
    REQUIRE_FALSE(isValidIpv4(""));
    REQUIRE_FALSE(isValidIpv4("256.1.1.1"));
    REQUIRE_FALSE(isValidIpv4("1.2.3"));
    REQUIRE_FALSE(isValidIpv4("1.2.3.4.5"));
    REQUIRE_FALSE(isValidIpv4("abc.def.ghi.jkl"));
    REQUIRE_FALSE(isValidIpv4("192.168.1.1."));   // trailing dot creates 5th part
}

// ── isSearchTypeNameValid / isExtensionListValid ──

TEST_CASE("Validation: search type name valid", "[gtk][validate]")
{
    REQUIRE(isSearchTypeNameValid("My Type"));
    REQUIRE_FALSE(isSearchTypeNameValid(""));
}

TEST_CASE("Validation: extension list valid", "[gtk][validate]")
{
    REQUIRE(isExtensionListValid({"mp3", "flac"}));
    REQUIRE_FALSE(isExtensionListValid({}));
}

// ── validateConnectionSettings ──

TEST_CASE("Validation: valid connection settings passive", "[gtk][validate]")
{
    auto r = validateConnectionSettings(411, 412, 413, "", false);
    REQUIRE(r.ok);
}

TEST_CASE("Validation: valid connection settings active", "[gtk][validate]")
{
    auto r = validateConnectionSettings(411, 412, 413, "192.168.1.100", true);
    REQUIRE(r.ok);
}

TEST_CASE("Validation: active mode requires IP", "[gtk][validate]")
{
    auto r = validateConnectionSettings(411, 412, 413, "", true);
    REQUIRE_FALSE(r.ok);
    REQUIRE(r.error.find("IP") != std::string::npos);
}

TEST_CASE("Validation: active mode invalid IP", "[gtk][validate]")
{
    auto r = validateConnectionSettings(411, 412, 413, "bad-ip", true);
    REQUIRE_FALSE(r.ok);
}

TEST_CASE("Validation: invalid TCP port", "[gtk][validate]")
{
    auto r = validateConnectionSettings(0, 412, 413, "", false);
    REQUIRE_FALSE(r.ok);
    REQUIRE(r.error.find("TCP") != std::string::npos);
}

TEST_CASE("Validation: invalid UDP port", "[gtk][validate]")
{
    auto r = validateConnectionSettings(411, 70000, 413, "", false);
    REQUIRE_FALSE(r.ok);
    REQUIRE(r.error.find("UDP") != std::string::npos);
}

TEST_CASE("Validation: invalid TLS port", "[gtk][validate]")
{
    auto r = validateConnectionSettings(411, 412, -5, "", false);
    REQUIRE_FALSE(r.ok);
    REQUIRE(r.error.find("TLS") != std::string::npos);
}

// ── ValidationResult helpers ──

TEST_CASE("Validation: ValidationResult success", "[gtk][validate]")
{
    auto r = ValidationResult::success();
    REQUIRE(r.ok);
    REQUIRE(r.error.empty());
}

TEST_CASE("Validation: ValidationResult fail", "[gtk][validate]")
{
    auto r = ValidationResult::fail("oops");
    REQUIRE_FALSE(r.ok);
    REQUIRE(r.error == "oops");
}
