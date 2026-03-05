cmake_minimum_required(VERSION 3.25)

# Fetch doctest via CPM
cpmaddpackage(
  NAME
  doctest
  GITHUB_REPOSITORY
  doctest/doctest
  GIT_TAG
  v2.4.11
  OPTIONS
  "DOCTEST_WITH_MAIN_IN_STATIC_LIB ON")

# ---- Support library --------------------------------------------------------
add_library(revenant_test_support STATIC support/test_main.cpp)
target_link_libraries(revenant_test_support PUBLIC doctest::doctest
                                                   revenant::revenant)
revenant_set_warnings(revenant_test_support PRIVATE)

# Helper to register a test executable with ctest labels
function(revenant_add_test name)
  cmake_parse_arguments(ARG "" "" "SOURCES;LABELS;LIBS" ${ARGN})
  add_executable(${name} ${ARG_SOURCES})
  target_link_libraries(${name} PRIVATE revenant_test_support
                                        revenant::revenant ${ARG_LIBS})
  revenant_set_warnings(${name} PRIVATE)
  revenant_apply_sanitizers(${name})
  add_test(NAME ${name} COMMAND ${name})
  foreach(label IN LISTS ARG_LABELS)
    set_property(
      TEST ${name}
      APPEND
      PROPERTY LABELS ${label})
  endforeach()
endfunction()

# ---- Unit tests -------------------------------------------------------------
revenant_add_test(test_smoke SOURCES unit/test_smoke.cpp LABELS unit)
