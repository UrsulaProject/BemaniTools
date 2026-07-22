#include "FileSupport.h"

#include <openssl/evp.h>

#include <array>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace fs = std::filesystem;

namespace bmt::detail
{
    std::vector<uint8_t> ReadFile(const fs::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input)
            throw std::runtime_error("cannot open " + path.string());
        input.seekg(0, std::ios::end);
        const auto size = input.tellg();
        if (size < 0 || static_cast<uintmax_t>(size) > std::numeric_limits<size_t>::max())
            throw std::runtime_error("cannot determine size of " + path.string());
        input.seekg(0, std::ios::beg);
        std::vector<uint8_t> output(static_cast<size_t>(size));
        if (!output.empty())
            input.read(reinterpret_cast<char*>(output.data()),
                       static_cast<std::streamsize>(output.size()));
        if (!input)
            throw std::runtime_error("cannot read " + path.string());
        return output;
    }

    void WriteFile(const fs::path& path, std::span<const uint8_t> data)
    {
        if (!path.parent_path().empty())
            fs::create_directories(path.parent_path());
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        if (!output)
            throw std::runtime_error("cannot create " + path.string());
        if (!data.empty())
            output.write(reinterpret_cast<const char*>(data.data()),
                         static_cast<std::streamsize>(data.size()));
        if (!output)
            throw std::runtime_error("cannot write " + path.string());
    }

    void AppendFileMD5(const fs::path& path)
    {
        const auto bytes = ReadFile(path);
        std::array<uint8_t, EVP_MAX_MD_SIZE> digest{};
        unsigned int digestLength = 0;
        if (EVP_Digest(bytes.data(), bytes.size(), digest.data(), &digestLength,
                       EVP_md5(), nullptr) != 1 || digestLength != 16)
            throw std::runtime_error("cannot calculate MD5 for " + path.string());
        std::ofstream output(path, std::ios::binary | std::ios::app);
        if (!output)
            throw std::runtime_error("cannot append MD5 to " + path.string());
        output.write(reinterpret_cast<const char*>(digest.data()), digestLength);
        if (!output)
            throw std::runtime_error("cannot write MD5 to " + path.string());
    }
}
