#ifndef BMT_FILE_SUPPORT_H
#define BMT_FILE_SUPPORT_H

#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

namespace bmt::detail
{
    std::vector<uint8_t> ReadFile(const std::filesystem::path& path);
    void WriteFile(const std::filesystem::path& path, std::span<const uint8_t> data);
    void AppendFileMD5(const std::filesystem::path& path);
}

#endif
