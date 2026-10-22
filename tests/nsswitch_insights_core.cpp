#include <gtest/gtest.h>

#include "nsswitch/parser.h"

TEST(NsswitchParserTest, NsswitchInsightsCore) {
  // Source:
  // https://insights-core.readthedocs.io/en/latest/shared_parsers_catalog/nsswitch_conf.html
  constexpr std::string_view input = R"(
# Example:
#passwd:    db files nisplus nis
#shadow:    db files nisplus nis
#group:     db files nisplus nis

passwd:     files sss
shadow:     files sss
group:      files sss
#initgroups: files

#hosts:     db files nisplus nis dns
hosts:      files dns myhostname
)";

  std::expected<nsswitch::Config, nsswitch::ParseError> result =
      std::unexpected(nsswitch::ParseError{0, ""});

  EXPECT_NO_THROW(result = nsswitch::parse(input));

  ASSERT_TRUE(result.has_value())
      << "Parsing failed at line " << result.error().line << ": "
      << result.error().message;

  const auto &config = *result;

  // Commented-out entries must not be parsed.
  ASSERT_EQ(config.entries.size(), 4);

  constexpr std::string_view databases[] = {"passwd", "shadow", "group"};

  for (std::size_t i = 0; i < std::size(databases); ++i) {
    const auto &entry = config.entries[i];

    EXPECT_EQ(entry.database, databases[i]);
    ASSERT_EQ(entry.services.size(), 2);

    EXPECT_EQ(entry.services[0].name, "files");
    EXPECT_TRUE(entry.services[0].criterions.empty());

    EXPECT_EQ(entry.services[1].name, "sss");
    EXPECT_TRUE(entry.services[1].criterions.empty());
  }

  // hosts: files dns myhostname
  const auto &hosts = config.entries[3];

  EXPECT_EQ(hosts.database, "hosts");
  ASSERT_EQ(hosts.services.size(), 3);
  EXPECT_EQ(hosts.services[0].name, "files");
  EXPECT_EQ(hosts.services[1].name, "dns");
  EXPECT_EQ(hosts.services[2].name, "myhostname");
}