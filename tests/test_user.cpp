/*
 * Tests for User — flags, CID, online/NMDC status.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "User.h"
#include "CID.h"
#include "TigerHash.h"

using namespace dcpp;

// Helper: create a CID from a string
static CID makeCID(const std::string& s) {
    TigerHash h;
    h.update(s.data(), s.size());
    h.finalize();
    return CID(h.getResult());
}

TEST_CASE("User: construction with CID", "[user]") {
    CID cid = makeCID("test-user");
    User u(cid);

    REQUIRE(u.getCID() == cid);
    REQUIRE(u.getCID().toBase32() == cid.toBase32());
}

TEST_CASE("User: default flags - not online, not NMDC", "[user]") {
    User u(makeCID("u1"));

    REQUIRE(u.isOnline() == false);
    REQUIRE(u.isNMDC() == false);
}

TEST_CASE("User: set ONLINE flag", "[user]") {
    User u(makeCID("u2"));

    u.setFlag(User::ONLINE);
    REQUIRE(u.isOnline() == true);

    u.unsetFlag(User::ONLINE);
    REQUIRE(u.isOnline() == false);
}

TEST_CASE("User: set NMDC flag", "[user]") {
    User u(makeCID("u3"));

    u.setFlag(User::NMDC);
    REQUIRE(u.isNMDC() == true);
}

TEST_CASE("User: multiple flags", "[user]") {
    User u(makeCID("u4"));

    u.setFlag(User::ONLINE);
    u.setFlag(User::TLS);
    u.setFlag(User::PASSIVE);

    REQUIRE(u.isOnline() == true);
    REQUIRE(u.isSet(User::TLS) == true);
    REQUIRE(u.isSet(User::PASSIVE) == true);
    REQUIRE(u.isNMDC() == false);
}

TEST_CASE("User: CID conversion operator", "[user]") {
    CID cid = makeCID("conv");
    User u(cid);

    // User has operator const CID&
    const CID& ref = u;
    REQUIRE(ref == cid);
}

TEST_CASE("User: flag bit values are distinct", "[user]") {
    // Verify no overlapping bit values
    REQUIRE(User::ONLINE != User::PASSIVE);
    REQUIRE(User::NMDC != User::BOT);
    REQUIRE(User::TLS != User::OLD_CLIENT);

    // Each should be a power of 2
    REQUIRE((User::ONLINE & (User::ONLINE - 1)) == 0);
    REQUIRE((User::PASSIVE & (User::PASSIVE - 1)) == 0);
    REQUIRE((User::NMDC & (User::NMDC - 1)) == 0);
    REQUIRE((User::TLS & (User::TLS - 1)) == 0);
}

TEST_CASE("User: BOT flag", "[user]") {
    User u(makeCID("bot"));
    u.setFlag(User::BOT);
    REQUIRE(u.isSet(User::BOT) == true);

    u.unsetFlag(User::BOT);
    REQUIRE(u.isSet(User::BOT) == false);
}
