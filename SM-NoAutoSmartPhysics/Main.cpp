// SM-NoAutoSmartPhysics: Patch for removing automatic switch to Smart physics when your game is frozen.
//
// This Patch removes the automatic switch to Smart Physics if your game is frozen for too long in Advanced
// Physics. This removes it basicly letting you be in Advanced Physics no matter what.
// 
//                           How to apply the patch yourself for future game updates.
// 
// What we do is we replace the 9 bytes in TargetAddress (which is 2 instructions, see commments of OldBytes)
// with 4 nop instructions and a jmp instruction to the case when it "fails", This means that the automatic
// detection does happen but it basicly changes the if statement check in the comparrasion into "if(false)"
// check making this patch work.
//
// To find the address by yourself & apply the patch yourself, Open IDA Pro (or your favourite reverse engineer tool)
// and search in strings for "Physics hung!", There should be only 1 mention at the time writing this, Then
// decompile it to psudocode and find the check outside the "Physics hung!" string. In VeraDev's case its
// "if(v18 == 8)", that would be the address you need. Now simply replace the bytes of that where it always fails
// the case no matter what (aka jumps) and that would be it.

#include <Windows.h>
#include <cstdint>
#include <array>

static const LPVOID pAddress = reinterpret_cast<LPVOID>((uintptr_t)GetModuleHandle(NULL) + 0x343E63); // Modify this offset if the dll doesn't work with the game version your on!

constexpr std::array<uint8_t, 9> OldBytes = {
    0x83, 0xFE, 0x08,                   // .text:0000000140343E63 83 FE 08          cmp     esi, 8          - Do comparrasion if the Physics quality is 8 (Advanced Physics)
    0x0F, 0x85, 0x78, 0x02, 0x00, 0x00  // .text:0000000140343E66 0F 85 78 02 00 00 jnz     loc_1403440E4   - Jump to loc_1403440E4 if it failed.
};

constexpr std::array<uint8_t, 9> newBytes = {
    0x90,                               // .text:0000000140343E63 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343E64 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343E65 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343E66 90                nop                     - Do nothing
    0xE9, 0x78, 0x02, 0x00, 0x00        // .text:0000000140343E67 E9 78 02 00 00    jmp     loc_1403440E4   - Jump to this loc.
};

static bool NeedsUpdateCheck()
{
    const size_t oldBytesSize = OldBytes.size();
    const void* OldBytesData = OldBytes.data();

    DWORD oldProtection;
    VirtualProtect(pAddress, oldBytesSize, PAGE_EXECUTE_READWRITE, &oldProtection);

    char* Buffer = new char[oldBytesSize];
    memcpy_s(Buffer, oldBytesSize, pAddress, oldBytesSize);

    VirtualProtect(pAddress, oldBytesSize, oldProtection, &oldProtection);

    if (memcmp(OldBytesData, Buffer, oldBytesSize) == 0)
        return true;
    
    MessageBox(
        NULL,
        L"SM-NoAutoSmartPhysics is not compattable with this game version!\n\nUpdate the DLL or change the offset to work with this version!",
        L"Incompattable Game Version!",
        MB_OK | MB_ICONERROR
    );

    return false;
}

static void Attach(HMODULE hModule)
{
    if (!NeedsUpdateCheck())
    {
        FreeLibraryAndExitThread(hModule, 1);
        return;
    }

    DWORD oldProtection;
    VirtualProtect(pAddress, OldBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtection);

    memcpy_s(pAddress, OldBytes.size(), newBytes.data(), newBytes.size());

    VirtualProtect(pAddress, OldBytes.size(), oldProtection, &oldProtection);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD Reason, LPVOID LPReserved)
{
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Attach, hModule, NULL, NULL);
        break;
    }
    return TRUE;
}

