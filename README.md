[![Linux](https://github.com/eoan-ermine-incubator/nsswitch/actions/workflows/linux.yml/badge.svg?branch=main)](
https://github.com/eoan-ermine-incubator/nsswitch/actions/workflows/linux.yml)

A C++23 library for parsing nsswitch.conf, the Name Service Switch configuration file.

[Try it online!](https://eoan-ermine-incubator.github.io/nsswitch/)

# Requirements

* C++23-compatible compiler (gcc 12+, clang 16+, msvc 19.33+)
* CMake (3.20+)

# Usage

## CMake

### [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)

Add to `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  nsswitch
  GIT_REPOSITORY https://github.com/eoan-ermine-incubator/nsswitch.git
  GIT_TAG 252a04abc1c8809eb57164c8f79b11acae34861e
)
FetchContent_MakeAvailable(nsswitch)

target_link_libraries(<insert-your-target-here> nsswitch::nsswitch)
```

### Install from source

1. Build from source

```bash
git clone https://github.com/eoan-ermine-incubator/nsswitch.git
cd nsswitch
cmake -B build .
cmake --build build
cmake --install build
```

2. Add to `CMakeLists.txt`:

```cmake
find_package(nsswitch CONFIG REQUIRED)
target_link_libraries(<insert-your-target-here> PRIVATE nsswitch::nsswitch)
```

### Add as a subdirectory

Add the repository to your source tree (for example, as a Git
submodule in `external/nsswitch`), then add the following to
your `CMakeLists.txt`:

```cmake
add_subdirectory(external/nsswitch)
target_link_libraries(<insert-your-target-here> PRIVATE nsswitch::nsswitch)
```

## API Reference

[nsswitch/config.h](include/nsswitch/config.h)

```cpp
namespace nsswitch {

struct Criterion {
    bool inversion;

    enum class Status { SUCCESS, NOTFOUND, UNAVAIL, TRYAGAIN } status;

    enum class Action { RETURN, CONTINUE, MERGE } action;
};

struct Service {
    std::string name;
    std::vector<Criterion> criterions;
};

struct Entry {
    std::string database;
    std::vector<Service> services;
};

struct Config {
    std::vector<Entry> entries;
};

std::string to_string(Criterion::Status);

std::string to_string(Criterion::Action);

}
```

[nsswitch/parser.h](include/nsswitch/parser.h)

```cpp
#include <nsswitch/config.h>

namespace nsswitch {

struct ParseError {
    std::size_t line;
    std::string message;
};

[[nodiscard]]
inline std::expected<Config, ParseError> parse(std::string_view input) {
    return detail::Parser{input}.parse();
}

}
```

## Example

```cpp
#include <iostream>
#include <string_view>

#include <nsswitch/parser.h>

int main() {
    constexpr std::string_view input = R"(
# Name Service Switch configuration
passwd: files systemd
group: files systemd
shadow: files
hosts: files dns [!UNAVAIL=return] my_custom_service
networks: files [SUCCESS=merge NOTFOUND=continue]
)";

    auto result = nsswitch::parse(input);

    if (!result) {
        const auto& error = result.error();

        std::cerr << "Line " << error.line << ": " << error.message
                  << '\n';

        return 1;
    }

    for (const auto& entry : result->entries) {
        std::cout << entry.database << ":\n";

        for (const auto& service : entry.services) {
            std::cout << "  " << service.name << '\n';

            for (const auto& criterion : service.criterions) {
                std::cout << "    " << (criterion.inversion ? "!" : "")
                          << nsswitch::to_string(criterion.status) << " -> "
                          << nsswitch::to_string(criterion.action) << '\n';
            }
        }
    }
}
```

Output:

```bash
eoanermine@eoanermine:~/nsswitch/build$ ./examples/nswitch_example
passwd:
  files
  systemd
group:
  files
  systemd
shadow:
  files
hosts:
  files
  dns
    !UNAVAIL -> RETURN
  my_custom_service
networks:
  files
    SUCCESS -> MERGE
    NOTFOUND -> CONTINUE
```

# Development guide

## CMake options

* `NSSWITCH_BUILD_TESTS`: Build [tests](https://github.com/eoan-ermine-incubator/nsswitch/tree/main/tests) (default: ON)
* `NSSWITCH_BUILD_EXAMPLES`: Build [examples](https://github.com/eoan-ermine-incubator/nsswitch/tree/main/examples) (default: ON)
* `NSSWITCH_BUILD_WASM_EXAMPLE`: Build [WASM example](https://github.com/eoan-ermine-incubator/nsswitch/tree/main/examples/wasm) (default: OFF)

# LICENSE

This project is licensed under the [MIT License](LICENSE).
