#include <gtest/gtest.h>

#include "nsswitch/parser.h"

TEST(NsswitchParserTest, NsswitchTldp) {
  // Source:
  // https://tldp.org/LDP/solrhe/Securing-Optimizing-Linux-RH-Edition-v1.3/chap6sec71.html
  constexpr std::string_view input = R"(
            passwd:files
            shadow:files
            group:files
            hosts:dns files
            bootparams:files
            ethers:files
            netmasks:files
            networks:files
            protocols:files
            rpc:files
            services:files
            automount:files
            aliases:files           
)";

  std::expected<nsswitch::Config, nsswitch::ParseError> result =
      std::unexpected(nsswitch::ParseError{0, ""});

  EXPECT_NO_THROW(result = nsswitch::parse(input));

  ASSERT_TRUE(result.has_value())
      << "Parsing failed at line " << result.error().line << ": "
      << result.error().message;

  const auto &config = *result;

  ASSERT_EQ(config.entries.size(), 13);

  // hosts: dns files
  EXPECT_EQ(config.entries[3].database, "hosts");
  ASSERT_EQ(config.entries[3].services.size(), 2);
  EXPECT_EQ(config.entries[3].services[0].name, "dns");
  EXPECT_EQ(config.entries[3].services[1].name, "files");

  // All remaining databases use the files service.
  constexpr std::string_view databases[] = {
      "passwd",   "shadow",    "group", "bootparams", "ethers",    "netmasks",
      "networks", "protocols", "rpc",   "services",   "automount", "aliases"};

  for (std::size_t i = 0; i < std::size(databases); ++i) {
    const std::size_t index = i < 3 ? i : i + 1;
    const auto &entry = config.entries[index];

    EXPECT_EQ(entry.database, databases[i]);
    ASSERT_EQ(entry.services.size(), 1);
    EXPECT_EQ(entry.services[0].name, "files");
    EXPECT_TRUE(entry.services[0].criterions.empty());
  }
}
