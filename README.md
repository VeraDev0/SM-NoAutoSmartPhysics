# SM-NoAutoSmartPhysics

Patch for removing automatic switch to Smart physics when your game is frozen.

This Patch removes the automatic switch to Smart Physics if your game is frozen for too long in Advanced Physics. This removes it basicly letting you be in Advanced Physics no matter what.

Also made by the day of the release of the **0.7.0 Build 711**

## How to Download and Enable

There are 2 ways to enable the NetworkingFix module:

<details>
<summary><small>SM-DLL-Injector</small></summary>

1. Download the latest release of [SM-DLL-Injector](https://github.com/QuestionableM/SM-DLL-Injector/releases/latest) and follow the instructions in the [README](https://github.com/QuestionableM/SM-DLL-Injector#readme).
2. Download the latest release of `SM-NoAutoSmartPhysics.dll` [here](https://github.com/Scrap-Mods/SM-NoAutoSmartPhysics/releases/latest).
3. Move `NoAutoSmartPhysics.dll` to `Steam/steamapps/common/Scrap Mechanic/Release/DLLModules` directory created by the SM-DLL-Injector installer.
4. Launch the game.

</details>

<details>
<summary><small>Manual Injection</small></summary>

1. Download the latest release of `SM-NoAutoSmartPhysics.dll` [here](https://github.com/Scrap-Mods/SM-NoAutoSmartPhysics/releases/latest).
2. Launch the game.
3. Inject `SM-NoAutoSmartPhysics.dll` using a DLL Injector of your choice.

</details>

## Updating the patch for future SM versions

What we do is we replace the 9 bytes in TargetAddress (which is 2 instructions, see commments of OldBytes)
with 4 nop instructions and a jmp instruction to the case when it "fails", This means that the automatic
detection does happen but it basicly changes the if statement check in the comparrasion into `if(false)`
check making this patch work.

To find the address by yourself & apply the patch yourself, Open IDA Pro (or your favourite reverse engineer tool)
and search in strings for `Physics hung!`, There should be only 1 mention at the time writing this, Then
decompile it to psudocode and find the check outside the `Physics hung!` string. In VeraDev's case its
`if(v18 == 8)` (A.K.A: `if(PhysicsQuality == Advanced)`), that would be the address you need. Now simply replace the bytes of that where it always fails
the case no matter what (aka jumps) and that would be it.

## How does this patch work?

Quite simple, just replaces some bytes in game memory and thats it.

In address `0x343EE3`, We replace bytes from this
```
.text:0000000140343EE3 83 FE 08          cmp     esi, 8          - Compares the PhysicsQuality if its 8
.text:0000000140343EE6 0F 85 78 02 00 00 jnz     loc_140344164   - Jump to this loc if it failed.
```

to this.
```
.text:0000000140343EE3 90                nop                     - Do nothing
.text:0000000140343EE4 90                nop                     - Do nothing
.text:0000000140343EE5 90                nop                     - Do nothing
.text:0000000140343EE6 90                nop                     - Do nothing
.text:0000000140343EE6 E9 78 02 00 00    jmp     loc_140344164   - Jump to this loc.
```

Which essentially removes the physics switching entirly by not letting it being called.