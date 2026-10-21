add_executable(nsswitch_example nsswitch_example.cpp)
target_link_libraries(nsswitch_example PRIVATE nsswitch)

if (NSSWITCH_BUILD_WASM_EXAMPLE)
    if (NOT EMSCRIPTEN)
        message(FATAL_ERROR
        "For WASM build run `emcmake cmake` instead of `cmake`\n"
        "If you are already doing it, then use separate (from non-WASM build) build directory"
    )
    endif()
    add_subdirectory(wasm)
endif()
