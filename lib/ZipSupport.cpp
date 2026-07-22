#include "ZipSupport.h"

#include "FileSupport.h"

#include <zip.h>

#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>

namespace fs = std::filesystem;

namespace
{
    std::string UTF8Path(const fs::path& path)
    {
        const auto utf8 = path.u8string();
        return {reinterpret_cast<const char*>(utf8.data()), utf8.size()};
    }

    struct ZipDeleter
    {
        void operator()(zip_t* value) const noexcept
        {
            if (value)
                zip_discard(value);
        }
    };
    using ZipPtr = std::unique_ptr<zip_t, ZipDeleter>;

    struct ZipFileDeleter
    {
        void operator()(zip_file_t* value) const noexcept
        {
            if (value)
                zip_fclose(value);
        }
    };
    using ZipFilePtr = std::unique_ptr<zip_file_t, ZipFileDeleter>;
}

namespace bmt::detail
{
    std::vector<std::string> ListZipEntries(const fs::path& path)
    {
        int error = 0;
        const std::string archivePath = UTF8Path(path);
        ZipPtr archive(zip_open(archivePath.c_str(), ZIP_RDONLY, &error));
        if (!archive)
            throw std::runtime_error("cannot open ZIP " + path.string());
        const zip_int64_t count = zip_get_num_entries(archive.get(), 0);
        if (count < 0)
            throw std::runtime_error("cannot enumerate ZIP " + path.string());
        std::vector<std::string> names;
        names.reserve(static_cast<size_t>(count));
        for (zip_uint64_t index = 0; index < static_cast<zip_uint64_t>(count); ++index)
        {
            const char* name = zip_get_name(archive.get(), index, ZIP_FL_ENC_GUESS);
            if (name && name[0] && name[std::strlen(name) - 1] != '/')
                names.emplace_back(name);
        }
        return names;
    }

    std::vector<uint8_t> ReadZipEntry(const fs::path& path, std::string_view name)
    {
        int error = 0;
        const std::string archivePath = UTF8Path(path);
        ZipPtr archive(zip_open(archivePath.c_str(), ZIP_RDONLY, &error));
        if (!archive)
            throw std::runtime_error("cannot open ZIP " + path.string());
        const std::string memberName(name);
        zip_stat_t stat{};
        zip_stat_init(&stat);
        if (zip_stat(archive.get(), memberName.c_str(), ZIP_FL_ENC_GUESS, &stat) != 0)
            throw std::runtime_error("ZIP member not found: " + memberName);
        if (stat.size > std::numeric_limits<size_t>::max())
            throw std::runtime_error("ZIP member is too large: " + memberName);
        ZipFilePtr member(zip_fopen(archive.get(), memberName.c_str(), ZIP_FL_ENC_GUESS));
        if (!member)
            throw std::runtime_error("cannot open ZIP member: " + memberName);
        std::vector<uint8_t> output(static_cast<size_t>(stat.size));
        size_t offset = 0;
        while (offset < output.size())
        {
            const zip_int64_t count = zip_fread(member.get(), output.data() + offset,
                                                 output.size() - offset);
            if (count <= 0)
                throw std::runtime_error("cannot read ZIP member: " + memberName);
            offset += static_cast<size_t>(count);
        }
        return output;
    }

    void WriteStoredZipWithMD5(const fs::path& path, std::span<const ZipMember> members)
    {
        if (!path.parent_path().empty())
            fs::create_directories(path.parent_path());
        int error = 0;
        const std::string archivePath = UTF8Path(path);
        ZipPtr archive(zip_open(archivePath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error));
        if (!archive)
            throw std::runtime_error("cannot create ZIP " + path.string());
        for (const auto& member : members)
        {
            zip_source_t* source = zip_source_buffer(archive.get(), member.data.data(),
                                                     member.data.size(), 0);
            if (!source)
                throw std::runtime_error("cannot allocate ZIP source for " + member.name);
            const zip_int64_t index = zip_file_add(archive.get(), member.name.c_str(), source,
                                                   ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
            if (index < 0)
            {
                zip_source_free(source);
                throw std::runtime_error("cannot add ZIP member " + member.name);
            }
            if (zip_set_file_compression(archive.get(), static_cast<zip_uint64_t>(index),
                                         ZIP_CM_STORE, 0) != 0)
                throw std::runtime_error("cannot set ZIP store mode for " + member.name);
        }
        zip_t* raw = archive.release();
        if (zip_close(raw) != 0)
        {
            const std::string message = zip_strerror(raw);
            zip_discard(raw);
            throw std::runtime_error("cannot finalize ZIP " + path.string() + ": " + message);
        }
        AppendFileMD5(path);
    }
}
