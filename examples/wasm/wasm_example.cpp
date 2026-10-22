#include <cstdlib>
#include <cstring>
#include <string>

#include <emscripten/emscripten.h>

#include "nsswitch/nsswitch.h"

namespace {

std::string parse_impl(const std::string& input) {
    auto result = nsswitch::parse(input);

    if (!result) {
        const auto& error = result.error();

        return "Line " + std::to_string(error.line) + ": " + error.message;
    }

    std::string output;

    for (const auto& entry : result->entries) {
        output += entry.database + ":\n";

        for (const auto& service : entry.services) {
            output += "  " + service.name + "\n";

            for (const auto& criterion : service.criterions) {
                output += "    ";

                if (criterion.inversion) output += "!";

                output += nsswitch::to_string(criterion.status);
                output += " -> ";
                output += nsswitch::to_string(criterion.action);
                output += "\n";
            }
        }
    }

    return output;
}

}  // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE
char* parse(const char* input) {
    const std::string text = input != nullptr ? input : "";
    const std::string result = parse_impl(text);
    char* buffer = static_cast<char*>(std::malloc(result.size() + 1));
    std::memcpy(buffer, result.c_str(), result.size() + 1);
    return buffer;
}

EMSCRIPTEN_KEEPALIVE
void free_string(char* ptr) {
    std::free(ptr);
}

}  // extern "C"
