/*
 * Tests for HubUserParams — Phase 2 data transformation extraction.
 *
 * Tests both the pure helper functions (iconForUser, nickOrderKey)
 * and the full paramsFromIdentity with constructed Identity objects.
 */
#include <catch2/catch_test_macros.hpp>
#include "HubUserParams.h"
#include "TestContext.h"

#include <dcpp/stdinc.h>
#include <dcpp/OnlineUser.h>
#include <dcpp/User.h>
#include <dcpp/CID.h>
#include <dcpp/Pointer.h>

using namespace gtk_hub;
using dcpp::CID;
using dcpp::test::TestContext;
using dcpp::UserPtr;
using dcpp::User;
using dcpp::Identity;

namespace {
    // Helper to create a UserPtr+Identity pair for testing
    struct TestUser {
        UserPtr user;
        Identity id;
        TestUser(const std::string &cidStr = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA1")
            : user(new User(CID(cidStr))), id(user, 1) {}
    };
}

// ── iconForUser ──

TEST_CASE("HubUserParams: iconForUser normal user", "[gtk][hubuserparams]")
{
    REQUIRE(iconForUser(false, false, false) == "normal");
}

TEST_CASE("HubUserParams: iconForUser dc++ user", "[gtk][hubuserparams]")
{
    REQUIRE(iconForUser(true, false, false) == "dc++");
}

TEST_CASE("HubUserParams: iconForUser passive user", "[gtk][hubuserparams]")
{
    REQUIRE(iconForUser(false, true, false) == "normal-fw");
    REQUIRE(iconForUser(true, true, false) == "dc++-fw");
}

TEST_CASE("HubUserParams: iconForUser operator", "[gtk][hubuserparams]")
{
    REQUIRE(iconForUser(false, false, true) == "normal-op");
    REQUIRE(iconForUser(true, false, true) == "dc++-op");
}

TEST_CASE("HubUserParams: iconForUser passive operator", "[gtk][hubuserparams]")
{
    REQUIRE(iconForUser(false, true, true) == "normal-fw-op");
    REQUIRE(iconForUser(true, true, true) == "dc++-fw-op");
}

// ── nickOrderKey ──

TEST_CASE("HubUserParams: nickOrderKey for operator", "[gtk][hubuserparams]")
{
    REQUIRE(nickOrderKey("Admin", true) == "OAdmin");
}

TEST_CASE("HubUserParams: nickOrderKey for regular user", "[gtk][hubuserparams]")
{
    REQUIRE(nickOrderKey("John", false) == "UJohn");
}

TEST_CASE("HubUserParams: nickOrderKey empty nick", "[gtk][hubuserparams]")
{
    REQUIRE(nickOrderKey("", true) == "O");
    REQUIRE(nickOrderKey("", false) == "U");
}

// ── paramsFromIdentity ──

TEST_CASE("HubUserParams: paramsFromIdentity basic user", "[gtk][hubuserparams]")
{
    TestContext ctx;

    TestUser tu;
    tu.id.setNick("TestNick");
    tu.id.setDescription("Some description");
    tu.id.setIp("10.0.0.1");
    tu.id.setEmail("test@example.com");
    tu.id.setBytesShared("1073741824"); // 1 GiB

    auto p = paramsFromIdentity(tu.id);

    REQUIRE(p["Nick"] == "TestNick");
    REQUIRE(p["Description"] == "Some description");
    REQUIRE(p["IP"] == "10.0.0.1");
    REQUIRE(p["eMail"] == "test@example.com");
    REQUIRE(p["Shared"] == "1073741824");
    REQUIRE(p["CID"] == tu.user->getCID().toBase32());
    // Normal user → "U" prefix
    REQUIRE(p["Nick Order"] == "UTestNick");
    // Not a DC++ client, not passive, not op → "normal"
    REQUIRE(p["Icon"] == "normal");
}

TEST_CASE("HubUserParams: paramsFromIdentity operator", "[gtk][hubuserparams]")
{
    TestContext ctx;

    TestUser tu;
    tu.id.setNick("OpNick");
    tu.id.setOp(true);

    auto p = paramsFromIdentity(tu.id);

    REQUIRE(p["Nick Order"] == "OOpNick");
    // Icon should end with "-op"
    std::string icon = p["Icon"];
    REQUIRE(icon.length() >= 3);
    REQUIRE(icon.substr(icon.length() - 3) == "-op");
}

TEST_CASE("HubUserParams: paramsFromIdentity passive user", "[gtk][hubuserparams]")
{
    TestContext ctx;

    TestUser tu;
    tu.id.setNick("FWUser");
    tu.user->setFlag(User::PASSIVE);

    auto p = paramsFromIdentity(tu.id);

    std::string icon = p["Icon"];
    REQUIRE(icon.find("-fw") != std::string::npos);
}

TEST_CASE("HubUserParams: paramsFromIdentity all keys present", "[gtk][hubuserparams]")
{
    TestContext ctx;

    TestUser tu;
    tu.id.setNick("X");

    auto p = paramsFromIdentity(tu.id);

    // Verify all expected keys exist
    REQUIRE(p.count("Icon") == 1);
    REQUIRE(p.count("Nick Order") == 1);
    REQUIRE(p.count("Nick") == 1);
    REQUIRE(p.count("Shared") == 1);
    REQUIRE(p.count("Description") == 1);
    REQUIRE(p.count("Tag") == 1);
    REQUIRE(p.count("Connection") == 1);
    REQUIRE(p.count("IP") == 1);
    REQUIRE(p.count("eMail") == 1);
    REQUIRE(p.count("CID") == 1);
}
