option(REVENANT_ENABLE_ASAN     "Enable AddressSanitizer + LeakSanitizer"   OFF)
option(REVENANT_ENABLE_UBSAN    "Enable UndefinedBehaviourSanitizer"        OFF)
option(REVENANT_ENABLE_TSAN     "Enable ThreadSanitizer"                    OFF)
option(REVENANT_ENABLE_MSAN     "Enable MemorySanitizer"                    OFF)
option(REVENANT_ENABLE_COVERAGE "Enable gcov coverage instrumentation"      OFF)
option(REVENANT_ENABLE_FUZZING  "Enable libFuzzer instrumentation"          OFF)

function(revenant_apply_sanitizers target)
    set(san_flags "")

    if(REVENANT_ENABLE_TSAN AND (REVENANT_ENABLE_ASAN OR REVENANT_ENABLE_MSAN))
        message(FATAL_ERROR "TSan is incompatible with ASan and MSan")
    endif()
    if(REVENANT_ENABLE_MSAN AND REVENANT_ENABLE_ASAN)
        message(FATAL_ERROR "MSan is incompatible with ASan")
    endif()

    if(REVENANT_ENABLE_ASAN)
        list(APPEND san_flags -fsanitize=address -fno-omit-frame-pointer)
        target_compile_definitions(${target} PRIVATE REVENANT_ASAN=1)
    endif()

    if(REVENANT_ENABLE_UBSAN)
        list(APPEND san_flags
            -fsanitize=undefined
            -fno-sanitize-recover=all
        )
    endif()

    if(REVENANT_ENABLE_TSAN)
        list(APPEND san_flags -fsanitize=thread)
        target_compile_definitions(${target} PRIVATE REVENANT_TSAN=1)
    endif()

    if(REVENANT_ENABLE_MSAN)
        list(APPEND san_flags -fsanitize=memory -fno-omit-frame-pointer)
    endif()

    if(REVENANT_ENABLE_COVERAGE)
        list(APPEND san_flags --coverage -fprofile-arcs -ftest-coverage)
    endif()

    if(REVENANT_ENABLE_FUZZING)
        list(APPEND san_flags -fsanitize=fuzzer,address,undefined)
    endif()

    if(san_flags)
        target_compile_options(${target} PRIVATE ${san_flags})
        target_link_options(${target} PRIVATE ${san_flags})
    endif()
endfunction()
