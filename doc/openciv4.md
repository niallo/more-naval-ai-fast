# Build More Naval AI Fast with OpenCiv4 on Apple Silicon

The native build links OpenCiv4 with this fork's game core. The engine and game
core both target arm64 and use the definitions from the fork's `ReleaseFast`
build. OpenCiv4 reads the installed Beyond the Sword 3.19 and More Naval AI
2.9.3u / Fall from Heaven II assets.

## Requirements

- An Apple Silicon Mac, Xcode or its Command Line Tools, CMake 3.28 or newer,
  Ninja, Git, and Python 3.9 or newer.
- A complete Beyond the Sword 3.19 installation. Its root contains `Assets`,
  `Warlords`, and `Beyond the Sword`.
- The installed More Naval AI mod folder with its full `Assets` directory.
  The fork contains only some of the art and other game data.
- An OpenCiv4 checkout with its `gamecore` submodule and macOS build dependencies.

The script reads OpenCiv4 commit `b7a98d46663357b0ae7ddc2141b8f184611f8d47` and
gamecore commit `7eb1d6be1ba76b795652af2c76e456b6a8183551` from local Git
objects. If you need a checkout, clone it and initialize the submodule from the
pinned revision.

```sh
git clone https://github.com/bssth/openciv4.git ../openciv4
git -C ../openciv4 checkout --detach b7a98d46663357b0ae7ddc2141b8f184611f8d47
git -C ../openciv4 submodule update --init
```

Follow [Building (macOS, Apple Silicon) in the OpenCiv4 README](https://github.com/bssth/openciv4/blob/b7a98d46663357b0ae7ddc2141b8f184611f8d47/README.md#building-macos-apple-silicon)
to populate its `external/` directory. That includes SDL3, FreeType, zlib,
DirectXMath, dr_libs, pybind11 2.9.2, and a native Python 2.7.18 shared library.
The build defaults to `external/python27`. When Python 2.7 is installed
elsewhere, pass its prefix with `--python27-dir`. FFmpeg is optional and can
be supplied with `--ffmpeg-dir "$(brew --prefix ffmpeg)"` for movie playback.

## Build

From the More Naval AI Fast repository, set `CIV4_ROOT` to your installation
and `MOD_SOURCE` to the installed mod folder. A Wine app wrapper can provide
both paths from its existing game data.

```sh
export CIV4_ROOT="/path/to/Civilization IV Complete/Civ4"
export MOD_SOURCE="$CIV4_ROOT/Beyond the Sword/Mods/More Naval AI"

python3 scripts/build_openciv4.py \
  --openciv4 ../openciv4 \
  --mod-from "$MOD_SOURCE"
```

With a pyenv Python 2.7 installation, add
`--python27-dir "$HOME/.pyenv/versions/2.7.18"`. The script stages the engine
and the fork's current C++ sources under `build/openciv4/source`, applies the
patches from `scripts/openciv4`, and compiles them against the same headers.
The resulting executable is `build/openciv4/native/bin/oc4core`.

`--mod-from` copies the complete mod into `build/openciv4/More Naval AI` and
layers this checkout's art, XML, Python, configuration, and maps on top.
Keep the folder name `More Naval AI` for saves that record that mod name.
The copied assets are regenerated on subsequent runs with `--mod-from`.
Native saves and BUG settings go in `build/openciv4/Documents` when using the
launch commands below. The installed game and both source checkouts are read
in place and the build writes its files under `build/openciv4`.

Omit `--mod-from` to compile the executable only.

## Run and verify

```sh
export OC4_DOCUMENTS="$PWD/build/openciv4/Documents"
engine="$PWD/build/openciv4/native/bin/oc4core"
mod="$PWD/build/openciv4/More Naval AI"

file "$engine" build/openciv4/native/gamecore/libCvGameCoreDLL-arm64.dylib
ctest --test-dir build/openciv4/native --output-on-failure
"$engine" --civ4 "$CIV4_ROOT" --mod "$mod" --menu
```

A headless test can generate an Erebus map, run a game, and verify save loading.

```sh
save="$PWD/build/openciv4/smoke.CivBeyondSwordSave"
"$engine" --civ4 "$CIV4_ROOT" --mod "$mod" \
  --map Erebus --size SMALL --players 4 --seed 7 --turns 30 --save "$save"
"$engine" --civ4 "$CIV4_ROOT" --mod "$mod" --load "$save" --turns 1
```

Use `--play --load "/path/to/game.CivBeyondSwordSave"` to try a More Naval AI
save from the original game. The native build has been tested with a
More Naval AI 2.9.3u save from the Wine installation.

## Maintaining the port

`gamecore.patch` carries the C++ portability changes applied to the staged
fork. `engine.patch` carries the engine changes for mod headers, mod maps,
the native plot-overlay API, and legacy save trailers. Fixes shared with the
Wine build can go directly into the fork when the legacy compiler supports
them. If an edit overlaps a native patch, refresh that hunk against the
updated source and rerun the native smoke test. Updating OpenCiv4 requires
rebasing the engine patch and updating both pinned revisions in the build script.
