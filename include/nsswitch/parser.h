#pragma once

#include <cctype>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "config.h"

namespace nsswitch {

struct ParseError {
    std::size_t line;
    std::string message;
};

[[nodiscard]]
inline std::expected<Criterion::Status, std::string> parse_status(
    std::string_view value) {
    if (value == "SUCCESS") return Criterion::Status::SUCCESS;
    if (value == "NOTFOUND") return Criterion::Status::NOTFOUND;
    if (value == "UNAVAIL") return Criterion::Status::UNAVAIL;
    if (value == "TRYAGAIN") return Criterion::Status::TRYAGAIN;

    return std::unexpected("unknown criterion status: " + std::string(value));
}

[[nodiscard]]
inline std::expected<Criterion::Action, std::string> parse_action(
    std::string_view value) {
    if (value == "RETURN") return Criterion::Action::RETURN;
    if (value == "CONTINUE") return Criterion::Action::CONTINUE;
    if (value == "MERGE") return Criterion::Action::MERGE;

    return std::unexpected("unknown criterion action: " + std::string(value));
}

namespace detail {

[[nodiscard]]
inline std::string uppercase(std::string_view value) {
    std::string result;
    result.reserve(value.size());

    for (char c : value) result.push_back(std::toupper(c));

    return result;
}

[[nodiscard]]
inline std::string_view trim(std::string_view value) noexcept {
    while (!value.empty() && std::isspace(value.front()))
        value.remove_prefix(1);

    while (!value.empty() && std::isspace(value.back())) value.remove_suffix(1);

    return value;
}

class Parser {
   public:
    explicit Parser(std::string_view input) noexcept : input_(input) {}

    [[nodiscard]]
    std::expected<Config, ParseError> parse() {
        Config config;

        std::size_t offset = 0;
        std::size_t line_number = 1;

        while (offset < input_.size()) {
            const auto newline = input_.find('\n', offset);

            const auto length = newline == std::string_view::npos
                                    ? input_.size() - offset
                                    : newline - offset;

            auto line = input_.substr(offset, length);

            if (auto result = parse_line(line, line_number, config); !result) {
                return std::unexpected(result.error());
            }

            if (newline == std::string_view::npos) break;

            offset = newline + 1;
            ++line_number;
        }

        return config;
    }

   private:
    std::string_view input_;

    [[nodiscard]]
    static ParseError make_error(std::size_t line,
                                 std::string message) {
        return {line, std::move(message)};
    }

    [[nodiscard]]
    static std::expected<void, ParseError> parse_line(std::string_view line,
                                                      std::size_t line_number,
                                                      Config& config) {
        // A comment starts at '#'.
        if (const auto comment = line.find('#');
            comment != std::string_view::npos) {
            line = line.substr(0, comment);
        }

        line = trim(line);

        if (line.empty()) return {};

        const auto colon = line.find(':');

        if (colon == std::string_view::npos) {
            return std::unexpected(
                make_error(line_number, "expected ':' after database name"));
        }

        const auto database = trim(line.substr(0, colon));

        if (database.empty()) {
            return std::unexpected(
                make_error(line_number, "empty database name"));
        }

        for (std::size_t i = 0; i < database.size(); ++i) {
            const char c = database[i];

            if (std::isspace(c) || c == '[' || c == ']' || c == ':' ||
                c == '=') {
                return std::unexpected(
                    make_error(line_number, "invalid database name"));
            }
        }

        Entry entry;
        entry.database = std::string(database);

        std::size_t pos = colon + 1;

        while (pos < line.size()) {
            while (pos < line.size() && std::isspace(line[pos])) ++pos;

            if (pos == line.size()) break;

            if (line[pos] == '[') {
                if (entry.services.empty()) {
                    return std::unexpected(
                        make_error(line_number,
                                   "criterion block has no preceding service"));
                }

                const auto close = line.find(']', pos + 1);

                if (close == std::string_view::npos) {
                    return std::unexpected(make_error(
                        line_number, "unterminated criterion block"));
                }

                auto result =
                    parse_criterions(line.substr(pos + 1, close - pos - 1),
                                     line_number, pos + 2);

                if (!result) return std::unexpected(result.error());

                auto& destination = entry.services.back().criterions;

                destination.insert(destination.end(), result->begin(),
                                   result->end());

                pos = close + 1;
                continue;
            }

            const auto start = pos;

            while (pos < line.size() && !std::isspace(line[pos]) &&
                   line[pos] != '[') {
                if (line[pos] == ']' || line[pos] == '=' || line[pos] == '#') {
                    return std::unexpected(
                        make_error(line_number,
                                   "invalid character in service name"));
                }

                ++pos;
            }

            if (start == pos) {
                return std::unexpected(
                    make_error(line_number, "expected service name"));
            }

            Service service;
            service.name = std::string(line.substr(start, pos - start));

            entry.services.push_back(std::move(service));
        }

        if (entry.services.empty()) {
            return std::unexpected(make_error(line_number,
                                              "expected at least one service"));
        }

        config.entries.push_back(std::move(entry));
        return {};
    }

    [[nodiscard]]
    static std::expected<std::vector<Criterion>, ParseError> parse_criterions(
        std::string_view body, std::size_t line_number,
        std::size_t column_offset) {
        std::vector<Criterion> result;
        std::size_t pos = 0;

        while (pos < body.size()) {
            while (pos < body.size() && std::isspace(body[pos])) ++pos;

            if (pos == body.size()) break;

            const auto start = pos;

            while (pos < body.size() && !std::isspace(body[pos])) ++pos;

            auto token = body.substr(start, pos - start);
            bool inversion = false;

            if (!token.empty() && token.front() == '!') {
                inversion = true;
                token.remove_prefix(1);
            }

            const auto eq = token.find('=');

            if (eq == std::string_view::npos ||
                token.find('=', eq + 1) != std::string_view::npos) {
                return std::unexpected(make_error(line_number,
                                                  "expected STATUS=ACTION"));
            }

            const auto status_text = uppercase(token.substr(0, eq));

            const auto action_text = uppercase(token.substr(eq + 1));

            auto status = parse_status(status_text);

            if (!status) {
                return std::unexpected(make_error(
                    line_number, status.error()));
            }

            auto action = parse_action(action_text);

            if (!action) {
                return std::unexpected(make_error(
                    line_number, action.error()));
            }

            result.push_back(Criterion{
                .inversion = inversion, .status = *status, .action = *action});
        }

        if (result.empty()) {
            return std::unexpected(make_error(line_number,
                                              "empty criterion block"));
        }

        return result;
    }
};

}  // namespace detail

[[nodiscard]]
inline std::expected<Config, ParseError> parse(std::string_view input) {
    return detail::Parser{input}.parse();
}

}  // namespace nsswitch