cmake_minimum_required(VERSION 3.20)

project(nsswitch VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP NEW
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)

add_library(nsswitch INTERFACE)
add_library(nsswitch::nsswitch ALIAS nsswitch)

target_include_directories(nsswitch INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)

install(TARGETS nsswitch
    EXPORT nsswitchTargets
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT nsswitchTargets
    FILE nsswitchTargets.cmake
    NAMESPACE nsswitch::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nsswitch
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/nsswitchConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/nsswitchConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nsswitch"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/nsswitchConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/nsswitchConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/nsswitchConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nsswitch"
)

add_subdirectory(examples)
add_subdirectory(tests)