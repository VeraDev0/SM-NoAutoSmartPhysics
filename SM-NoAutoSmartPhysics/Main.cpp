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

const uintptr_t TargetAddress = (uintptr_t)GetModuleHandle(NULL) + 0x343EE3;

// The old bytes
const std::array<uint8_t, 9> OldBytes = {
    0x83, 0xFE, 0x08,                   // .text:0000000140343EE3 83 FE 08          cmp     esi, 8          - Compares the PhysicsQuality if its 8
    0x0F, 0x85, 0x78, 0x02, 0x00, 0x00  // .text:0000000140343EE6 0F 85 78 02 00 00 jnz     loc_140344164   - Jump to this loc if it failed.
};

const std::array<uint8_t, 9> NewBytes = {
    0x90,                               // .text:0000000140343EE3 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343EE4 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343EE5 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343EE6 90                nop                     - Do nothing
    0xE9, 0x78, 0x02, 0x00, 0x00        // .text:0000000140343EE6 E9 78 02 00 00    jmp     loc_140344164   - Jump to this loc.
};

// Called when the DLL attaches
void Attach()
{
    // Convert Address to a void pointer
    void* Destination = (void*)TargetAddress;

    // Let us be able to read, write & execute to that address.
    DWORD OldProtection;
    VirtualProtect(Destination, OldBytes.size(), PAGE_EXECUTE_READWRITE, &OldProtection);

    // Apply the patch
    memcpy_s(Destination, OldBytes.size(), NewBytes.data(), NewBytes.size());

    // Reset the protection
    VirtualProtect(Destination, OldBytes.size(), OldProtection, &OldProtection);
}

// Called when the DLL deattaches
void Deattach()
{
    // Convert Address to a void pointer
    void* Destination = (void*)TargetAddress;

    // Let us be able to read, write & execute to that address.
    DWORD OldProtection;
    VirtualProtect(Destination, NewBytes.size(), PAGE_EXECUTE_READWRITE, &OldProtection);

    // Unapply the patch
    memcpy_s(Destination, NewBytes.size(), OldBytes.data(), OldBytes.size());

    // Reset the protection
    VirtualProtect(Destination, NewBytes.size(), OldProtection, &OldProtection);
}

BOOL APIENTRY DllMain(HMODULE HModule, DWORD Reason, LPVOID LPReserved)
{
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        // Create a new thread for the attaching process
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Attach, nullptr, NULL, NULL);
        break;
    case DLL_PROCESS_DETACH:
        // Create a new thread for the deattaching process.
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Deattach, nullptr, NULL, NULL);
        break;
    }
    return TRUE;
}

