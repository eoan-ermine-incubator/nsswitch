cmake_minimum_required(VERSION 3.20)

project(nsswitch)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(nsswitch INTERFACE)

target_include_directories(nsswitch INTERFACE include)

add_executable(nsswitch_example examples/nsswitch_example.cpp)
target_link_libraries(nsswitch_example PRIVATE nsswitch)

enable_testing()
include(GoogleTest)

set(NSSWITCH_TEST_SOURCES
    altlinux_man
    freeBSD_man
    linux_man
    sberlinux_man
    tldp
)

foreach(source IN LISTS NSSWITCH_TEST_SOURCES)
  add_executable(nsswitch_${source}_test tests/nsswitch_${source}.cpp)
  target_link_libraries(nsswitch_${source}_test PRIVATE
      GTest::gtest_main
      nsswitch
  )

  gtest_discover_tests(nsswitch_${source}_test)
endforeach()
