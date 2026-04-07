/*
 * Tests for UserCommand — construction, type checking, context flags,
 * ADC URL detection, and display name path splitting.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "UserCommand.h"

using namespace dcpp;

// ----- Construction -----

TEST_CASE("UserCommand: default construction", "[usercommand]") {
    UserCommand uc;
    REQUIRE(uc.getId() == 0);
    REQUIRE(uc.getType() == 0);
    REQUIRE(uc.getCtx() == 0);
    REQUIRE(uc.getName().empty());
    REQUIRE(uc.getCommand().empty());
    REQUIRE(uc.getTo().empty());
    REQUIRE(uc.getHub().empty());
}

TEST_CASE("UserCommand: parameterized construction", "[usercommand]") {
    UserCommand uc(42, UserCommand::TYPE_RAW, UserCommand::CONTEXT_HUB,
                   UserCommand::FLAG_NOSAVE, "Test Command", "/raw cmd", "target", "adc://hub.example.com");

    REQUIRE(uc.getId() == 42);
    REQUIRE(uc.getType() == UserCommand::TYPE_RAW);
    REQUIRE(uc.getCtx() == UserCommand::CONTEXT_HUB);
    REQUIRE(uc.isSet(UserCommand::FLAG_NOSAVE));
    REQUIRE(uc.getName() == "Test Command");
    REQUIRE(uc.getCommand() == "/raw cmd");
    REQUIRE(uc.getTo() == "target");
    REQUIRE(uc.getHub() == "adc://hub.example.com");
}

// ----- Type checking -----

TEST_CASE("UserCommand: isRaw", "[usercommand]") {
    UserCommand raw(1, UserCommand::TYPE_RAW, 0, 0, "", "", "", "");
    UserCommand rawOnce(2, UserCommand::TYPE_RAW_ONCE, 0, 0, "", "", "", "");
    UserCommand chat(3, UserCommand::TYPE_CHAT, 0, 0, "", "", "", "");
    UserCommand sep(4, UserCommand::TYPE_SEPARATOR, 0, 0, "", "", "", "");

    REQUIRE(raw.isRaw() == true);
    REQUIRE(rawOnce.isRaw() == true);
    REQUIRE(chat.isRaw() == false);
    REQUIRE(sep.isRaw() == false);
}

TEST_CASE("UserCommand: isChat", "[usercommand]") {
    UserCommand chat(1, UserCommand::TYPE_CHAT, 0, 0, "", "", "", "");
    UserCommand chatOnce(2, UserCommand::TYPE_CHAT_ONCE, 0, 0, "", "", "", "");
    UserCommand raw(3, UserCommand::TYPE_RAW, 0, 0, "", "", "", "");

    REQUIRE(chat.isChat() == true);
    REQUIRE(chatOnce.isChat() == true);
    REQUIRE(raw.isChat() == false);
}

TEST_CASE("UserCommand: once", "[usercommand]") {
    UserCommand rawOnce(1, UserCommand::TYPE_RAW_ONCE, 0, 0, "", "", "", "");
    UserCommand chatOnce(2, UserCommand::TYPE_CHAT_ONCE, 0, 0, "", "", "", "");
    UserCommand raw(3, UserCommand::TYPE_RAW, 0, 0, "", "", "", "");
    UserCommand chat(4, UserCommand::TYPE_CHAT, 0, 0, "", "", "", "");

    REQUIRE(rawOnce.once() == true);
    REQUIRE(chatOnce.once() == true);
    REQUIRE(raw.once() == false);
    REQUIRE(chat.once() == false);
}

// ----- ADC URL detection -----

TEST_CASE("UserCommand: adc static method", "[usercommand]") {
    REQUIRE(UserCommand::adc("adc://hub.example.com") == true);
    REQUIRE(UserCommand::adc("adcs://secure.hub.com") == true);
    REQUIRE(UserCommand::adc("dchub://nmdc.hub.com") == false);
    REQUIRE(UserCommand::adc("http://web.com") == false);
    REQUIRE(UserCommand::adc("") == false);
}

TEST_CASE("UserCommand: adc instance method", "[usercommand]") {
    UserCommand adcUc(1, UserCommand::TYPE_RAW, 0, 0, "", "", "", "adc://example.com");
    REQUIRE(adcUc.adc() == true);

    UserCommand nmdcUc(2, UserCommand::TYPE_RAW, 0, 0, "", "", "", "dchub://example.com");
    REQUIRE(nmdcUc.adc() == false);
}

// ----- Context flags -----

TEST_CASE("UserCommand: context flags", "[usercommand]") {
    UserCommand uc(1, UserCommand::TYPE_RAW,
                   UserCommand::CONTEXT_HUB | UserCommand::CONTEXT_USER, 0, "", "", "", "");

    REQUIRE((uc.getCtx() & UserCommand::CONTEXT_HUB) != 0);
    REQUIRE((uc.getCtx() & UserCommand::CONTEXT_USER) != 0);
    REQUIRE((uc.getCtx() & UserCommand::CONTEXT_SEARCH) == 0);
    REQUIRE((uc.getCtx() & UserCommand::CONTEXT_FILELIST) == 0);
}

TEST_CASE("UserCommand: CONTEXT_MASK covers all contexts", "[usercommand]") {
    REQUIRE((UserCommand::CONTEXT_MASK & UserCommand::CONTEXT_HUB) == UserCommand::CONTEXT_HUB);
    REQUIRE((UserCommand::CONTEXT_MASK & UserCommand::CONTEXT_USER) == UserCommand::CONTEXT_USER);
    REQUIRE((UserCommand::CONTEXT_MASK & UserCommand::CONTEXT_SEARCH) == UserCommand::CONTEXT_SEARCH);
    REQUIRE((UserCommand::CONTEXT_MASK & UserCommand::CONTEXT_FILELIST) == UserCommand::CONTEXT_FILELIST);
}

// ----- Display name -----

TEST_CASE("UserCommand: display name splits on /", "[usercommand]") {
    UserCommand uc(1, UserCommand::TYPE_RAW, 0, 0, "Menu/Submenu/Action", "", "", "");

    const auto& dn = uc.getDisplayName();
    REQUIRE(dn.size() == 3);
    REQUIRE(dn[0] == "Menu");
    REQUIRE(dn[1] == "Submenu");
    REQUIRE(dn[2] == "Action");
}

TEST_CASE("UserCommand: display name with escaped //", "[usercommand]") {
    // // in the name should become a literal / in the display name
    UserCommand uc(1, UserCommand::TYPE_RAW, 0, 0, "Menu//Item/Action", "", "", "");

    const auto& dn = uc.getDisplayName();
    REQUIRE(dn.size() == 2);
    REQUIRE(dn[0] == "Menu/Item");
    REQUIRE(dn[1] == "Action");
}

TEST_CASE("UserCommand: display name simple (no slashes)", "[usercommand]") {
    UserCommand uc(1, UserCommand::TYPE_RAW, 0, 0, "SimpleAction", "", "", "");

    const auto& dn = uc.getDisplayName();
    REQUIRE(dn.size() == 1);
    REQUIRE(dn[0] == "SimpleAction");
}

// ----- Copy constructor & assignment -----

TEST_CASE("UserCommand: copy constructor", "[usercommand]") {
    UserCommand orig(42, UserCommand::TYPE_CHAT, UserCommand::CONTEXT_SEARCH,
                     UserCommand::FLAG_NOSAVE, "Menu/Copy", "/msg test", "to", "adcs://hub");

    UserCommand copy(orig);
    REQUIRE(copy.getId() == 42);
    REQUIRE(copy.getType() == UserCommand::TYPE_CHAT);
    REQUIRE(copy.getCtx() == UserCommand::CONTEXT_SEARCH);
    REQUIRE(copy.isSet(UserCommand::FLAG_NOSAVE));
    REQUIRE(copy.getName() == "Menu/Copy");
    REQUIRE(copy.getCommand() == "/msg test");
    REQUIRE(copy.getDisplayName().size() == 2);
}

TEST_CASE("UserCommand: assignment operator", "[usercommand]") {
    UserCommand orig(99, UserCommand::TYPE_RAW_ONCE, UserCommand::CONTEXT_FILELIST,
                     0, "Assign/Test", "/raw x", "", "dchub://h");
    UserCommand dest;

    dest = orig;
    REQUIRE(dest.getId() == 99);
    REQUIRE(dest.getType() == UserCommand::TYPE_RAW_ONCE);
    REQUIRE(dest.getName() == "Assign/Test");
    REQUIRE(dest.getHub() == "dchub://h");
    REQUIRE(dest.getDisplayName().size() == 2);
    REQUIRE(dest.getDisplayName()[0] == "Assign");
    REQUIRE(dest.getDisplayName()[1] == "Test");
}

// ----- Type constants -----

TEST_CASE("UserCommand: TYPE_CLEAR is 255", "[usercommand]") {
    REQUIRE(UserCommand::TYPE_CLEAR == 255);
}

TEST_CASE("UserCommand: TYPE_SEPARATOR is 0", "[usercommand]") {
    REQUIRE(UserCommand::TYPE_SEPARATOR == 0);
}
