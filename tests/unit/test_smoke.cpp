#include <revenant/revenant.hpp>
#include <revenant/version.hpp>

#include <doctest/doctest.h>
#include <string_view>

TEST_CASE("version_string is non-empty") {
    const char* v = revenant::version_string();
    REQUIRE(v != nullptr);
    REQUIRE(std::string_view(v).size() > 0);
}

TEST_CASE("version_string matches CMake project version") {
    std::string_view v = revenant::version_string();
    REQUIRE(v == REVENANT_VERSION_STRING);
}

TEST_CASE("version macros are consistent") {
    REQUIRE(REVENANT_VERSION_MAJOR >= 0);
    REQUIRE(REVENANT_VERSION_MINOR >= 0);
    REQUIRE(REVENANT_VERSION_PATCH >= 0);
}
