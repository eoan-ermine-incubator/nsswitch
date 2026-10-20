#include <iostream>
#include <string_view>

#include "nsswitch/parser.h"

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
                          << static_cast<int>(criterion.status) << " -> "
                          << static_cast<int>(criterion.action) << '\n';
            }
        }
    }
}