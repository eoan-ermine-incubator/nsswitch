#include <gtest/gtest.h>

#include "nsswitch/nsswitch.h"

TEST(NsswitchParserTest, NsswitchSberlinuxMan1) {
  // Source:
  // https://platformv.sbertech.ru/docs/public/SLO/9.0-fstec/common/documents/pfstec/administration-guide/nsswitch.conf.html
  constexpr std::string_view input = R"(
passwd:    db files
shadow:    db files
group:     db files
)";

  std::expected<nsswitch::Config, nsswitch::ParseError> result =
      std::unexpected(nsswitch::ParseError{0, ""});

  EXPECT_NO_THROW(result = nsswitch::parse(input));

  ASSERT_TRUE(result.has_value())
      << "Parsing failed at line " << result.error().line << ": "
      << result.error().message;

  const auto &config = *result;

  ASSERT_EQ(config.entries.size(), 3);

  constexpr std::string_view databases[] = {"passwd", "shadow", "group"};

  for (std::size_t i = 0; i < std::size(databases); ++i) {
    const auto &entry = config.entries[i];

    EXPECT_EQ(entry.database, databases[i]);
    ASSERT_EQ(entry.services.size(), 2);

    EXPECT_EQ(entry.services[0].name, "db");
    EXPECT_TRUE(entry.services[0].criterions.empty());

    EXPECT_EQ(entry.services[1].name, "files");
    EXPECT_TRUE(entry.services[1].criterions.empty());
  }
}

TEST(NsswitchParserTest, NsswitchSberlinuxMan2) {
  // Source:
  // https://platformv.sbertech.ru/docs/public/SLO/9.0-fstec/common/documents/pfstec/administration-guide/nsswitch.conf.html
  constexpr std::string_view input = R"(
passwd:     sss files systemd
shadow:     files
group:      sss files systemd
hosts:      files dns myhostname
services:   files sss
netgroup:   sss
automount:  files sss
aliases:    files
ethers:     files
gshadow:    files
initgroups: files
networks:   files dns
protocols:  files
publickey:  files
rpc:        files
)";

  std::expected<nsswitch::Config, nsswitch::ParseError> result =
      std::unexpected(nsswitch::ParseError{0, ""});

  EXPECT_NO_THROW(result = nsswitch::parse(input));

  ASSERT_TRUE(result.has_value())
      << "Parsing failed at line " << result.error().line << ": "
      << result.error().message;

  const auto &config = *result;

  ASSERT_EQ(config.entries.size(), 15);

  struct ExpectedEntry {
    std::string_view database;
    std::vector<std::string_view> services;
  };

  const std::vector<ExpectedEntry> expected = {
      {"passwd", {"sss", "files", "systemd"}},
      {"shadow", {"files"}},
      {"group", {"sss", "files", "systemd"}},
      {"hosts", {"files", "dns", "myhostname"}},
      {"services", {"files", "sss"}},
      {"netgroup", {"sss"}},
      {"automount", {"files", "sss"}},
      {"aliases", {"files"}},
      {"ethers", {"files"}},
      {"gshadow", {"files"}},
      {"initgroups", {"files"}},
      {"networks", {"files", "dns"}},
      {"protocols", {"files"}},
      {"publickey", {"files"}},
      {"rpc", {"files"}},
  };

  for (std::size_t i = 0; i < expected.size(); ++i) {
    const auto &entry = config.entries[i];

    EXPECT_EQ(entry.database, expected[i].database);
    ASSERT_EQ(entry.services.size(), expected[i].services.size());

    for (std::size_t j = 0; j < expected[i].services.size(); ++j) {
      EXPECT_EQ(entry.services[j].name, expected[i].services[j]);
      EXPECT_TRUE(entry.services[j].criterions.empty());
    }
  }
}
