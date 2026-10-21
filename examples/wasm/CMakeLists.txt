add_executable(wasm_example wasm_example.cpp)
set_target_properties(wasm_example PROPERTIES SUFFIX ".js")
target_link_libraries(wasm_example PRIVATE nsswitch)
target_link_options(wasm_example PRIVATE
    "SHELL:-s EXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString"
    "SHELL:-s EXPORTED_FUNCTIONS=_parse,_free_string")

add_custom_command(TARGET wasm_example POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
        "${CMAKE_CURRENT_BINARY_DIR}/index.html"
    COMMENT "Copying shell.html to output directory")
