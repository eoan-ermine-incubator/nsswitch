#include <gtest/gtest.h>

#include "nsswitch/parser.h"

TEST(NsswitchParserTest, NsswitchFreeBSDMan) {
  // Source:
  // https://man.freebsd.org/cgi/man.cgi?query=nsswitch.conf&sektion=5&format=html
  constexpr std::string_view input = R"(
     hosts:   files cache dns
     passwd:  nis [notfound=return] files
     group:   nis [notfound=return] files
)";

  std::expected<nsswitch::Config, nsswitch::ParseError> result =
      std::unexpected(nsswitch::ParseError{0, ""});

  EXPECT_NO_THROW(result = nsswitch::parse(input));

  ASSERT_TRUE(result.has_value())
      << "Parsing failed at line " << result.error().line << ": "
      << result.error().message;

  const auto &config = *result;

  ASSERT_EQ(config.entries.size(), 3);

  // hosts: files cache dns
  EXPECT_EQ(config.entries[0].database, "hosts");
  ASSERT_EQ(config.entries[0].services.size(), 3);
  EXPECT_EQ(config.entries[0].services[0].name, "files");
  EXPECT_EQ(config.entries[0].services[1].name, "cache");
  EXPECT_EQ(config.entries[0].services[2].name, "dns");

  // passwd and group: nis [notfound=return] files
  constexpr std::string_view databases[] = {"passwd", "group"};

  for (std::size_t i = 0; i < std::size(databases); ++i) {
    const auto &entry = config.entries[i + 1];

    EXPECT_EQ(entry.database, databases[i]);
    ASSERT_EQ(entry.services.size(), 2);

    EXPECT_EQ(entry.services[0].name, "nis");
    ASSERT_EQ(entry.services[0].criterions.size(), 1);

    EXPECT_FALSE(entry.services[0].criterions[0].inversion);
    EXPECT_EQ(entry.services[0].criterions[0].status,
              nsswitch::Criterion::Status::NOTFOUND);
    EXPECT_EQ(entry.services[0].criterions[0].action,
              nsswitch::Criterion::Action::RETURN);

    EXPECT_EQ(entry.services[1].name, "files");
    EXPECT_TRUE(entry.services[1].criterions.empty());
  }
}