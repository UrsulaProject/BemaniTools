#ifndef BMT_ZIP_SUPPORT_H
#define BMT_ZIP_SUPPORT_H

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace bmt::detail
{
    struct ZipMember
    {
        std::string name;
        std::vector<uint8_t> data;
    };

    std::vector<std::string> ListZipEntries(const std::filesystem::path& path);
    std::vector<uint8_t> ReadZipEntry(const std::filesystem::path& path,
                                      std::string_view name);
    void WriteStoredZipWithMD5(const std::filesystem::path& path,
                               std::span<const ZipMember> members);
}

#endif
