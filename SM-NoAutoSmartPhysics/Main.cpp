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

// TODO:
//  - Make the DLL work accross more versions, aka, automaticly detect game version and get the needed address & bytes.

#include <Windows.h>
#include <cstdint>
#include <array>

const uintptr_t TargetAddress = (uintptr_t)GetModuleHandle(NULL) + 0x343E63; // Modify this offset if the dll doesn't work with the game version your on!
const LPVOID TargetPointer = (LPVOID)TargetAddress;

// If the game was patched with the DLL.
static bool IsGamePatched = false;

// The old bytes
const std::array<uint8_t, 9> OldBytes = {
    0x83, 0xFE, 0x08,                   // .text:0000000140343E63 83 FE 08          cmp     esi, 8          - Do comparrasion if the Physics quality is 8 (Advanced Physics)
    0x0F, 0x85, 0x78, 0x02, 0x00, 0x00  // .text:0000000140343E66 0F 85 78 02 00 00 jnz     loc_1403440E4   - Jump to loc_1403440E4 if it failed.
};

// The new bytes
const std::array<uint8_t, 9> NewBytes = {
    0x90,                               // .text:0000000140343E63 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343E64 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343E65 90                nop                     - Do nothing
    0x90,                               // .text:0000000140343E66 90                nop                     - Do nothing
    0xE9, 0x78, 0x02, 0x00, 0x00        // .text:0000000140343E67 E9 78 02 00 00    jmp     loc_1403440E4   - Jump to this loc.
};

static bool NeedsUpdateCheck()
{
    printf("[SM-NoAutoSmartPhysics] " __FUNCTION__ "\n");

    // Get size of OldBytes
    const size_t OldBytesSize = OldBytes.size();
    const void* OldBytesData = OldBytes.data();

    // Let us be able to read, write & execute to that address.
    DWORD OldProtection;
    VirtualProtect(TargetPointer, OldBytesSize, PAGE_EXECUTE_READWRITE, &OldProtection);
    printf("[SM-NoAutoSmartPhysics] Disabled Protection on address %llx\n", TargetAddress);

    // Buffer containing the bytes from memory
    char* Buffer = new char[OldBytesSize];

    // Copy data from Pointer to address
    memcpy_s(Buffer, OldBytesSize, TargetPointer, OldBytesSize);
    printf("[SM-NoAutoSmartPhysics] Copied contents of %llx\n", TargetAddress);

    // Reset the protection
    VirtualProtect(TargetPointer, OldBytesSize, OldProtection, &OldProtection);
    printf("[SM-NoAutoSmartPhysics] Re-enabled Protection on address %llx\n", TargetAddress);

    // Check if there was a mismatch
    if (memcmp(OldBytesData, Buffer, OldBytesSize) != 0)
    {
        // Send message of the mismatching data.
        printf("[SM-NoAutoSmartPhysics] Incompattable game version! Game is NOT patchable for this DLL Version!\n");
        MessageBoxA(
            NULL,
            "SM-NoAutoSmartPhysics is not compattable with this Game version.\n\nUpdate the DLL or change the offset to work with this version!",
            "Incompattable Game Version!",
            MB_OK | MB_ICONERROR
        );

        // Return false, indicating that it failed the check.
        return false;
    }

    printf("[SM-NoAutoSmartPhysics] Compattable game version! Game is patchable for this DLL Version!\n");

    // Return true, indicating that it succeeded.
    return true;
}

// Called when the DLL attaches
static void Attach(HMODULE HModule)
{
    // Open console
    AttachConsole(GetCurrentProcessId());
    printf("[SM-NoAutoSmartPhysics] " __FUNCTION__ "\n");

    // Dont continue if the needs update check failed
    if (!NeedsUpdateCheck())
    {
        // Free & Exit the DLL.
        FreeLibraryAndExitThread(HModule, 1);
        return; // Stop further execution.
    }

    // Let us be able to read, write & execute to that address.
    DWORD OldProtection;
    VirtualProtect(TargetPointer, OldBytes.size(), PAGE_EXECUTE_READWRITE, &OldProtection);
    printf("[SM-NoAutoSmartPhysics] Disabled Protection on address %llx\n", TargetAddress);

    // Apply the patch
    memcpy_s(TargetPointer, OldBytes.size(), NewBytes.data(), NewBytes.size());
    printf("[SM-NoAutoSmartPhysics] Patched the game!\n");

    // Reset the protection
    VirtualProtect(TargetPointer, OldBytes.size(), OldProtection, &OldProtection);
    printf("[SM-NoAutoSmartPhysics] Re-enabled Protection on address %llx\n", TargetAddress);

    // Set IsGamePatched to true so the dll knows it successfully patched the game.
    IsGamePatched = true;
}

// Called when the DLL deattaches
static void Deattach()
{
    // Dont continue if the game wasent patched with this dll.
    if (!IsGamePatched)
    {
        printf("[SM-NoAutoSmartPhysics] " __FUNCTION__ "\n");
        goto END;
    }

    // Let us be able to read, write & execute to that address.
    DWORD OldProtection;
    VirtualProtect(TargetPointer, NewBytes.size(), PAGE_EXECUTE_READWRITE, &OldProtection);
    printf("[SM-NoAutoSmartPhysics] Disabled Protection on address %llx\n", TargetAddress);

    // Unapply the patch
    memcpy_s(TargetPointer, NewBytes.size(), OldBytes.data(), OldBytes.size());
    printf("[SM-NoAutoSmartPhysics] Unpatched the game!\n");

    // Reset the protection
    VirtualProtect(TargetPointer, NewBytes.size(), OldProtection, &OldProtection);
    printf("[SM-NoAutoSmartPhysics] Re-enabled Protection on address %llx\n", TargetAddress);
END:
    // Close console
    FILE* OldStream = nullptr;
    freopen_s(0, "CONOUT$", "w", OldStream);
}

BOOL APIENTRY DllMain(HMODULE HModule, DWORD Reason, LPVOID LPReserved)
{
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        // Create a new thread for the attaching process
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Attach, HModule, NULL, NULL);
        break;
    case DLL_PROCESS_DETACH:
        // Create a new thread for the deattaching process.
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Deattach, nullptr, NULL, NULL);
        break;
    }
    return TRUE;
}

