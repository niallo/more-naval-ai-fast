# Agent Notes

This repository is an optimization workspace for More Naval AI v2.9.3u. It is
intended for agents and maintainers to continue improving Civ4 BTS turn
performance while keeping the optimization process reproducible.

## Guardrails

- Keep a clean, playable More Naval AI wrapper as the control build.
- Install experimental DLLs only into a separate fast/test wrapper.
- Back up the test wrapper's `Assets/CvGameCoreDLL.dll` before each install.
- Keep generated DLLs, logs, INI backups, and per-run profiling artifacts out
  of git.
- Make one scoped optimization at a time and record accepted/rejected results.
- Prefer deterministic, single-threaded repeated-work removal before attempting
  multi-threading.

## Current Benchmark Result

The latest local profile benchmark used a fixed local save over turns 111-113.
The save and raw run logs are intentionally not committed.

```text
Baseline median: 848.7 ms/turn
Current best median: 652.3 ms/turn
Delta: -196.3 ms/turn / -23.1%
Current profile DLL SHA256: ed8d967ca7a5a6e6a15b4001a861019a4b13e2c9438217c56518f91dcdf13445
```

Validation gates for that local run:

```text
3-run AutoVerify batch completed
PythonErr.log empty
CivilizationIV.ini restored
transient AutoVerify.ini removed
no test-wrapper Wine processes remained
control wrapper DLL unchanged
```

Important caveat: the measured best is a profile DLL. It mixes gameplay-code
optimizations with profiler-overhead reductions. Validate a release-fast build
before claiming the full benchmark delta as player-visible turn-time speedup.

## Accepted Optimization Stack

1. `MNAI_PROFILE_TRUE_COMBAT_CACHE`
   - Caches `CvPlayerAI::AI_trueCombatValue` by player, unit, and turn.

2. Path callback flag hoisting
   - Reads FAStar flags once in hot path callbacks instead of repeatedly
     calling `GetInfo`.

3. `MNAI_PROFILE_UPGRADE_LIST_CACHE`
   - Uses cached available-upgrade lists for the normal
     `CvUnit::getUpgradeCity(bool)` scan path.

4. `CvUnitAI::AI_anyAttack` filter ordering
   - Checks cheap city/enemy/defender conditions before calling the profiled
     plot-valid helper.

5. `CvPlayer::canBuild` tech-first ordering
   - Checks player tech prerequisites before expensive plot validity work.

6. `CvSelectionGroup` first-unit unrolls
   - Optimizes common single-unit path predicate helpers while preserving
     multi-unit scan order.

7. `MNAI_PROFILE_PATHVALID_MOVE_CACHE`
   - Adds a per-`generatePath` cache for `canMoveThrough` and
     `canMoveOrAttackInto` results used by `pathValid`.

8. `MNAI_PROFILE_BASE_BONUS_CACHE`
   - Re-enables the existing per-player `AI_baseBonusVal` cache with targeted
     invalidation for plot-group bonuses and bonus import/export changes.

9. `MNAI_PROFILE_ACTIVE_DANGER_CACHE_SKIP_CLEAN`
   - Avoids clearing `isActivePlayerNoDangerCache` on plots already known
     false.

10. `MNAI_PROFILE_SKIP_TINY_HELPERS`
    - Removes profiler markers from very hot tiny helper paths in the profile
      build. This is a profile overhead reduction, not a gameplay change.

## Rejected Candidate Themes

Do not retry these without new evidence or a different design:

- Path-failure caching inside `CvSelectionGroup::generatePath`.
- Per-plot `canBuild` cache inside `CvCityAI::AI_bestPlotBuild`.
- Zero-cost build-cost fast path in `CvPlayer::canBuild`.
- Static build lists for `CvCityAI::AI_bestPlotBuild`.
- Route-build list cache for `CvPlayer::getBestRoute`.
- `AI_exploreRange` zero-value prefilter.
- Early land/sea domain reject for the tested `canMoveThrough` path.
- Summon-only defender scan before `AI_anyAttack` pathing.
- Route-territory ownership prefilter.
- `AI_plotValue` final-improvement no-op shortcut.
- Broad base-bonus cache invalidation through `AI_makeAssignWorkDirty` and
  `AI_makeProductionDirty`.

## Build

Profile build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-profile-wine.sh
```

Release build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-release-wine.sh
```

The current plain release target does not enable the `MNAI_PROFILE_*` feature
flags. A useful next task is adding a `ReleaseFast` target that enables safe
gameplay optimization flags while excluding `CUSTOM_PROFILER`, AutoVerify, and
profiler-only marker skips.

## Benchmark

Place a representative test save at:

```text
$HOME/Documents/my games/Beyond The Sword/Saves/single/PROFILE_TEST.CivBeyondSwordSave
```

Then run:

```sh
cd "$HOME/Applications/more-naval-ai-fast"

scripts/benchmark_autoverify.sh \
  --label candidate-name \
  --runs 3 \
  --turns 3 \
  --turn-filter 111-113
```

Use `--save PATH` for a different save. Benchmark output is written under
`profiling/autoverify/`, but per-run artifacts are gitignored by default.

## Next Useful Work

1. Add and validate a `ReleaseFast` target.
2. Benchmark profile and release-fast builds separately.
3. Expand AutoVerify coverage beyond one fixed save.
4. Continue with city-turn and unit-dispatch hotspots:
   `CvCityAI::AI_doTurn`, `AI_assignWorkingPlots`, `AI_addBestCitizen`,
   `AI_plotValue`, `AI_bestPlotBuild`, `CvPlayer::canBuild`,
   `CvUnitAI::AI_anyAttack`, `AI_exploreMove`, and
   `CvSelectionGroup::pushMission`.
5. Treat multi-threading as a later design track. Start only with read-only
   snapshot precompute and main-thread application of results.
