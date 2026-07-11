# More Naval AI Fast

> **Civ IV AI turns: 848.7 → 163.3 ms. Same tested game state, 80.8% less waiting.**

This repository is a working optimization fork of More Naval AI v2.9.3u for
Fall from Heaven II / Civilization IV: Beyond the Sword. It is not the upstream
MNAI project README; it is the local engineering workspace used to build,
profile, benchmark, and continue improving a faster `CvGameCoreDLL.dll`.

**Jump to:** [current performance](#current-state) ·
[accepted optimizations](#what-is-optimized) · [builds](#build-releasefast) ·
[benchmarking](#run-the-unattended-benchmark) ·
[contributor workflow](#guidance-for-future-agents)

## Current State

The player-facing `ReleaseFast` build is installed and playable. It contains
the full accepted optimization stack, but none of the AutoVerify, custom
profiler, release-trace, scheduler-trace, or fingerprint hooks used to measure
it.

```text
Original   ████████████████████  848.7 ms/turn
Current    ████                  163.3 ms/turn
```

| Fixed-save measurement | Before | Current | Result |
| --- | ---: | ---: | ---: |
| No-profiler AI turn | 848.7 ms | **163.3 ms** | **-80.8%** |
| Release-trace turn | 582.0 ms | **170.7 ms** | **-70.7%** |
| Idle engine callback gap | 412.8 ms | **0 ms** | Eliminated |

Three independent no-profiler runs measured **163.3, 162.3, and 172.0
ms/turn**. Three trace runs reproduced the flag-off baseline's deterministic
state hashes and city/unit/group counts over every measured turn. No AI score,
search-depth, RNG-order, or path approximation was traded for speed.

### What is running now

- `MNAI_OPT_CONTIGUOUS_TURN_UPDATES` removes safe-to-skip engine round trips
  while still yielding for missions, combat, human input, multiplayer,
  WorldBuilder, game over, and a fixed safety cap.
- The original `More Naval AI.app` remains the untouched control; experiments
  and the playable optimized DLL live only in `More Naval AI Fast.app`.
- Numeric roadmap Gates A-D—400, 300, 212.2, and 169.7 ms/turn—are complete.
  The next stretch target is **127.3 ms/turn**.

### Where the remaining time hides

`ReleaseFastTrace` accounts for about 168.5 ms/turn of current CPU work:

| Exclusive phase | ms/turn |
| --- | ---: |
| Global end-turn work | 48.1 |
| Player unit turns | 19.3 |
| Other player-turn work | 14.5 |
| Path generation | 13.1 |
| City build planning | 13.0 |
| City turns | 11.7 |

That makes Gate E a CPU-work problem, not a callback-wait problem. The next
useful candidates should attack these shared services while preserving the
guards and exact behavior that made the large scheduler win safe.

> **Benchmark scope:** current validation is strongest on the private primary
> save over turns 111-113. The midgame, late-large, and early-pathing matrix
> predates contiguous turn updates and needs a fresh run before its numbers are
> treated as current.

<details>
<summary>Build fingerprints and benchmark evidence</summary>

- Accepted batch:
  `candidate-contiguous-turn-updates-releaseverify-batch-20260710-211556`
- Player `ReleaseFast` DLL SHA256:
  `9cbfb4cc162bd145d1533e1c63fd17ace94524cd11a3e0a090099eaf6ad82e1d`
- Wrapper `CvGameUtils.py` SHA256:
  `587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c`
- Control DLL SHA256:
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`

Equivalent VC++ 2003/Wine builds are not byte-stable. Batch paths and source
flags—not reproducible DLL hashes—are the authoritative performance evidence.

</details>

## What Is Optimized

All player-facing optimizations are exact and deterministic: they remove
repeated work or idle scheduling without reducing search depth, changing AI
scores, or approximating paths. Benefits below use the fixed private save over
turns 111-113. Paired `ReleaseFastVerify` medians are shown where an isolated
candidate/control measurement exists; profiler numbers explain the targeted
work removed. These gains are not additive because the batches were measured
at different points in the accepted stack.

### Turn scheduling

| Optimization | What it changes | Measured benefit |
| --- | --- | --- |
| `MNAI_OPT_CONTIGUOUS_TURN_UPDATES` | Runs the next complete logical update slice immediately when single-player AI can progress and no mission, combat, or human input requires an engine frame. It yields for busy state, multiplayer, WorldBuilder, game over, and a 256-slice safety cap. | Matching release trace fell **582.0 → 170.7 ms/turn** (-70.7%), with callback-gap time **412.8 → 0 ms/turn** and identical state hashes. The no-profiler median is **163.3 ms/turn**, an 80.8% reduction from the original 848.7 ms baseline. |

### City, economy, and player evaluation

| Optimization | What it changes | Measured benefit |
| --- | --- | --- |
| `MNAI_PROFILE_TRUE_COMBAT_CACHE` | Caches `AI_trueCombatValue` by player, unit, and turn instead of recomputing the same combat valuation for multiple AI consumers. | Removes repeated combat-value scans from the initial accepted stack; its isolated release delta was not retained. |
| `MNAI_PROFILE_UPGRADE_LIST_CACHE` | Reuses the exact available-upgrade list during normal `CvUnit::getUpgradeCity(bool)` city scans. | Avoids rebuilding upgrade candidates for every city examined; accepted as part of the initial stack without an isolated release delta. |
| `CvPlayer::canBuild` tech-first ordering | Tests player technology prerequisites before expensive plot legality. | Later attribution showed **41,130 of 55,521 calls (74.1%)** reject at this first tech check, avoiding plot work for most requests. |
| `MNAI_PROFILE_BASE_BONUS_CACHE` | Re-enables per-player `AI_baseBonusVal` reuse with targeted invalidation for plot-group, import, and export changes. | Converts repeated strategic bonus valuation into a lookup until relevant resource state changes; isolated timing was not retained. |
| `MNAI_OPT_CITY_YIELD_DELTA_CONTEXT` | Snapshots stable city yield rates, yield modifiers, and the active production modifier once per citizen scan, while recomputing state-sensitive citizen choices normally. | Release median **593.0 → 570.7 ms/turn** (-22.3 ms, -3.8%). `AI_yieldValue::yield_delta` fell **201 → 42 ms** (-79.1%) in the median profile. |
| `MNAI_OPT_NOBONUS_VOTE_SOURCE_FIRST` | Checks the side-effect-free vote-source no-bonus table before the expensive `isFullMember` test. | Release median **570.7 → 567.0 ms/turn** (-3.7 ms). `isFullMember` fell from **638,638 calls / 49 ms to 4,774 calls / ~0 ms**. |
| `MNAI_OPT_TOWER_MANA_OWNED_PLOT_SERVICE` | Keeps Tower Mastery decisions in Python but counts relevant mana only over the shared, ordered owned-plot service. | Paired release median **574.7 → 570.0 ms/turn** (-4.7 ms, -0.8%). The Tower Mastery profile band fell **67 → 0 ms** over 24 calls. |

### Pathing, tactical AI, and workers

| Optimization | What it changes | Measured benefit |
| --- | --- | --- |
| Path callback flag hoisting | Reads FAStar flags once per hot callback instead of repeatedly calling `GetInfo`. | Removes repeated pathfinder metadata calls from every visited node; accepted in the initial stack without an isolated release delta. |
| `CvUnitAI::AI_anyAttack` filter ordering | Runs cheap city, enemy, and defender tests before the more expensive plot-valid helper. | Prevents legality/path work for trivially invalid attack plots; isolated timing was not retained. |
| `CvSelectionGroup` first-unit unrolls | Uses direct first-unit checks for the common single-unit group in `canMoveInto`, `canMoveThrough`, `canFight`, and related predicates while retaining original multi-unit order. | Avoids linked-list traversal across high-volume path predicates; accepted in the initial stack without an isolated release delta. |
| `MNAI_PROFILE_PATHVALID_MOVE_CACHE` | Caches exact `canMoveThrough` and `canMoveOrAttackInto` answers for one `generatePath` request. | Replaces repeated movement-legality work for the same path node with a scoped lookup; isolated timing was not retained. |
| `MNAI_PROFILE_ACTIVE_DANGER_CACHE_SKIP_CLEAN` | Does not clear `isActivePlayerNoDangerCache` entries already known false. | Avoids redundant map writes during danger-cache invalidation; accepted in the initial stack without an isolated release delta. |
| `MNAI_OPT_ATTACK_ODDS_BEFORE_PATH` | Applies the existing exact attack-odds threshold before pathfinding low-odds city/any-attack candidates. | Release median **567.0 → 563.7 ms/turn**. `generatePath` fell **204 → 161 ms** and **3,107 → 2,796 calls**; `AI_anyAttack` requests fell **298 → 46**. |
| `MNAI_OPT_PATHCOST_SINGLE_UNIT_NEXT_SKIP` | Skips `nextUnitNode` in `pathCost` when a group contains one unit. | Release median **563.7 → 559.0 ms/turn** (-4.7 ms, -0.8%) with unchanged path decisions and multi-unit behavior. |
| `MNAI_OPT_PATHVALID_STICKY_UPDATE_CACHE` | Reuses exact path-valid movement answers across repeated searches within one unchanged `CvUnitAI::AI_update`; a group-state signature forces refresh after mutation. | Release median **559.0 → 551.0 ms/turn** (-8.0 ms, -1.4%). `canMoveInto` calls fell **30,457 → 29,371**. |
| `MNAI_OPT_ROUTE_TERRITORY_OWNED_PLOT_CACHE` | Scans an invalidated, map-index-ordered owned-plot list in `AI_routeTerritory` instead of the whole map. | Release median **551.0 → 546.0 ms/turn**. `AI_routeTerritory` fell **46 → 1 ms** and plot-valid visits **133,120 → 2,842** (-97.9%). |
| `MNAI_OPT_PILLAGE_VALUE_BEFORE_PATH` | Computes the exact pillage-score upper bound before pathfinding and skips only candidates that cannot beat the current winner. | Release median **570.0 → 564.7 ms/turn** (-5.3 ms, -0.9%). Caller path requests fell **98 → 83**. |
| `MNAI_OPT_PATHVALID_MOVE_BEFORE_DANGER` | Tests cached movement legality before danger/invisibility; both exact predicates must still pass. | Paired release median **570.7 → 565.7 ms/turn** (-5.0 ms, -0.9%). Danger checks fell **168,472 → 112,920** (-33.0%); `generatePath` fell **164 → 149 ms**. |
| `MNAI_OPT_WORKER_OWNED_PLOT_SERVICE` | Reuses the ordered owned-plot service in `AI_irrigateTerritory` and `AI_connectBonus`, preserving original candidate and tie order. | Paired release median improved **567.3 → 567.0 ms/turn**. Profiled `AI_plotValid` calls fell **384,830 → 254,552** (-33.9%); both targeted 9 ms scans rounded to zero. |
| `MNAI_OPT_IMPROVE_BONUS_OWNED_PLOT_SERVICE` | Uses owned plots plus the one permitted current unowned plot for the restricted `AI_improveBonus` mode; unrestricted outside-border behavior keeps the full scan. | Paired release median **568.0 → 567.0 ms/turn**. Plot-filter visits fell **74,240 → 1,710** (-97.7%) and `AI_improveBonus` fell **16 → 5 ms**. |

The first nine gameplay changes were originally validated as one cumulative
profile stack rather than as clean paired release candidates. Together with a
profile-only tiny-marker reduction, that early work moved the measured profile
median from 848.7 to 652.3 ms/turn (-23.1%); that aggregate should not be
assigned to any one early row.

### Diagnostic-only improvements

- `MNAI_PROFILE_SKIP_TINY_HELPERS` removes profiler markers from extremely hot
  tiny helpers in the legacy profile build. It reduces measurement overhead;
  it is not enabled in the player DLL and is not a gameplay optimization.
- `ReleaseFastTrace` adds fixed enum-indexed exclusive phase timing, scheduler
  snapshots, and deterministic state fingerprints without the custom
  profiler's map-based registration overhead.
- `ProfileFast` retains detailed path, city, worker, explore, unit-dispatch,
  player-turn, and city-turn attribution markers for selecting future work.

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

The legacy measured target is the profile build:

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

## Build ProfileFast

`ProfileFast` is the preferred profiling target for new optimization work. It
uses the same gameplay optimization flags as `ReleaseFast`, plus profiler and
AutoVerify hooks. It intentionally does not define
`MNAI_PROFILE_SKIP_TINY_HELPERS`, so profiler-overhead reductions are not mixed
into gameplay-speed measurements.

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-profilefast-wine.sh
```

Output:

```text
CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll
```

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

## Build ReleaseFast

`ReleaseFast` is the preferred player-facing target for optimization results.
It enables the safe gameplay/cache optimization flags while excluding
`CUSTOM_PROFILER`, AutoVerify hooks, profiler detail markers, and
profiler-only tiny-helper marker skips.

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-releasefast-wine.sh
```

Output:

```text
CvGameCoreDLL/ReleaseFast/CvGameCoreDLL.dll
```

## Build ReleaseFastVerify

`ReleaseFastVerify` is the unattended benchmark target for release-speed
measurement. It is the same no-profiler code path as `ReleaseFast`, plus only
the test-only `MNAI_AUTOVERIFY_AUTO_END_TURN` hook needed to advance the fixed
AutoVerify save without human input and a one-row-per-turn
`autoverify_dll.csv` marker for no-profiler timing.

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-releasefastverify-wine.sh
```

Output:

```text
CvGameCoreDLL/ReleaseFastVerify/CvGameCoreDLL.dll
```

## Install Into The Fast App

Only install test DLLs into `More Naval AI Fast.app`. Do not overwrite the
original `More Naval AI.app` control wrapper.

```sh
export FAST_ASSETS="$HOME/Applications/More Naval AI Fast.app/Contents/SharedSupport/prefix/drive_c/GOG Games/Civilization IV Complete/Civ4/Beyond the Sword/Mods/More Naval AI/Assets"

cp "$FAST_ASSETS/CvGameCoreDLL.dll" \
  "$FAST_ASSETS/CvGameCoreDLL.dll.before-local-test"

cp "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll" \
  "$FAST_ASSETS/CvGameCoreDLL.dll"
```

For a release test, replace `ProfileFast` with `ReleaseFast` in the second
`cp`. For unattended release timing, use `ReleaseFastVerify`.

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

The numeric 400, 300, 212.2, and 169.7 ms/turn gates are complete. The open
stretch target is 127.3 ms/turn. First rebaseline the secondary-save matrix
with the contiguous-update build; then use the fixed-cost trace to select exact
CPU-side pathing, city/worker, and player-turn services. See the roadmap and
rejected-candidate ledger in [AGENTS.md](AGENTS.md).

## Upstream Context

More Naval AI by Tholal is a continuation of the Fall from Heaven II mod for
Civilization IV. More Naval AI Unofficial is maintained by lfgr.

- MNAI CivFanatics subforum:
  <https://forums.civfanatics.com/forums/more-naval-ai-modmod.476/>
- MNAI-U thread:
  <https://forums.civfanatics.com/threads/mnai-u-unofficial-build-bugfixes.645898/>

## Historical Local Optimization Cycle (Pre-Scheduler)

This chronology records the final experiments before
`MNAI_OPT_CONTIGUOUS_TURN_UPDATES` was accepted. References below to the
"latest" wrapper or its DLL hash describe the state at that stage; the
authoritative installed build and performance are in **Current State** above.

<details>
<summary>Open the pre-scheduler experiment log</summary>

An earlier retained source change is ProfileFast-only attribution for
`AI_ConquestMove` and `AI_pickTargetCity`. The fixed turns 111-113 profile put
15 ms of the conquest routine in target-city selection and 4 ms in its late
fallback; a follow-up counted 52 target-city path attempts, 44 successes, and
no new-war candidates per run. These hooks compile out of `ReleaseFast` and
`ReleaseFastVerify`.

An earlier exact shared static-plot-input service for citizen assignment was
tested and rejected. Its release-path median was 569.0 ms/turn versus a paired
flag-off 567.0 ms/turn control, so the source and build flag were removed. At
that point the player wrapper was restored to hook-free ReleaseFast SHA256
`afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.
See `profiling/autoverify/README.md` and `AGENTS.md` for exact batch paths,
hashes, and validation evidence.

A subsequent ProfileFast pass timed the accepted sticky path-valid cache's
full group-state signature: 3,217 hashes per run rounded to 0 ms total, versus
115-117 ms in the 3,271 path requests themselves. The marker is retained, but
no release mutation-generation replacement was attempted.

The preceding accepted gameplay optimization was
`MNAI_OPT_WORKER_OWNED_PLOT_SERVICE`. It reuses the existing ordered owned-plot
service for the owner-only `AI_irrigateTerritory` and `AI_connectBonus` scans.
The paired no-profiler median moved 567.3→567.0 ms/turn, while ProfileFast moved
both helpers from 9 ms to 0, worker dispatch from 70 ms to 51 ms, and removed
130,278 `AI_plotValid` calls. The playable wrapper now contains hook-free
ReleaseFast SHA256
`302d0f39dc162ec5b5c37443c7cd0f6c697a6f4327ab891fe98dcca23d302c59`.

ProfileFast also now emits `path_request_cross_context.csv`. The first batch
found stable turn-level overlap between explorer range/full searches and
between group attack/path movement, but those counts are deliberately treated
as upper bounds until a narrower sequence and complete movement-state key can
preserve mutable FAStar behavior.

The owned-plot worker service now also covers `AI_improveBonus` through
`MNAI_OPT_IMPROVE_BONUS_OWNED_PLOT_SERVICE`. The implementation preserves the
Advanced Tactics outside-border mode and original map-index order, including
the one allowed unowned current plot in inside/current mode. Its paired release
median moved 568.0→567.0 ms/turn; ProfileFast reduced `AI_improveBonus`
16→5 ms and plot-filter visits 74,240→1,710. The playable wrapper now contains
hook-free ReleaseFast SHA256
`898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

The latest leaf attribution found that 17,183 of 17,933 turn-duplicate
`CvPlayer::canBuild` identities (95.8%) occur through
`CvPlayer::getBestRoute`. A new plot-scoped service computed one route answer
for an entire `AI_bestPlotBuild`, rather than once per candidate improvement.
ProfileFast confirmed the intended reduction: `getBestRoute` fell
27,655→22,526 calls, `canBuild` fell 55,521→52,709 calls, and the profile
median moved 606.3→593.7 ms/turn. The no-profiler result did not hold:
`candidate-best-plot-route-service-releaseverify-batch-20260710-074054`
measured 571.7 ms/turn versus 567.7 for paired flag-off
`clean-best-plot-route-service-control-releaseverify-batch-20260710-074706`.
The candidate was rejected and completely removed. The retained caller ledger
is ProfileFast-only; the freshly rebuilt playable wrapper is hook-free
ReleaseFast SHA256
`747e84bf21525cb3083ebf65dd83c5e9b0c84d51b9aeec74ddc7d1ba0d18a501`.

ProfileFast now also records exact overlap between the failed range-3 explorer
search and the subsequent full search. Batch
`profilefast-explore-sequence-overlap-batch-20260710-061846` found 6,942
revisited plots per run, but only 1,464 passed plot validity and 362 repeated
adjacent scoring; both reusable overlap bands rounded to 0 ms. No release cache
was built because the measured ceiling is negligible and path state is mutable
across the intervening pillage search. The wrapper remains on the hook-free
ReleaseFast hash above.

A broader ProfileFast `canBuild` ledger found 17,933 repeated player/plot/build
request identities among 55,521 calls per run. Caller refinement assigned
3,925 to `AI_bestPlotBuild`, 130 to unit AI, and 13,878 to other callers. A
lazy assignment-scoped best-route service was then tested: its paired release
median was 569.0 versus 571.0 ms/turn control, but ProfileFast showed no
movement in `getBestRoute`, `canBuild`, assignment, or plot scoring. The small
wall-clock edge was rejected, the service was removed, and the duplicate ledger
was retained to guide a correctly scoped shared legality service.

The next exact owned-list extension, for `AI_fortTerritory` when Advanced
Tactics is disabled, was rejected. Attribution found 81,920 plot-valid checks
for only 656 owned eligible plots, but the paired release median regressed from
564.0 to 568.7 ms/turn. The candidate was removed, its ProfileFast-only mode
and ownership counters were retained, and the playable wrapper was restored to
the ReleaseFast hash above.

The latest ProfileFast attribution split the current plot's yield calculations
inside `AI_getImprovementValue`. Batch
`profilefast-current-yield-attribution-batch-20260710-060653` found 2,196
current-nature and 948 current-improvement calculation scopes per run, but both
rounded to 0 ms; the full yield-difference band was 12 ms while the enclosing
improvement loop remained 70-73 ms. A release context was therefore not built.
The markers remain for future comparisons, and the player wrapper is still on
hook-free ReleaseFast SHA256
`898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

</details>

## Benchmark History

The primary AutoVerify benchmark uses a fixed local save measured over turns
111-113 only. The save itself is local test data and is not included in this
repository.

<details>
<summary>Open the full benchmark chronology</summary>

```text
Baseline batch: profiling/autoverify/baseline-m3-batch-20260703-231157
Baseline median: 848.7 ms/turn
Best recorded no-profiler batch: profiling/autoverify/candidate-contiguous-turn-updates-releaseverify-batch-20260710-211556
Best recorded no-profiler median: 163.3 ms/turn
ReleaseFastVerify runs: 163.3, 162.3, 172.0 ms/turn
Delta from original baseline: -685.4 ms/turn / -80.8%
Matching ReleaseFastTrace baseline: 582.0 ms/turn
Matching ReleaseFastTrace candidate: 170.7 ms/turn
Matching trace delta: -411.3 ms/turn / -70.7%
Installed player ReleaseFast DLL SHA256: 9cbfb4cc162bd145d1533e1c63fd17ace94524cd11a3e0a090099eaf6ad82e1d
Installed fast-wrapper CvGameUtils.py SHA256: 587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c
Note: equivalent VC++ 2003/Wine rebuilds are not byte-stable; use benchmark
batch paths and source flags as the performance evidence.
```

Validation gates passed for the accepted trace and release batches: three unattended runs completed,
`PythonErr.log` was empty, `CivilizationIV.ini` was restored, transient
`AutoVerify.ini` was removed, no `More Naval AI Fast.app` Wine process
remained, and the original `More Naval AI.app` DLL hash was unchanged.

The largest accepted improvement is `MNAI_OPT_CONTIGUOUS_TURN_UPDATES`. A
fixed-cost release trace showed that 412.8 ms/turn—71% of the measured
window—was idle time between engine callbacks even though no group was busy,
in combat, or waiting on a mission timer. The optimization executes the next
unchanged logical update slice immediately in single-player when there is no
engine work or human input to wait for. It retains the original event/update
order and yields on busy state, human input, multiplayer, WorldBuilder, and a
fixed safety cap. Three candidate trace runs matched the baseline state hashes
on every measured turn while callback-gap time fell to zero. The result clears
the roadmap's 169.7 ms Gate D without AI-quality shortcuts or parallelism.

An earlier accepted optimization, `MNAI_OPT_TOWER_MANA_OWNED_PLOT_SERVICE`, keeps
the Tower Mastery state machine in Python but replaces its per-player whole-map
mana scan with an exact bonus-class count over the accepted runtime owned-plot
list. The candidate batch above improved 574.7 to 570.0 ms/turn in a paired
same-session comparison. Its ProfileFast batch,
`profiling/autoverify/profilefast-tower-mana-owned-plot-service-batch-20260709-233312`,
measured 589.3 ms/turn versus the prior accepted 609.7 ms/turn attribution
batch, while `CvPlayer::doTurn::tower_mastery` fell from 67 ms / 24 calls to
0 ms / 24 calls. This is a different design from the previously rejected full
C++ port and Python XML-ID cache.

A subsequent accepted `MNAI_OPT_PILLAGE_VALUE_BEFORE_PATH` candidate checks
an exact pillage-score upper bound before pathfinding in `AI_pillageRange`.
The release median improved from 570.0 to 564.7 ms/turn, and ProfileFast path
requests in that caller fell from 98 to 83 per run while preserving original
integer division and tie behavior.

The later accepted `MNAI_OPT_PATHVALID_MOVE_BEFORE_DANGER` candidate runs
cached movement legality before danger checks in `pathValid`. ProfileFast
attribution found 55,440 movement rejects versus only 1,528 danger rejects.
The release candidate measured 565.7 ms/turn against a same-session clean
570.7 ms/turn control (-5.0 ms / -0.9%); ProfileFast then reduced the danger
band from 168,472 to 112,920 calls and `generatePath` from 164 to 149 ms.
The historical pre-scheduler absolute best was 546.0 ms/turn; the accepted
contiguous-update result above supersedes it at 163.3 ms/turn.

Recent attribution/rejection: the ProfileFast city-growth sub-band batch
`profiling/autoverify/profilefast-city-growth-subband-batch-20260710-001733`
showed the measurable 8-9 ms growth-capacity band is almost entirely the
already-known 7 ms good-tile/specialist scan. A subsequent exact target-city
adjacent-score-before-path candidate measured 565.3 ms/turn versus the accepted
564.7 ms/turn and was removed; see
`candidate-target-city-adjacent-score-before-path-releaseverify-batch-20260710-002629`.
An exact follow-up that hoisted unit-stable team, owner, group, and coordinate
reads out of `AI_explore` / `AI_exploreRange` loops was also rejected: batch
`candidate-explore-context-hoist-releaseverify-batch-20260710-004507`
measured 566.3 ms/turn versus 564.7 ms/turn. The source and build flag were
removed and the player wrapper was restored.

Latest construct attribution/rejection: ProfileFast batch
`profilefast-construct-path-context-batch-20260710-014010` attributed 18-19
ms and 142 successful paths to `AI_construct`. The exact follow-up eligibility
pre-scan duplicated enough XML/city work to regress to 571.7 ms/turn versus
the accepted 565.7 ms/turn and was removed; see
`candidate-construct-eligibility-before-path-releaseverify-batch-20260710-014906`.
The ProfileFast caller context remains for broader build-service designs.

The subsequent single-unit `pathCost` path-context candidate was also
rejected: `candidate-pathcost-single-unit-path-context-releaseverify-batch-20260710-020716`
measured 568.3 ms/turn versus the accepted 565.7 ms/turn. The original callback
source and accepted player wrapper were restored.

Latest predicate attribution: `profilefast-canbuild-order-batch-20260710-022517`
showed 41,130 of 55,521 `CvPlayer::canBuild` calls already reject at the
accepted tech-first check, while only 68 reach and fail feature tech and none
fail gold. This rules out another exact reorder before plot legality; the
ProfileFast-only counters remain available.

Latest rejected city-planning experiment:
`candidate-city-build-validity-context-releaseverify-batch-20260709-225234`
measured 617.7 ms/turn versus a contemporaneous clean
`ReleaseFastVerify` control median of 580.3 ms/turn in
`clean-city-build-validity-context-control-releaseverify-batch-20260709-230206`.
The exact plot-scoped context replaced repeated per-improvement XML build scans
with one build-legality scan per `AI_bestPlotBuild` call, but regressed by
37.4 ms/turn / 6.4%, so it was removed. This reinforces that the next city-side
candidate needs to change the broader work-planning/scoring shape rather than
repackage build-validity scans.

Benchmark matrix status: the primary benchmark remains the niallohiggins
turn-0110 save over turns 111-113. The secondary local matrix now has
3-run/one-turn baselines for midgame city/worker, late-large, and early-pathing
saves: 846.0, 1407.0, and 954.0 ms/turn respectively. See
[profiling/autoverify/MATRIX.md](profiling/autoverify/MATRIX.md) and
`profiling/autoverify/matrix-run-20260706-105554.md`. Matrix save paths are
local-only and are not committed.

Profile attribution batch:
`profiling/autoverify/profilefast-private-batch-20260704-134517`, median
752.0 ms/turn, `Metric source: custom_profile`.

Path-request attribution batch:
`profiling/autoverify/profilefast-path-requests-batch-20260705-002808`,
median 653.0 ms/turn, `Metric source: custom_profile`. This ProfileFast-only
instrumentation found 3,107 `generatePath` calls in the median run, with only
252 strict duplicate requests under owner/group/from/to/flags/reuse keys
(8.1%), so future path work should focus beyond exact whole-path result
dedupe.

City-yield attribution batch:
`profiling/autoverify/profilefast-city-yield-delta-context-batch-20260705-013702`,
median 625.0 ms/turn, `Metric source: custom_profile`. The accepted
`MNAI_OPT_CITY_YIELD_DELTA_CONTEXT` optimization reduced the median-run
`CvCityAI::AI_yieldValue::yield_delta` aggregate from 201 ms to 42 ms,
`CvCityAI::AI_plotValue` from 218 ms to 57 ms, and
`CvCityAI::AI_assignWorkingPlots` from 245 ms to 114 ms versus
`profilefast-cityyield-detail-batch-20260705-011825`.

No-bonus vote-source attribution batch:
`profiling/autoverify/profilefast-nobonus-votesource-first-batch-20260705-020138`,
median 592.7 ms/turn, `Metric source: custom_profile`. The accepted
`MNAI_OPT_NOBONUS_VOTE_SOURCE_FIRST` optimization checks the vote-source
no-bonus table before calling `isFullMember` in hot bonus queries. In the
median ProfileFast run, `CvPlayer::isFullMember` fell from 49 ms / 638,638
calls to 0 ms / 4,774 calls, and the no-bonus callers disappeared from
`full_member_callers.csv`.

Attack-odds-before-path attribution batch:
`profiling/autoverify/profilefast-attack-odds-before-path-batch-20260705-022736`,
median 586.0 ms/turn, `Metric source: custom_profile`. The accepted
`MNAI_OPT_ATTACK_ODDS_BEFORE_PATH` optimization tests exact attack odds before
spending pathfinder work on low-odds `AI_cityAttack` and `AI_anyAttack`
candidates. Median-run `generatePath` fell from 204 ms / 3,107 calls to
161 ms / 2,796 calls versus
`profilefast-nobonus-votesource-first-batch-20260705-020138`; path-attribution
calls from `AI_anyAttack` fell from 298 to 46, and calls from `AI_cityAttack`
fell from 68 to 9.

Path-cost single-unit attribution batch:
`profiling/autoverify/profilefast-pathcost-single-unit-next-skip-batch-20260705-031508`,
median 585.3 ms/turn, `Metric source: custom_profile`. The accepted
`MNAI_OPT_PATHCOST_SINGLE_UNIT_NEXT_SKIP` optimization avoids the linked-list
advance helper inside `pathCost` for single-unit groups. It is a small exact
loop optimization: the no-profiler release batch improved by 4.7 ms/turn, and
the ProfileFast median moved from 586.0 to 585.3 ms/turn.

Sticky path-valid attribution batch:
`profiling/autoverify/profilefast-pathvalid-sticky-update-cache-batch-20260705-084627`,
median 599.7 ms/turn, `Metric source: custom_profile`. The accepted
`MNAI_OPT_PATHVALID_STICKY_UPDATE_CACHE` optimization reuses exact
`pathValid` move-through/move-or-attack callback answers across repeated
`generatePath` calls only inside one `CvUnitAI::AI_update` scope and only while
the group state signature is unchanged. The release-path companion batch
improved from 559.0 to 551.0 ms/turn; median ProfileFast `CvUnit::canMoveInto`
calls fell from 30,457 to 29,371 and `pathValid danger & invisible` moved from
31 ms to 30 ms.

Route-territory owned-plot attribution batch:
`profiling/autoverify/profilefast-route-territory-owned-plot-cache-batch-20260705-194614`,
median 609.7 ms/turn, `Metric source: custom_profile`. The accepted
`MNAI_OPT_ROUTE_TERRITORY_OWNED_PLOT_CACHE` optimization gives
`AI_routeTerritory` a runtime owned-plot list so it scans owned plots in
map-index order instead of every map plot while preserving the original
candidate checks. The release-path companion batch improved from 551.0 to
546.0 ms/turn; median ProfileFast `AI_routeTerritory` fell from 46 ms / 52
calls to 1 ms / 52 calls, and its `plot_valid` sub-scan fell from 29 ms /
133,120 calls to 0 ms / 2,842 calls.

Unit-dispatch attribution batch:
`profiling/autoverify/profilefast-unit-dispatch-detail-batch-20260705-092534`,
median 603.7 ms/turn, `Metric source: custom_profile`. The ProfileFast-only
`MNAI_PROFILE_UNIT_DISPATCH_DETAIL` markers showed the remaining dispatch cost
is concentrated in workers and explorers: `dispatch_worker` 79 ms / 29 calls
and `dispatch_explore` 72 ms / 22 calls in the median run.

City-emphasis attribution batch:
`profiling/autoverify/profilefast-city-emphasize-detail-batch-20260705-041905`,
median 582.3 ms/turn, `Metric source: custom_profile`. The new
`MNAI_PROFILE_CITY_EMPHASIZE_DETAIL` markers showed `CvCityAI::AI_doEmphasize`
at 47 ms / 75 calls and `CvCityAI::AI_setEmphasize::assign_working_plots` at
46 ms / 157 calls in the median run. A release candidate that folded the
forced avoid-angry-citizens emphasis into the first setter call regressed to
567.7 ms/turn, so future city-emphasis work should reduce reassignment shape
more broadly rather than only removing that false/true pair.

Legacy caveat: the older 652.3 ms/turn profile result mixed gameplay-code
optimizations with profiler-overhead reductions. Use `ProfileFast` for hotspot
attribution and `ReleaseFastVerify` for no-profiler release-path timing.

</details>
