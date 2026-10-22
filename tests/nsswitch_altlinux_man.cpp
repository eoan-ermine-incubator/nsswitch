#include <gtest/gtest.h>

#include "nsswitch/nsswitch.h"

TEST(NsswitchParserTest, NsswitchAltLinuxMan) {
  // Source:
  // https://platformv.sbertech.ru/docs/public/SLO/9.0-fstec/common/documents/pfstec/administration-guide/nsswitch.conf.html
  constexpr std::string_view input = R"(
passwd:     files winbind systemd
shadow:     tcb files winbind
group:      files [SUCCESS=merge] winbind role systemd
gshadow:    files

hosts:      files myhostname dns

ethers:     files
netmasks:   files
networks:   files
protocols:  files
rpc:        files
services:   files

automount:  files
aliases:    files
)";

  std::expected<nsswitch::Config, nsswitch::ParseError> result =
      std::unexpected(nsswitch::ParseError{0, ""});

  EXPECT_NO_THROW(result = nsswitch::parse(input));

  ASSERT_TRUE(result.has_value())
      << "Parsing failed at line " << result.error().line << ": "
      << result.error().message;

  const auto &config = *result;

  ASSERT_EQ(config.entries.size(), 13);

  // passwd: files winbind systemd
  EXPECT_EQ(config.entries[0].database, "passwd");
  ASSERT_EQ(config.entries[0].services.size(), 3);
  EXPECT_EQ(config.entries[0].services[0].name, "files");
  EXPECT_EQ(config.entries[0].services[1].name, "winbind");
  EXPECT_EQ(config.entries[0].services[2].name, "systemd");

  // shadow: tcb files winbind
  EXPECT_EQ(config.entries[1].database, "shadow");
  ASSERT_EQ(config.entries[1].services.size(), 3);
  EXPECT_EQ(config.entries[1].services[0].name, "tcb");
  EXPECT_EQ(config.entries[1].services[1].name, "files");
  EXPECT_EQ(config.entries[1].services[2].name, "winbind");

  // group: files [SUCCESS=merge] winbind role systemd
  const auto &group = config.entries[2];

  EXPECT_EQ(group.database, "group");
  ASSERT_EQ(group.services.size(), 4);

  EXPECT_EQ(group.services[0].name, "files");
  ASSERT_EQ(group.services[0].criterions.size(), 1);

  EXPECT_FALSE(group.services[0].criterions[0].inversion);
  EXPECT_EQ(group.services[0].criterions[0].status,
            nsswitch::Criterion::Status::SUCCESS);
  EXPECT_EQ(group.services[0].criterions[0].action,
            nsswitch::Criterion::Action::MERGE);

  EXPECT_EQ(group.services[1].name, "winbind");
  EXPECT_EQ(group.services[2].name, "role");
  EXPECT_EQ(group.services[3].name, "systemd");

  // gshadow: files
  EXPECT_EQ(config.entries[3].database, "gshadow");
  ASSERT_EQ(config.entries[3].services.size(), 1);
  EXPECT_EQ(config.entries[3].services[0].name, "files");

  // hosts: files myhostname dns
  EXPECT_EQ(config.entries[4].database, "hosts");
  ASSERT_EQ(config.entries[4].services.size(), 3);
  EXPECT_EQ(config.entries[4].services[0].name, "files");
  EXPECT_EQ(config.entries[4].services[1].name, "myhostname");
  EXPECT_EQ(config.entries[4].services[2].name, "dns");

  // Remaining databases use the files service.
  constexpr std::string_view databases[] = {"ethers",    "netmasks", "networks",
                                            "protocols", "rpc",      "services",
                                            "automount", "aliases"};

  for (std::size_t i = 0; i < std::size(databases); ++i) {
    const auto &entry = config.entries[i + 5];

    EXPECT_EQ(entry.database, databases[i]);
    ASSERT_EQ(entry.services.size(), 1);
    EXPECT_EQ(entry.services[0].name, "files");
    EXPECT_TRUE(entry.services[0].criterions.empty());
  }
}
