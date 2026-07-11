# AutoVerify Harness

Purpose:

```text
Run a fixed More Naval AI save for a fixed number of turns without human input,
capture profiler logs, and summarize the result.
```

Fast app integration:

```text
python/AutoVerify.py is copied into More Naval AI Fast.app under Assets/python.
python/CvEventManager.py imports AutoVerify and forwards load/start/turn/player/end events.
The app-side CvEventManager.py backup is CvEventManager.py.before-autoverify.
```

The hook is inert unless this file exists and enables it:

```text
$HOME/Documents/my games/Beyond The Sword/FFH - More Naval AI/Settings/AutoVerify.ini
```

The harness writes that file temporarily, then restores the previous state.

Run the default 3-turn verification:

```sh
cd "$HOME/Applications/more-naval-ai-fast"
scripts/autoverify_mnai.sh
```

Run with an explicit save and baseline comparison:

```sh
scripts/autoverify_mnai.sh \
  --turns 3 \
  --save "$HOME/Documents/my games/Beyond The Sword/Saves/single/PROFILE_TEST_niallohiggins TURN-0110.CivBeyondSwordSave" \
  --compare path/to/baseline/custom_profile.csv
```

Run the standard repeated benchmark batch:

```sh
scripts/benchmark_autoverify.sh \
  --label candidate-name \
  --runs 3 \
  --turns 3 \
  --turn-filter 111-113
```

For low-overhead phase attribution and deterministic state verification, build
the dedicated trace target before running the same batch:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"
./build-releasefasttrace-wine.sh
```

Back up the fast wrapper DLL, install
`temp_files/ReleaseFastTrace/CvGameCoreDLL.dll` only in More Naval AI Fast.app,
then run the standard batch. Restore/install hook-free `ReleaseFast` after the
diagnostic run.

For `ProfileFast` and `ReleaseFastVerify` builds, prefer the no-focus path:

```sh
scripts/benchmark_autoverify.sh \
  --label candidate-name \
  --runs 3 \
  --turns 3 \
  --turn-filter 111-113 \
  --save "path/to/private/profile-save.CivBeyondSwordSave"
```

`autoverify_mnai.sh` launches Civ directly through the fast wrapper's bundled
Wine prefix by default. This avoids the Porting Kit/Wineskin LaunchServices
path, which can show a wrapper UI or steal focus on macOS. Direct-Wine runs keep
a focus guard active for the full run because the game window can still make
itself frontmost. Use `--app-launch` only when manually debugging wrapper
startup; that path uses `open -g -j -n` and the same guard. If the Wineskin,
Wine, Civ IV, Civilization IV, or Beyond the Sword process makes itself
frontmost, the guard hides that benchmark-owned process and reactivates the app
that was frontmost before launch when macOS automation is available.
`AutoVerify.py` also sends Civ's native turn-complete message after enabling AI
autoplay, so the normal unattended path does not need to activate the Wine
window just to press Return. The AppleScript Return-key fallback is off by
default; use `--keypress-fallback` only when manually debugging a save that
stalls at a human turn. Use `--foreground-launch` only when manually debugging
the UI. Use `--no-hide-launch` only when diagnosing
whether hidden launch changes wrapper startup, `--focus-guard-seconds N` to
shorten the guard for debugging, and `--no-focus-guard` only when investigating
focus guard behavior. Direct-Wine cleanup kills the wrapper's Wine prefix and
must not AppleScript-quit the `.app`, since that can relaunch the Wineskin UI
after the benchmark has already finished.

Direct-Wine launches require the wrapper framework directory in
`DYLD_FALLBACK_LIBRARY_PATH` and pass the mod argument as `mod=\More Naval AI`.
`directwine-modload-smoke-batch-20260706-100606` completed one unattended turn
through this path, and
`candidate-city-workable-plot-list-releaseverify-batch-20260706-100706`
completed a full 3-run batch through the same path.

For private/local benchmark saves, keep the save out of git and pass it
explicitly:

```sh
scripts/benchmark_autoverify.sh \
  --label candidate-name \
  --runs 3 \
  --turns 3 \
  --turn-filter 111-113 \
  --save "path/to/private/profile-save.CivBeyondSwordSave"
```

The batch summary is written to:

```text
profiling/autoverify/<label>-batch-<stamp>/BENCHMARK.md
profiling/autoverify/<label>-batch-<stamp>/benchmark.json
```

When every run uses `ReleaseFastTrace`, the batch also writes:

```text
profiling/autoverify/<label>-batch-<stamp>/RELEASE_TRACE.md
profiling/autoverify/<label>-batch-<stamp>/release_trace.json
```

The trace summarizer reports fixed enum-indexed exclusive phase medians,
engine-callback gap time, scheduler busy/timer/combat state, and deterministic
post-turn state hashes. It exits nonzero if selected-turn fingerprints are
missing or differ across runs.

Per-run outputs are written under:

```text
profiling/autoverify/<run-id>/
```

Run the local secondary-save matrix:

```sh
scripts/benchmark_autoverify_matrix.sh --config /path/to/local-matrix.tsv
```

The matrix TSV contains private save paths and stays untracked. Sanitized
summary rows are safe to keep in git; the first 3-run/one-turn secondary
baseline is `profiling/autoverify/matrix-run-20260706-105554.md`. It measured
846.0 ms/turn for the midgame city/worker save, 1407.0 ms/turn for the
late-large save, and 954.0 ms/turn for the early-pathing save. The run used
the direct-Wine path, left empty `PythonErr.log` files, restored
`AutoVerify.ini` state, left no Civ/Wine/Wineskin processes, and restored the
fast wrapper to the player-facing `ReleaseFast` DLL.

Expected artifacts:

```text
autoverify.csv
custom_profile.csv
custom_profile.log
autoverify_dll.csv
release_trace.csv (ReleaseFastTrace only)
scheduler_trace.csv (ReleaseFastTrace only)
state_fingerprint.csv (ReleaseFastTrace only)
PythonDbg.log
PythonErr.log
SUMMARY.md
CivilizationIV.ini.before
```

Behavior:

```text
1. Stage the requested save as Saves/single/AUTOVERIFY.CivBeyondSwordSave.
2. Back up CivilizationIV.ini and AutoVerify.ini.
3. Set GameType=spLoad and FileName to the staged save.
4. Enable AutoVerify.ini for the requested number of turns.
5. Launch Civ with the bundled Wine prefix, or launch More Naval AI Fast.app
   when `--app-launch` is explicitly selected.
6. Wait for AutoVerify.py to write event=complete, or infer completion once
   custom_profile.csv reaches the target turn.
7. Stop this wrapper's Wine prefix, quit the fast app if the app-launch path was
   used, copy logs, summarize custom_profile.csv, restore settings.
```

Failure handling:

```text
The script restores CivilizationIV.ini and AutoVerify.ini on timeout, error, or
Ctrl-C. Existing custom_profile/autoverify logs are moved into the run
directory before launch instead of being overwritten.
```

Smoke result:

```text
smoke-final-20260703-224811 completed one unattended turn from 110 to 111.
Completion was inferred from custom_profile.csv reaching turn 111.
The script restored CivilizationIV.ini, removed AutoVerify.ini, and left no
More Naval AI Fast.app Wine processes running.
```

Multi-turn fix:

```text
python/AutoVerify.py now prefers CyGame.setForcedAIAutoPlay(..., forced=True)
and falls back to setAIAutoPlay when the forced API is unavailable.
CvPlayer::setTurnActive also has a profile-build-only
MNAI_AUTOVERIFY_AUTO_END_TURN hook that ends the active turn for human-disabled
autoplay players. Together these allow a fixed save loaded at turn 110 to
advance unattended through turns 111-113.
```

Current benchmark baseline and best:

```text
Baseline batch: profiling/autoverify/baseline-m3-batch-20260703-231157
Baseline median: 848.7 ms/turn
20% target: <= 678.9 ms/turn

Best recorded no-profiler release-path batch:
candidate-contiguous-turn-updates-releaseverify-batch-20260710-211556
Best recorded no-profiler release-path median: 163.3 ms/turn (-80.8%)
No-profiler metric source: autoverify_dll_turn
ReleaseFastVerify runs: 163.3, 162.3, 172.0 ms/turn
Matching fixed-cost trace baseline:
releasefasttrace-scheduler-batch-20260710-205418 at 582.0 ms/turn
Matching fixed-cost trace candidate:
candidate-contiguous-turn-updates-trace-batch-20260710-210856 at 170.7 ms/turn
Trace callback-gap movement: 412.8 ms/turn -> 0 ms/turn
Installed player-facing ReleaseFast DLL SHA256:
9cbfb4cc162bd145d1533e1c63fd17ace94524cd11a3e0a090099eaf6ad82e1d
Installed fast-wrapper CvGameUtils.py SHA256:
587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c

Equivalent VC++ 2003/Wine rebuilds are not byte-stable; use benchmark batch
paths and source flags as the performance evidence.

Profile attribution batch: profilefast-private-batch-20260704-134517
ProfileFast median: 752.0 ms/turn
ProfileFast metric source: custom_profile

City-assignment attribution batch:
profilefast-cityassign-detail-batch-20260704-163950
City-assignment attribution median: 736.7 ms/turn
City-assignment metric source: custom_profile
Median-run city samples: CvCityAI::AI_plotValue 204 ms / 14703 calls,
CvCityAI::AI_addBestCitizen::plot_scan 193 ms / 1073 calls,
CvCityAI::AI_assignWorkingPlots::add_population 170 ms / 247 calls,
CvCityAI::AI_assignWorkingPlots::juggle_citizens 54 ms / 247 calls.
This makes exact city score-context hoisting the first Gate A target; do not
reuse stale `AI_plotValue` rows across changed city food/yield state in the
exact release path.

Path-request attribution batch:
profilefast-path-requests-batch-20260705-002808
Path-request attribution median: 653.0 ms/turn
Path-request metric source: custom_profile
The ProfileFast-only `MNAI_PROFILE_PATH_REQUESTS` logger archived
`path_request_callers.csv` for all three runs. Median-run summary:
3107 `generatePath` calls, 181 ms attributed by the path-request logger,
252 strict duplicate requests under owner/group/from/to/flags/reuse keys
(8.1%), and these top buckets: `CvUnitAI::AI_anyAttack` flags 0 at
55 ms / 298 calls / 87 duplicate calls; `CvUnitAI::AI_update` flags 0 at
29 ms / 144 calls / 12 duplicate calls; `CvUnitAI::AI_exploreRange` flags 4 at
16 ms / 664 calls / 27 duplicate calls; `CvUnitAI::AI_cityAttack` flags 0 at
16 ms / 68 calls / 24 duplicate calls; `CvUnitAI::AI_pillageRange` flags 0 at
12 ms / 108 calls / 16 duplicate calls. Exact whole-path result dedupe alone is
not a large enough pathing lever on this save; next path work should target
shared callback work, danger maps, and broader tactical/path services.

City-yield attribution batch:
profilefast-cityyield-detail-batch-20260705-011825
City-yield attribution median: 654.0 ms/turn
City-yield metric source: custom_profile
Median-run city-yield samples: CvCityAI::AI_yieldValue::yield_delta
201 ms / 21197 calls, CvCityAI::AI_plotValue 218 ms / 14703 calls,
CvCityAI::AI_addBestCitizen::plot_scan 205 ms / 1073 calls,
CvCityAI::AI_assignWorkingPlots 245 ms / 247 calls.

Accepted city-yield context batch:
profilefast-city-yield-delta-context-batch-20260705-013702
Accepted city-yield context median: 625.0 ms/turn
Accepted city-yield context metric source: custom_profile
The release-path companion batch
candidate-city-yield-delta-context-releaseverify-batch-20260705-012951
measured 570.7 ms/turn. Median-run profiler movement: yield-delta fell from
201 ms to 42 ms, AI_plotValue fell from 218 ms to 57 ms, and
AI_assignWorkingPlots fell from 245 ms to 114 ms.

Accepted no-bonus vote-source-first batch:
profilefast-nobonus-votesource-first-batch-20260705-020138
Accepted no-bonus vote-source-first median: 592.7 ms/turn
Accepted no-bonus vote-source-first metric source: custom_profile
The release-path companion batch
candidate-nobonus-votesource-first-releaseverify-batch-20260705-015446
measured 567.0 ms/turn. Median-run profiler movement:
CvPlayer::isFullMember fell from 49 ms / 638638 calls to 0 ms / 4774 calls;
`CvCity::isNoBonus` and `CvPlayer::getNumAvailableBonuses` disappeared from
`full_member_callers.csv` because `CvGame::isNoBonus` is checked before
membership testing.

Accepted attack-odds-before-path batch:
profilefast-attack-odds-before-path-batch-20260705-022736
Accepted attack-odds-before-path median: 586.0 ms/turn
Accepted attack-odds-before-path metric source: custom_profile
The release-path companion batch
candidate-attack-odds-before-path-releaseverify-batch-20260705-022039
measured 563.7 ms/turn. Median-run profiler movement:
CvSelectionGroup::generatePath fell from 204 ms / 3107 calls to 161 ms /
2796 calls, pathCost fell from 47 ms / 169160 calls to 35 ms / 130464 calls,
and path-attribution calls fell from 298 to 46 for `AI_anyAttack` and from
68 to 9 for `AI_cityAttack`.

Accepted pathCost single-unit next-skip batch:
profilefast-pathcost-single-unit-next-skip-batch-20260705-031508
Accepted pathCost single-unit next-skip median: 585.3 ms/turn
Accepted pathCost single-unit next-skip metric source: custom_profile
The release-path companion batch
candidate-pathcost-single-unit-next-skip-releaseverify-batch-20260705-030818
measured 559.0 ms/turn. Median-run profiler movement was intentionally small:
the ProfileFast median moved from 586.0 to 585.3 ms/turn while keeping the same
2796 `generatePath` calls and 130464 `pathCost` calls. The optimization avoids
one linked-list advance helper in each single-unit `pathCost` loop.

Explore-detail attribution batch:
profilefast-explore-detail-v2-batch-20260705-082702
Explore-detail attribution median: 604.7 ms/turn
Explore-detail metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_EXPLORE_DETAIL, ProfileFast only.
Median-run explore rows: `AI_exploreMove` 70 ms / 22 calls,
`AI_exploreRange::generate_path` 18 ms / 664 calls, and
`AI_explore::generate_path` 1 ms / 955 calls. This showed explore-only path
work was too small for the next strategic release candidate.

Accepted path-valid sticky update-cache batch:
profilefast-pathvalid-sticky-update-cache-batch-20260705-084627
Accepted path-valid sticky update-cache median: 599.7 ms/turn
Accepted path-valid sticky update-cache metric source: custom_profile
The release-path companion batch
candidate-pathvalid-sticky-update-cache-releaseverify-batch-20260705-083907
measured 551.0 ms/turn. The optimization keeps reuse scoped to one unchanged
`CvUnitAI::AI_update` group signature. Median-run profiler movement was small:
`CvUnit::canMoveInto` calls fell from 30457 to 29371, `pathValid danger &
invisible` moved from 31 ms to 30 ms, and `AI_workerMove` moved from 79 ms to
77 ms versus the explore-detail baseline.

Accepted route-territory owned-plot cache batch:
profilefast-route-territory-owned-plot-cache-batch-20260705-194614
Accepted route-territory owned-plot cache median: 609.7 ms/turn
Accepted route-territory owned-plot cache metric source: custom_profile
The release-path companion batch
candidate-route-territory-owned-plot-cache-releaseverify-batch-20260705-193825
measured 546.0 ms/turn, improving the previous accepted 551.0 ms/turn median
by 5.0 ms/turn / 0.9%. The candidate adds a runtime-only `CvPlayerAI`
owned-plot list and uses it in `AI_routeTerritory` so worker route scans visit
owned plots in map-index order instead of every map plot while preserving all
existing live candidate checks. Median-run profiler movement versus
`profilefast-food-value-detail-r3-20260705-185700`: `AI_routeTerritory` fell
from 46 ms / 52 calls to 1 ms / 52 calls, `AI_routeTerritory::plot_valid`
fell from 29 ms / 133120 calls to 0 ms / 2842 calls, `AI_workerMove` fell
from 103 ms / 29 calls to 59-60 ms / 29 calls, and total
`CvUnitAI::AI_plotValid` fell from 35 ms / 503257 calls to 26 ms / 372979
calls. The 3-run release batch had one outlier at 586.3 ms/turn; the other
two runs were 546.0 and 545.7 ms/turn.

Unit-dispatch attribution batch:
profilefast-unit-dispatch-detail-batch-20260705-092534
Unit-dispatch attribution median: 603.7 ms/turn
Unit-dispatch attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_UNIT_DISPATCH_DETAIL, ProfileFast only.
Median-run dispatch rows: `AI_update::dispatch_worker` 79 ms / 29 calls,
`AI_update::dispatch_explore` 72 ms / 22 calls, `AI_update::dispatch_attack`
30 ms / 7 calls, and `AI_update::dispatch_merchant` 14 ms / 3 calls. This
confirmed that the next release candidate should target shared worker/explorer
path and scoring services rather than hidden sea/air or city-defense dispatch.

Unit-prep attribution batch:
profilefast-unit-prep-detail-batch-20260705-100022
Unit-prep attribution median: 610.7 ms/turn
Unit-prep attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_UNIT_PREP_DETAIL, ProfileFast only.
Median-run prep rows: `AI_update::non_automated_prep` 72 ms / 332 calls and
`AI_update::prep_groupflag` 72 ms / 239 calls. The hero XP, adept UnitAI,
ready-check, and suicide-summon prep rows rounded to 0 ms, proving that the
hidden prep cost is actually groupflag movement dispatch.

Unit-prep groupflag attribution batch:
profilefast-unit-prep-groupflag-detail-batch-20260705-100836
Unit-prep groupflag attribution median: 607.3 ms/turn
Unit-prep groupflag attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_UNIT_PREP_DETAIL, ProfileFast only.
Median-run groupflag rows: `prep_groupflag_patrol` 34 ms / 78 calls across
turns 111-113, `prep_groupflag_conquest` 22 ms / 18 calls, and
`prep_groupflag_permdefense` 7 ms / 99 calls. The next useful attribution pass
should split `AI_PatrolMove` internally.

Patrol attribution instrumentation:
Instrumentation flag: MNAI_PROFILE_PATROL_DETAIL, ProfileFast only.
Build status only as of 2026-07-05: `./build-profilefast-wine.sh` completed and
produced `CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll` SHA256
`89b6631a3a6f04ab0e855355b1d3c8f37aeca54edbad66cc9bb611ddd74b823b`.

No-focus smoke:
patrol-smoke-bg-nokey-20260705 completed one turn using `open -g -n` and
`--no-keypress-fallback`, emitted `CvUnitAI::AI_PatrolMove::*` rows, left
`PythonErr.log` empty, restored INI state, and left no fast-wrapper process
running.

Focus hardening after a visible Wineskin UI incident:
`autoverify_mnai.sh` now uses `open -g -j -n` for default background launches
and restores focus by previous app name if macOS does not return a previous
bundle id. `focus-hidden-smoke-20260706` loaded the save with the plain
`ReleaseFast` DLL but timed out at turn 110 because the wrapper did not contain
the AutoVerify-capable DLL. After installing clean `ReleaseFastVerify`,
`focus-hidden-smoke-verify-20260706` completed one hidden-background turn
against the niallohiggins save, left `PythonErr.log` empty, restored INI state,
removed transient `AutoVerify.ini`, and left no fast-wrapper Wine process
running. Use `ReleaseFastVerify` or `ProfileFast` for unattended smoke and
benchmark runs.

Follow-up focus hardening after the game stole focus on macOS:
the guard now treats foreground process names containing `Civ IV`,
`Civilization IV`, or `Beyond The Sword`, plus `Wine` and `Wineskin`, as
benchmark-owned windows. When any of those names becomes frontmost during a
background run, the guard hides it and restores the previous foreground app.
After a later direct-Wine focus-steal report, the guard was moved onto the
direct-Wine path too; previously it only covered the LaunchServices app-bundle
path. The direct-Wine guard now also hides process-name variants containing
`Wineskin`, and `AutoVerify.py` sends `CyMessageControl().sendTurnComplete()`
after enabling AI autoplay so the default no-keypress path does not need to
activate the game window to advance the first turn. A one-turn smoke rerun was
blocked by the Codex usage/approval limit on 2026-07-06, so this latest
turn-complete addition still needs a live Wine smoke test.

Patrol attribution batch:
profilefast-patrol-detail-batch-20260705-104553
Patrol attribution median: 609.7 ms/turn
Patrol attribution metric source: custom_profile
Comparison: +2.3 ms / +0.4% versus the previous groupflag attribution batch,
consistent with extra profiler markers rather than a gameplay change.
Median-run patrol rows: `prep_groupflag_patrol` 37 ms / 78 calls across turns
111-113. Detailed `AI_PatrolMove::*` rows summed to 22 ms / 1516 scoped calls:
`small_group_options` 13 ms / 78 calls, `guard_protect_airlift` 4 ms / 30
calls, `fort_airlift_guard1` 2 ms / 57 calls, and
`fallback_switch_group_patrol_safety` 2 ms / 12 calls. Most other patrol bands
rounded to 0 ms. This makes patrol a low-yield exact target compared with the
same median run's `dispatch_worker` 78 ms / 29 calls, `dispatch_explore`
71 ms / 22 calls, and `generatePath` 170 ms / 2796 calls.

Worker/path context attribution batch:
profilefast-worker-path-context-batch-20260705-110545
Worker/path context median: 581.0 ms/turn
Worker/path context metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_PATH_REQUESTS, ProfileFast only.
This run added worker-helper path attribution contexts for
`AI_improveLocalPlot`, `AI_nextCityToImprove`, `AI_irrigateTerritory`,
`AI_fortTerritory`, `AI_improveBonus`, `AI_connectBonus`, `AI_connectCity`,
`AI_routeCity`, `AI_routeTerritory`, and `AI_travelToUpgradeCity`.
Median-run samples still showed `CvSelectionGroup::generatePath()` at 164 ms /
2796 calls, `AI_workerMove` at 75 ms / 29 calls, `AI_exploreMove` at 69 ms /
22 calls, `AI_assignWorkingPlots` at 71 ms / 247 calls, and
`AI_addBestCitizen` at 63 ms / 1073 calls. Aggregated path-request rows across
the batch put worker helper path buckets below the broad `AI_update`,
`AI_exploreRange`, and `AI_anyAttack` buckets, so another narrow worker-helper
prefilter is unlikely to move the primary benchmark.

Groupflag path context attribution:
Instrumentation flag: MNAI_PROFILE_PATH_REQUESTS, ProfileFast only.
Added `MNAI_PATH_REQUEST_CONTEXT` scopes at `AI_ConquestMove`,
`AI_cityDefenseMove`, and `AI_PatrolMove` to split the generic
`CvUnitAI::AI_update` path-request bucket by groupflag movement routine.
Build status:
`env WINEDEBUG=-all WINEDLLOVERRIDES=winemenubuilder.exe=d
./build-profilefast-wine.sh` completed and produced
`CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll` SHA256
`dea2d236235246e2d4140edeae96b3e8cd77e7bf0aca4a11ceb8abcd632d1b6c`.
Batch:
profilefast-groupflag-path-context-batch-20260706-081613
Groupflag path context median: 583.7 ms/turn
Groupflag path context metric source: custom_profile
Run averages were 580.0, 588.3, and 583.7 ms/turn. All three
`PythonErr.log` files were empty; the harness restored INI state, removed the
transient `AutoVerify.ini`, left no fast-wrapper process running, restored the
fast wrapper to accepted ReleaseFast DLL SHA256
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and left
the original wrapper DLL unchanged at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
Median-run evidence: `CvSelectionGroup::generatePath()` was still
161 ms / 2796 calls, `prep_groupflag` was 65 ms / 239 calls,
`AI_exploreMove` was 66 ms / 22 calls, `AI_workerMove` was 57 ms / 29 calls,
and city-side rows stayed material (`AI_assignWorkingPlots` 83 ms / 247 calls,
`AI_addBestCitizen` 74 ms / 1073 calls, `AI_bestPlotBuild`
63 ms / 1167 calls). The path-request split showed `AI_ConquestMove` at only
14 ms / 36 calls / 0 duplicate calls, generic `AI_update` flags 0 at
15 ms / 54 calls / 0 duplicates, `AI_PatrolMove` at 1 ms / 49 calls with
12 duplicates, and `AI_cityDefenseMove` rounding to 0 ms / 5 calls. This does
not support a narrow groupflag duplicate-path cache as the next large win.

Danger/threat attribution batch:
profilefast-danger-detail-batch-20260706-083202
Danger/threat attribution median: 596.7 ms/turn
Danger/threat metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_DANGER_DETAIL, ProfileFast only.
Build SHA256: `CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll`
`851915ad25687fa4d4ccd5fcd7613c40fc1126db838a3f8eb7c029a4cc128edf`.
Run averages were 589.7, 597.3, and 596.7 ms/turn. All three
`PythonErr.log` files were empty; the harness restored INI state, removed the
transient `AutoVerify.ini`, left no fast-wrapper process running, restored the
fast wrapper to accepted ReleaseFast DLL SHA256
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and left
the original wrapper DLL unchanged at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
Median-run danger rows were small: `CvPlayerAI::AI_getAnyPlotDanger`
12 ms / 34574 calls, `AI_getAnyPlotDanger::plot_unit_scan`
2 ms / 3908 calls, `AI_getAnyPlotDanger::active_no_danger_cache`
1 ms / 34181 calls, `AI_getPlotDanger` 0 ms / 1051 calls,
`AI_isPlotThreatened` 0 ms / 89 calls, and `AI_getWaterDanger`
0 ms / 11 calls. The larger nearby row is still path callback work:
`pathValid danger & invisible` 35 ms / 194296 calls. A standalone
`AI_getPlotDanger`/threat cache is therefore not a large primary-save lever;
future danger-map work must replace shared path callback danger/invisibility
checks to be worth the architectural cost.

Found-value attribution batch:
profilefast-found-value-detail-batch-20260705-114144
Found-value attribution median: 611.3 ms/turn
Found-value attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_FOUND_VALUE_DETAIL, ProfileFast only.
The run added detailed bands inside `CvPlayerAI::AI_updateFoundValues`.
Median-run rows across turns 111-113 were `AI_updateFoundValues` 36 ms /
24 calls, `plot_loop` 26 ms / 24 calls, `AI_foundValue` 22 ms / 11233 calls,
and `update_city_sites` 8 ms / 21 calls. Reset, invalidation, and record-value
bands rounded to 0 ms. This is too small on the primary save to justify a
found-value optimization before shared unit/pathing and city/worker scoring
services.

Worker-detail attribution batch:
profilefast-worker-detail-batch-20260705-035427
Worker-detail attribution median: 585.0 ms/turn
Worker-detail metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_WORKER_DETAIL, ProfileFast only.
Median-run worker rows: `AI_workerMove` 75 ms / 29 calls,
`AI_routeTerritory` 20 ms / 52 calls, `AI_fortTerritory` 12 ms / 33 calls,
`AI_connectBonus` 9 ms / 26 calls, `AI_irrigateTerritory` 9 ms / 26 calls,
`AI_improveBonus` 4 ms / 29 calls, and `AI_improveLocalPlot` 0 ms / 52 calls.

Worker-bonus detail attribution batch:
profilefast-worker-bonus-detail-batch-20260706-113640
Worker-bonus detail median: 599.0 ms/turn
Worker-bonus detail metric source: custom_profile
Comparison: -10.7 ms / -1.7% versus the previous route-territory ProfileFast
batch. Median-run `AI_improveBonus` rows across turns 111-113: parent sample
15 ms / 29 calls; `plot_filter` 3 ms / 74240 calls; `candidate_path`
2 ms / 17 calls; `can_improve`, `build_scan`, and `candidate_score` rounded to
0 ms. This rules out `AI_improveBonus` as a standalone exact optimization
target on the primary save.

Route-territory detail attribution batch:
profilefast-route-territory-detail-batch-20260705-134739
Route-territory detail median: 595.0 ms/turn
Route-territory detail metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_ROUTE_TERRITORY_DETAIL, ProfileFast only.
Median-run `AI_routeTerritory` totaled 46 ms / 52 calls. Its dominant internal
row was `AI_routeTerritory::plot_valid` at 29 ms / 133120 calls, followed by
`owner_check` at 0 ms / 36712 calls and `best_route` at 0 ms / 1114 calls. The
target-mission, visible-enemy, generate-path, and score-success bands rounded
to 0 ms. The obvious owner-first release candidate was tested separately and
rejected, so future worker-route work should be a broader exact service rather
than another local filter reorder.

Best-plot-build attribution batch:
profilefast-best-plot-build-detail-batch-20260705-141454
Best-plot-build attribution median: 596.7 ms/turn
Best-plot-build attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_BEST_PLOT_BUILD_DETAIL, ProfileFast only.
Median-run rows showed `CvCityAI::AI_bestPlotBuild` at 44 ms / 1167 calls and
`AI_bestPlotBuild::improvement_value_loop` at 43 ms / 1167 calls, with
`CvPlayer::canBuild` at 43 ms / 55573 calls, `CvPlayer::getBestRoute` at
28 ms / 27436 calls, and `CvPlot::calculateImprovementYieldChange` at 23 ms /
73505 calls.

Improvement-value attribution batch:
profilefast-improvement-value-detail-batch-20260705-142247
Improvement-value attribution median: 599.3 ms/turn
Improvement-value attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_BEST_PLOT_BUILD_DETAIL, ProfileFast only.
Median-run rows showed `AI_getImprovementValue::build_validity_scan` at
36 ms / 94286 calls and `AI_getImprovementValue::build_validity_can_build` at
23 ms / 35979 calls. The obvious exact build-list release candidate was tested
separately and rejected, so future city-plot work should target shared
dirty-region scoring/cache services rather than only replacing the local XML
build scan.

Player-turn attribution batch:
profilefast-player-turn-detail-batch-20260705-145708
Player-turn attribution median: 596.7 ms/turn
Player-turn attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_PLAYER_TURN_DETAIL, ProfileFast only.
Median-run rows showed `CvPlayer::setTurnActive::inactive_do_turn` and
`CvPlayer::doTurn` at 302 ms / 24 calls, `CvPlayer::doTurn::city_do_turn_loop`
at 175 ms / 24 calls, `CvPlayer::setTurnActive::active_turn_units` and
`CvPlayer::doTurnUnits` at 65 ms / 24 calls, and
`CvPlayer::doTurn::tower_mastery` at 62 ms / 24 calls. A full C++ port was
rejected, but the later narrow owned-plot service was accepted and reduced the
marker to 0 ms / 24 calls without moving the Python state machine.

City-turn attribution batch:
profilefast-city-turn-detail-batch-20260705-152947
City-turn attribution median: 598.3 ms/turn
City-turn attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_CITY_TURN_DETAIL, ProfileFast only.
Median-run rows showed `CvPlayer::doTurn::city_do_turn_loop` at 180 ms /
24 calls, `CvCity::doTurn::AI_doTurn` and `CvCityAI::AI_doTurn` at
142 ms / 89 calls, `CvCityAI::AI_doTurn::update_best_build` at 66 ms /
75 calls, `CvCityAI::AI_assignWorkingPlots` at 72 ms / 247 calls, and
`CvCityAI::AI_bestPlotBuild::improvement_value_loop` at 62 ms / 1167 calls.
The city-side target remains shared city/worker scoring and dirty-region cache
design, not another narrow XML scan wrapper.

Food-value attribution batch:
profilefast-food-value-detail-batch-20260705-185700
Food-value attribution median: 600.7 ms/turn
Food-value attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_CITY_YIELD_DETAIL, ProfileFast only.
Median-run rows showed `AI_yieldValue::food_value` at 15-16 ms/turn. The
dominant measurable sub-band was `food_value::good_tiles` at 7 ms/turn;
`food_value::state` was 3 ms/turn, and starvation, anger recovery, and
growth-value arithmetic rounded to 0-1 ms/turn. The prior scoped good-tile /
specialist count cache was rejected on the release path, so the next city-side
candidate should avoid another local food-value cache and instead move toward
shared dirty-region city scoring.

City-emphasis detail attribution batch:
profilefast-city-emphasize-detail-batch-20260705-041905
City-emphasis detail median: 582.3 ms/turn
City-emphasis detail metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_CITY_EMPHASIZE_DETAIL, ProfileFast only.
Median-run city-emphasis rows: `AI_doEmphasize` 47 ms / 75 calls,
`AI_setEmphasize::assign_working_plots` 46 ms / 157 calls,
`AI_doEmphasize::set_emphasize` 24 ms / 600 calls, and
`AI_doEmphasize::force_avoid_angry` 22 ms / 75 calls. The tile-summary,
good-tile, production-multiplier, and population-rank markers rounded to 0 ms.

The no-profiler release-path benchmark beats the 20% target by about
127.9 ms/turn. The 75% goal remains open: the primary target is still
212.2 ms/turn. The continuation roadmap now uses more aggressive gates of 400,
300, and 212.2 ms/turn, followed by stretch targets of 169.7 and
127.3 ms/turn for 80% and 85% reductions from the original baseline.

Gate check: 3-run AutoVerify batch completed, PythonErr logs were empty,
CivilizationIV.ini was restored, transient AutoVerify.ini was removed, no test
wrapper Wine processes remained, and the control wrapper DLL stayed unchanged.

Caveat: the older 652.3 ms/turn profile result mixed gameplay-code
optimizations with profile-build instrumentation reductions. Use `ProfileFast`
for hotspot attribution and `ReleaseFastVerify` for no-profiler release-path
timing.

Future benchmark work should prefer:

```text
ReleaseFast: gameplay optimization flags, no profiler or AutoVerify hooks.
ReleaseFastVerify: ReleaseFast plus only the test-only AutoVerify end-turn hook.
ProfileFast: same gameplay flags plus profiler and AutoVerify hooks.
```

`ProfileFast` benchmark summaries normally use `custom_profile.csv`.
`ReleaseFast` and `ReleaseFastVerify` do not emit profiler rows.
`ReleaseFastVerify` emits one benchmark-only DLL row per completed game turn in
`autoverify_dll.csv`; those summaries report
`Metric source: autoverify_dll_turn`. If that file is absent, summaries can
fall back to wall-clock turn intervals from `autoverify.csv` when the fast app
has the updated `python/AutoVerify.py` installed; those summaries report
`Metric source: autoverify_wall_ms`.

Raw local run directories, logs, INI backups, and generated benchmark artifacts
are intentionally gitignored. Keep only sanitized summaries in committed docs.
```

Rejected follow-up:

```text
candidate-build-list-cache-batch-20260704-002639 reached 731.0 ms/turn, but
CvCityAI::AI_bestPlotBuild and CvPlayer::canBuild samples did not improve
against the current best. The candidate was removed and the fast/profile DLLs
were restored to 23e730459374e71b4bc78312d5d8b2722060467b3c699d0324bfb22cf5574276.

candidate-path-unit-micro-batch-20260704-003945 reached 726.7 ms/turn, but
pathValid, pathCost, and CvUnitAI::AI_plotValid samples did not improve against
the current best. The candidate was removed and the fast/profile DLLs were
restored to 23e730459374e71b4bc78312d5d8b2722060467b3c699d0324bfb22cf5574276.

candidate-route-build-list-batch-20260704-004958 reached 736.7 ms/turn and
CvPlayer::getBestRoute stayed at 54 ms. The candidate was removed and the
fast/profile DLLs were restored to
23e730459374e71b4bc78312d5d8b2722060467b3c699d0324bfb22cf5574276.

candidate-improvement-build-lookup-releaseverify-batch-20260705-153959
measured 570.7 ms/turn against the accepted 551.0 ms/turn batch. The
`MNAI_OPT_IMPROVEMENT_BUILD_LOOKUP` candidate used a static
improvement-to-build vector to avoid repeated all-build XML scans inside
`AI_getImprovementValue`, while preserving XML build order and the same
`CvPlayer::canBuild` checks. The release-path median regressed by
19.7 ms/turn, so the helper and Fast/ProfileFast build flags were removed, the
fast wrapper was restored to accepted ReleaseFast DLL hash
9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892, and
`ReleaseFastVerify` was rebuilt without the flag.

candidate-fastheur-explore-prune-releaseverify-batch-20260705-155730
measured 568.3 ms/turn against the accepted 551.0 ms/turn batch. The
`MNAI_FAST_HEURISTIC_EXPLORE_PRUNE` candidate deterministically skipped half
of `AI_explore` and `AI_exploreRange` candidate plots before expensive
`AI_plotValid` and pathing. It was explicitly an optional fast-heuristic test,
not an exact default optimization, but still regressed by 17.3 ms/turn. The
source hook and `ReleaseFastVerify` build flag were removed, the fast wrapper
was restored to accepted ReleaseFast DLL hash
9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892, and
`ReleaseFastVerify` was rebuilt without the flag.

candidate-fastheur-skip-city-emphasize-releaseverify-batch-20260705-164440
measured 556.0 ms/turn against the accepted 551.0 ms/turn batch. The
`MNAI_FAST_HEURISTIC_SKIP_CITY_EMPHASIZE` candidate skipped
`CvCityAI::AI_doEmphasize` under an explicit fast-heuristic compile flag.
Although ProfileFast city-turn evidence showed `AI_doEmphasize` at roughly
14-17 ms/turn, the release-path median regressed by 5.0 ms/turn. The source
guard and `ReleaseFastVerify` build flag were removed, the fast wrapper was
restored to accepted ReleaseFast DLL hash
9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892, and
`ReleaseFastVerify` was rebuilt without the flag.

candidate-route-to-city-pair-cache-releaseverify-batch-20260705-170148
measured 564.0 ms/turn against the accepted 551.0 ms/turn batch. The
`MNAI_OPT_ROUTE_TO_CITY_PAIR_CACHE` candidate cached `AI_updateRouteToCity`
route-finder results for current-turn city pairs and invalidated them on
`CvPlot::setRouteType` route-network changes. The release-path median
regressed by 13.0 ms/turn, so the route-generation counter, city-pair cache,
and `ReleaseFastVerify` build flag were removed, and the fast wrapper was
restored to accepted ReleaseFast DLL hash
9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892. A
follow-up clean `ReleaseFastVerify` rebuild was not run in that turn because
the environment rejected further escalated commands after the benchmark due
usage limits.

candidate-route-territory-inline-valid-releaseverify-batch-20260705-161540
measured 563.3 ms/turn against the accepted 551.0 ms/turn batch. The
`MNAI_OPT_ROUTE_TERRITORY_INLINE_PLOT_VALID` candidate inlined the exact
`AI_plotValid` logic inside `AI_routeTerritory`, caching unit domain, area,
all-terrain movement, and no-reveal-map invariants across the full-map scan.
The release-path median regressed by 12.3 ms/turn, so the helper and
`ReleaseFastVerify` build flag were removed, the fast wrapper was restored to
accepted ReleaseFast DLL hash
9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892, and
`ReleaseFastVerify` was rebuilt without the flag.

Earlier route-territory owned-plot cache attempt
candidate-route-territory-owned-plot-cache-releaseverify-batch-20260705-163006
measured 569.7 ms/turn against the accepted 551.0 ms/turn batch. The
candidate cached only the
map-order list of plots owned by the worker's player, keyed by game turn,
owner, map size, and `getTotalLand()`, while rechecking all live route,
improvement, visibility, target-mission, and path conditions per worker. The
release-path median regressed by 18.7 ms/turn, so that keyed implementation
was removed before the later simpler runtime `CvPlayerAI` owned-plot service
was built and accepted. The fast wrapper was restored to accepted ReleaseFast
DLL hash
9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892, and
`ReleaseFastVerify` was rebuilt without the flag.

candidate-explorerange-prefilter-batch-20260704-082130 reached 711.0 ms/turn,
but CvUnitAI::AI_exploreRange itself got slower and AI_exploreRange 2 calls
increased sharply, so the apparent median gain was not accepted as a targeted
hotspot improvement. The candidate was removed and the fast/profile DLLs were
restored to 23e730459374e71b4bc78312d5d8b2722060467b3c699d0324bfb22cf5574276.

candidate-canmovethrough-domain-batch-20260704-084159 reached 736.0 ms/turn,
and the intended pathing samples regressed: CvSelectionGroup::generatePath rose
from about 254 ms to 258 ms and pathValid move through rose from about 74 ms to
77 ms. The candidate was removed and the fast/profile DLLs were restored to
2ef85e794574eb8f5664792dc4547e046072be31e10429540fb9646e93b0633d.

candidate-anyattack-summon-prefilter-batch-20260704-085211 reached
757.0 ms/turn, and the intended AI_anyAttack sample regressed from about 75 ms
to 79 ms while generatePath rose from about 254 ms to 268 ms. The candidate was
removed and the fast/profile DLLs were restored to
2ef85e794574eb8f5664792dc4547e046072be31e10429540fb9646e93b0633d.

candidate-routeterritory-owner-prefilter-batch-20260704-090254 reached
737.7 ms/turn. CvUnitAI::AI_workerMove improved from about 81 ms to 69 ms, but
CvSelectionGroup::generatePath regressed from about 254 ms to 264 ms and
pathValid move through rose from about 74 ms to 77 ms. The candidate was removed
and the fast/profile DLLs were restored to
2ef85e794574eb8f5664792dc4547e046072be31e10429540fb9646e93b0633d.

candidate-plotvalue-finalupgrade-noop-batch-20260704-091902 reached
735.0 ms/turn. CvCityAI::AI_plotValue dropped from about 54 ms to 45 ms, but
CvPlayerAI::AI_unitUpdate rose to 389 ms and CvSelectionGroup::generatePath rose
to 262 ms, so the overall median regressed versus the accepted best. The
candidate was removed and the fast/profile DLLs were restored to
2ef85e794574eb8f5664792dc4547e046072be31e10429540fb9646e93b0633d.

candidate-path-invariant-cache-batch-20260704-094549 reached 706.7 ms/turn.
CvSelectionGroup::generatePath improved from about 207 ms to 196 ms and
CvPlayerAI::AI_unitUpdate improved from about 334 ms to 321 ms, but the median
regressed from the accepted 698.7 ms result and pathCost rose from about
49 ms to 51 ms. The candidate was removed and the fast/profile DLLs were
restored to 4ab21d6fc55ac62c27043e7960cacf1f685e7c4beb258e62292443e6d0b9819f.

candidate-discovery-bonus-cache-batch-20260704-100107 reached 703.0 ms/turn.
The intended city-side samples stayed effectively flat versus the accepted
best: CvCityAI::AI_plotValue stayed at about 53 ms, CvCityAI::AI_bestPlotBuild
stayed at about 46 ms, and CvPlayer::canBuild regressed from about 42 ms to
45 ms. The candidate was removed and the fast/profile DLLs were restored to
4ab21d6fc55ac62c27043e7960cacf1f685e7c4beb258e62292443e6d0b9819f.

candidate-base-bonus-plotvalue-noop-batch-20260704-105723 reached
699.3 ms/turn. It did not produce a useful wall-clock gain versus the current
pathvalid/base-bonus direction. The candidate was removed.

candidate-base-bonus-cache-hardened-batch-20260704-110938 reached
696.0 ms/turn. Broad invalidation through AI_makeAssignWorkDirty and
AI_makeProductionDirty was safer than a stale cache but too expensive; it was
removed in favor of resource-specific invalidation.

candidate-hardened-base-bonus-routefast-batch-20260704-111903 reached
703.3 ms/turn and CvPlayer::getBestRoute stayed around 27 ms. The route fast
path was removed.

candidate-city-goodtile-cache-releaseverify-batch-20260704-143705 reached
602.7 ms/turn with `Metric source: autoverify_dll_turn`. The scoped
`AI_countGoodTiles` / `AI_countGoodSpecialists` cache compiled and completed
cleanly, but the restored clean release-path batch measured 593.0 ms/turn, so
the candidate was removed as below the acceptance gate and likely noise.

candidate-votesource-religion-array-releaseverify-batch-20260704-145936 reached
606.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_VOTE_SOURCE_RELIGION_ARRAY` candidate replaced repeated
vote-source religion hash lookups with an array mirror, but the release-path
median regressed against the restored clean 593.0 ms/turn batch, so it was
removed.

candidate-hasvotes-cache-releaseverify-batch-20260704-151913 reached
602.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_HAS_VOTES_CACHE` candidate cached `CvPlayer::hasVotes` by player
and religion with invalidation on religion, population, and diplomatic-vote unit
changes, but the release-path median regressed against the restored clean
593.0 ms/turn batch, so it was removed.

candidate-pathcost-enemy-cache-releaseverify-batch-20260704-153831 reached
618.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_PATHCOST_ENEMY_CACHE` candidate cached per-path, per-unit
enemy-adjacent path-cost classification for `MOVE_AVOID_ENEMY_WEIGHT_2/3`, but
the release-path median regressed against the restored clean 593.0 ms/turn
batch, so it was removed.

candidate-pathvalid-group-cache-releaseverify-batch-20260704-154920 reached
606.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_PATHVALID_GROUP_CACHE` candidate cached per-path group-level
`canFight()` and `alwaysInvisible()` values for the path danger check, but the
release-path median regressed against the restored clean 593.0 ms/turn batch,
so it was removed.

candidate-pathvalid-danger-cache-releaseverify-batch-20260704-160958 reached
621.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_PATHVALID_DANGER_CACHE` candidate cached per-path
`AI_getAnyPlotDanger` results during one `generatePath` call, but the
release-path median regressed against the restored clean 593.0 ms/turn batch,
so it was removed and the fast wrapper DLL was restored.

candidate-emphasize-avoid-angry-single-set-releaseverify-batch-20260704-162303
reached 603.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_EMPHASIZE_AVOID_ANGRY_SINGLE_SET` candidate folded the forced
avoid-angry-citizens emphasis state into the first `AI_setEmphasize` call to
avoid an immediate false/true reassignment pair during `AI_doEmphasize`, but
the release-path median regressed against the restored clean 593.0 ms/turn
batch, so it was removed and the fast wrapper DLL was restored.

candidate-city-yield-scan-context-releaseverify-batch-20260704-165907 reached
603.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_CITY_YIELD_SCAN_CONTEXT` candidate hoisted city/player invariants
for the `AI_addBestCitizen` worker plot scan into a per-call context, but the
release-path median regressed against the restored clean 593.0 ms/turn batch.
It was removed and the fast wrapper DLL was restored.

candidate-full-member-cache-releaseverify-batch-20260704-172507 reached
602.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_FULL_MEMBER_CACHE` candidate cached `CvPlayer::isFullMember` by
player/vote source with conservative invalidation, but the release-path median
regressed against the restored clean 593.0 ms/turn batch. It was removed and
the fast wrapper DLL was restored.

candidate-city-plot-static-cache-releaseverify-batch-20260704-174918 reached
608.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_CITY_PLOT_STATIC_CACHE` candidate cached per-plot static
yield/improvement/discovery/upgrade pieces during `AI_assignWorkingPlots`, but
still called `AI_yieldValue` for exact current city state. The release-path
median regressed against the restored clean 593.0 ms/turn batch, so it was
removed and the fast wrapper DLL was restored.

candidate-city-yield-count-cache-releaseverify-batch-20260704-181323 reached
597.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_CITY_YIELD_COUNT_CACHE` candidate reused exact
`AI_countGoodTiles` and `AI_countGoodSpecialists` results during a stable
citizen scan, but the release-path median still regressed against the restored
clean 593.0 ms/turn batch. It was removed and the fast wrapper DLL was
restored.

candidate-force-civic-option-cache-releaseverify-batch-20260704-182928 reached
602.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_FORCE_CIVIC_OPTION_CACHE` candidate cached
`CvGame::isForceCivicOption` and avoided `CvPlayer::isFullMember` unless a
vote source actually forced the relevant civic option, but the release-path
median regressed against the restored clean 593.0 ms/turn batch. It was
removed and the fast wrapper DLL was restored.

candidate-no-bonus-votesource-cache-releaseverify-batch-20260704-190734
reached 606.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_NO_BONUS_CACHE` candidate cached the exact
per-player/per-bonus vote-source NoBonus membership result, but the
release-path median regressed against the restored clean 593.0 ms/turn batch.
It was removed and the fast wrapper DLL was restored.

candidate-no-bonus-check-order-releaseverify-batch-20260704-191856 reached
610.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_NO_BONUS_CHECK_ORDER` candidate tested the exact cheap-check-first
loop order for vote-source NoBonus queries, but the release-path median
regressed against the restored clean 593.0 ms/turn batch. It was removed and
the fast wrapper DLL was restored.

candidate-active-danger-nomove-cache-releaseverify-batch-20260704-193244
reached 596.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_PROFILE_ACTIVE_DANGER_NOMOVE_CACHE` candidate cached no-danger results
for active-player `AI_getAnyPlotDanger(..., bTestMoves=false)` calls by range,
but the release-path median regressed against the restored clean
593.0 ms/turn batch. It was removed and the fast wrapper DLL was restored.

candidate-city-best-route-context-releaseverify-batch-20260705-024616 reached
566.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_CITY_BEST_ROUTE_CONTEXT` candidate scoped one `CvPlayer::getBestRoute`
answer to each valid `AI_getImprovementValue` call and reused it across the
repeated best-route yield calculations, but the release-path median regressed
against the accepted 563.7 ms/turn batch. It was removed and the fast wrapper
DLL was restored.

candidate-unit-construct-scan-cache-releaseverify-batch-20260705-025723 reached
565.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_UNIT_CONSTRUCT_SCAN_CACHE` candidate cached the static
unit/civilization XML answer for whether `AI_update::construct_scan` should
call `AI_construct()`, but the release-path median regressed against the
accepted 563.7 ms/turn batch. It was removed and the fast wrapper DLL was
restored.

candidate-ai-plot-valid-update-cache-releaseverify-batch-20260705-032902
reached 565.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_AI_PLOT_VALID_UPDATE_CACHE` candidate scoped an exact per-unit
`AI_plotValid` memo inside `CvUnitAI::AI_update`, invalidating when the unit
area changed, but the release-path median regressed against the accepted
559.0 ms/turn batch. It was removed and the fast wrapper DLL was restored.

candidate-city-can-work-assign-cache-releaseverify-batch-20260705-034300
reached 566.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_CITY_CAN_WORK_ASSIGN_CACHE` candidate cached `CvCity::canWork` once
per `AI_assignWorkingPlots` pass and reused it in repeated
`AI_addBestCitizen` plot scans, but the release-path median regressed against
the accepted 559.0 ms/turn batch. It was removed and the fast wrapper DLL was
restored.

candidate-worker-ownership-before-plotvalid-releaseverify-batch-20260705-040246
reached 566.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_WORKER_OWNERSHIP_BEFORE_PLOTVALID` candidate moved exact
ownership/revealed filters before `AI_plotValid` in worker territory scans, but
the release-path median regressed against the accepted 559.0 ms/turn batch. It
was removed and the fast wrapper DLL was restored.

candidate-emphasize-avoid-angry-single-set-v2-releaseverify-batch-20260705-042730
reached 567.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_EMPHASIZE_AVOID_ANGRY_SINGLE_SET` candidate folded the forced
avoid-angry-citizens emphasis state into the first `AI_setEmphasize` call after
city-emphasis attribution showed repeated reassignment as hot, but the
release-path median regressed against the accepted 559.0 ms/turn batch. It was
removed and the fast wrapper DLL was restored.

candidate-emphasize-skip-unchanged-setter-releaseverify-batch-20260705-080706
reached 565.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_EMPHASIZE_SKIP_UNCHANGED_SETTER` candidate skipped no-op
`AI_setEmphasize` calls when `AI_doEmphasize` already knew the requested value
matched the current emphasis bit, but the release-path median regressed against
the accepted 559.0 ms/turn batch. It was removed and the fast wrapper DLL was
restored.

candidate-emphasize-tail-assign-batch-releaseverify-batch-20260705-091110
reached 567.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_EMPHASIZE_TAIL_ASSIGN_BATCH` candidate deferred repeated
`AI_assignWorkingPlots` calls only after the Great People emphasis decision had
read the current worked plots, but the release-path median regressed against
the accepted 551.0 ms/turn batch. It was removed and the fast wrapper DLL was
restored.

candidate-worker-route-improvement-prefilter-releaseverify-batch-20260705-093539
reached 566.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_WORKER_ROUTE_TERRITORY_IMPROVEMENT_PREFILTER` candidate skipped
`CvPlayer::getBestRoute` during `AI_routeTerritory(true)` for plots whose
current improvement has no positive route-yield change for any route, but the
release-path median regressed against the accepted 551.0 ms/turn batch. It was
removed and the fast wrapper DLL was restored.

candidate-pathcost-single-unit-compare-skip-releaseverify-batch-20260705-094836
reached 573.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_PATHCOST_SINGLE_UNIT_COMPARE_SKIP` candidate skipped worst-unit
comparison checks in `pathCost` when the selection group has exactly one unit,
but the release-path median regressed against the accepted 551.0 ms/turn
batch. It was removed and the fast wrapper DLL was restored.

candidate-pathvalid-group-fact-cache-releaseverify-batch-20260706-084411
reached 566.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_PATHVALID_GROUP_FACT_CACHE` candidate cached `AI_isControlled`,
`canFight`, and `alwaysInvisible` once per `generatePath`, but the
release-path median regressed against the accepted 546.0 ms/turn batch. The
source branch and Fast/ProfileFast build flags were removed, the fast wrapper
was restored to accepted ReleaseFast DLL SHA256
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and
`ReleaseFastVerify` was rebuilt without the flag with DLL SHA256
`e6f87c85094be901af4317bf6cc39f6ecbabf5857f823009e6d9612e8945d8f7`.

candidate-improvement-yield-same-final-fold-releaseverify-batch-20260705-111819
reached 570.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_IMPROVEMENT_YIELD_SAME_FINAL_FOLD` candidate folded duplicate
`calculateImprovementYieldChange` calls inside `AI_getImprovementValue` when
the final improvement was the same as the candidate/current improvement, but
the release-path median regressed against the accepted 551.0 ms/turn batch. It
was removed and the fast wrapper DLL was restored.

candidate-city-yield-static-context-releaseverify-batch-20260705-120502
reached 573.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_CITY_YIELD_STATIC_CONTEXT` candidate hoisted invariant city/player
state into the existing per-scan yield context for `AI_yieldValue`, but the
release-path median regressed against the accepted 551.0 ms/turn batch. The
candidate build flag was removed from the fast build targets, a follow-up
`ReleaseFastVerify` rebuild without the flag succeeded, and the fast wrapper
DLL was restored.

candidate-explore-plotvalid-fast-reject-releaseverify-batch-20260705-123132
reached 567.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_EXPLORE_PLOTVALID_FAST_REJECT` candidate added an exact land/sea
domain and land-area reject before `AI_plotValid` in `AI_explore` and
`AI_exploreRange`, but the release-path median regressed against the accepted
551.0 ms/turn batch. The source hook and fast build flag were removed, a
follow-up `ReleaseFastVerify` rebuild without the flag succeeded, and the fast
wrapper DLL was restored.

candidate-patrol-context-cache-releaseverify-batch-20260705-124952 reached
658.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_PATROL_CONTEXT_CACHE` candidate cached owner/team/group/plot facts
once at the start of `AI_PatrolMove`, but the release-path median regressed
badly against the accepted 551.0 ms/turn batch and turn 112 spiked to
844-858 ms. The cached group/plot facts likely changed decision shape or
introduced stale assumptions across helper calls. The source hook and fast
build flag were removed, a follow-up `ReleaseFastVerify` rebuild without the
flag succeeded, and the fast wrapper DLL was restored.

candidate-active-danger-update-cache-releaseverify-batch-20260705-131105
reached 576.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_ACTIVE_DANGER_UPDATE_CACHE` candidate cached `AI_getAnyPlotDanger`
and `AI_getPlotDanger` results only inside one `CvUnitAI::AI_update`, keyed by
player, plot, range, and test-moves mode, but the release-path median
regressed against the accepted 551.0 ms/turn batch. The scoped lookup and
generation overhead outweighed any danger-scan reuse in the authoritative
save. The source hook and fast build flag were removed, a follow-up
`ReleaseFastVerify` rebuild without the flag succeeded, and the fast wrapper
DLL was restored.

candidate-unit-construct-xml-cache-releaseverify-batch-20260705-132717
reached 563.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_UNIT_CONSTRUCT_XML_CACHE` candidate cached whether each
`(unit type, civilization)` pair has any XML `Buildings` or `ForceBuildings`
entry before entering `AI_construct`, but the release-path median regressed
against the accepted 551.0 ms/turn batch. The tiny `construct_scan` saving did
not pay for the extra cache path/code shape. The source hook and fast build
flag were removed, a follow-up `ReleaseFastVerify` rebuild without the flag
succeeded, and the fast wrapper DLL was restored.

candidate-route-territory-owner-first-releaseverify-batch-20260705-135712
reached 563.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_ROUTE_TERRITORY_OWNER_FIRST` candidate used new
`MNAI_PROFILE_ROUTE_TERRITORY_DETAIL` evidence to check plot owner before
`AI_plotValid` only inside `AI_routeTerritory`, but the no-profiler median
regressed by 12.0 ms/turn against the accepted 551.0 ms/turn batch. The source
branch and fast build flag were removed, a follow-up `ReleaseFastVerify`
rebuild without the flag succeeded, and the fast wrapper DLL was restored.

candidate-improvement-build-list-releaseverify-batch-20260705-143826 reached
568.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_IMPROVEMENT_BUILD_LIST` candidate replaced the repeated full
`GC.getNumBuildInfos()` scan inside `AI_getImprovementValue` with an XML-order
build list grouped by created improvement, while still calling
`CvPlayer::canBuild` for each candidate build. The release-path median
regressed by 17.3 ms/turn against the accepted 551.0 ms/turn batch, so the
source cache, accessors, and Fast/ProfileFast build flags were removed. A
follow-up `ReleaseFastVerify` rebuild without the flag succeeded, and the fast
wrapper DLL was restored.

candidate-improvement-has-build-prefilter-releaseverify-batch-20260706-090609
reached 564.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_IMPROVEMENT_HAS_BUILD_PREFILTER` candidate cached a static
XML-derived answer for whether any build can create each improvement and
skipped `AI_getImprovementValue` only when the improvement was not the plot's
current improvement and no build could ever create it. The release-path median
regressed by 18.0 ms/turn against the accepted 546.0 ms/turn batch, so the
source cache, prefilter, and Verify/ProfileFast build flags were removed. A
follow-up `ReleaseFastVerify` rebuild without the flag succeeded with DLL
SHA256 `b375fb02b65bd5f77d61e2c0b7db8d77d0a36cf008f19ee711a7abf58b760994`,
the fast wrapper DLL was restored to accepted ReleaseFast hash
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and the
original wrapper DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

candidate-hidden-flag-symbol-skip-releaseverify-batch-20260706-092205 reached
567.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_HIDDEN_FLAG_SYMBOL_SKIP` candidate skipped `CvPlot::updateFlagSymbol`
for plots that were not visible to the active team and had no existing flag
entities to destroy. It was visual-only and deterministic, but the release-path
median regressed by 21.7 ms/turn against the accepted 546.0 ms/turn batch, so
the source guard and Verify/ProfileFast build flags were removed. A follow-up
`ReleaseFastVerify` rebuild without the flag succeeded with DLL SHA256
`b36addf46930d8e2ad4bd5672fd08fc1950c87a4c1c1fed3f80d345799b1a8fa`, the fast
wrapper DLL was restored to accepted ReleaseFast hash
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and the
original wrapper DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

fastheur-found-value-cadence-releaseverify-batch-20260706-093734 reached
564.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_FAST_HEURISTIC_FOUND_VALUE_CADENCE` candidate updated city founding
values from `AI_doTurnUnitsPre` only every third staggered player turn unless
the cached city-site list was empty. It avoided RNG changes and was kept behind
an explicit optional fast-heuristic flag, but the release-path median regressed
by 18.7 ms/turn against the accepted 546.0 ms/turn batch, so the source guard
and Verify build flag were removed. A follow-up `ReleaseFastVerify` rebuild
without the flag succeeded with DLL SHA256
`184bcc07154fadf44ffd16bc76ceac5a4842ef934d54991abaa98fa1e06d3272`, the fast
wrapper DLL was restored to accepted ReleaseFast hash
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and the
original wrapper DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

candidate-city-workable-plot-list-releaseverify-batch-20260706-100706 reached
567.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_CITY_WORKABLE_PLOT_LIST` candidate built an exact ascending list of
`canWork` city-radius plots once per `AI_assignWorkingPlots` pass and reused it
inside `AI_addBestCitizen` and `AI_juggleCitizens`, while still recomputing
`AI_plotValue` after each citizen mutation. It preserved tie order and avoided
value caching, but the release-path median regressed by 21.7 ms/turn against
the accepted 546.0 ms/turn batch. The helper, internal overload, and
Verify/ProfileFast build flags were removed. A clean `ReleaseFastVerify`
rebuild without the flag succeeded with DLL SHA256
`ba6f606b4a3cdb9cd3b3db7c73a9faa202534c58390292c386893c44f6e2e3a4`, the fast
wrapper DLL was restored to accepted ReleaseFast hash
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and the
original wrapper DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

candidate-tower-mastery-cpp-releaseverify-batch-20260705-150547 reached
562.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_TOWER_MASTERY_CPP` candidate moved the per-turn `AI_TowerMastery`
Python flag state machine into C++, including the owned mana scan and tower
completion checks. Although ProfileFast player-turn markers showed the Python
hook at 62 ms / 24 calls, the release-path median regressed by 11.3 ms/turn
against the accepted 551.0 ms/turn batch. The source branch and
Fast/ProfileFast build flags were removed, a follow-up `ReleaseFastVerify`
rebuild without the flag succeeded, and the fast wrapper DLL was restored.

candidate-tower-mastery-python-cache-releaseverify-batch-20260706-102245
reached 553.7 ms/turn with `Metric source: autoverify_dll_turn`. The candidate
cached repeated `AI_TowerMastery` XML ID lookups in Python and localized the
map object inside the full mana scan, preserving the same flag, tech, building,
civic, and mana-count decisions. `PythonErr.log` stayed empty, but the
release-path median regressed by 7.7 ms/turn against the accepted 546.0 ms/turn
batch, so the Python change was removed. The fast-wrapper Python file was
restored to SHA256
`7a0b159677ec1462cad97281c4d29909aebb62444d1381e4d97f32929913d78e`, the fast
wrapper DLL was restored to accepted ReleaseFast hash
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and the
original wrapper DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

candidate-pathcost-sticky-movement-cache-releaseverify-batch-20260705-184011
reached 572.3 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_PATHCOST_STICKY_MOVEMENT_CACHE` candidate cached exact single-unit
`CvPlot::movementCost` edge results under the accepted path-valid sticky group
signature, but the lookup/generation overhead regressed the release-path median
by 21.3 ms/turn against the accepted 551.0 ms/turn batch. The source helper and
Fast/ProfileFast build flags were removed, a clean `ReleaseFastVerify` rebuild
without the flag succeeded, and the fast wrapper DLL was restored.

candidate-path-edge-movement-cache-releaseverify-batch-20260706-111401 reached
567.7 ms/turn with `Metric source: autoverify_dll_turn`. This narrower
candidate cached exact single-unit `CvPlot::movementCost` edge results only
inside one `generatePath`, keyed by directed map edge, but it still regressed
by 21.7 ms/turn against the accepted 546.0 ms/turn batch. The repeated result
matches the earlier sticky movement-cost cache: narrow edge-cache lookup and
generation overhead outweighs the saved duplicate movement-cost calls in the
primary save. The helper, path-scope hooks, and Fast/ProfileFast build flags
were removed; all three `PythonErr.log` files were empty; transient
`AutoVerify.ini` was removed; no Civ/Wine/Wineskin process remained; the fast
wrapper DLL was restored to accepted ReleaseFast hash
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`; the
original wrapper DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`; and a
clean `ReleaseFastVerify` rebuild without the flag succeeded with DLL SHA256
`6586198ad608002e1b2847c9c52668d0e981d922622c39b80d9f3fb7e9ceb35f`.

candidate-build-list-by-improvement-releaseverify-batch-20260706-115205
reached 574.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_BUILD_LIST_BY_IMPROVEMENT` candidate pre-indexed XML build actions
by created improvement and used that list inside `AI_getImprovementValue`,
preserving the same `CvPlayer::canBuild` checks for relevant builds while
skipping unrelated build actions. It regressed by 28.7 ms/turn against the
accepted 546.0 ms/turn batch, so the source helper, city call-site, and
Fast/ProfileFast build flags were removed. All three `PythonErr.log` files
were empty; transient `AutoVerify.ini` was removed; no Civ/Wine/Wineskin
process remained; the fast wrapper DLL was restored to accepted ReleaseFast
hash `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`; the
original wrapper DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`; and a
clean `ReleaseFastVerify` rebuild without the flag succeeded with DLL SHA256
`0d788da61dc9f6fdc9692c03129d69983d13dfd066fe314de64633662cbaf19c`.

candidate-city-build-validity-context-releaseverify-batch-20260709-225234
reached 617.7 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_CITY_BUILD_VALIDITY_CONTEXT` candidate replaced the repeated full
build-info scan inside each `AI_getImprovementValue` call with one exact,
plot-scoped legality pass per `AI_bestPlotBuild` call. It preserved build XML
tie order, build-time scoring, feature-removal behavior, and all
`CvPlayer::canBuild` results; a separate 20,000-case randomized equivalence
check also passed. Because the candidate batch was noisy, a clean no-flag
control was rebuilt and measured in
`clean-city-build-validity-context-control-releaseverify-batch-20260709-230206`.
That control reached 580.3 ms/turn, confirming the candidate regressed by
37.4 ms/turn / 6.4% in the same session. The context, internal overload, and
Verify/ProfileFast build flags were removed. All six candidate/control
`PythonErr.log` files were empty, `CivilizationIV.ini` was restored, transient
`AutoVerify.ini` was removed, and the original wrapper stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
The fast wrapper was restored to accepted ReleaseFast hash
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`.
A clean `ReleaseFastVerify` rebuild without the flag succeeded with SHA256
`6b2ca42bf8c8676f50cf11c2d0cfdfe8ba7dac5b8464f214850f6b90d1e23911`.
The candidate batch exposed that direct-Wine `wineserver -k` could leave
wrapper service processes alive; `autoverify_mnai.sh` now falls back to
wrapper-path-scoped TERM/KILL cleanup, and the clean control batch verified no
wrapper Wine process remained.

candidate-tower-mana-owned-plot-service-releaseverify-batch-20260709-231611
was accepted with `Metric source: autoverify_dll_turn`. Its three run averages
were 570.0, 572.7, and 569.7 ms/turn, for a 570.0 ms/turn median. The clean
same-session control batch
`clean-tower-mana-owned-plot-service-control-releaseverify-batch-20260709-232502`
ran at 574.0, 574.7, and 581.3 ms/turn, for a 574.7 ms/turn median. The paired
improvement was 4.7 ms/turn / 0.8%. The older 546.0 ms/turn accepted batch
remains the best absolute result because this session ran slower overall.

`MNAI_OPT_TOWER_MANA_OWNED_PLOT_SERVICE` exposes one narrow Python binding that
counts either of two bonus classes over the accepted `CvPlayerAI` owned-plot
list. `AI_TowerMastery` still resolves the same two XML bonus-class IDs and
keeps the original flag, tech, building, civic, and return-state logic; only
the whole-map `CyMap` traversal is replaced. The native loop re-reads every
owned plot's current `getBonusType(NO_TEAM)`, so bonus changes remain visible
and ownership-cache invalidation remains the existing exact `CvPlot::setOwner`
path. A 20,000-case randomized equivalence check of the old and new counts
also passed. This design is intentionally different from the rejected full
C++ state-machine port and Python XML-ID cache.

The companion attribution batch
`profilefast-tower-mana-owned-plot-service-batch-20260709-233312` measured
589.3 ms/turn with `Metric source: custom_profile`, versus the prior accepted
route-territory attribution batch at 609.7 ms/turn. Median-run
`CvPlayer::doTurn::tower_mastery` moved from 67 ms / 24 calls to 0 ms / 24
calls. Candidate `ReleaseFastVerify` DLL SHA256 was
`6bf1d2bbcb182feb27f136a7eb71ef31f7a8ce32771e0e82cf9890835cb2de6d`;
the clean control Verify rebuild was
`026feb71dceda4a1b31ee1b46a4757f334ac0cb7081e2444e4f28167cbcb4c4f`;
candidate ProfileFast was
`53fea7003f06e7f9ae1f04c6e6c2ff24e6284896eba986f6c177f5f0783c8769`.
The final hook-free player `ReleaseFast` DLL installed in
`More Naval AI Fast.app` is
`e5d983e53234c89b33a35417ab21c293068e78d99ed93bb927fd881ed6205e7b`,
and its matching `CvGameUtils.py` is
`587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c`.
All nine candidate/control/profile `PythonErr.log` files were empty,
`CivilizationIV.ini` was restored, transient `AutoVerify.ini` was removed, no
fast-wrapper Wine process remained, and the control wrapper DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

candidate-pillage-value-before-path-releaseverify-batch-20260709-235143 was
accepted at 564.7 ms/turn (`autoverify_dll_turn`), with run averages 568.7,
564.7, and 564.7. That improves the immediately preceding accepted Tower Mana
batch at 570.0 ms/turn by 5.3 ms/turn / 0.9%. The candidate computes the exact
side-effect-free pillage value and its best possible post-war-devaluation score
before `generatePath`; only candidates that cannot beat the current winner are
skipped. For successful paths the original path-turn division occurs before
the original war-declaration division, preserving integer rounding and strict
greater-than tie selection.

The companion `profilefast-pillage-value-before-path-batch-20260709-235754`
measured 590.3 ms/turn versus 589.3 ms/turn for the preceding ProfileFast batch,
which is noise, but the targeted path-request evidence moved consistently:
`CvUnitAI::AI_pillageRange` fell from 98 calls / 34 strict duplicates / 9-10 ms
per run to 83 calls / 27 duplicates / 8-9 ms. Candidate Verify DLL SHA256 was
`b586f7f4f4db008af0ef9068a197ddb56ca106650e178e483d38a7ad23374479`;
candidate ProfileFast was
`b6adcee1160d7b38e6d9881c66b21462d230bb4056de91b2e118b889e0082d9a`;
the installed hook-free ReleaseFast DLL is
`982d92b4e33f5472e4cbcdf75981055efde50e2c0c1a695cf6a9598c0e730188`.
All six release/profile `PythonErr.log` files were empty, INI state was
restored, transient `AutoVerify.ini` was removed, no fast-wrapper process
remained, and the control wrapper DLL remained unchanged.

profilefast-city-growth-subband-batch-20260710-001733 added three nested
ProfileFast-only bands inside `AI_yieldValue` and measured a 592.0 ms/turn
median. Across all runs, `food_value::growth_capacity` was 8-9 ms/turn and its
nested `good_tiles` scan was 7 ms/turn; `growth_state_adjustment`,
`growth_overrides`, and `growth_value` rounded to 0 ms. This confirms the
remaining measurable growth work is the already-rejected
`AI_countGoodTiles`/`AI_countGoodSpecialists` cache shape, not arithmetic or
state setup. The batch's ProfileFast DLL SHA256 was
`19277bd42bd70b60a0a7e242cc6dc957de7c1ba67c1e7b56008575a9d996641b`.
All three Python error logs were empty and the normal validation gates passed.

candidate-target-city-adjacent-score-before-path-releaseverify-batch-20260710-002629
was rejected at 565.3 ms/turn versus the accepted 564.7 ms/turn batch. Runs
were 565.3, 564.3, and 570.0 ms/turn. The exact candidate evaluated the
deterministic adjacent defense/river/ownership score before pathfinding and
preserved the original path-turn division and strict greater-than tie order,
but it produced no release gain. Candidate Verify DLL SHA256 was
`5e0830358f6a51448f32bfd53f9a05827e639f713f3a716bdb9286a29280f71c`.
The optimization and test flags were removed; a clean Verify rebuild succeeded
at `87bcd6f0dcf5fbb9f68bbb071a27166b2ce244dcc8c92bb20e648cbbdff65379`.
The ProfileFast-only `AI_goToTargetCity` path context remains for future
attribution. All three Python error logs were empty, INI state was restored,
transient `AutoVerify.ini` was removed, no fast-wrapper process remained, and
the player wrapper was restored to
`982d92b4e33f5472e4cbcdf75981055efde50e2c0c1a695cf6a9598c0e730188`.

candidate-explore-context-hoist-releaseverify-batch-20260710-004507 was
rejected at 566.3 ms/turn versus the accepted paired 564.7 ms/turn batch
(+1.6 ms / +0.3%). Runs were 566.0, 566.3, and 566.3 ms/turn, with
`Metric source: autoverify_dll_turn`. ProfileFast evidence immediately before
the candidate put `AI_exploreMove` at 72-73 ms/turn and showed tens of
thousands of plot/reveal checks. The exact candidate therefore hoisted stable
team, owner, group, coordinate, and domain reads once per `AI_explore` /
`AI_exploreRange` call, without changing iteration order, RNG consumption,
path calls, scoring, or ties. It did not improve release timing, so source and
`MNAI_OPT_EXPLORE_CONTEXT_HOIST` were removed without a candidate ProfileFast
run. Candidate Verify DLL SHA256 was
`e08f383b2cd8b06ad6d50ccd25f8cc81dde70fca65c5bd608835d23d1f7e269a`;
the clean no-candidate Verify rebuild was
`e1b4a01dfbb03067d262b1e411874f8d33c8a7943152490f8e3ebf0ab68d6ec8`.
The ignored backup path is
`profiling/dll-backups/explore-context-hoist-20260710-004459`. All three
`PythonErr.log` files were empty, each run's `CivilizationIV.ini.before`
matched the restored INI, transient `AutoVerify.ini` was absent, no wrapper
Wine process remained, the control DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
the playable fast wrapper was restored to ReleaseFast SHA256
`982d92b4e33f5472e4cbcdf75981055efde50e2c0c1a695cf6a9598c0e730188`.

profilefast-pathvalid-order-attribution-batch-20260710-010058 added
ProfileFast-only reject markers around the existing `pathValid` danger and
movement-legality exits. It measured 589.7 ms/turn; all three runs reported
1,528 danger rejects, 528 move-or-attack rejects, and 54,912 move-through
rejects. The enclosing danger band ran 168,472 times / 36 ms while movement
legality ran 166,944 times / 26 ms. Attribution DLL SHA256 was
`5a039ffb976968178e108d44ca367239bf19d90f6cdfe22f076192fdd3e85cb3`.

candidate-pathvalid-move-before-danger-releaseverify-batch-20260710-010843
was accepted at 565.7 ms/turn against same-session clean control
clean-pathvalid-move-before-danger-control-releaseverify-batch-20260710-011521
at 570.7 ms/turn, a paired improvement of 5.0 ms/turn / 0.9%. Candidate runs
were 570.0, 565.0, and 565.7 ms/turn; control runs were 573.0, 570.7, and
564.3 ms/turn. The exact candidate runs cached movement legality before the
danger predicate, so nodes rejected by `canMoveThrough` /
`canMoveOrAttackInto` no longer perform danger work. Both original predicates
still must pass; iteration order, RNG, path scoring, and gameplay decisions are
unchanged. Candidate Verify DLL SHA256 was
`8af0e5a0f78b0d4f8c591abdf5c056e35f02df54cc13041e5583fe460c8da0b5`;
clean-control Verify was
`1c7f8a9078b1f6f5a7ac8f8751b03108111be4e0c2f0fb0aaa8134f946866ccc`.

profilefast-pathvalid-move-before-danger-batch-20260710-012207 confirmed the
accepted movement at 587.0 ms/turn versus the 589.7 ms/turn attribution batch.
Danger-band calls fell by 55,552 / 33.0%, from 168,472 to 112,920, with time
falling from 36 ms to 24-25 ms; `CvSelectionGroup::generatePath()` fell from
164 ms to 149 ms in the median-profile comparison. Accepted ProfileFast DLL
SHA256 was
`91c306d6e7a2db073c2855acb4b01cd51d5ba71a94996158e5031a6ca36aec34`.
All twelve attribution/candidate/control/profile `PythonErr.log` files were
empty across the four relevant batches, every run restored its exact INI
backup, transient `AutoVerify.ini` was removed, no fast-wrapper Wine process
remained, and the control DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
The installed player-facing, profiler-free ReleaseFast DLL SHA256 is
`afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.

profilefast-construct-path-context-batch-20260710-014010 added a
ProfileFast-only path-request context at `CvUnitAI::AI_construct` and measured
591.3 ms/turn. Each run attributed 142 successful paths / 127 unique requests
/ 15 strict duplicates and 18-19 ms to the caller; `AI_construct` itself was
20-21 ms / 36 calls. Attribution ProfileFast DLL SHA256 was
`4fe71c0645b2a42cec4b70aacf8f2f6c23120af38ab81594042f1bd0eab5199d`.

candidate-construct-eligibility-before-path-releaseverify-batch-20260710-014906
was rejected at 571.7 ms/turn versus the accepted paired candidate at 565.7
ms/turn and the same-session clean control at 570.7 ms/turn. Runs were 570.7,
571.7, and 573.7 ms/turn. The exact candidate scanned the unit's
civilization-specific building actions before pathfinding and skipped only
cities with no existing construct-count contribution and no currently
constructible unit-provided building. Retained cities still executed the
original path, count, building-value, and strict tie logic. The duplicate
eligibility scan outweighed any avoided paths, so
`MNAI_OPT_CONSTRUCT_ELIGIBILITY_BEFORE_PATH` and its Verify flag were removed;
the ProfileFast path context remains. Candidate Verify DLL SHA256 was
`06cfb3a692b9ba340f0971e19d145f60ff3deec82a6eb8a634ec18328284d2e6`;
clean no-candidate Verify rebuilt at
`e445b02982f350de867a0f7ab29f5bdaba985c3cb8cd63b99a9cee2546a4b928`.
The ignored backup path is
`profiling/dll-backups/construct-eligibility-before-path-20260710-014854`.
All six attribution/candidate Python error logs were empty, INI state was
restored exactly, transient `AutoVerify.ini` was absent, no wrapper Wine
process remained, the control DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
the playable fast wrapper was restored to ReleaseFast SHA256
`afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.

candidate-pathcost-single-unit-path-context-releaseverify-batch-20260710-020716
was rejected at 568.3 ms/turn versus the accepted paired candidate at 565.7
ms/turn (+2.6 ms / +0.5%). Runs were 566.7, 578.3, and 568.3 ms/turn. The
exact candidate extended the existing per-`generatePath` callback context with
the single head-unit pointer, allowing `pathCost` to skip repeated linked-list
head, unit-map, group-size, and next-node lookups for single-unit groups while
preserving the original multi-unit loop and every cost calculation. It did not
improve the release median, so `MNAI_OPT_PATHCOST_SINGLE_UNIT_PATH_CONTEXT`
and its Verify flag were removed and the original callback source was
byte-restored. Candidate Verify DLL SHA256 was
`6f27f5f935234bd90467a6445dc5c207c385425e29e680b2441662de4cb598f0`;
clean no-candidate Verify rebuilt at
`ec43a3bd0221b28b022ec71c53e8ddb01f8f21a42f99189080a6b01a4286ea99`.
The ignored backup path is
`profiling/dll-backups/pathcost-single-unit-path-context-20260710-020708`.
All three Python error logs were empty, the exact INI backup was restored,
transient `AutoVerify.ini` was absent, no wrapper Wine process remained, the
control DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
the fast wrapper was restored to player ReleaseFast SHA256
`afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.

profilefast-canbuild-order-batch-20260710-022517 added ProfileFast-only exit
counters to `CvPlayer::canBuild` and measured 597.0 ms/turn. All three runs
had the same distribution across 55,521 calls: 41,130 tech-prerequisite
rejects, 5,084 plot-legality rejects, 68 feature-removal-tech rejects, zero
gold rejects, and 9,239 accepted calls. The parent marker rose to 50-51 ms
because of counter overhead; compare decision counts rather than that timing
to the prior uninstrumented 41-42 ms sample. The evidence supports the existing
tech-first order and rules out moving feature-tech or gold ahead of plot
legality, so no release candidate was built. ProfileFast DLL SHA256 was
`8c25e63e42a1aef6b3eca04cbd7c4492011b7d84fe21bae049947e204e3b5853`;
the ignored backup path is
`profiling/dll-backups/profilefast-canbuild-order-20260710-022507`.
All three Python error logs were empty, each run restored its exact INI backup,
transient `AutoVerify.ini` was absent, no wrapper Wine process remained, the
control DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
the fast wrapper was restored to player ReleaseFast SHA256
`afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.

candidate-upgrade-list-precompute-releaseverify-batch-20260706-121552 reached
569.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_UPGRADE_LIST_PRECOMPUTE` candidate precomputed the sorted
available-upgrade unit list per civilization/source-unit at XML cache init
time, then reused it in `CvUnit::getUpgradeCity(bool)` instead of rebuilding
and sorting the same list during turn processing. It preserved the existing
candidate order and city legality checks, but regressed by 23.0 ms/turn
against the accepted 546.0 ms/turn batch, so the source cache, accessor,
call-site, and Fast/ProfileFast build flags were removed. All three
`PythonErr.log` files were empty; transient `AutoVerify.ini` was removed; no
Civ/Wine/Wineskin process remained; the fast wrapper DLL was restored to
accepted ReleaseFast hash
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`; the
original wrapper DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`; and a
clean `ReleaseFastVerify` rebuild without the flag succeeded with DLL SHA256
`80e28e7a4eeb5495a9f5ef5ad2397f5116ae9252509f47c299040aa875fc435b`.

candidate-city-emphasize-batch-assign-releaseverify-batch-20260705-191906
reached 582.0 ms/turn with `Metric source: autoverify_dll_turn`. The
`MNAI_OPT_CITY_EMPHASIZE_BATCH_ASSIGN` candidate deferred repeated
`AI_setEmphasize` citizen assignment inside `AI_doEmphasize` and reassigned
once after all emphasis flags were updated. It regressed by 31.0 ms/turn
against the accepted 551.0 ms/turn batch, likely because intermediate
assignment state feeds later emphasis decisions and special-yield multiplier
state. The helper and Fast/ProfileFast build flags were removed, a clean
`ReleaseFastVerify` rebuild without the flag succeeded, and the fast wrapper
DLL was restored.

profilefast-conquest-detail-batch-20260710-024002 retained
`MNAI_PROFILE_CONQUEST_DETAIL` stage scopes and measured 595.7 ms/turn with
`Metric source: custom_profile`. Runs were 596.3, 595.7, and 590.7 ms/turn.
The median run attributed 15 ms / 12 calls to
`CvUnitAI::AI_ConquestMove::pick_target_city`, 4 ms / 12 calls to
`late_fallback`, and zero rounded milliseconds to every other measured stage.
Its ProfileFast DLL SHA256 was
`9594324b32c1a4a906b8959515ddff51b575dd7751afeb880fcfee822c0c2482`;
the ignored player backup is
`profiling/dll-backups/profilefast-conquest-detail-20260710-023959`.

profilefast-pick-target-detail-batch-20260710-024651 added retained
ProfileFast-only `MNAI_PROFILE_PICK_TARGET_CITY_DETAIL` counters and measured
590.0 ms/turn (590.0, 593.7, 587.0). Every run recorded 52
`AI_pickTargetCity` path attempts and 44 successes across all callers, with
zero new-war candidates and therefore zero new-war insufficient-stack exits.
The conquest context separately recorded 12 successful, unique, reuse-enabled
requests per turn, 36 total over turns 111-113, costing about 15 ms. This rules
out the contemplated new-war-before-path candidate on the fixed save; a general
reorder would also have to preserve randomized target-value calls across path
failures. ProfileFast DLL SHA256 was
`4a719755bd4e74b314a4e162bfe80ffc79e31f0229163f7dca078902960acd15`;
the ignored backup is
`profiling/dll-backups/profilefast-pick-target-detail-20260710-024648`.

candidate-city-plot-static-service-releaseverify-batch-20260710-025747 was
rejected at 569.0 ms/turn with `Metric source: autoverify_dll_turn`. Runs were
569.0, 570.3, and 564.3 ms/turn. `MNAI_OPT_CITY_PLOT_STATIC_SERVICE` built one
exact, assignment-scoped snapshot of current/final plot yields, improvement
chain, bonus-discovery constants, and upgrade-time constants, while continuing
to recompute `AI_yieldValue` after every citizen addition/removal. Candidate
Verify DLL SHA256 was
`4cb8fdfffe4396d04f57a89d75243323916c5a26a1d9c6a334d69636cf74f560`.

The paired flag-off
clean-city-plot-static-service-control-releaseverify-batch-20260710-030352
measured 567.0 ms/turn (564.3, 567.0, 568.3), making the candidate 2.0 ms/turn
slower. The context, overload plumbing, and test flag were removed, and
`CvCityAI.cpp` / `CvCityAI.h` were byte-restored. The fully reverted clean
`ReleaseFastVerify` rebuild SHA256 was
`8adc0df26e377c365bb63a34184534d86bf175dcfaf9c759b29ff8263a4d0601`.
Ignored backups are
`profiling/dll-backups/city-plot-static-service-20260710-025743`,
`profiling/dll-backups/clean-control-city-plot-static-service-20260710-030345`,
and
`profiling/dll-backups/restore-player-after-city-plot-static-service-20260710-031222`.
All 12 logs across the two profile and two release batches had empty
`PythonErr.log`; every saved INI matched restored `CivilizationIV.ini`;
transient `AutoVerify.ini` was absent; no test-wrapper Wine process remained;
the control DLL remained
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`;
and the player wrapper was restored to hook-free ReleaseFast SHA256
`afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.

profilefast-pathvalid-signature-detail-batch-20260710-032035 added a retained
ProfileFast-only timer around the exact group/unit-state signature used by the
sticky path-valid movement cache. It measured 595.3 ms/turn with
`Metric source: custom_profile`; runs were 598.7, 589.7, and 595.3. Every run
computed 3,217 signatures, but their aggregate rounded to 0 ms. The same runs
recorded 3,271 path requests totaling 115-117 ms, so replacing the hash with a
broad mutation-generation invalidation scheme has no supported payoff.
ProfileFast DLL SHA256 was
`cd2aae651698564b7bb663789a71d6d253a9fc33475da04de3d00d565b613aa1`.
Ignored backups are
`profiling/dll-backups/profilefast-pathvalid-signature-detail-20260710-032030`
and
`profiling/dll-backups/restore-player-after-pathvalid-signature-profile-20260710-032258`.
All three `PythonErr.log` files were empty, each INI backup matched restored
state, transient `AutoVerify.ini` was absent, no fast-wrapper Wine process
remained, the control DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
the player wrapper was restored to ReleaseFast SHA256
`afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.

profilefast-cross-context-paths-batch-20260710-033034 added
`path_request_cross_context.csv` to the retained ProfileFast path-request
ledger and measured 589.0 ms/turn (`custom_profile`; runs 587.7, 589.0,
591.7). The new exact request identity is owner/group/from/to/flags/reuse,
reported for every pair of caller contexts that touched it during the same
turn. Each run found 103 shared keys between `AI_exploreRange:4` and
`AI_explore:4`, 93 between `groupAttack:0` and `groupPathTo:0`, and smaller
worker/tactical overlaps. These are turn-level upper bounds because the same
identity can recur after mutable pathfinder or unit state changes; the full
explore caller's requests already averaged about 1 microsecond. No whole-path
cache was built. ProfileFast DLL SHA256 was
`bf2dc85a52420000623f4c23efebf6acdaa35ba9674bb3e2d4ed1627b57993aa`;
ignored backup:
`profiling/dll-backups/profilefast-cross-context-paths-20260710-033028`.

candidate-worker-owned-plot-service-releaseverify-batch-20260710-034041 was
accepted at 567.0 ms/turn with `Metric source: autoverify_dll_turn`; runs were
563.0, 573.0, and 567.0. The paired flag-off
clean-worker-owned-plot-service-control-releaseverify-batch-20260710-034650
measured 567.3 ms/turn (567.3, 565.0, 568.0), a 0.3 ms/turn median edge.
`MNAI_OPT_WORKER_OWNED_PLOT_SERVICE` uses the existing map-index-ordered,
ownership-invalidated `AI_getOwnedPlots()` list in `AI_irrigateTerritory` and
`AI_connectBonus`, both of which retain their original owner and all downstream
legality/mission/path/scoring checks. Candidate Verify SHA256 was
`a954283a72e24d9548ad29ebb6facbe8f1bbfc3b892607dd34cda686f9bb44a8`;
paired control was
`8551f6116b1425704a1ecbc6a819a405ec98617e08adf6c9bb836ad0fb2a29fd`.

profilefast-worker-owned-plot-service-batch-20260710-035300 confirmed the
targeted movement at a 587.7 ms/turn median (584.3, 587.7, 588.7). Relative to
the cross-context profile median, `AI_irrigateTerritory` and
`AI_connectBonus` each fell from 9 ms / 26 calls to 0 ms / 26 calls,
`AI_workerMove` and dispatch fell 70→51 ms, and total `AI_plotValid` calls fell
384,830→254,552. ProfileFast DLL SHA256 was
`a2a67e982999a60cd4d3f0bea471ffe2d87546e42bf5e2755e0898cfd922a250`.
The installed player-facing ReleaseFast SHA256 is
`302d0f39dc162ec5b5c37443c7cd0f6c697a6f4327ab891fe98dcca23d302c59`;
matching wrapper `CvGameUtils.py` remains
`587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c`.
All 12 attribution/candidate/control/profile logs had empty `PythonErr.log`,
every INI backup matched restored state, transient `AutoVerify.ini` was absent,
no fast-wrapper Wine process remained, and the control DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

profilefast-improve-bonus-mode-batch-20260710-040628 added retained
ProfileFast-only mode/ownership counters around `AI_improveBonus` and measured
587.7 ms/turn (`custom_profile`; runs 591.3, 585.3, 587.7). Every run recorded
29 `bInsideBordersOrCurrentPlot` calls, zero unrestricted-outside calls,
Advanced Tactics disabled, 74,240 plot-filter visits, 811 eligible owned plots,
and zero eligible unowned plots. `AI_improveBonus` was 16 ms / 29 calls.
Attribution ProfileFast DLL SHA256 was
`9cbc9c619ee2ee6cfb55ad7054a5b4219b5e08f81f9f9d6854a585a5c09d2c8a`;
ignored backup:
`profiling/dll-backups/profilefast-improve-bonus-mode-20260710-040620`.

candidate-improve-bonus-owned-plot-service-releaseverify-batch-20260710-041346
was accepted at 567.0 ms/turn with `Metric source: autoverify_dll_turn`; runs
were 567.0, 565.3, and 568.0. Paired flag-off
clean-improve-bonus-owned-plot-service-control-releaseverify-batch-20260710-041953
measured 568.0 ms/turn (568.0, 571.7, 566.0), so the candidate improved the
median by 1.0 ms/turn and mean by 1.8 ms/turn. Candidate Verify DLL SHA256 was
`e4ef1288388f04c25e1e9935e9b610ed671ce6043c2cd9c66caf80c016bc859d`;
control Verify DLL SHA256 was
`115942d048f2bd3b9f9bf3d9b966be6452be5fffc10f1943d582ec8c375674d9`.

`MNAI_OPT_IMPROVE_BONUS_OWNED_PLOT_SERVICE` uses the existing ordered,
ownership-invalidated plot service whenever arbitrary outside-border plots are
impossible. With Advanced Tactics enabled in inside/current mode, it merges the
one permitted unowned current plot at its original map index; unrestricted
outside mode retains the old full scan. All candidate filters and scoring are
still rechecked.

profilefast-improve-bonus-owned-plot-service-batch-20260710-042602 confirmed
the expected movement at a 583.7 ms/turn median (583.7, 585.0, 578.0).
`AI_improveBonus` fell 16→5 ms, plot-filter visits fell 74,240→1,710, and
worker dispatch / `AI_workerMove` fell 53→41 ms. ProfileFast DLL SHA256 was
`0fa14f8783945b6a66370988579feb8e67328bcc94a22f6f55f2bba0f5e3e91e`.
The final installed player ReleaseFast SHA256 is
`898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`;
wrapper `CvGameUtils.py` remains
`587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c`.
All 12 logs had empty `PythonErr.log`, every INI backup matched restored state,
transient `AutoVerify.ini` was absent, no wrapper process remained, and the
control DLL stayed at
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

profilefast-fort-mode-batch-20260710-044342 retained ProfileFast-only mode and
ownership attribution for `AI_fortTerritory` and measured 587.7 ms/turn
(587.0, 587.7, 590.0). All 32 calls had Advanced Tactics disabled. The routine
made 81,920 `AI_plotValid` checks and reached 656 owned eligible plots; its
instrumented total was 23 ms. ProfileFast DLL SHA256 was
`d783d618085401bffa75eb5c96b03efc9534da18a5b0328d4f41779e99d761a2`.

candidate-fort-owned-plot-service-releaseverify-batch-20260710-045054 tested
an exact conditional use of the map-index-ordered owned list, retaining the
original full scan whenever Advanced Tactics is enabled. It was rejected at
568.7 ms/turn (568.7, 566.0, 569.7; mean 568.1) versus paired flag-off
clean-fort-owned-plot-service-control-releaseverify-batch-20260710-045702 at
564.0 ms/turn (564.0, 564.3, 564.0; mean 564.1). Candidate Verify SHA256 was
`2fe15da79f0a8d46f10295e050cd0b6483028707095e8b7dfb930484159185f1`;
control Verify SHA256 was
`c302c9f56922e18273809532cb5e56bd01eccf38a3cbf4a1c43baaa0140719eb`.
The candidate and flag were removed, and the player wrapper was restored to
hook-free ReleaseFast SHA256
`898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

profilefast-current-yield-attribution-batch-20260710-060653 measured
604.3 ms/turn (`custom_profile`; runs 604.3, 612.0, 594.3) after adding
ProfileFast-only scopes around the reusable current-plot half of
`AI_getImprovementValue::yield_diff_loop`. Each run recorded 2,196
current-nature scopes and 948 current-improvement scopes; both rounded to
0 ms. The complete yield-difference loop was 12 ms / 732 calls, while
`AI_bestPlotBuild::improvement_value_loop` remained 70-73 ms / 1,159 calls.
ProfileFast DLL SHA256 was
`6a2858c3d179d0aafb95c69179e458882bba6e2332fec594747ea62c40c7f385`.
No release candidate was justified because the reusable work was below timer
resolution and could not account for the dominant loop. All three
`PythonErr.log` files were empty, each INI backup matched restored state,
transient `AutoVerify.ini` was absent, no wrapper process remained, and the
player DLL was restored to hook-free ReleaseFast SHA256
`898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

profilefast-explore-sequence-overlap-batch-20260710-061846 measured
594.0 ms/turn (`custom_profile`; runs 600.0, 589.0, 594.0). A retained
ProfileFast-only sequence ledger recorded plots visited by
`AI_exploreRange(3)` and revisited by the later full `AI_explore()` call in the
same `AI_exploreMove`. Every run found 6,942 overlapping plots, 1,464 that
again passed `AI_plotValid`, and 362 that repeated adjacent fog/contact
scoring. The overlap-specific counters rounded to 0 ms; the entire full-search
adjacent scan was only 1-2 ms / 1,800 calls. ProfileFast DLL SHA256 was
`4dfd33efa7169c4a0619bd8f6d7bc49d4b2adede21712a6156e23bf3ecf466cf`.
No release cache was built: the measured eligibility/scoring ceiling is below
timer resolution, and exact path reuse cannot cross the intervening pillage
search without a complete mutable group/FAStar state key. All three
`PythonErr.log` files were empty, every INI backup matched restored state,
transient `AutoVerify.ini` was absent, no wrapper process remained, and the
player DLL was restored to hook-free ReleaseFast SHA256
`898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

profilefast-canbuild-turn-duplicates-batch-20260710-062751 measured
597.0 ms/turn and added a ProfileFast-only turn ledger keyed by player, plot,
build, era-test, and visibility-test. Every run recorded 37,588 first requests
and 17,933 duplicate identities among 55,521 `canBuild` calls. ProfileFast DLL
SHA256 was
`25b893701097d0d69ac3f3034326fb7405950932a007859b7376754f1c550916`.

profilefast-canbuild-caller-duplicates-batch-20260710-063521 refined the same
17,933 duplicates at a 591.7 ms/turn median: 3,925 occurred under
`AI_bestPlotBuild`, 130 under unit-AI contexts, and 13,878 under other callers.
ProfileFast DLL SHA256 was
`5857cf8e5756ecb1a1a6e36b79b03d8cb792684f6489e21bb958b79f189d556c`.

candidate-assign-best-route-service-releaseverify-batch-20260710-064425
tested a lazy generation-stamped `getBestRoute(plot)` cache scoped to one
`AI_assignWorkingPlots`. It measured 569.0 ms/turn (572.0, 569.0, 566.7;
mean 569.2) versus paired flag-off
clean-assign-best-route-service-control-releaseverify-batch-20260710-065038 at
571.0 ms/turn (573.3, 571.0, 566.7; mean 570.3). Candidate/control DLL SHA256
values were
`a42a61989eecec6df5b5766f083771c92c94c3960d3b71c2ad7f799a484f4284` and
`c7d0122c2844cc9a54cfd6e6b6f88cf7af411c9e4d7e9db3ebe409debed43c5c`.

The candidate was rejected because
profilefast-assign-best-route-service-batch-20260710-065641 showed no expected
movement at a 591.7 ms/turn median: `getBestRoute` remained 39 ms / 27,655
calls, `canBuild` remained 69-70 ms / 55,521 calls, assignment remained 89 ms,
and `AI_plotValue` remained 75 ms. Candidate ProfileFast SHA256 was
`a4e2eb75cc80fa56512b29f90feebc42c53f51523a648f7f654f8c732f21c7ae`.
The service and flags were removed; the identity/caller ledger remains. All 15
logs were clean, INI state was restored, no transient config/process remained,
and the player wrapper returned to hook-free ReleaseFast SHA256
`898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

profilefast-canbuild-updatebest-refinement-batch-20260710-070757 measured
605.0 ms/turn and found no duplicate bucket under the added
AI_updateBestBuild or AI_plotValue contexts. ProfileFast SHA256 was
3702abd38e04dde70231f0cdaf13d4ecd0c4f4fbae65984a5c4cc6acd5e83587.
profilefast-canbuild-mapscan-refinement-batch-20260710-071553 then attributed
only 208 duplicate identities to CvPlayer::countUnimprovedBonuses (no city
improvable-bonus bucket); its noisy median was 692.0 ms/turn and SHA256 was
dbb77d98c7b202d8f138d3b6020bc14dff89351fb858dcfa1e2de7838cca27bf.

profilefast-canbuild-leaf-refinement-batch-20260710-072428 provided the useful
split at a 606.3 ms/turn median. Of 17,933 duplicate request identities,
17,183 passed through CvPlayer::getBestRoute, 446 remained directly under
AI_bestPlotBuild, 208 came from countUnimprovedBonuses, and 96 passed through
CvUnit::canBuild. ProfileFast SHA256 was
347c08910a38405b585be01f2329dc5346a2f3aaa9bde96b690fc135a322af81.

MNAI_OPT_BEST_PLOT_ROUTE_SERVICE therefore tested one exact route result per
complete AI_bestPlotBuild plot evaluation, a broader scope than the rejected
per-improvement context. profilefast-best-plot-route-service-batch-20260710-073441
confirmed the expected movement at 593.7 ms/turn: getBestRoute fell from
27,655 to 22,526 calls and about 39 to 30-32 ms, canBuild fell from 55,521 to
52,709 calls and about 71 to 63 ms, and AI_bestPlotBuild fell from about 85 to
75 ms. Candidate ProfileFast SHA256 was
c354a9d0bd19fef8d6e6419c71f1a1d4f8e763580adbc5db09473cd773d9e93a.

candidate-best-plot-route-service-releaseverify-batch-20260710-074054 was
still rejected at 571.7 ms/turn (576.0, 569.0, 571.7; mean 572.2) versus
paired flag-off clean-best-plot-route-service-control-releaseverify-batch-20260710-074706
at 567.7 ms/turn (576.3, 567.7, 565.3; mean 569.8). Candidate/control Verify
SHA256 values were
189ff6cff4bd397d3c0a2540aa181f53fd547b25bcf63e7520ead0646c03466d and
7a660cfdc1b228de7e5ef7ea1d012e9479217055929e3c60f3844d63e7566117.
The service, overloads, and flags were removed; do not retry either this
plot-scoped form or the older per-improvement hoist without a materially
different design. All 18 run logs had empty PythonErr.log, every INI was
restored, transient AutoVerify.ini was absent, no wrapper process remained,
and the control DLL stayed
11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6.
The final hook-free ReleaseFast rebuild installed in the player wrapper is
747e84bf21525cb3083ebf65dd83c5e9b0c84d51b9aeec74ddc7d1ba0d18a501.
```
