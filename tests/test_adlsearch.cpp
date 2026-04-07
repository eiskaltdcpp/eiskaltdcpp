/*
 * Tests for ADLSearch — source type/size type conversions, size base
 * calculation, and file/directory matching.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "ADLSearch.h"

using namespace dcpp;

// ----- SourceType string conversions -----

TEST_CASE("ADLSearch: SourceType string round-trips", "[adlsearch]") {
    ADLSearch adl;
    REQUIRE(adl.SourceTypeToString(ADLSearch::OnlyFile) == "Filename");
    REQUIRE(adl.SourceTypeToString(ADLSearch::OnlyDirectory) == "Directory");
    REQUIRE(adl.SourceTypeToString(ADLSearch::FullPath) == "Full Path");

    REQUIRE(adl.StringToSourceType("Filename") == ADLSearch::OnlyFile);
    REQUIRE(adl.StringToSourceType("Directory") == ADLSearch::OnlyDirectory);
    REQUIRE(adl.StringToSourceType("Full Path") == ADLSearch::FullPath);
}

TEST_CASE("ADLSearch: SourceType unknown string defaults to OnlyFile", "[adlsearch]") {
    ADLSearch adl;
    REQUIRE(adl.StringToSourceType("bogus") == ADLSearch::OnlyFile);
    REQUIRE(adl.StringToSourceType("") == ADLSearch::OnlyFile);
}

TEST_CASE("ADLSearch: SourceType case insensitive", "[adlsearch]") {
    ADLSearch adl;
    REQUIRE(adl.StringToSourceType("filename") == ADLSearch::OnlyFile);
    REQUIRE(adl.StringToSourceType("DIRECTORY") == ADLSearch::OnlyDirectory);
    REQUIRE(adl.StringToSourceType("full path") == ADLSearch::FullPath);
}

// ----- SizeType string conversions -----

TEST_CASE("ADLSearch: SizeType string round-trips", "[adlsearch]") {
    ADLSearch adl;
    REQUIRE(adl.SizeTypeToString(ADLSearch::SizeBytes) == "B");
    REQUIRE(adl.SizeTypeToString(ADLSearch::SizeKibiBytes) == "KiB");
    REQUIRE(adl.SizeTypeToString(ADLSearch::SizeMebiBytes) == "MiB");
    REQUIRE(adl.SizeTypeToString(ADLSearch::SizeGibiBytes) == "GiB");

    REQUIRE(adl.StringToSizeType("B") == ADLSearch::SizeBytes);
    REQUIRE(adl.StringToSizeType("KiB") == ADLSearch::SizeKibiBytes);
    REQUIRE(adl.StringToSizeType("MiB") == ADLSearch::SizeMebiBytes);
    REQUIRE(adl.StringToSizeType("GiB") == ADLSearch::SizeGibiBytes);
}

TEST_CASE("ADLSearch: SizeType unknown defaults to SizeBytes", "[adlsearch]") {
    ADLSearch adl;
    REQUIRE(adl.StringToSizeType("bogus") == ADLSearch::SizeBytes);
    REQUIRE(adl.StringToSizeType("") == ADLSearch::SizeBytes);
}

// ----- GetSizeBase -----

TEST_CASE("ADLSearch: GetSizeBase returns correct multipliers", "[adlsearch]") {
    ADLSearch adl;

    adl.typeFileSize = ADLSearch::SizeBytes;
    REQUIRE(adl.GetSizeBase() == 1);

    adl.typeFileSize = ADLSearch::SizeKibiBytes;
    REQUIRE(adl.GetSizeBase() == 1024);

    adl.typeFileSize = ADLSearch::SizeMebiBytes;
    REQUIRE(adl.GetSizeBase() == 1024 * 1024);

    adl.typeFileSize = ADLSearch::SizeGibiBytes;
    REQUIRE(adl.GetSizeBase() == int64_t(1024) * 1024 * 1024);
}

// ----- Default construction -----

TEST_CASE("ADLSearch: default construction values", "[adlsearch]") {
    ADLSearch adl;
    REQUIRE(adl.isActive == true);
    REQUIRE(adl.isAutoQueue == false);
    REQUIRE(adl.sourceType == ADLSearch::OnlyFile);
    REQUIRE(adl.minFileSize == -1);
    REQUIRE(adl.maxFileSize == -1);
    REQUIRE(adl.typeFileSize == ADLSearch::SizeBytes);
    REQUIRE(adl.destDir == "ADLSearch");
    REQUIRE(adl.ddIndex == 0);
}

// NOTE: matchesFile, matchesDirectory, and prepare are private (friend of ADLSearchManager).
// Full matching tests require Phase 2 TestContext infrastructure.
// See TEST_COVERAGE_PLAN.md Phase 2 for ADLSearchManager integration tests.
