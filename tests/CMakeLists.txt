cmake_minimum_required(VERSION 3.25)

cpmaddpackage(
  NAME
  doctest
  GITHUB_REPOSITORY
  doctest/doctest
  GIT_TAG
  v2.4.11
  OPTIONS
  "DOCTEST_WITH_MAIN_IN_STATIC_LIB ON")

cpmaddpackage(
  NAME
  rapidcheck
  GITHUB_REPOSITORY
  emil-e/rapidcheck
  GIT_TAG
  6e8dadfdafa3a74eabb52ead87f8787f72eccd0b
  OPTIONS
  "RC_ENABLE_DOCTEST ON")

add_library(revenant_test_support STATIC support/test_main.cpp)
target_link_libraries(revenant_test_support PUBLIC doctest::doctest
                                                   revenant::revenant)
revenant_set_warnings(revenant_test_support PRIVATE)

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

# ---- Tests ------------------------------------------------------------------
revenant_add_test(prop_block_index SOURCES property/prop_block_index.cpp LABELS
                  property)
revenant_add_test(test_block_index SOURCES unit/test_block_index.cpp LABELS
                  unit)
revenant_add_test(test_frame_codec SOURCES unit/test_frame_codec.cpp LABELS
                  unit)
revenant_add_test(test_layout_offsets SOURCES unit/test_layout_offsets.cpp
                  LABELS unit)
revenant_add_test(test_position SOURCES unit/test_position.cpp LABELS unit)
revenant_add_test(test_ring_math SOURCES unit/test_ring_math.cpp LABELS unit)
revenant_add_test(test_smoke SOURCES unit/test_smoke.cpp LABELS unit)

if(TARGET rapidcheck)
  revenant_add_test(
    prop_ring_arithmetic
    SOURCES
    property/prop_ring_arithmetic.cpp
    LABELS
    property
    unit
    LIBS
    rapidcheck)
  revenant_add_test(
    prop_block_index
    SOURCES
    property/prop_block_index.cpp
    LABELS
    property
    unit
    LIBS
    rapidcheck)
endif()
