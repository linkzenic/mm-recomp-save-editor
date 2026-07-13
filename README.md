# MM Recomp Save Editor

A save editor for Zelda 64: Recompiled.

The editor opens in-game and writes directly to the active save context. It is meant for quick testing, routing, restoration, and experimentation without leaving Zelda 64: Recompiled.

## Features

- Edit name, Bombers code, day, time, time speed, Tatl, intro-complete state, and owl-save state.
- Adjust wallet, held rupees, bank rupees, hearts, health, magic, double defense, Great Spin, and Chateau Romani.
- Set sword, shield, quiver, bomb bag, Deku Stick and Deku Nut upgrades, ammo counts, powder keg, magic beans count, bottle contents, major inventory items, and all masks.
- Toggle Tingle maps, Bombers Notebook, remains, songs, skull tokens, dungeon maps, compasses, boss keys, small keys, and stray fairies.
- Use quick actions for maxing or resetting quest and dungeon progress.

## Usage

1. Install the `.nrm` file in Zelda 64: Recompiled.
2. Enable the mod from the Mods menu.
3. Start or load a save file.
4. Open the editor with the configured hotkey.
5. Make changes and press `Apply`.

Default hotkey:

```text
L + C-Up
```

The hotkey can be changed or disabled from `Mods > Save Editor > Configure`.

The Configure menu also includes a small setup section for applying name, day, Tatl, intro-complete, and owl-save values when a chosen save file opens.

## Notes

- Edits are applied to the currently loaded save. Save normally in-game if you want the changes persisted.
- Some fields affect live game state immediately; others may be easier to verify after changing rooms, saving, or reloading.
- Keep a backup of important saves before testing large changes.

## Building

Initialize submodules first:

```sh
git submodule update --init --recursive
```

Build the mod code:

```sh
make
```

On macOS, Apple Clang cannot build MIPS code. Use an LLVM release with MIPS support and pass `CC`/`LD` explicitly:

```sh
make CC=/path/to/llvm/bin/clang LD=/path/to/llvm/bin/ld.lld
```

Package the mod:

```sh
RecompModTool mod.toml build
```

The packaged mod is written to:

```text
build/mm_recomp_save_editor.nrm
```
