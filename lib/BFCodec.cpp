//
// Created by Naville on 2026/7/16.
//

#include "Bemani/BFCodec.h"
#include <stdexcept>
#include <tuple>
namespace bmt
{
    BFCodec::BFCodec(const std::array<uint32_t, 18>& P_,
                     const std::array<std::array<uint32_t, 256>, 4> S_, const std::vector<uint8_t>& Key) : P(P_), S(S_)
    {
        if (Key.empty())
            throw std::invalid_argument("BFCodec key must not be empty");
        size_t j = 0;
        const size_t keySize = Key.size();
        for (unsigned i = 0;i < 18;i++)
        {
            const uint32_t word = (static_cast<uint32_t>(Key[j]) << 24) |
                    (static_cast<uint32_t>(Key[(j + 1) % keySize]) << 16)
                | (static_cast<uint32_t>(Key[(j + 2) % keySize]) << 8)
                | static_cast<uint32_t>(Key[(j + 3) % keySize]);
            P[i] = P[i] ^ word;
            j = (j + 4) % keySize;
        }
        uint32_t L = 0, R = 0;
        for (uint32_t i =0;i < 18;i = i + 2)
        {
            std::tie(L, R) = EncryptBlock(L, R);
            P[i] = L;
            P[i + 1] = R;
        }
        for (auto& Box : S)
        {
            for (uint32_t i = 0; i < 256; i = i + 2)
            {
                std::tie(L, R) = EncryptBlock(L, R);
                Box[i] = L;
                Box[i + 1] = R;
            }
        }
    }

    uint32_t BFCodec::F(uint32_t x) const noexcept
    {
        const auto a = static_cast<uint8_t>(x >> 24);
        const auto b = static_cast<uint8_t>(x >> 16);
        const auto c = static_cast<uint8_t>(x >> 8);
        const auto d = static_cast<uint8_t>(x);
        const uint32_t left = S[0][a] + S[1][b];
        const uint32_t right = S[2][c] + S[3][d];
        return left ^ right;
    }

    std::pair<uint32_t, uint32_t> BFCodec::EncryptBlock(uint32_t L, uint32_t R) const noexcept
    {
        for (uint32_t i = 0; i < 16; i++)
        {
            uint32_t x = L ^ P[i];
            L = R ^ F(x);
            R = x;
        }
        uint32_t outL = R ^ P[17];
        uint32_t outR = L ^ P[16];
        return std::make_pair(outL, outR);
    }

    std::pair<uint32_t, uint32_t> BFCodec::DecryptBlock(uint32_t L, uint32_t R) const noexcept
    {
        for (uint32_t i = 17; i >= 2; i--)
        {
            uint32_t x = L ^ P[i];
            L = R ^ F(x);
            R = x;
        }
        const uint32_t outL = R ^ P[0];
        const uint32_t outR = L ^ P[1];
        return std::make_pair(outL, outR);
    }

    std::vector<uint8_t> BFCodec::DecryptCBC(const std::vector<uint8_t>& cipherText, const std::array<uint8_t, 8>& IV) const
    {
        if (cipherText.size() % 8 != 0)
            throw std::invalid_argument("BFCodec ciphertext length must be a multiple of 8");
        std::vector<uint8_t> out(cipherText.size());
        auto readBE32 = [](const uint8_t* ptr) -> uint32_t
        {
            return (static_cast<uint32_t>(ptr[0]) << 24) |
                (static_cast<uint32_t>(ptr[1]) << 16) |
                (static_cast<uint32_t>(ptr[2]) << 8) |
                static_cast<uint32_t>(ptr[3]);
        };
        auto writeBE32 = [](uint8_t* ptr, uint32_t value) -> void
        {
            ptr[0] = static_cast<uint8_t>(value >> 24);
            ptr[1] = static_cast<uint8_t>(value >> 16);
            ptr[2] = static_cast<uint8_t>(value >> 8);
            ptr[3] = static_cast<uint8_t>(value);
        };
        uint32_t prevL = readBE32(IV.data());
        uint32_t prevR = readBE32(IV.data() + 4);

        for (size_t i = 0; i < cipherText.size(); i = i + 8)
        {
            uint32_t cL = readBE32(cipherText.data() + i);
            uint32_t cR = readBE32(cipherText.data() + i + 4);
            auto [pL, pR] = DecryptBlock(cL, cR);
            pL = pL ^ prevL;
            pR = pR ^ prevR;
            writeBE32(out.data() + i, pL);
            writeBE32(out.data() + i + 4, pR);
            prevL = cL;
            prevR = cR;
        }
        return out;
    }

    std::vector<uint8_t> BFCodec::EncryptCBC(const std::vector<uint8_t>& plainText, const std::array<uint8_t, 8>& IV) const
    {
        if (plainText.size() % 8 != 0)
            throw std::invalid_argument("BFCodec plaintext length must be a multiple of 8");
        std::vector<uint8_t> out(plainText.size());
        auto readBE32 = [](const uint8_t* ptr) -> uint32_t
        {
            return (static_cast<uint32_t>(ptr[0]) << 24) |
                (static_cast<uint32_t>(ptr[1]) << 16) |
                (static_cast<uint32_t>(ptr[2]) << 8) |
                static_cast<uint32_t>(ptr[3]);
        };
        auto writeBE32 = [](uint8_t* ptr, uint32_t value) -> void
        {
            ptr[0] = static_cast<uint8_t>(value >> 24);
            ptr[1] = static_cast<uint8_t>(value >> 16);
            ptr[2] = static_cast<uint8_t>(value >> 8);
            ptr[3] = static_cast<uint8_t>(value);
        };
        uint32_t prevL = readBE32(IV.data());
        uint32_t prevR = readBE32(IV.data() + 4);

        for (size_t i = 0; i < plainText.size(); i = i + 8)
        {
            uint32_t pL = readBE32(plainText.data() + i);
            uint32_t pR = readBE32(plainText.data() + i + 4);
            auto [cL, cR] = EncryptBlock(pL ^ prevL, pR ^ prevR);
            writeBE32(out.data() + i, cL);
            writeBE32(out.data() + i + 4, cR);
            prevL = cL;
            prevR = cR;
        }
        return out;
    }
}
