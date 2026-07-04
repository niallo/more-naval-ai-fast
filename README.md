# More Naval AI Fast

This repository is a working optimization fork of More Naval AI v2.9.3u for
Fall from Heaven II / Civilization IV: Beyond the Sword. It is not the upstream
MNAI project README; it is the local engineering workspace used to build,
profile, benchmark, and continue improving a faster `CvGameCoreDLL.dll`.

The workspace is intentionally agent-friendly. The detailed build log, accepted
and rejected optimization history, wrapper setup notes, and continuation plan
live in [AGENTS.md](AGENTS.md). The unattended benchmark harness is documented
in [profiling/autoverify/README.md](profiling/autoverify/README.md).

## Current Result

The current measured target was a fixed AutoVerify save measured over turns
111-113 only. The save itself is local test data and is not included in this
repository.

```text
Baseline batch: profiling/autoverify/baseline-m3-batch-20260703-231157
Baseline median: 848.7 ms/turn
Current best batch: profiling/autoverify/candidate-pathvalid-detail-tinyhelper-batch-20260704-115933
Current best median: 652.3 ms/turn
Delta: -196.3 ms/turn / -23.1%
Current profile DLL SHA256: ed8d967ca7a5a6e6a15b4001a861019a4b13e2c9438217c56518f91dcdf13445
```

Validation gates passed for that batch: three unattended runs completed,
`PythonErr.log` was empty, `CivilizationIV.ini` was restored, transient
`AutoVerify.ini` was removed, no `More Naval AI Fast.app` Wine process
remained, and the original `More Naval AI.app` DLL hash was unchanged.

Important caveat: the current measured best is a profile DLL. It mixes real
gameplay-code optimizations with profiler-overhead reductions. Validate a
release build separately before claiming the full 23.1% as player-visible turn
speedup.

## What Is Optimized

Accepted optimization work in this tree includes:

- `MNAI_PROFILE_TRUE_COMBAT_CACHE`: caches
  `CvPlayerAI::AI_trueCombatValue` by player, unit, and turn.
- Path callback flag hoisting: reads FAStar flags once in hot path callbacks.
- `MNAI_PROFILE_UPGRADE_LIST_CACHE`: uses cached available-upgrade lists for
  normal `CvUnit::getUpgradeCity(bool)` scans.
- `CvUnitAI::AI_anyAttack` filter ordering: checks cheap city/enemy/defender
  conditions before calling the profiled plot-valid helper.
- `CvPlayer::canBuild` tech-first ordering: checks player tech prerequisites
  before expensive plot validity work.
- `CvSelectionGroup` first-unit unrolls in path predicate helpers such as
  `canMoveInto`, `canMoveOrAttackInto`, `canMoveThrough`, `canFight`, and
  `alwaysInvisible`.
- `MNAI_PROFILE_PATHVALID_MOVE_CACHE`: scopes a per-`generatePath` cache for
  `canMoveThrough` and `canMoveOrAttackInto` results used by `pathValid`.
- `MNAI_PROFILE_BASE_BONUS_CACHE`: re-enables the existing per-player
  `AI_baseBonusVal` cache with targeted resource invalidation.
- `MNAI_PROFILE_ACTIVE_DANGER_CACHE_SKIP_CLEAN`: skips clearing
  `isActivePlayerNoDangerCache` on plots already known false.
- `MNAI_PROFILE_SKIP_TINY_HELPERS`: removes profiler markers from very hot
  tiny helpers in the profile build (`AI_plotValid`, `pathCost`, and detailed
  `pathValid` markers).

Rejected candidates and the reasons they were removed are recorded in
[AGENTS.md](AGENTS.md) and the AutoVerify README. Future agents should read
those before retrying a similar idea.

## Repository Layout

```text
CvGameCoreDLL/                  C++ DLL source and Wine build scripts
python/                         Python hook changes, including AutoVerify
scripts/                        unattended run and benchmark helpers
profiling/autoverify/           benchmark batches and summaries
AGENTS.md                       detailed setup, history, and continuation notes
README.md                       this overview
```

## Build Prerequisites

This build uses the Civ4-era Windows toolchain under Wine:

- GOG Civilization IV: The Complete Edition installed in a Porting Kit /
  Wineskin-style wrapper.
- Microsoft Visual C++ Toolkit 2003.
- Windows SDK / Platform SDK headers and libraries.
- Civ4 BTS SDK dependencies: Boost 1.32 and Python 2.4 libraries from the game
  install.
- A Wine build prefix containing the SDK tools at:
  `C:\Program Files\Civ4SDK\Microsoft Visual C++ Toolkit 2003` and
  `C:\Program Files\Civ4SDK\WindowsSDK`.

The exact macOS/Wine/Porting Kit setup used to construct this environment is in
[AGENTS.md](AGENTS.md). Start there if you are rebuilding from a clean Mac.

## Build The Profile DLL

The measured fast target is currently the profile build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-profile-wine.sh
```

Output:

```text
CvGameCoreDLL/Profile/CvGameCoreDLL.dll
```

This target defines `CUSTOM_PROFILER`, `FP_PROFILE_ENABLE`, AutoVerify support,
and the current `MNAI_PROFILE_*` optimization flags in
`CvGameCoreDLL/build-profile-manual.bat`.

## Build The Release DLL

The plain release target can be built with:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-release-wine.sh
```

Output:

```text
CvGameCoreDLL/Release/CvGameCoreDLL.dll
```

Current warning: `build-release-manual.bat` intentionally omits
`CUSTOM_PROFILER`, but it also omits the `MNAI_PROFILE_*` feature flags. That
means the plain release target includes only the unconditional source changes.
One useful future task is to add a separate `ReleaseFast` target that enables
safe gameplay optimization flags while excluding profiler-only markers and
AutoVerify hooks, then benchmark that as the player-facing build.

## Install Into The Fast App

Only install test DLLs into `More Naval AI Fast.app`. Do not overwrite the
original `More Naval AI.app` control wrapper.

```sh
export FAST_ASSETS="$HOME/Applications/More Naval AI Fast.app/Contents/SharedSupport/prefix/drive_c/GOG Games/Civilization IV Complete/Civ4/Beyond the Sword/Mods/More Naval AI/Assets"

cp "$FAST_ASSETS/CvGameCoreDLL.dll" \
  "$FAST_ASSETS/CvGameCoreDLL.dll.before-local-test"

cp "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL/Profile/CvGameCoreDLL.dll" \
  "$FAST_ASSETS/CvGameCoreDLL.dll"
```

For a release test, replace `Profile` with `Release` in the second `cp`.

## Run The Unattended Benchmark

Run the standard three-run benchmark against the fixed save:

```sh
cd "$HOME/Applications/more-naval-ai-fast"

scripts/benchmark_autoverify.sh \
  --label candidate-name \
  --runs 3 \
  --turns 3 \
  --turn-filter 111-113 \
  --compare-json profiling/autoverify/baseline-m3-batch-20260703-231157/benchmark.json
```

The batch writes:

```text
profiling/autoverify/<label>-batch-<timestamp>/BENCHMARK.md
profiling/autoverify/<label>-batch-<timestamp>/benchmark.json
```

A single unattended run is also available:

```sh
scripts/autoverify_mnai.sh
```

## Guidance For Future Agents

Before changing code, read:

```text
AGENTS.md
profiling/autoverify/README.md
```

Use this workflow:

1. Keep `More Naval AI.app` as the untouched control wrapper.
2. Build and install experimental DLLs only into `More Naval AI Fast.app`.
3. Back up the fast app DLL before each installed test.
4. Make one scoped optimization at a time.
5. Run a three-run AutoVerify batch against the same fixed save.
6. Accept a change only when median turn time improves and the intended hotspot
   improves in the profiler, not merely from noise.
7. Record accepted and rejected candidates in `AGENTS.md`.
8. Keep any private reference copy in sync if you maintain one.

Prefer single-threaded repeated-work removal, cache invalidation improvements,
and better benchmark coverage before attempting multi-threading. Civ4 DLL code
relies heavily on shared global state, deterministic random sequencing,
pathfinder state, Python hooks, and save/OOS-sensitive behavior. A future
multi-threaded design should start with read-only snapshot precompute and
main-thread application of results.

## Upstream Context

More Naval AI by Tholal is a continuation of the Fall from Heaven II mod for
Civilization IV. More Naval AI Unofficial is maintained by lfgr.

- MNAI CivFanatics subforum:
  <https://forums.civfanatics.com/forums/more-naval-ai-modmod.476/>
- MNAI-U thread:
  <https://forums.civfanatics.com/threads/mnai-u-unofficial-build-bugfixes.645898/>
