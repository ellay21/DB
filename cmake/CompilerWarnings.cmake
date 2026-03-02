function(revenant_set_warnings target scope)
    set(common_warnings
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wnull-dereference
    )

    set(gcc_warnings
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    set(clang_warnings
        -Wthread-safety
        -Wdocumentation
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND common_warnings ${gcc_warnings})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND common_warnings ${clang_warnings})
    endif()

    # hardware_destructive_interference_size triggers -Winterference-size on GCC
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND common_warnings -Wno-interference-size)
    endif()

    target_compile_options(${target} ${scope} ${common_warnings})
endfunction()
