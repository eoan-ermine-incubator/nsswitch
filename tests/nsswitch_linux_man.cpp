#include <gtest/gtest.h>

#include "nsswitch/parser.h"

TEST(NsswitchParserTest, NsswitchLinuxMan) {
    // Source: https://man7.org/linux/man-pages/man5/nsswitch.conf.5.html
    constexpr std::string_view input = R"(
passwd:         compat
group:          compat
shadow:         compat

hosts:          dns [!UNAVAIL=return] files
networks:       nis [NOTFOUND=return] files
ethers:         nis [NOTFOUND=return] files
protocols:      nis [NOTFOUND=return] files
rpc:            nis [NOTFOUND=return] files
services:       nis [NOTFOUND=return] files
)";

    std::expected<nsswitch::Config, nsswitch::ParseError> result =
        std::unexpected(nsswitch::ParseError{0, ""});

    EXPECT_NO_THROW(result = nsswitch::parse(input));

    ASSERT_TRUE(result.has_value())
        << "Parsing failed at line " << result.error().line << ": "
        << result.error().message;

    const auto& config = *result;

    ASSERT_EQ(config.entries.size(), 9);

    // passwd: compat
    EXPECT_EQ(config.entries[0].database, "passwd");
    ASSERT_EQ(config.entries[0].services.size(), 1);
    EXPECT_EQ(config.entries[0].services[0].name, "compat");
    EXPECT_TRUE(config.entries[0].services[0].criterions.empty());

    // group: compat
    EXPECT_EQ(config.entries[1].database, "group");
    ASSERT_EQ(config.entries[1].services.size(), 1);
    EXPECT_EQ(config.entries[1].services[0].name, "compat");
    EXPECT_TRUE(config.entries[1].services[0].criterions.empty());

    // shadow: compat
    EXPECT_EQ(config.entries[2].database, "shadow");
    ASSERT_EQ(config.entries[2].services.size(), 1);
    EXPECT_EQ(config.entries[2].services[0].name, "compat");
    EXPECT_TRUE(config.entries[2].services[0].criterions.empty());

    // hosts: dns [!UNAVAIL=return] files
    const auto& hosts = config.entries[3];

    EXPECT_EQ(hosts.database, "hosts");
    ASSERT_EQ(hosts.services.size(), 2);

    EXPECT_EQ(hosts.services[0].name, "dns");
    ASSERT_EQ(hosts.services[0].criterions.size(), 1);

    EXPECT_TRUE(hosts.services[0].criterions[0].inversion);
    EXPECT_EQ(hosts.services[0].criterions[0].status,
              nsswitch::Criterion::Status::UNAVAIL);
    EXPECT_EQ(hosts.services[0].criterions[0].action,
              nsswitch::Criterion::Action::RETURN);

    EXPECT_EQ(hosts.services[1].name, "files");
    EXPECT_TRUE(hosts.services[1].criterions.empty());

    // The remaining databases use nis [NOTFOUND=return] files.
    constexpr std::string_view databases[] = {
        "networks", "ethers", "protocols", "rpc", "services"};

    for (std::size_t i = 0; i < std::size(databases); ++i) {
        const auto& entry = config.entries[i + 4];

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
