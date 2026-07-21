#ifndef BMT_BFDEFAULTS_H
#define BMT_BFDEFAULTS_H

#include <array>
#include <cstdint>

namespace bmt
{
    const std::array<uint32_t, 18>& DefaultBlowfishP() noexcept;
    const std::array<std::array<uint32_t, 256>, 4>& DefaultBlowfishS() noexcept;
    const std::array<uint8_t, 8>& DefaultBlowfishIV() noexcept;
}

#endif
