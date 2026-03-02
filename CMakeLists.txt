cmake_minimum_required(VERSION 3.25)

project(revenant
    VERSION 0.1.0
    DESCRIPTION "Crash-tolerant shared-memory transport for C++20"
    HOMEPAGE_URL "https://github.com/you/revenant"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(cmake/CPM.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

# ---- Options ----------------------------------------------------------------
option(REVENANT_BUILD_TESTS   "Build tests"             ON)
option(REVENANT_BUILD_BENCH   "Build benchmarks"        OFF)
option(REVENANT_BUILD_TOOLS   "Build revenantctl"       OFF)
option(REVENANT_BUILD_EXAMPLES "Build examples"         OFF)
option(REVENANT_INSTALL       "Generate install rules"  ON)

# ---- Version header ---------------------------------------------------------
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/revenant/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/revenant/version.hpp"
    @ONLY
)

# ---- Core interface target --------------------------------------------------
add_library(revenant_core INTERFACE)
add_library(revenant::core ALIAS revenant_core)
set_target_properties(revenant_core PROPERTIES EXPORT_NAME core)

target_include_directories(revenant_core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(revenant_core INTERFACE cxx_std_20)
revenant_set_warnings(revenant_core INTERFACE)

# ---- Main library target ----------------------------------------------------
add_library(revenant_lib STATIC src/errors.cpp)
add_library(revenant::revenant ALIAS revenant_lib)
set_target_properties(revenant_lib PROPERTIES OUTPUT_NAME "revenant" EXPORT_NAME revenant)

target_link_libraries(revenant_lib PUBLIC revenant::core)
revenant_set_warnings(revenant_lib PRIVATE)

# ---- Tests ------------------------------------------------------------------
if(REVENANT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---- Install rules ----------------------------------------------------------
if(REVENANT_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS revenant_lib revenant_core
        EXPORT RevenantTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(DIRECTORY include/revenant
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/revenant/version.hpp"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/revenant
    )

    install(EXPORT RevenantTargets
        FILE revenantTargets.cmake
        NAMESPACE revenant::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/revenant
    )

    configure_package_config_file(
        cmake/RevenantConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/revenantConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/revenant
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/revenantConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/revenantConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/revenantConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/revenant
    )

    configure_file(
        packaging/revenant.pc.in
        "${CMAKE_CURRENT_BINARY_DIR}/revenant.pc"
        @ONLY
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/revenant.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()
