/*
 * Tests for AdlSearchDisplay — Phase 4 complex logic extraction.
 */
#include <catch2/catch_test_macros.hpp>
#include "AdlSearchDisplay.h"

using namespace gtk_adl;

// ── sourceTypeToString / stringToSourceType ──

TEST_CASE("ADL: sourceTypeToString Filename", "[gtk][adl]")
{
    REQUIRE(sourceTypeToString(SourceType::OnlyFile) == "Filename");
}

TEST_CASE("ADL: sourceTypeToString Directory", "[gtk][adl]")
{
    REQUIRE(sourceTypeToString(SourceType::OnlyDirectory) == "Directory");
}

TEST_CASE("ADL: sourceTypeToString Full Path", "[gtk][adl]")
{
    REQUIRE(sourceTypeToString(SourceType::FullPath) == "Full Path");
}

TEST_CASE("ADL: stringToSourceType roundtrip", "[gtk][adl]")
{
    REQUIRE(stringToSourceType("Filename") == SourceType::OnlyFile);
    REQUIRE(stringToSourceType("Directory") == SourceType::OnlyDirectory);
    REQUIRE(stringToSourceType("Full Path") == SourceType::FullPath);
    REQUIRE(stringToSourceType("Unknown") == SourceType::OnlyFile); // default
}

// ── sizeTypeToString / stringToSizeType ──

TEST_CASE("ADL: sizeTypeToString", "[gtk][adl]")
{
    REQUIRE(sizeTypeToString(SizeType::Bytes) == "B");
    REQUIRE(sizeTypeToString(SizeType::KibiBytes) == "KiB");
    REQUIRE(sizeTypeToString(SizeType::MebiBytes) == "MiB");
    REQUIRE(sizeTypeToString(SizeType::GibiBytes) == "GiB");
}

TEST_CASE("ADL: stringToSizeType roundtrip", "[gtk][adl]")
{
    REQUIRE(stringToSizeType("B") == SizeType::Bytes);
    REQUIRE(stringToSizeType("KiB") == SizeType::KibiBytes);
    REQUIRE(stringToSizeType("MiB") == SizeType::MebiBytes);
    REQUIRE(stringToSizeType("GiB") == SizeType::GibiBytes);
    REQUIRE(stringToSizeType("???") == SizeType::Bytes); // default
}

// ── formatSizeColumn ──

TEST_CASE("ADL: formatSizeColumn positive size", "[gtk][adl]")
{
    REQUIRE(formatSizeColumn(100, SizeType::MebiBytes) == "100 MiB");
    REQUIRE(formatSizeColumn(0, SizeType::Bytes) == "0 B");
    REQUIRE(formatSizeColumn(5, SizeType::GibiBytes) == "5 GiB");
}

TEST_CASE("ADL: formatSizeColumn negative returns empty", "[gtk][adl]")
{
    REQUIRE(formatSizeColumn(-1, SizeType::Bytes) == "");
    REQUIRE(formatSizeColumn(-100, SizeType::MebiBytes) == "");
}

// ── makeDisplay ──

TEST_CASE("ADL: makeDisplay complete row", "[gtk][adl]")
{
    auto d = makeDisplay("*.mp3", true, false,
                         SourceType::OnlyFile, "Music",
                         10, 500, SizeType::MebiBytes);

    REQUIRE(d.searchString == "*.mp3");
    REQUIRE(d.isActive == true);
    REQUIRE(d.isAutoQueue == false);
    REQUIRE(d.sourceTypeStr == "Filename");
    REQUIRE(d.destDir == "Music");
    REQUIRE(d.minSizeStr == "10 MiB");
    REQUIRE(d.maxSizeStr == "500 MiB");
    REQUIRE(d.minSize == 10);
    REQUIRE(d.maxSize == 500);
    REQUIRE(d.sourceType == SourceType::OnlyFile);
    REQUIRE(d.sizeType == SizeType::MebiBytes);
}

TEST_CASE("ADL: makeDisplay with negative sizes", "[gtk][adl]")
{
    auto d = makeDisplay("test", true, true,
                         SourceType::FullPath, "ADLSearch",
                         -1, -1, SizeType::Bytes);

    REQUIRE(d.minSizeStr.empty());
    REQUIRE(d.maxSizeStr.empty());
}

TEST_CASE("ADL: makeDisplay all source types", "[gtk][adl]")
{
    auto d1 = makeDisplay("f", true, false, SourceType::OnlyFile, "", -1, -1, SizeType::Bytes);
    auto d2 = makeDisplay("d", true, false, SourceType::OnlyDirectory, "", -1, -1, SizeType::Bytes);
    auto d3 = makeDisplay("p", true, false, SourceType::FullPath, "", -1, -1, SizeType::Bytes);

    REQUIRE(d1.sourceTypeStr == "Filename");
    REQUIRE(d2.sourceTypeStr == "Directory");
    REQUIRE(d3.sourceTypeStr == "Full Path");
}
