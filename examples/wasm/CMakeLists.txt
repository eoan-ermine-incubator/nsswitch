add_executable(wasm_example wasm_example.cpp)
set_target_properties(wasm_example PROPERTIES SUFFIX ".js")
target_link_libraries(wasm_example PRIVATE nsswitch)
target_link_options(wasm_example PRIVATE
    "SHELL:-s EXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString"
    "SHELL:-s EXPORTED_FUNCTIONS=_validate,_free_string")