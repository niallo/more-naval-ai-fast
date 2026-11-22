#!/usr/bin/env python3
"""Build the More Naval AI Fast game core and OpenCiv4 together on Apple Silicon."""

import argparse
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import tempfile


REPO = Path(__file__).resolve().parents[1]
PORT = REPO / "scripts/openciv4"
OUTPUT = REPO / "build/openciv4"
ENGINE_REVISION = "b7a98d46663357b0ae7ddc2141b8f184611f8d47"
GAMECORE_REVISION = "7eb1d6be1ba76b795652af2c76e456b6a8183551"


def run(*command, **kwargs):
    return subprocess.run(command, check=True, **kwargs)


def archive(repo, revision, destination):
    try:
        source = subprocess.check_output(["git", "-C", repo, "archive", revision])
    except subprocess.CalledProcessError as error:
        raise ValueError(f"Revision {revision} is unavailable in {repo}. See doc/openciv4.md.") from error
    destination.mkdir(parents=True, exist_ok=True)
    run("tar", "-xf", "-", "-C", destination, input=source)


def apply_patch(tree, name):
    run("patch", "-p1", "-F", "0", "-t", "-i", PORT / name, cwd=tree)


def sync(source, destination, *options):
    destination.mkdir(parents=True, exist_ok=True)
    run("rsync", "-a", *options, f"{source}/", f"{destination}/")


def stage_sources(checkout):
    with tempfile.TemporaryDirectory(prefix="stage-", dir=OUTPUT) as directory:
        stage = Path(directory)
        engine = stage / "engine"
        archive(checkout, ENGINE_REVISION, engine)
        archive(checkout / "gamecore", GAMECORE_REVISION, engine / "gamecore")
        apply_patch(engine, "engine.patch")

        # Patch LF copies of the live sources. This includes local edits and newly added
        # translation units while leaving every Windows source file in the checkout alone.
        gamecore = engine / "CvGameCoreDLL"
        gamecore.mkdir()
        for source in (REPO / "CvGameCoreDLL").iterdir():
            if source.is_file() and source.suffix.lower() in {".cpp", ".h", ".hpp", ".inc", ".inl"}:
                gamecore.joinpath(source.name).write_bytes(source.read_bytes().replace(b"\r\n", b"\n"))
        apply_patch(engine, "gamecore.patch")

        (engine / "external").symlink_to(checkout / "external", target_is_directory=True)
        sync(engine, OUTPUT / "source", "--checksum", "--delete")


def release_defines():
    batch = (REPO / "CvGameCoreDLL/build-releasefast-manual.bat").read_bytes()
    match = re.search(rb"(?m)^set DEFINES=(.+)", batch)
    if not match:
        raise ValueError("Could not find the ReleaseFast compiler definitions.")
    flags = re.findall(rb"/D(LOG_AI|MNAI_[A-Z0-9_]+)\b", match[1])
    return ";".join(flag.decode("ascii") for flag in flags)


def build(checkout, python_dir, ffmpeg_dir):
    ninja = shutil.which("ninja") or checkout / "external/ninja/bin/ninja"
    if not Path(ninja).is_file():
        raise ValueError("Ninja is required. Install it with brew install ninja.")
    sdk = subprocess.check_output(["xcrun", "--sdk", "macosx", "--show-sdk-path"], text=True).strip()
    source = OUTPUT / "source"
    build_dir = OUTPUT / "native"
    options = [
        f"-DCMAKE_MAKE_PROGRAM={ninja}",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_OSX_ARCHITECTURES=arm64",
        f"-DCMAKE_OSX_SYSROOT={sdk}",
        "-DOC4_LTO=OFF",
        f"-DOC4_MNAI_SOURCE_DIR={source / 'CvGameCoreDLL'}",
        f"-DOC4_MNAI_DEFINITIONS={release_defines()}",
    ]
    if python_dir:
        options.append(f"-DOC4_PYTHON27_DIR={python_dir}")
    if ffmpeg_dir:
        options.append(f"-DOC4_FFMPEG_DIR={ffmpeg_dir}")

    run("cmake", "-S", source, "-B", build_dir, "-G", "Ninja", *options)
    parallel = os.environ.get("CMAKE_BUILD_PARALLEL_LEVEL") or str(os.cpu_count() or 1)
    run("cmake", "--build", build_dir, "--parallel", parallel)


def stage_mod(source):
    mod = OUTPUT / "More Naval AI"
    excludes = ("--exclude=*.dll*", "--exclude=*.exe", "--exclude=*.pyc", "--exclude=*.pyo", "--exclude=*.bak")
    sync(source, mod, "--delete", *excludes)
    for layer in ("Art", "Config", "XML", "python"):
        sync(REPO / layer, mod / "Assets" / layer, *excludes)
    sync(REPO / "PrivateMaps", mod / "PrivateMaps", *excludes)
    shutil.copy2(REPO / "More Naval AI.ini", mod / "More Naval AI.ini")
    shutil.copy2(PORT / "CvAltRoot.py", mod / "Assets/python/CvAltRoot.py")

    profile = OUTPUT / "Documents/My Games/Beyond the Sword"
    for folder in ("single/auto", "multi", "WorldBuilder"):
        (profile / "Saves" / folder).mkdir(parents=True, exist_ok=True)
    ini = profile / "CivilizationIV.ini"
    if not ini.exists():
        ini.write_text("[CONFIG]\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--openciv4", type=Path, required=True, help="OpenCiv4 checkout with dependencies")
    parser.add_argument("--python27-dir", type=Path, help="Python 2.7 shared-library prefix")
    parser.add_argument("--ffmpeg-dir", type=Path, help="optional FFmpeg prefix for movies")
    parser.add_argument("--mod-from", type=Path, help="installed More Naval AI folder containing Assets")
    args = parser.parse_args()

    if (platform.system(), platform.machine()) != ("Darwin", "arm64"):
        parser.error("Run this build on Apple Silicon in a native arm64 shell.")

    checkout = args.openciv4.expanduser().resolve()
    if not (checkout / "external").is_dir():
        parser.error(f"OpenCiv4 dependencies are missing under {checkout / 'external'}. See doc/openciv4.md.")
    if args.mod_from:
        mod_from = args.mod_from.expanduser().resolve()
        if not (mod_from / "Assets/XML").is_dir() or mod_from.is_relative_to(OUTPUT):
            parser.error("--mod-from must be an installed mod folder with Assets/XML outside the build output.")

    OUTPUT.mkdir(parents=True, exist_ok=True)
    try:
        stage_sources(checkout)
        build(
            checkout,
            args.python27_dir.expanduser().resolve() if args.python27_dir else None,
            args.ffmpeg_dir.expanduser().resolve() if args.ffmpeg_dir else None,
        )
        if args.mod_from:
            stage_mod(mod_from)
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"OpenCiv4 build failed: {error}\n")

    print(f"Native executable: {OUTPUT / 'native/bin/oc4core'}")
    if args.mod_from:
        print(f"Mod data: {OUTPUT / 'More Naval AI'}")


if __name__ == "__main__":
    main()
