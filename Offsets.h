GEngine: 0x1A655088
FNameToString: 0x4ECB4
GNames: 0x1A506240
StaticFindObject: 0x823CFB
StaticLoadObject: 0x183EAE
ProcessEvent: 0x719E0
GetWorldCtxObj: 0x9E0A848
drawtrans: 102/103 one of these 2 
  
inline auto DecryptUWorld() -> std::uintptr_t
{
    uint64_t v = read<uint64_t>(globals::game::base + 0x1A6536B0);
    if (!v) { return 0; }

    v -= 28021308;

    v = _rotl64(v, 16);

    v ^= 0x937B782;

    return static_cast<uintptr_t>(v);
}


  static std::string GetName(uint32_t RawIndex)
    {
        const uint32_t Index = DecryptIndex(RawIndex);
        if (Index == 0) { return ""; }
        uintptr_t Entry = 0;
        uint16_t Header = 0;
        if (!ReadEntry(Index, Entry, Header)) { return ""; }
        uint32_t Suffix = 0xFFFFFFFFu;
        if (static_cast<uint32_t>(Header >> 6) == 0x276u) {
            const uint32_t NextEnc = read<uint32_t>(Entry + 2);
            const uint32_t SufEnc = read<uint32_t>(Entry + 6);
            Suffix = DecryptNumber(SufEnc);
            const uint32_t Next = DecryptIndex(NextEnc);
            if (Next == 0) { return ""; }
            if (!ReadEntry(Next, Entry, Header)) { return ""; }
            if (static_cast<uint32_t>(Header >> 6) == 0x276u) { return ""; }
        }
        const bool IsWide = (Header & 1u) != 0u;
        const uint32_t CharLength = (static_cast<uint32_t>(Header >> 6) ^ 0x276u);
        if (CharLength == 0 || CharLength > 1024u) { return ""; }
        const uint32_t ByteLength = IsWide ? (CharLength * 2u) : CharLength;
        std::vector<uint8_t> Enc(ByteLength);
        std::vector<uint8_t> Dec(ByteLength);
        read_bytes(Entry + 2, Enc.data(), ByteLength);
        DecryptChars(Enc.data(), Dec.data(), ByteLength, CharLength);
        std::string Result;
        if (IsWide) {
            Result.resize(CharLength);
            for (uint32_t I = 0; I < CharLength; ++I) {
                Result[I] = static_cast<char>(Dec[I * 2]);
            }
        }
        else {
            Result.assign(reinterpret_cast<char*>(Dec.data()), CharLength);
        }
        if (Suffix != 0xFFFFFFFFu && Suffix != 0) {
            Result += "_";
            Result += std::to_string(static_cast<int32_t>(Suffix - 1));
        }
        return Result;
    }
};



class FName
{
public:
    static constexpr uintptr_t GNamesOffset = 0x1A506240;
    static constexpr uintptr_t ChunkArrayOff = 16;

    static std::string Resolve(int32_t Index) { return GetName(static_cast<uint32_t>(Index)); }
    static std::string Get(int32_t Index) { return GetName(static_cast<uint32_t>(Index)); }
    static std::string ResolveWithNumber(int32_t Index, uint32_t Number)
    {
        std::string Base = GetName(static_cast<uint32_t>(Index));
        if (Number != 0) {
            Base += "_";
            Base += std::to_string(Number - 1);
        }
        return Base;
    }

private:
    static inline uint32_t Rol32(uint32_t V, unsigned int C) { C &= 31; return (V << C) | (V >> ((32 - C) & 31)); }
    static inline uint8_t Ror8(uint8_t V, unsigned int C) { C &= 7; return (V >> C) | (V << ((8 - C) & 7)); }

    static uint32_t DecryptIndex(uint32_t Enc)
    {
        if (!Enc) { return 0; }
        const uint32_t Decoded = (Rol32(Enc - 24734891u, 29) ^ 0x7295BA33u) + 1u;
        return Decoded ? Decoded : 0xCD3BB99Au;
    }
    static uint32_t DecryptNumber(uint32_t Enc)
    {
        if (!Enc) { return 0xFFFFFFFFu; }
        const uint32_t Off = Enc - 24734891u;
        if (Off == 450903847u) { return 0xA37D5D71u; }
        return Rol32(Off, 29) ^ 0x1CA3F81Bu;
    }
    static void DecryptChars(const uint8_t* Src, uint8_t* Dst, uint32_t ByteLength, uint32_t CharLength)
    {
        const uint32_t A4 = (CharLength << 8) + ((8299u * CharLength + 5248801u) ^ ((8299u * CharLength + 5248801u) >> 19));
        const uint32_t V6 = ((8299u * A4 + 5248801u) ^ ((8299u * A4 + 5248801u) >> 19)) + Rol32(A4, 8);
        uint32_t V7 = static_cast<uint16_t>(V6 - 21955u);
        if (ByteLength == 0) { return; }
        for (uint32_t I = 0; I < ByteLength; ++I) {
            const uint32_t SrcIdx = V7 % ByteLength;
            const uint8_t Raw = Src[SrcIdx];
            Dst[I] = static_cast<uint8_t>(Ror8(static_cast<uint8_t>(Raw - 72u), 1) ^ (V6 + 116u));
            ++V7;
        }
    }
    static bool ReadEntry(uint32_t Idx, uintptr_t& OutEntry, uint16_t& OutHeader)
    {
        const uintptr_t ChunkSlot = globals::game::base + GNamesOffset + ChunkArrayOff + 8 * (Idx >> 16);
        const uintptr_t ChunkPtr = read<uintptr_t>(ChunkSlot);
        if (!ChunkPtr) { return false; }
        OutEntry = ChunkPtr + 2 * static_cast<uint16_t>(Idx);
        OutHeader = read<uint16_t>(OutEntry);
        return OutHeader != 0;
    }





  
