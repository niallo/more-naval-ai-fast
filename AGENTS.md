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

The latest accepted local benchmark used a fixed local save over turns 111-113.
The save and raw run logs are intentionally not committed.

```text
Baseline median: 848.7 ms/turn
Best recorded no-profiler release-path median: 163.3 ms/turn
Delta: -685.4 ms/turn / -80.8%
Best recorded accepted release batch:
profiling/autoverify/candidate-contiguous-turn-updates-releaseverify-batch-20260710-211556
Latest accepted ReleaseFastVerify runs: 163.3, 162.3, 172.0 ms/turn
Latest accepted median versus previous documented best: -382.7 ms/turn / -70.1%
Matching trace baseline: 582.0 ms/turn
Matching trace candidate: 170.7 ms/turn
Matching trace delta: -411.3 ms/turn / -70.7%
Installed player-facing ReleaseFast DLL SHA256: 9cbfb4cc162bd145d1533e1c63fd17ace94524cd11a3e0a090099eaf6ad82e1d
Installed fast-wrapper CvGameUtils.py SHA256: 587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c
Note: equivalent VC++ 2003/Wine rebuilds are not byte-stable, so benchmark
batch paths and source flags are the authoritative performance evidence.
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

Important caveat: older benchmark notes mention a 652.3 ms/turn profile-DLL
result that mixed gameplay-code optimizations with profiler-overhead
reductions. Use `ReleaseFastVerify` for no-profiler release-path timing and
`ProfileFast` for hotspot attribution so player-visible speed and profiler
overhead stay separate.

The current result clears Gate A (400 ms), Gate B (300 ms), Gate C
(212.2 ms), and Gate D (169.7 ms) on the primary fixed save. It does so by
removing idle callback round trips, not by reducing AI search quality or
changing decisions. Gate E (127.3 ms) remains a stretch target for future
CPU-side service work.

Milestone 1 measured targets:

```text
ProfileFast batch: profiling/autoverify/profilefast-private-batch-20260704-134517
ProfileFast median: 752.0 ms/turn
ProfileFast metric source: custom_profile

ReleaseFastVerify batch: profiling/autoverify/releasefastverify-restored-batch-20260704-144657
ReleaseFastVerify median: 593.0 ms/turn
ReleaseFastVerify metric source: autoverify_dll_turn
Accepted city-yield context batch: profiling/autoverify/candidate-city-yield-delta-context-releaseverify-batch-20260705-012951
Accepted city-yield context median: 570.7 ms/turn
Accepted city-yield context metric source: autoverify_dll_turn
Accepted city-yield context delta vs restored release path: -22.3 ms/turn / -3.8%
Accepted no-bonus vote-source-first batch: profiling/autoverify/candidate-nobonus-votesource-first-releaseverify-batch-20260705-015446
Accepted no-bonus vote-source-first median: 567.0 ms/turn
Accepted no-bonus vote-source-first metric source: autoverify_dll_turn
Accepted no-bonus vote-source-first delta vs previous accepted release path:
-3.7 ms/turn / -0.6%
Accepted attack-odds-before-path batch: profiling/autoverify/candidate-attack-odds-before-path-releaseverify-batch-20260705-022039
Accepted attack-odds-before-path median: 563.7 ms/turn
Accepted attack-odds-before-path metric source: autoverify_dll_turn
Accepted attack-odds-before-path delta vs previous accepted release path:
-3.3 ms/turn / -0.6%
Accepted pathCost single-unit next-skip batch: profiling/autoverify/candidate-pathcost-single-unit-next-skip-releaseverify-batch-20260705-030818
Accepted pathCost single-unit next-skip median: 559.0 ms/turn
Accepted pathCost single-unit next-skip metric source: autoverify_dll_turn
Accepted pathCost single-unit next-skip delta vs previous accepted release path:
-4.7 ms/turn / -0.8%
Accepted path-valid sticky update-cache batch: profiling/autoverify/candidate-pathvalid-sticky-update-cache-releaseverify-batch-20260705-083907
Accepted path-valid sticky update-cache median: 551.0 ms/turn
Accepted path-valid sticky update-cache metric source: autoverify_dll_turn
Accepted path-valid sticky update-cache delta vs previous accepted release path:
-8.0 ms/turn / -1.4%
Accepted route-territory owned-plot cache batch: profiling/autoverify/candidate-route-territory-owned-plot-cache-releaseverify-batch-20260705-193825
Accepted route-territory owned-plot cache median: 546.0 ms/turn
Accepted route-territory owned-plot cache metric source: autoverify_dll_turn
Accepted route-territory owned-plot cache delta vs previous accepted release path:
-5.0 ms/turn / -0.9%
Accepted Tower Mana owned-plot service batch: profiling/autoverify/candidate-tower-mana-owned-plot-service-releaseverify-batch-20260709-231611
Accepted Tower Mana owned-plot service median: 570.0 ms/turn
Contemporaneous clean control batch: profiling/autoverify/clean-tower-mana-owned-plot-service-control-releaseverify-batch-20260709-232502
Contemporaneous clean control median: 574.7 ms/turn
Accepted Tower Mana owned-plot service paired delta: -4.7 ms/turn / -0.8%
Accepted Tower Mana ProfileFast batch: profiling/autoverify/profilefast-tower-mana-owned-plot-service-batch-20260709-233312
Accepted Tower Mana ProfileFast median: 589.3 ms/turn
Accepted Tower Mana ProfileFast delta vs prior accepted attribution batch:
-20.4 ms/turn / -3.3%

City-assignment detail batch: profiling/autoverify/profilefast-cityassign-detail-batch-20260704-163950
City-assignment detail median: 736.7 ms/turn
City-assignment detail metric source: custom_profile
Key median-run samples: CvCityAI::AI_plotValue 204 ms / 14703 calls,
CvCityAI::AI_addBestCitizen::plot_scan 193 ms / 1073 calls,
CvCityAI::AI_assignWorkingPlots::add_population 170 ms / 247 calls,
CvCityAI::AI_assignWorkingPlots::juggle_citizens 54 ms / 247 calls.

Path-request attribution batch: profiling/autoverify/profilefast-path-requests-batch-20260705-002808
Path-request attribution median: 653.0 ms/turn
Path-request attribution metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_PATH_REQUESTS, ProfileFast only.
Median-run path_request_callers.csv summary: 3107 generatePath calls,
181 ms attributed in the path-request logger, 252 strict duplicate calls
under owner/group/from/to/flags/reuse keys (8.1%). Top median-run buckets:
CvUnitAI::AI_anyAttack flags 0: 55 ms / 298 calls / 87 duplicate calls;
CvUnitAI::AI_update flags 0: 29 ms / 144 calls / 12 duplicate calls;
CvUnitAI::AI_exploreRange flags 4: 16 ms / 664 calls / 27 duplicate calls;
CvUnitAI::AI_cityAttack flags 0: 16 ms / 68 calls / 24 duplicate calls;
CvUnitAI::AI_pillageRange flags 0: 12 ms / 108 calls / 16 duplicate calls.
Conclusion: exact whole-path request dedupe is not large enough on the primary
save to be a Gate A lever by itself. Future path work should target shared
callback state, danger/threat maps, candidate pruning with exact invalidation,
or hierarchical/coarse path services rather than only memoizing identical
group/from/to/flags results.

City-yield detail batch: profiling/autoverify/profilefast-cityyield-detail-batch-20260705-011825
City-yield detail median: 654.0 ms/turn
City-yield detail metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_CITY_YIELD_DETAIL, ProfileFast only.
Median-run pre-optimization samples: CvCityAI::AI_yieldValue::yield_delta
201 ms / 21197 calls, CvCityAI::AI_plotValue 218 ms / 14703 calls,
CvCityAI::AI_addBestCitizen::plot_scan 205 ms / 1073 calls,
CvCityAI::AI_assignWorkingPlots 245 ms / 247 calls.

Accepted city-yield context profile batch:
profiling/autoverify/profilefast-city-yield-delta-context-batch-20260705-013702
Accepted city-yield context profile median: 625.0 ms/turn
Accepted city-yield context profile metric source: custom_profile
Optimization flag: MNAI_OPT_CITY_YIELD_DELTA_CONTEXT.
Median-run movement: CvCityAI::AI_yieldValue::yield_delta fell to 42 ms /
21197 calls, CvCityAI::AI_plotValue fell to 57 ms / 14703 calls,
CvCityAI::AI_addBestCitizen::plot_scan fell to 75 ms / 1073 calls, and
CvCityAI::AI_assignWorkingPlots fell to 114 ms / 247 calls.

Accepted no-bonus vote-source-first profile batch:
profiling/autoverify/profilefast-nobonus-votesource-first-batch-20260705-020138
Accepted no-bonus vote-source-first profile median: 592.7 ms/turn
Accepted no-bonus vote-source-first profile metric source: custom_profile
Optimization flag: MNAI_OPT_NOBONUS_VOTE_SOURCE_FIRST.
Median-run movement: CvPlayer::isFullMember fell from 49 ms / 638638 calls
to 0 ms / 4774 calls. `full_member_callers.csv` no longer listed the
previous hot no-bonus callers (`CvCity::isNoBonus` and
`CvPlayer::getNumAvailableBonuses`) because their side-effect-free
`CvGame::isNoBonus` table check now runs before membership testing.

Accepted attack-odds-before-path profile batch:
profiling/autoverify/profilefast-attack-odds-before-path-batch-20260705-022736
Accepted attack-odds-before-path profile median: 586.0 ms/turn
Accepted attack-odds-before-path profile metric source: custom_profile
Optimization flag: MNAI_OPT_ATTACK_ODDS_BEFORE_PATH.
Median-run movement: CvSelectionGroup::generatePath fell from 204 ms /
3107 calls to 161 ms / 2796 calls, pathCost fell from 47 ms / 169160 calls to
35 ms / 130464 calls, `AI_anyAttack` path requests fell from 298 to 46, and
`AI_cityAttack` path requests fell from 68 to 9.

Accepted pathCost single-unit next-skip profile batch:
profiling/autoverify/profilefast-pathcost-single-unit-next-skip-batch-20260705-031508
Accepted pathCost single-unit next-skip profile median: 585.3 ms/turn
Accepted pathCost single-unit next-skip profile metric source: custom_profile
Optimization flag: MNAI_OPT_PATHCOST_SINGLE_UNIT_NEXT_SKIP.
Median-run movement: this is a narrow `pathCost` leaf-loop optimization.
Release-path median improved by 4.7 ms/turn, and ProfileFast median moved from
586.0 to 585.3 ms/turn with the same 2796 `generatePath` calls and the same
130464 `pathCost` calls.

Worker-detail attribution batch:
profiling/autoverify/profilefast-worker-detail-batch-20260705-035427
Worker-detail attribution median: 585.0 ms/turn
Worker-detail metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_WORKER_DETAIL, ProfileFast only.
Median-run worker rows: `CvUnitAI::AI_workerMove` 75 ms / 29 calls,
`CvUnitAI::AI_routeTerritory` 20 ms / 52 calls,
`CvUnitAI::AI_fortTerritory` 12 ms / 33 calls,
`CvUnitAI::AI_connectBonus` 9 ms / 26 calls,
`CvUnitAI::AI_irrigateTerritory` 9 ms / 26 calls,
`CvUnitAI::AI_improveBonus` 4 ms / 29 calls, and
`CvUnitAI::AI_improveLocalPlot` 0 ms / 52 calls.

Worker-bonus detail attribution batch:
profiling/autoverify/profilefast-worker-bonus-detail-batch-20260706-113640
Worker-bonus detail median: 599.0 ms/turn
Worker-bonus detail metric source: custom_profile
Comparison: -10.7 ms / -1.7% versus the previous route-territory ProfileFast
batch, consistent with ordinary profile-run variance plus marker shape rather
than a gameplay change.
Median-run `AI_improveBonus` rows across turns 111-113: parent sample
15 ms / 29 calls; `plot_filter` 3 ms / 74240 calls; `candidate_path`
2 ms / 17 calls; `can_improve`, `build_scan`, and `candidate_score` rounded to
0 ms. This makes `AI_improveBonus` too small for an exact optimization lever on
the primary save; keep attention on pathing, city assignment/build evaluation,
explore, and broader worker scan structure.

City-emphasis detail attribution batch:
profiling/autoverify/profilefast-city-emphasize-detail-batch-20260705-041905
City-emphasis detail median: 582.3 ms/turn
City-emphasis detail metric source: custom_profile
Instrumentation flag: MNAI_PROFILE_CITY_EMPHASIZE_DETAIL, ProfileFast only.
Median-run city-emphasis rows: `CvCityAI::AI_doEmphasize` 47 ms / 75 calls,
`CvCityAI::AI_setEmphasize::assign_working_plots` 46 ms / 157 calls,
`CvCityAI::AI_doEmphasize::set_emphasize` 24 ms / 600 calls, and
`CvCityAI::AI_doEmphasize::force_avoid_angry` 22 ms / 75 calls. The great
people plot summary, good-tile count, production multiplier, and population
rank markers rounded to 0 ms in the median run, so the useful target is city
reassignment frequency/shape rather than tile-summary math.
```

Plain `ReleaseFast` remains the player-facing artifact, but it does not yet
complete the unattended 111-113 AutoVerify window without a test hook; it
stalled at turn 112 during local testing. Use `ReleaseFastVerify` only for
no-profiler release-path automation.

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

11. `MNAI_OPT_CITY_YIELD_DELTA_CONTEXT`
    - During stable citizen plot scans, snapshots exact city base yield rates,
      base yield modifiers, and the active production modifier once, then passes
      that context into `AI_plotValue`/`AI_yieldValue`.
    - Accepted release batch:
      `profiling/autoverify/candidate-city-yield-delta-context-releaseverify-batch-20260705-012951`
      measured 570.7 ms/turn versus the restored 593.0 ms/turn release-path
      median.
    - Profiler movement:
      `profilefast-city-yield-delta-context-batch-20260705-013702` reduced
      `CvCityAI::AI_yieldValue::yield_delta` from 201 ms to 42 ms in the
      median run.

12. `MNAI_OPT_NOBONUS_VOTE_SOURCE_FIRST`
    - In `CvCity::getNumBonuses` and `CvPlayer::getNumAvailableBonuses`, checks
      `CvGame::isNoBonus(eVoteSource, eBonus)` before calling
      `CvPlayer::isFullMember`.
    - This preserves release behavior because `isNoBonus` is a serialized bool
      table lookup with no gameplay side effects, while avoiding repeated full
      membership checks for bonuses that no active vote source bans.
    - Accepted release batch:
      `profiling/autoverify/candidate-nobonus-votesource-first-releaseverify-batch-20260705-015446`
      measured 567.0 ms/turn versus the previous accepted 570.7 ms/turn.
    - Profiler movement:
      `profilefast-nobonus-votesource-first-batch-20260705-020138` reduced
      `CvPlayer::isFullMember` from 49 ms / 638638 calls to 0 ms / 4774 calls
      in the median run.

13. `MNAI_OPT_ATTACK_ODDS_BEFORE_PATH`
    - In `CvUnitAI::AI_cityAttack` and `CvUnitAI::AI_anyAttack`, computes exact
      attack odds before running `generatePath` for a candidate plot, and skips
      pathfinder work only when the candidate already fails the existing final
      odds threshold.
    - Selection behavior still requires `generatePath` success and uses the
      original path-end turn plot before accepting a destination.
    - Accepted release batch:
      `profiling/autoverify/candidate-attack-odds-before-path-releaseverify-batch-20260705-022039`
      measured 563.7 ms/turn versus the previous accepted 567.0 ms/turn.
    - Profiler movement:
      `profilefast-attack-odds-before-path-batch-20260705-022736` reduced
      `CvSelectionGroup::generatePath()` from 204 ms / 3107 calls to
      161 ms / 2796 calls, with `AI_anyAttack` path requests falling from
      298 to 46 and `AI_cityAttack` path requests falling from 68 to 9.

14. `MNAI_OPT_PATHCOST_SINGLE_UNIT_NEXT_SKIP`
    - In `pathCost`, avoids calling `CvSelectionGroup::nextUnitNode` when the
      pathing group contains only one unit, preserving the exact work done for
      all multi-unit groups.
    - Accepted release batch:
      `profiling/autoverify/candidate-pathcost-single-unit-next-skip-releaseverify-batch-20260705-030818`
      measured 559.0 ms/turn versus the previous accepted 563.7 ms/turn.
    - Profiler movement:
      `profilefast-pathcost-single-unit-next-skip-batch-20260705-031508`
      moved ProfileFast median from 586.0 to 585.3 ms/turn. This is a small
      release-path leaf-loop win, not a strategic pathing reduction.

15. `MNAI_OPT_PATHVALID_STICKY_UPDATE_CACHE`
    - Extends the existing per-`generatePath` path-valid callback cache so
      repeated path searches inside one `CvUnitAI::AI_update` may reuse exact
      `canMoveThrough` / `canMoveOrAttackInto` answers when the same group,
      flags, plot, composition, and basic unit-state signature are unchanged.
    - The scope is deliberately narrow: it starts at `AI_update`, ends before
      the next unit update, and falls back to a fresh cache generation whenever
      the signature changes.
    - Accepted release batch:
      `profiling/autoverify/candidate-pathvalid-sticky-update-cache-releaseverify-batch-20260705-083907`
      measured 551.0 ms/turn versus the previous accepted 559.0 ms/turn.
    - Profiler movement:
      `profilefast-pathvalid-sticky-update-cache-batch-20260705-084627`
      moved ProfileFast median from 604.7 to 599.7 ms/turn versus the
      explore-detail baseline. In the median runs, `CvUnit::canMoveInto` calls
      fell from 30457 to 29371, `pathValid danger & invisible` moved from
      31 ms to 30 ms, and `AI_workerMove` moved from 79 ms to 77 ms. This is a
      small exact callback-reuse win, not the full path service needed for
      Gate A.

16. `MNAI_OPT_ROUTE_TERRITORY_OWNED_PLOT_CACHE`
    - Adds a runtime-only `CvPlayerAI` owned-plot list and uses it inside
      `CvUnitAI::AI_routeTerritory` so worker route scans visit the player's
      owned plots in map-index order instead of rescanning the whole map.
    - The candidate still rechecks all existing route, improvement, visibility,
      target-mission, enemy, and path conditions for each candidate plot, and
      invalidates the cache on `CvPlot::setOwner`.
    - Accepted release batch:
      `profiling/autoverify/candidate-route-territory-owned-plot-cache-releaseverify-batch-20260705-193825`
      measured 546.0 ms/turn versus the previous accepted 551.0 ms/turn.
      Run 2 was an outlier at 586.3 ms/turn, but runs 1 and 3 were 546.0 and
      545.7 ms/turn.
    - Profiler movement:
      `profilefast-route-territory-owned-plot-cache-batch-20260705-194614`
      reduced `CvUnitAI::AI_routeTerritory` from 46 ms / 52 calls to
      1 ms / 52 calls, `AI_routeTerritory::plot_valid` from 29 ms /
      133120 calls to 0 ms / 2842 calls, `AI_workerMove` from 103 ms /
      29 calls to 59-60 ms / 29 calls, and total `CvUnitAI::AI_plotValid`
      from 35 ms / 503257 calls to 26 ms / 372979 calls.

17. `MNAI_OPT_TOWER_MANA_OWNED_PLOT_SERVICE`
    - Reuses the accepted `CvPlayerAI` owned-plot list for the flag-zero
      `AI_TowerMastery` mana availability check. A narrow Python binding counts
      plots whose current `NO_TEAM` bonus belongs to either requested bonus
      class, preserving the original ownership, bonus-class, tech, building,
      civic, and flag decisions.
    - This is distinct from the rejected full C++ Tower Mastery port and the
      rejected Python XML-ID cache: the state machine remains in Python and
      only its whole-map plot traversal moves into the shared owned-plot
      service.
    - Accepted release batch:
      `profiling/autoverify/candidate-tower-mana-owned-plot-service-releaseverify-batch-20260709-231611`
      measured 570.0 ms/turn versus the same-session clean control
      `profiling/autoverify/clean-tower-mana-owned-plot-service-control-releaseverify-batch-20260709-232502`
      at 574.7 ms/turn, a paired improvement of 4.7 ms/turn / 0.8%. The best
      recorded absolute accepted median remains 546.0 ms/turn because the
      paired session ran slower overall.
    - Profiler movement:
      `profiling/autoverify/profilefast-tower-mana-owned-plot-service-batch-20260709-233312`
      measured 589.3 ms/turn versus the prior accepted attribution batch at
      609.7 ms/turn. Median-run `CvPlayer::doTurn::tower_mastery` fell from
      67 ms / 24 calls to 0 ms / 24 calls.
    - All nine candidate, control, and profile `PythonErr.log` files were
      empty; `CivilizationIV.ini` was restored; transient `AutoVerify.ini` was
      removed; no fast-wrapper Wine process remained; and the control wrapper
      DLL stayed at
      `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
      The installed player-facing `ReleaseFast` DLL is
      `e5d983e53234c89b33a35417ab21c293068e78d99ed93bb927fd881ed6205e7b`,
      with wrapper `CvGameUtils.py`
      `587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c`.

18. `MNAI_OPT_PILLAGE_VALUE_BEFORE_PATH`
    - In `CvUnitAI::AI_pillageRange`, evaluates the side-effect-free pillage
      score before pathfinding and skips a candidate only when its best
      possible score cannot exceed the current winner. Successful candidates
      retain the original path-turn and war-declaration division order, so
      integer rounding and tie selection are unchanged.
    - Accepted release batch:
      `profiling/autoverify/candidate-pillage-value-before-path-releaseverify-batch-20260709-235143`
      measured 564.7 ms/turn versus the immediately preceding accepted
      570.0 ms/turn batch, an improvement of 5.3 ms/turn / 0.9%.
    - Profiler movement:
      `profiling/autoverify/profilefast-pillage-value-before-path-batch-20260709-235754`
      measured 590.3 ms/turn versus 589.3 ms/turn (noise), while the targeted
      `AI_pillageRange` path requests fell consistently from 98 to 83 per run
      and strict duplicates fell from 34 to 27. Rounded caller path time moved
      from 9-10 ms to 8-9 ms.
    - All six release/profile `PythonErr.log` files were empty, INI state was
      restored, transient `AutoVerify.ini` was removed, no fast-wrapper Wine
      process remained, and the control wrapper stayed at
      `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
      The installed hook-free ReleaseFast DLL is
      `982d92b4e33f5472e4cbcdf75981055efde50e2c0c1a695cf6a9598c0e730188`.

19. `MNAI_OPT_PATHVALID_MOVE_BEFORE_DANGER`
    - Reorders two exact `pathValid` predicates for AI-controlled path nodes:
      cached `canMoveThrough` / `canMoveOrAttackInto` legality now runs before
      danger/invisibility checks. Both predicates must still pass, and neither
      consumes RNG or mutates gameplay state; only cache-population order
      changes.
    - ProfileFast-only reject attribution batch:
      `profiling/autoverify/profilefast-pathvalid-order-attribution-batch-20260710-010058`
      measured 589.7 ms/turn. Across every run, danger rejected only 1,528
      callbacks while movement rejected 55,440 (528 attack-entry and 54,912
      move-through), supporting the reorder.
    - Accepted release batch:
      `profiling/autoverify/candidate-pathvalid-move-before-danger-releaseverify-batch-20260710-010843`
      measured 565.7 ms/turn versus same-session clean control
      `profiling/autoverify/clean-pathvalid-move-before-danger-control-releaseverify-batch-20260710-011521`
      at 570.7 ms/turn, a paired gain of 5.0 ms/turn / 0.9%. Candidate runs
      were 570.0, 565.0, and 565.7; control runs were 573.0, 570.7, and 564.3.
      The best recorded absolute accepted median remains 546.0 ms/turn.
    - Accepted ProfileFast batch:
      `profiling/autoverify/profilefast-pathvalid-move-before-danger-batch-20260710-012207`
      measured 587.0 ms/turn versus 589.7 ms/turn for the attribution control.
      The danger band fell from 36 ms / 168472 calls to 24-25 ms / 112920
      calls, and `CvSelectionGroup::generatePath()` fell from 164 ms to
      149 ms in the median-profile comparison.
    - Candidate Verify DLL SHA256 was
      `8af0e5a0f78b0d4f8c591abdf5c056e35f02df54cc13041e5583fe460c8da0b5`;
      clean-control Verify was
      `1c7f8a9078b1f6f5a7ac8f8751b03108111be4e0c2f0fb0aaa8134f946866ccc`;
      accepted ProfileFast was
      `91c306d6e7a2db073c2855acb4b01cd51d5ba71a94996158e5031a6ca36aec34`;
      and the installed hook-free ReleaseFast DLL is
      `afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.
    - All twelve attribution/candidate/control/profile run logs were clean
      across the four relevant batches, every INI backup matched the restored INI,
      transient `AutoVerify.ini` was removed, no fast-wrapper Wine process
      remained, and the control wrapper DLL stayed at
      `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

## Accepted Instrumentation

1. `MNAI_PROFILE_PATH_REQUESTS`
   - ProfileFast-only path attribution. It times every
     `CvSelectionGroup::generatePath` call, records the active AI context,
     flags, success/reuse counts, strict request uniqueness, duplicate-call
     counts, and path-turn summaries into `path_request_callers.csv`.
   - First batch:
     `profiling/autoverify/profilefast-path-requests-batch-20260705-002808`.
     It completed cleanly with empty `PythonErr.log`, restored INI state, no
     lingering fast-wrapper process, restored fast-wrapper DLL hash
     `26996314334e2fc466b09984b099c3bf340d03e07e205e54742db951fa580229`, and
     unchanged control-wrapper DLL hash
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
   - This is not a release-path optimization and is intentionally not enabled
     in `ReleaseFast` or `ReleaseFastVerify`.
   - Compile guard: `./build-releasefastverify-wine.sh` completed after adding
     the instrumentation; rebuilt `ReleaseFastVerify/CvGameCoreDLL.dll` SHA256
     was `acfa53000d13d163c839d057ecc814a225299c21749c6e9a2e6123077dbf48db`
     (rebuild hashes are not byte-stable, so this is only a local build record).

2. `MNAI_PROFILE_WORKER_DETAIL`
   - ProfileFast-only worker attribution for `AI_workerMove` call sites that
     were previously hidden inside the aggregate worker move sample.
   - First batch:
     `profiling/autoverify/profilefast-worker-detail-batch-20260705-035427`.
     It completed cleanly with empty `PythonErr.log`, restored INI state, no
     lingering fast-wrapper process, and restored fast-wrapper DLL hash
     `26996314334e2fc466b09984b099c3bf340d03e07e205e54742db951fa580229`.
   - Median-run evidence showed `AI_improveLocalPlot` was not material; next
     worker optimization should target the full-map route, fort, bonus, and
     irrigation scans.
   - Follow-up batch
     `profiling/autoverify/profilefast-worker-bonus-detail-batch-20260706-113640`
     added scoped markers inside `CvUnitAI::AI_improveBonus`. It completed
     cleanly with empty `PythonErr.log`, removed transient `AutoVerify.ini`,
     left no Civ/Wine/Wineskin process running, restored the fast wrapper to
     accepted ReleaseFast DLL hash
     `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and
     left the control wrapper at
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
     Median-run evidence showed only 15 ms total in `AI_improveBonus` over
     turns 111-113, so this branch is not a useful standalone target.

3. `MNAI_PROFILE_CITY_EMPHASIZE_DETAIL`
   - ProfileFast-only attribution for `AI_doEmphasize` and nested
     `AI_setEmphasize` reassignment work.
   - First batch:
     `profiling/autoverify/profilefast-city-emphasize-detail-batch-20260705-041905`.
     It completed cleanly with empty `PythonErr.log`, restored INI state, and
     restored fast-wrapper DLL hash
     `26996314334e2fc466b09984b099c3bf340d03e07e205e54742db951fa580229`.
   - Median-run evidence showed nearly all `AI_doEmphasize` time came from
     repeated `AI_assignWorkingPlots` calls triggered by emphasis state changes.

4. `MNAI_PROFILE_EXPLORE_DETAIL`
   - ProfileFast-only attribution for `AI_exploreMove`, `AI_explore`, and
     `AI_exploreRange` branch/path blocks.
   - First completed batch:
     `profiling/autoverify/profilefast-explore-detail-v2-batch-20260705-082702`.
     Median was 604.7 ms/turn with empty `PythonErr.log`, restored INI state,
     and restored fast-wrapper DLL hash
     `26996314334e2fc466b09984b099c3bf340d03e07e205e54742db951fa580229`.
   - Median-run evidence showed `AI_exploreRange::generate_path` at 18 ms /
     664 calls and `AI_explore::generate_path` at 1 ms / 955 calls, so
     explore-only path work is too small to be the next strategic lever.

5. `MNAI_PROFILE_UNIT_DISPATCH_DETAIL`
   - ProfileFast-only attribution for the main `CvUnitAI::AI_update`
     UnitAI dispatch switch. It compiles out of `ReleaseFast` and
     `ReleaseFastVerify`.
   - First completed batch:
     `profiling/autoverify/profilefast-unit-dispatch-detail-batch-20260705-092534`.
     Median was 603.7 ms/turn with empty `PythonErr.log`, restored INI state,
     and restored fast-wrapper DLL hash
     `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`.
   - Median-run evidence showed `AI_update::dispatch_worker` at 79 ms / 29
     calls, `AI_update::dispatch_explore` at 72 ms / 22 calls,
     `AI_update::dispatch_attack` at 30 ms / 7 calls, and
     `AI_update::dispatch_merchant` at 14 ms / 3 calls. The next release
     candidate should target shared worker/explorer path and scoring services,
     not hidden sea/air/city-defense dispatch.

6. `MNAI_PROFILE_UNIT_PREP_DETAIL`
   - ProfileFast-only attribution for the non-automated FfH prep/groupflag
     path in `CvUnitAI::AI_update`. It compiles out of `ReleaseFast` and
     `ReleaseFastVerify`.
   - First batch:
     `profiling/autoverify/profilefast-unit-prep-detail-batch-20260705-100022`.
     Median was 610.7 ms/turn with empty `PythonErr.log`, restored INI state,
     and restored fast-wrapper DLL hash
     `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`.
     The broad split showed `prep_groupflag` at 72 ms / 239 calls, while
     hero/adept/readiness/suicide-summon checks rounded to 0 ms.
   - Refined batch:
     `profiling/autoverify/profilefast-unit-prep-groupflag-detail-batch-20260705-100836`.
     Median was 607.3 ms/turn with empty `PythonErr.log`, restored INI state,
     and restored fast-wrapper DLL hash
     `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`.
     Median-run evidence showed `prep_groupflag_patrol` at 34 ms / 78 calls
     across the measured window, `prep_groupflag_conquest` at 22 ms / 18 calls,
     and `prep_groupflag_permdefense` at 7 ms / 99 calls. The next attribution
     pass should split `AI_PatrolMove`, since patrol is a large staged
     decision routine rather than a single pathing call.

7. `MNAI_PROFILE_PATROL_DETAIL`
   - ProfileFast-only attribution for coarse decision bands inside
     `CvUnitAI::AI_PatrolMove`. It compiles out of `ReleaseFast` and
     `ReleaseFastVerify`.
   - Build status: `./build-profilefast-wine.sh` completed successfully on
     2026-07-05 after adding the markers. Built DLL:
     `CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll`, SHA256
     `89b6631a3a6f04ab0e855355b1d3c8f37aeca54edbad66cc9bb611ddd74b823b`.
   - Harness fix: `autoverify_mnai.sh` now launches Civ directly through the
     fast wrapper's bundled Wine prefix by default, avoiding the
     Porting Kit/Wineskin LaunchServices path that can show wrapper UI or steal
     focus. `benchmark_autoverify.sh` passes `--no-keypress-fallback` and
     `--direct-wine-launch` by default. The direct-Wine path also keeps a
     full-run focus guard because the game window can still make itself
     frontmost. The old app-launch path is still available through
     `--app-launch`; it uses `open -g -j -n` with the same guard. The guard
     restores the app that was frontmost before launch if Wineskin/Wine
     activates itself, restores by app name when macOS does not return a
     previous bundle id, treats Wine, Wineskin, Civ IV, Civilization IV, and
     Beyond the Sword foreground process names as benchmark-owned windows to
     hide, also hides process-name variants containing `Wineskin`, and leaves
     the AppleScript Return-key fallback opt-in through `--keypress-fallback`.
     `AutoVerify.py` sends `CyMessageControl().sendTurnComplete()` after
     enabling AI autoplay so the default no-keypress path should not need to
     activate the Wine window just to advance the first turn.
     ProfileFast/ReleaseFastVerify batches should use the default direct
     Wine/no-keypress path to avoid stealing focus. Use `--app-launch` and
     `--no-hide-launch` only when diagnosing wrapper startup. Direct-Wine
     cleanup must stop only the wrapper's Wine prefix; do not AppleScript-quit
     the `.app` from that path, because macOS can launch Wineskin just to
     deliver the quit event and show the wrapper UI.
     A one-turn smoke rerun after this latest turn-complete change was blocked
     by the Codex usage/approval limit on 2026-07-06, so the next live step is
     `scripts/autoverify_mnai.sh --turns 1 --timeout 240 --direct-wine-launch
     --no-keypress-fallback` and then a process/INI cleanup check.
   - Direct-Wine validation: initial direct launch needed
     `DYLD_FALLBACK_LIBRARY_PATH` pointed at the wrapper `Contents/Frameworks`
     and needed the Wine argv form `mod=\More Naval AI` rather than the quoted
     Wineskin plist string. After that fix,
     `directwine-modload-smoke-batch-20260706-100606` completed one unattended
     turn through the bundled Wine prefix, and
     `candidate-city-workable-plot-list-releaseverify-batch-20260706-100706`
     completed a full 3-run batch through the same path. Both left
     `PythonErr.log` empty and removed transient `AutoVerify.ini`.
   - Benchmark matrix status: `scripts/benchmark_autoverify_matrix.sh` runs
     local TSV rows and writes sanitized summaries that omit private save
     paths. The first secondary-save 3-run baselines,
     `profiling/autoverify/matrix-run-20260706-105554.md`, completed one
     measured turn per run for `midgame-worker-turn-0152`,
     `late-large-turn-0228`, and `early-pathing-turn-0079` through the
     direct-Wine path. The row medians were 846.0, 1407.0, and 954.0 ms/turn
     respectively. All nine `PythonErr.log` files were empty, transient
     `AutoVerify.ini` was removed, no Civ/Wine/Wineskin process remained, the
     fast wrapper was restored to the accepted `ReleaseFast` DLL hash
     `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and
     the original/control wrapper remained
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
   - Focus hardening smoke status: `focus-hidden-smoke-20260706` loaded the
     save with plain `ReleaseFast` but timed out at turn 110 because the
     wrapper did not contain the AutoVerify-capable DLL. After installing clean
     `ReleaseFastVerify`, `focus-hidden-smoke-verify-20260706` completed one
     hidden-background turn against the niallohiggins save, left
     `PythonErr.log` empty, restored INI state, removed transient
     `AutoVerify.ini`, left no fast-wrapper Wine process running, and left the
     original/control wrapper DLL hash unchanged. Use `ReleaseFastVerify` or
     `ProfileFast` for unattended smoke and benchmark runs.
   - Smoke status: `profiling/autoverify/patrol-smoke-bg-nokey-20260705`
     completed one turn with `--no-keypress-fallback`, emitted patrol rows,
     left `PythonErr.log` empty, restored INI state, and left no fast-wrapper
     process running.
   - First batch:
     `profiling/autoverify/profilefast-patrol-detail-batch-20260705-104553`.
     Median was 609.7 ms/turn versus 607.3 ms/turn for the previous
     groupflag attribution batch, consistent with small extra profiler
     overhead. All three `PythonErr.log` files were empty, transient
     `AutoVerify.ini` was removed, no fast-wrapper process remained, the fast
     wrapper was restored to accepted ReleaseFast DLL hash
     `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`,
     and the original wrapper remained
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
   - Median-run evidence: `prep_groupflag_patrol` totaled 37 ms / 78 calls
     across turns 111-113, but detailed `AI_PatrolMove::*` rows summed to only
     22 ms / 1516 scoped calls. The only non-trivial rows were
     `small_group_options` at 13 ms / 78 calls,
     `guard_protect_airlift` at 4 ms / 30 calls,
     `fort_airlift_guard1` at 2 ms / 57 calls, and
     `fallback_switch_group_patrol_safety` at 2 ms / 12 calls.
   - Conclusion: patrol is not the next large exact optimization lever on the
     primary save. Continue toward shared worker/explorer path/scoring work:
     median-run `dispatch_worker` was 78 ms / 29 calls,
     `dispatch_explore` was 71 ms / 22 calls, and `generatePath` remained
     170 ms / 2796 calls.

8. Worker/path context attribution
   - Added `MNAI_PATH_REQUEST_CONTEXT` scopes inside worker helper routines
     (`AI_improveLocalPlot`, `AI_nextCityToImprove`, `AI_irrigateTerritory`,
     `AI_fortTerritory`, `AI_improveBonus`, `AI_connectBonus`,
     `AI_connectCity`, `AI_routeCity`, `AI_routeTerritory`, and
     `AI_travelToUpgradeCity`). These compile only in `ProfileFast` with
     `MNAI_PROFILE_PATH_REQUESTS`.
   - Batch:
     `profiling/autoverify/profilefast-worker-path-context-batch-20260705-110545`.
     Median was 581.0 ms/turn versus 609.7 ms/turn for the previous patrol
     detail batch. Treat that delta as ProfileFast/instrumentation noise, not
     an accepted release-path optimization.
   - Validation: all three `PythonErr.log` files were empty, transient
     `AutoVerify.ini` was removed, no fast-wrapper process remained, the fast
     wrapper was restored to accepted ReleaseFast DLL hash
     `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`,
     and the original wrapper remained
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
   - Median-run evidence: `CvSelectionGroup::generatePath()` was 164 ms /
     2796 calls, `dispatch_worker` and `AI_workerMove` were 75 ms / 29 calls,
     `dispatch_explore` and `AI_exploreMove` were 69 ms / 22 calls,
     `AI_assignWorkingPlots` was 71 ms / 247 calls, `AI_addBestCitizen` was
     63 ms / 1073 calls, and `AI_bestPlotBuild` was 42 ms / 1167 calls.
     Aggregated path-request rows across the batch showed `AI_update` flags 0
     at 104 ms / 432 calls, `AI_exploreRange` flags 4 at 47 ms / 1992 calls,
     `AI_anyAttack` flags 0 at 45 ms / 138 calls, and worker helper path
     buckets at roughly 0-33 ms each. This makes another narrow worker helper
     prefilter low-yield; the next useful work should be shared path/scoring
     services or broader city/worker evaluation services.

8a. Groupflag path context attribution
   - Added `MNAI_PATH_REQUEST_CONTEXT` scopes at the entry of
     `CvUnitAI::AI_ConquestMove`, `CvUnitAI::AI_cityDefenseMove`, and
     `CvUnitAI::AI_PatrolMove`.
   - This compiles out of `ReleaseFast` / `ReleaseFastVerify` unless
     `MNAI_PROFILE_PATH_REQUESTS` is enabled, so it is attribution only.
   - Build status: `env WINEDEBUG=-all WINEDLLOVERRIDES=winemenubuilder.exe=d
     ./build-profilefast-wine.sh` completed successfully on 2026-07-06 after
     adding the scopes. Built `CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll`
     SHA256:
     `dea2d236235246e2d4140edeae96b3e8cd77e7bf0aca4a11ceb8abcd632d1b6c`.
   - Batch:
     `profiling/autoverify/profilefast-groupflag-path-context-batch-20260706-081613`.
     Median was 583.7 ms/turn with run averages 580.0, 588.3, and
     583.7 ms/turn. All three `PythonErr.log` files were empty, transient
     `AutoVerify.ini` was removed, no fast-wrapper process remained, the fast
     wrapper was restored to accepted ReleaseFast DLL hash
     `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`,
     and the original wrapper remained
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
   - Median-run evidence: `CvSelectionGroup::generatePath()` remained
     161 ms / 2796 calls, `CvUnitAI::AI_update::prep_groupflag` was
     65 ms / 239 calls, `AI_exploreMove` was 66 ms / 22 calls,
     `AI_workerMove` was 57 ms / 29 calls, and the city-side rows were still
     material (`AI_assignWorkingPlots` 83 ms / 247 calls,
     `AI_addBestCitizen` 74 ms / 1073 calls, and `AI_bestPlotBuild`
     63 ms / 1167 calls).
   - Path-request split: `AI_ConquestMove` accounted for only
     14 ms / 36 calls / 0 duplicate calls across the median run, while generic
     `AI_update` flags 0 shrank to 15 ms / 54 calls / 0 duplicates.
     `AI_PatrolMove` was 1 ms / 49 calls with 12 duplicates, and
     `AI_cityDefenseMove` rounded to 0 ms / 5 calls.
   - Conclusion: the new scopes made attribution cleaner, but they do not
     reveal a large exact duplicate-path result cache. Continue toward broader
     city/worker scoring services, danger maps, or shared path callback state
     rather than another narrow groupflag path memoization candidate.

8b. Danger/threat attribution
   - Added `MNAI_PROFILE_DANGER_DETAIL` markers around
     `CvPlayerAI::AI_getAnyPlotDanger`, `AI_getPlotDanger`,
     `AI_getWaterDanger`, and `AI_isPlotThreatened`, plus coarse cache/scan
     bands. This is `ProfileFast` only and compiles out of release builds.
   - Build status: `env WINEDEBUG=-all WINEDLLOVERRIDES=winemenubuilder.exe=d
     ./build-profilefast-wine.sh` completed successfully on 2026-07-06 after
     adding the markers. Built `CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll`
     SHA256:
     `851915ad25687fa4d4ccd5fcd7613c40fc1126db838a3f8eb7c029a4cc128edf`.
   - Batch:
     `profiling/autoverify/profilefast-danger-detail-batch-20260706-083202`.
     Median was 596.7 ms/turn with run averages 589.7, 597.3, and
     596.7 ms/turn. All three `PythonErr.log` files were empty, transient
     `AutoVerify.ini` was removed, no fast-wrapper process remained, the fast
     wrapper was restored to accepted ReleaseFast DLL hash
     `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`,
     and the original wrapper remained
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
   - Median-run evidence: `CvPlayerAI::AI_getAnyPlotDanger` totaled
     12 ms / 34574 calls, with 2 ms / 3908 calls in the real plot/unit scan
     band and 1 ms / 34181 calls in the active-player no-danger cache check.
     `AI_getPlotDanger` was 0 ms / 1051 calls, `AI_isPlotThreatened` was
     0 ms / 89 calls, and `AI_getWaterDanger` was 0 ms / 11 calls.
     `pathValid danger & invisible` remained larger at 35 ms / 194296 calls.
   - Conclusion: a broad per-team danger map is not the next direct primary
     save lever unless it also replaces path callback danger/invisibility work.
     Continue toward shared path callback state or city/worker scoring rather
     than a standalone `AI_getPlotDanger` cache.

9. Found-value attribution
   - Added `MNAI_PROFILE_FOUND_VALUE_DETAIL` markers inside
     `CvPlayerAI::AI_updateFoundValues`. These compile only in `ProfileFast`
     and split the routine into reset, city-site invalidation, plot loop,
     Python callback, `AI_foundValue`, record-value, and city-site update
     bands.
   - Build status: `./build-profilefast-wine.sh` completed successfully on
     2026-07-05. Built DLL:
     `CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll`, SHA256
     `26e0901a8a13c6378496490696d6c07bfb93b8d361144ce67788ca7f89c5d39b`.
   - Batch:
     `profiling/autoverify/profilefast-found-value-detail-batch-20260705-114144`.
     Median was 611.3 ms/turn versus 581.0 ms/turn for the previous
     worker/path context attribution batch. Treat that delta as
     ProfileFast/instrumentation overhead, not a release-path regression.
   - Validation: all three `PythonErr.log` files were empty, transient
     `AutoVerify.ini` was removed, no fast-wrapper process remained, the fast
     wrapper was restored to accepted ReleaseFast DLL hash
     `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`,
     and the original wrapper remained
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
   - Median-run evidence across turns 111-113:
     `AI_updateFoundValues` totaled 36 ms / 24 calls, `plot_loop` 26 ms /
     24 calls, `AI_foundValue` 22 ms / 11233 calls, `update_city_sites`
     8 ms / 21 calls, and the reset, invalidation, and record-value bands
     rounded to 0 ms. This is a real repeated scan, but it is too small in the
     authoritative save to be the next broad lever. Continue prioritizing
     unit/pathing and city/worker scoring services.

10. Route-territory attribution
   - Added `MNAI_PROFILE_ROUTE_TERRITORY_DETAIL` markers inside
     `CvUnitAI::AI_routeTerritory`. These compile only in `ProfileFast` and
     split the full-map worker route scan into plot-valid, owner, best-route,
     current-route, improvement-route-yield, visible-enemy, target-mission,
     generate-path, and scoring bands.
   - Build status: `./build-profilefast-wine.sh` completed successfully on
     2026-07-05. Built DLL:
     `CvGameCoreDLL/ProfileFast/CvGameCoreDLL.dll`, SHA256
     `207a4991b8e2bba7d71c64685442e23f3a5438200e3850aa2b80bb4a0aca057f`.
   - Batch:
     `profiling/autoverify/profilefast-route-territory-detail-batch-20260705-134739`.
     Median was 595.0 ms/turn with `Metric source: custom_profile`. Treat the
     higher worker time as ProfileFast marker overhead, not a release-path
     regression.
   - Validation: all three `PythonErr.log` files were empty, transient
     `AutoVerify.ini` was removed, no fast-wrapper process remained, the fast
     wrapper was restored to accepted ReleaseFast DLL hash
     `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`,
     and the original wrapper remained
     `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
   - Median-run evidence across turns 111-113:
     `AI_routeTerritory` totaled 46 ms / 52 calls. Its dominant internal row
     was `plot_valid` at 29 ms / 133120 calls; `owner_check` was 0 ms /
     36712 calls; `best_route` was 0 ms / 1114 calls; `target_mission`,
     `visible_enemy`, `generate_path`, and scoring rounded to 0 ms. The
     obvious owner-first release candidate was tested and rejected, so future
     worker-route work needs a broader exact candidate service rather than only
     reordering the local filters.

11. Best-plot-build and improvement-value attribution
   - Added `MNAI_PROFILE_BEST_PLOT_BUILD_DETAIL` markers inside
     `CvCityAI::AI_bestPlotBuild` and `CvCityAI::AI_getImprovementValue`.
     These compile only in `ProfileFast` and split city plot build scoring
     into feature setup, improvement-value, feature-clear, chop, route, build
     validity, bonus-discovery, yield-diff, and final-scoring bands.
   - First batch:
     `profiling/autoverify/profilefast-best-plot-build-detail-batch-20260705-141454`.
     Median was 596.7 ms/turn with `Metric source: custom_profile`.
     Median-run evidence showed `CvCityAI::AI_bestPlotBuild` at 44 ms /
     1167 calls and `AI_bestPlotBuild::improvement_value_loop` at 43 ms /
     1167 calls, with `CvPlayer::canBuild` at 43 ms / 55573 calls,
     `CvPlayer::getBestRoute` at 28 ms / 27436 calls, and
     `CvPlot::calculateImprovementYieldChange` at 23 ms / 73505 calls.
   - Deeper batch:
     `profiling/autoverify/profilefast-improvement-value-detail-batch-20260705-142247`.
     Median was 599.3 ms/turn with `Metric source: custom_profile`.
     Median-run evidence showed `AI_getImprovementValue::build_validity_scan`
     at 36 ms / 94286 calls and `build_validity_can_build` at 23 ms /
     35979 calls. The obvious exact build-list release candidate was tested
     and rejected, so future city-plot work should target shared dirty-region
     scoring/cache services rather than only replacing the local XML build
     scan.

12. Player-turn attribution
   - Added `MNAI_PROFILE_PLAYER_TURN_DETAIL` markers inside
     `CvPlayer::doTurn`, `CvPlayer::doTurnUnits`, and
     `CvPlayer::setTurnActive`. These compile only in `ProfileFast` and split
     player-turn work into city turn loops, unit-turn activation, Tower
     Mastery, AI pre/post hooks, economy/history/events, warnings, UI, and
     active-player danger-cache invalidation.
   - Batch:
     `profiling/autoverify/profilefast-player-turn-detail-batch-20260705-145708`.
     Median was 596.7 ms/turn with `Metric source: custom_profile`.
   - Median-run evidence showed `CvPlayer::setTurnActive::inactive_do_turn`
     and `CvPlayer::doTurn` at 302 ms / 24 calls,
     `CvPlayer::doTurn::city_do_turn_loop` at 175 ms / 24 calls,
     `CvPlayer::setTurnActive::active_turn_units` and
     `CvPlayer::doTurnUnits` at 65 ms / 24 calls, and
     `CvPlayer::doTurn::tower_mastery` at 62 ms / 24 calls. A full C++ port was
     rejected, but the later narrow accepted owned-plot service removed the
     whole-map Python traversal and reduced this marker to 0 ms / 24 calls in
     `profilefast-tower-mana-owned-plot-service-batch-20260709-233312`.

13. City-turn attribution
   - Added `MNAI_PROFILE_CITY_TURN_DETAIL` markers inside `CvCity::doTurn`
     and `CvCityAI::AI_doTurn`. These compile only in `ProfileFast` and split
     city work into city AI, production, culture, growth, event hooks,
     worked-plot improvements, route-to-city, worker-needed, best-build,
     hurry/draft/panic, and emphasize work.
   - Batch:
     `profiling/autoverify/profilefast-city-turn-detail-batch-20260705-152947`.
     Median was 598.3 ms/turn with `Metric source: custom_profile`, only
     +1.7 ms / +0.3% versus the previous player-turn ProfileFast batch.
   - Median-run evidence showed `CvPlayer::doTurn::city_do_turn_loop` at
     180 ms / 24 calls, `CvCity::doTurn::AI_doTurn` and
     `CvCityAI::AI_doTurn` at 142 ms / 89 calls,
     `CvCityAI::AI_doTurn::update_best_build` at 66 ms / 75 calls,
     `CvCityAI::AI_assignWorkingPlots` at 72 ms / 247 calls, and
     `CvCityAI::AI_bestPlotBuild::improvement_value_loop` at 62 ms / 1167
     calls. This confirms the next city-side work needs shared city/worker
     scoring services rather than another one-off wrapper around the build
     validity scan.

14. Food-value attribution
   - Added ProfileFast-only `MNAI_PROFILE_CITY_YIELD_DETAIL` sub-markers inside
     `CvCityAI::AI_yieldValue::food_value`. These compile out of
     `ReleaseFast` / `ReleaseFastVerify` and split food scoring into state
     setup, starvation, growth, anger recovery, good-tile counting, and growth
     value.
   - Batch:
     `profiling/autoverify/profilefast-food-value-detail-batch-20260705-185700`.
     Median was 600.7 ms/turn with `Metric source: custom_profile`, only
     +2.3 ms / +0.4% versus `profilefast-city-turn-detail`.
   - Median-run evidence showed `AI_yieldValue::food_value` at roughly
     15-16 ms/turn. The measurable sub-band was
     `food_value::good_tiles` at 7 ms/turn; `food_value::state` was 3 ms/turn,
     while starvation, anger recovery, and growth-value arithmetic rounded to
     0-1 ms/turn. Since scoped `AI_countGoodTiles` /
     `AI_countGoodSpecialists` caching was already rejected on the release
     path, this reinforces that city-side gains need shared dirty-region
     scoring, not another local food-value cache.
   - Follow-up batch:
     `profiling/autoverify/profilefast-city-growth-subband-batch-20260710-001733`
     measured 592.0 ms/turn. New ProfileFast-only sub-bands showed
     `growth_capacity` at 8-9 ms/turn, almost entirely its nested
     `good_tiles` scan at 7 ms/turn; `growth_state_adjustment`,
     `growth_overrides`, and `growth_value` rounded to 0 ms. This closes the
     remaining growth arithmetic as a standalone target and confirms that the
     previously rejected good-tile cache must not be retried without a broader
     assignment design.
   - Added a ProfileFast path-request context for `AI_goToTargetCity` so future
     city-target path attribution is no longer folded into its caller.

15. Construct-path attribution
   - Added a ProfileFast-only `MNAI_PATH_REQUEST_CONTEXT` at
     `CvUnitAI::AI_construct`; it compiles out of both release targets.
   - Batch:
     `profiling/autoverify/profilefast-construct-path-context-batch-20260710-014010`
     measured 591.3 ms/turn. Every run attributed 142 successful paths
     (127 unique, 15 strict duplicates) and 18-19 ms to `AI_construct`, whose
     parent sample was only 20-21 ms / 36 calls.
   - This evidence supported one exact eligibility-before-path experiment, but
     its duplicated XML/city legality scan regressed on the release path and
     was removed. Keep the context for future construct/build-service designs;
     do not retry the same pre-scan shape.

16. `canBuild` predicate-order attribution
   - Added ProfileFast-only `MNAI_PROFILE_CANBUILD_ORDER_DETAIL` counters for
     tech, plot, feature-tech, and gold rejection plus successful acceptance.
     These compile out of both release targets.
   - Batch:
     `profiling/autoverify/profilefast-canbuild-order-batch-20260710-022517`
     measured 597.0 ms/turn. Every run recorded 55,521 calls: 41,130 rejected
     at the already accepted tech-first check, 5,084 at plot legality, only 68
     at feature-removal tech, none at gold, and 9,239 accepted.
   - Conclusion: do not move feature-tech or gold checks before plot legality
     on this workload. Their selectivity is far too low to repay the extra
     checks, and the existing tech-first ordering is already doing the useful
     pruning. No release candidate was created.

## Rejected Candidate Themes

Do not retry these without new evidence or a different design:

- Path-failure caching inside `CvSelectionGroup::generatePath`.
- Per-plot `canBuild` cache inside `CvCityAI::AI_bestPlotBuild`.
- Zero-cost build-cost fast path in `CvPlayer::canBuild`.
- Static build lists for `CvCityAI::AI_bestPlotBuild`.
- Plot-scoped city build-validity context:
  `candidate-city-build-validity-context-releaseverify-batch-20260709-225234`
  measured 617.7 ms/turn against a contemporaneous clean control median of
  580.3 ms/turn from
  `clean-city-build-validity-context-control-releaseverify-batch-20260709-230206`
  and the accepted 546.0 ms/turn batch. The
  `MNAI_OPT_CITY_BUILD_VALIDITY_CONTEXT` candidate scanned build actions once
  per `AI_bestPlotBuild` call, preserved XML tie order and every exact
  `CvPlayer::canBuild` result, and reused the selected legal build for each
  improvement only within that plot-scoring call. It compiled cleanly and a
  20,000-case randomized equivalence check passed, but the release-path median
  regressed by 37.4 ms/turn / 6.4% versus the same-session control. The source
  context, overload, and Verify/ProfileFast flags were removed. All six batch
  `PythonErr.log` files were empty, INI state was restored, transient
  `AutoVerify.ini` was removed, the fast wrapper was restored to accepted
  ReleaseFast hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`,
  and the original wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
  A clean `ReleaseFastVerify` rebuild without the flag succeeded with DLL
  SHA256
  `6b2ca42bf8c8676f50cf11c2d0cfdfe8ba7dac5b8464f214850f6b90d1e23911`.
  The direct-Wine cleanup fallback was also hardened after the candidate batch
  left wrapper service processes behind `wineserver -k`; the clean control
  batch verified that no wrapper Wine process remained.
- Route-build list cache for `CvPlayer::getBestRoute`.
- Plot-scoped best-route hoist for `AI_bestPlotBuild`:
  `candidate-best-plot-route-service-releaseverify-batch-20260710-074054`
  reduced the expected ProfileFast work, but its 571.7 ms/turn release median
  regressed against the paired flag-off 567.7 ms/turn control. The service,
  overload, and flags were removed. Do not retry either the older
  per-improvement route context or this one-route-per-plot form without a
  materially different design.
- Sticky pathCost movement-cost edge cache inside pathfinding.
- Per-path single-unit `pathCost` head-unit context:
  `candidate-pathcost-single-unit-path-context-releaseverify-batch-20260710-020716`
  measured 568.3 ms/turn against the accepted paired candidate at 565.7
  ms/turn (+2.6 ms / +0.5%), with runs of 566.7, 578.3, and 568.3 ms/turn.
  Building on the accepted next-node skip, the exact
  `MNAI_OPT_PATHCOST_SINGLE_UNIT_PATH_CONTEXT` candidate stored a single-unit
  group's head-unit pointer once in the existing per-`generatePath` callback
  context, avoiding repeated list-head, unit-map, group-size, and next-node
  lookups while leaving multi-unit iteration and all costs unchanged. The
  added context/code shape did not improve release timing, so the branch and
  Verify flag were removed and the original `pathCost` source was byte-restored.
  Candidate Verify DLL SHA256 was
  `6f27f5f935234bd90467a6445dc5c207c385425e29e680b2441662de4cb598f0`;
  clean Verify rebuilt at
  `ec43a3bd0221b28b022ec71c53e8ddb01f8f21a42f99189080a6b01a4286ea99`.
  The ignored backup is
  `profiling/dll-backups/pathcost-single-unit-path-context-20260710-020708`.
  All three `PythonErr.log` files were empty, INI state was restored exactly,
  transient `AutoVerify.ini` was removed, no wrapper Wine process remained,
  the control DLL stayed at
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
  the playable wrapper was restored to ReleaseFast SHA256
  `afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.
- PathValid group-fact cache:
  `candidate-pathvalid-group-fact-cache-releaseverify-batch-20260706-084411`
  measured 566.3 ms/turn against the accepted 546.0 ms/turn batch. The
  `MNAI_OPT_PATHVALID_GROUP_FACT_CACHE` candidate cached
  `AI_isControlled`, `canFight`, and `alwaysInvisible` once per
  `generatePath`, but the branch/code-shape overhead outweighed the saved
  group scans in the release path. The source branch and Fast/ProfileFast
  build flags were removed, the fast wrapper was restored to accepted
  ReleaseFast DLL hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and
  `ReleaseFastVerify` was rebuilt without the flag with DLL SHA256
  `e6f87c85094be901af4317bf6cc39f6ecbabf5857f823009e6d9612e8945d8f7`.
- `AI_exploreRange` zero-value prefilter.
- Explore-loop invariant context hoist:
  `candidate-explore-context-hoist-releaseverify-batch-20260710-004507`
  measured 566.3 ms/turn against the accepted 564.7 ms/turn batch, with runs
  of 566.0, 566.3, and 566.3 ms/turn. The exact candidate cached the unit's
  stable team, owner, selection group, coordinates, and domain once per
  `AI_explore` / `AI_exploreRange` call while leaving plot order, RNG calls,
  target-mission checks, path requests, scoring, and strict tie behavior
  unchanged. The getter hoist did not improve the release path, so
  `MNAI_OPT_EXPLORE_CONTEXT_HOIST` and its Verify flag were removed. Candidate
  Verify DLL SHA256 was
  `e08f383b2cd8b06ad6d50ccd25f8cc81dde70fca65c5bd608835d23d1f7e269a`;
  the clean no-candidate Verify rebuild succeeded at
  `e1b4a01dfbb03067d262b1e411874f8d33c8a7943152490f8e3ebf0ab68d6ec8`.
  The pre-install player DLL backup is under ignored path
  `profiling/dll-backups/explore-context-hoist-20260710-004459` and the player
  wrapper was restored to ReleaseFast SHA256
  `982d92b4e33f5472e4cbcdf75981055efde50e2c0c1a695cf6a9598c0e730188`.
  All three `PythonErr.log` files were empty, every run restored the exact INI
  state, transient `AutoVerify.ini` was removed, no wrapper Wine process
  remained, and the control wrapper DLL stayed at
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
- Target-city adjacent score-before-path ordering:
  `candidate-target-city-adjacent-score-before-path-releaseverify-batch-20260710-002629`
  measured 565.3 ms/turn against the accepted 564.7 ms/turn batch. The exact
  candidate computed deterministic defense, river, ownership, and current-plot
  score before `generatePath` and skipped only candidates whose maximum score
  could not beat the current winner, while preserving the original path-turn
  division and strict tie order. It did not improve the release median, so the
  optimization code and Verify/ProfileFast flags were removed. The clean
  no-candidate `ReleaseFastVerify` rebuild succeeded with SHA256
  `87bcd6f0dcf5fbb9f68bbb071a27166b2ce244dcc8c92bb20e648cbbdff65379`.
  All three `PythonErr.log` files were empty, INI state was restored,
  transient `AutoVerify.ini` was removed, no wrapper process remained, and
  the player wrapper was restored to ReleaseFast SHA256
  `982d92b4e33f5472e4cbcdf75981055efde50e2c0c1a695cf6a9598c0e730188`.
- Early land/sea domain reject for the tested `canMoveThrough` path.
- Summon-only defender scan before `AI_anyAttack` pathing.
- Route-territory ownership prefilter.
- Exact explore-side `AI_plotValid` domain/area fast reject:
  `candidate-explore-plotvalid-fast-reject-releaseverify-batch-20260705-123132`
  measured 567.7 ms/turn against the accepted 551.0 ms/turn batch. The
  source hook and Fast/ProfileFast build flag were removed, and
  `ReleaseFastVerify` was rebuilt without the flag.
- `AI_PatrolMove` owner/team/group/plot context cache:
  `candidate-patrol-context-cache-releaseverify-batch-20260705-124952`
  measured 658.3 ms/turn against the accepted 551.0 ms/turn batch, with a
  turn-112 spike. The cached group/plot facts likely changed decision shape or
  introduced stale assumptions, so the source hook and Fast/ProfileFast build
  flag were removed and `ReleaseFastVerify` was rebuilt without the flag.
- Scoped active-danger update cache:
  `candidate-active-danger-update-cache-releaseverify-batch-20260705-131105`
  measured 576.0 ms/turn against the accepted 551.0 ms/turn batch. The
  candidate cached `AI_getAnyPlotDanger` and `AI_getPlotDanger` results only
  within one `CvUnitAI::AI_update`, keyed by player/plot/range/test-moves, but
  the lookup/generation overhead outweighed reuse on the authoritative save.
  The source hook and Fast/ProfileFast build flag were removed and
  `ReleaseFastVerify` was rebuilt without the flag.
- Unit construct XML scan cache:
  `candidate-unit-construct-xml-cache-releaseverify-batch-20260705-132717`
  measured 563.0 ms/turn against the accepted 551.0 ms/turn batch. The
  candidate cached whether a `(unit type, civilization)` pair has any XML
  `Buildings` or `ForceBuildings` entry before calling `AI_construct`, but the
  release-path overhead/code shape outweighed the tiny `construct_scan` saving.
- Construct eligibility before path:
  `candidate-construct-eligibility-before-path-releaseverify-batch-20260710-014906`
  measured 571.7 ms/turn against the accepted paired candidate at 565.7
  ms/turn and the same-session clean control at 570.7 ms/turn. Runs were
  570.7, 571.7, and 573.7 ms/turn. The exact
  `MNAI_OPT_CONSTRUCT_ELIGIBILITY_BEFORE_PATH` candidate scanned unit-provided
  civilization buildings before pathfinding and skipped a city only when it
  could neither contribute to the original construct-count limit nor construct
  any eligible building. Retained cities still ran the untouched path, count,
  value, and strict tie logic, but the duplicate scan cost outweighed saved
  work. Candidate Verify DLL SHA256 was
  `06cfb3a692b9ba340f0971e19d145f60ff3deec82a6eb8a634ec18328284d2e6`.
  The source branch and Verify flag were removed; clean Verify rebuilt at
  `e445b02982f350de867a0f7ab29f5bdaba985c3cb8cd63b99a9cee2546a4b928`.
  The ignored pre-install backup is
  `profiling/dll-backups/construct-eligibility-before-path-20260710-014854`.
  All six attribution/candidate `PythonErr.log` files were empty, every INI
  backup matched the restored INI, transient `AutoVerify.ini` was removed, no
  wrapper Wine process remained, the control DLL stayed at
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
  the playable wrapper was restored to ReleaseFast SHA256
  `afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.
  The source hook and Fast/ProfileFast build flag were removed and
  `ReleaseFastVerify` was rebuilt without the flag.
- Route-territory owner-first prefilter:
  `candidate-route-territory-owner-first-releaseverify-batch-20260705-135712`
  measured 563.0 ms/turn against the accepted 551.0 ms/turn batch. The
  candidate used new `MNAI_PROFILE_ROUTE_TERRITORY_DETAIL` evidence to check
  owner before `AI_plotValid` only inside `AI_routeTerritory`, but the
  no-profiler median still regressed by 12.0 ms/turn. The source branch and
  Fast/ProfileFast build flag were removed, the fast wrapper was restored to
  accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  `ReleaseFastVerify` was rebuilt without the flag.
- Improvement build-list cache:
  `candidate-improvement-build-list-releaseverify-batch-20260705-143826`
  measured 568.3 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_OPT_IMPROVEMENT_BUILD_LIST` candidate replaced the repeated
  `GC.getNumBuildInfos()` scan inside `AI_getImprovementValue` with an
  XML-order build list grouped by created improvement, while still calling
  `CvPlayer::canBuild` for each candidate build. The release-path median
  regressed by 17.3 ms/turn, so the source cache, accessors, and
  Fast/ProfileFast build flags were removed, the fast wrapper was restored to
  accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  `ReleaseFastVerify` was rebuilt without the flag.
- Tower Mastery C++ port:
  `candidate-tower-mastery-cpp-releaseverify-batch-20260705-150547` measured
  562.3 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_OPT_TOWER_MASTERY_CPP` candidate moved the per-turn Python
  `AI_TowerMastery` flag state machine into C++, including the owned mana scan
  and tower completion checks. ProfileFast player-turn markers showed the
  Python hook at 62 ms / 24 calls, but the no-profiler release median regressed
  by 11.3 ms/turn. The source branch and Fast/ProfileFast build flags were
  removed, the fast wrapper was restored to accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  `ReleaseFastVerify` was rebuilt without the flag.
- Tower Mastery Python ID cache:
  `candidate-tower-mastery-python-cache-releaseverify-batch-20260706-102245`
  measured 553.7 ms/turn against the accepted 546.0 ms/turn batch. The
  candidate cached repeated `AI_TowerMastery` XML ID lookups in Python and
  localized the map object inside the full mana scan, preserving the same flag,
  tech, building, civic, and mana-count decisions. `PythonErr.log` stayed
  empty, but the no-profiler median still regressed by 7.7 ms/turn, so the
  Python change was removed. The fast-wrapper Python file was restored to
  SHA256 `7a0b159677ec1462cad97281c4d29909aebb62444d1381e4d97f32929913d78e`,
  the fast wrapper DLL was restored to accepted ReleaseFast hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and
  the original wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
- Improvement build lookup:
  `candidate-improvement-build-lookup-releaseverify-batch-20260705-153959`
  measured 570.7 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_OPT_IMPROVEMENT_BUILD_LOOKUP` candidate used a static
  improvement-to-build vector to avoid the repeated all-build XML scan inside
  `AI_getImprovementValue`, while preserving XML build order and the same
  `CvPlayer::canBuild` checks. Despite the ProfileFast city-turn evidence
  showing `build_validity_scan` cost, the no-profiler median regressed by
  19.7 ms/turn. The helper and Fast/ProfileFast build flags were removed, the
  fast wrapper was restored to accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  `ReleaseFastVerify` was rebuilt without the flag.
- Improvement has-build prefilter:
  `candidate-improvement-has-build-prefilter-releaseverify-batch-20260706-090609`
  measured 564.0 ms/turn against the accepted 546.0 ms/turn batch. The
  `MNAI_OPT_IMPROVEMENT_HAS_BUILD_PREFILTER` candidate cached a static
  XML-derived answer for whether any build can create each improvement and
  skipped `AI_getImprovementValue` only when the improvement was not the plot's
  current improvement and no build could ever create it. The release-path
  median still regressed by 18.0 ms/turn, so the source cache, prefilter, and
  Verify/ProfileFast build flags were removed. The fast wrapper was restored
  to accepted ReleaseFast DLL hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, the
  original wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
  `ReleaseFastVerify` was rebuilt without the flag with DLL SHA256
  `b375fb02b65bd5f77d61e2c0b7db8d77d0a36cf008f19ee711a7abf58b760994`.
- Hidden flag-symbol skip:
  `candidate-hidden-flag-symbol-skip-releaseverify-batch-20260706-092205`
  measured 567.7 ms/turn against the accepted 546.0 ms/turn batch. The
  `MNAI_OPT_HIDDEN_FLAG_SYMBOL_SKIP` candidate skipped `CvPlot::updateFlagSymbol`
  for plots that were not visible to the active team and had no existing flag
  entities to destroy. It was visual-only and deterministic, but the
  release-path median regressed by 21.7 ms/turn, so the source guard and
  Verify/ProfileFast build flags were removed. The fast wrapper was restored
  to accepted ReleaseFast DLL hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, the
  original wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
  `ReleaseFastVerify` was rebuilt without the flag with DLL SHA256
  `b36addf46930d8e2ad4bd5672fd08fc1950c87a4c1c1fed3f80d345799b1a8fa`.
- Fast-heuristic found-value cadence:
  `fastheur-found-value-cadence-releaseverify-batch-20260706-093734`
  measured 564.7 ms/turn against the accepted 546.0 ms/turn batch. The
  `MNAI_FAST_HEURISTIC_FOUND_VALUE_CADENCE` candidate updated city founding
  values from `AI_doTurnUnitsPre` only every third staggered player turn unless
  the cached city-site list was empty. It avoided RNG changes and was kept
  behind an explicit optional fast-heuristic flag, but the release-path median
  still regressed by 18.7 ms/turn, so the source guard and Verify build flag
  were removed. The fast wrapper was restored to accepted ReleaseFast DLL hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, the
  original wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
  `ReleaseFastVerify` was rebuilt without the flag with DLL SHA256
  `184bcc07154fadf44ffd16bc76ceac5a4842ef934d54991abaa98fa1e06d3272`.
- City workable-plot list:
  `candidate-city-workable-plot-list-releaseverify-batch-20260706-100706`
  measured 567.7 ms/turn against the accepted 546.0 ms/turn batch. The
  `MNAI_OPT_CITY_WORKABLE_PLOT_LIST` candidate built an exact ascending list
  of `canWork` city-radius plots once per `AI_assignWorkingPlots` pass, reused
  it for `AI_addBestCitizen` and `AI_juggleCitizens`, and still recomputed
  `AI_plotValue` after each citizen mutation. It preserved tie order and
  avoided value caching, but the extra indirection/list construction still
  regressed the no-profiler median by 21.7 ms/turn, so the helper, internal
  overload, and Verify/ProfileFast build flags were removed. The fast wrapper
  was restored to accepted ReleaseFast DLL hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, the
  original wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and
  clean `ReleaseFastVerify` was rebuilt without the flag with DLL SHA256
  `ba6f606b4a3cdb9cd3b3db7c73a9faa202534c58390292c386893c44f6e2e3a4`.
- Optional explore prune heuristic:
  `candidate-fastheur-explore-prune-releaseverify-batch-20260705-155730`
  measured 568.3 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_FAST_HEURISTIC_EXPLORE_PRUNE` candidate deterministically skipped
  half of `AI_explore` and `AI_exploreRange` candidate plots before expensive
  `AI_plotValid` and pathing. It was explicitly a fast-heuristic candidate,
  not an exact default optimization, but still regressed by 17.3 ms/turn on
  the authoritative save. The source hook and `ReleaseFastVerify` build flag
  were removed, the fast wrapper was restored to accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  `ReleaseFastVerify` was rebuilt without the flag.
- Optional skip-city-emphasize heuristic:
  `candidate-fastheur-skip-city-emphasize-releaseverify-batch-20260705-164440`
  measured 556.0 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_FAST_HEURISTIC_SKIP_CITY_EMPHASIZE` candidate skipped
  `CvCityAI::AI_doEmphasize` under an explicit fast-heuristic compile flag.
  Although ProfileFast city-turn evidence showed `AI_doEmphasize` at roughly
  14-17 ms/turn, the no-profiler median still regressed by 5.0 ms/turn. The
  source guard and `ReleaseFastVerify` build flag were removed, the fast
  wrapper was restored to accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  `ReleaseFastVerify` was rebuilt without the flag.
- Route-to-city pair cache:
  `candidate-route-to-city-pair-cache-releaseverify-batch-20260705-170148`
  measured 564.0 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_OPT_ROUTE_TO_CITY_PAIR_CACHE` candidate cached
  `AI_updateRouteToCity` route-finder results for current-turn city pairs and
  invalidated them on `CvPlot::setRouteType` route-network changes. The
  release-path median regressed by 13.0 ms/turn, so the route-generation
  counter, city-pair cache, and `ReleaseFastVerify` build flag were removed,
  and the fast wrapper was restored to accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`.
  A follow-up clean `ReleaseFastVerify` rebuild was not run in that turn
  because the environment rejected further escalated commands after the
  benchmark due usage limits.
- Sticky pathCost movement-cost cache:
  `candidate-pathcost-sticky-movement-cache-releaseverify-batch-20260705-184011`
  measured 572.3 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_OPT_PATHCOST_STICKY_MOVEMENT_CACHE` candidate cached exact single-unit
  `CvPlot::movementCost` edge results under the accepted path-valid sticky
  group signature, but the lookup/generation overhead regressed the
  release-path median by 21.3 ms/turn. The source helper and Fast/ProfileFast
  build flags were removed, the fast wrapper was restored to accepted
  ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  clean `ReleaseFastVerify` was rebuilt.
- Per-path single-unit movement-cost edge cache:
  `candidate-path-edge-movement-cache-releaseverify-batch-20260706-111401`
  measured 567.7 ms/turn against the accepted 546.0 ms/turn batch. The
  candidate cached exact single-unit `CvPlot::movementCost` results only
  inside one `generatePath`, keyed by directed map edge, so invalidation was
  naturally per-search. The release-path median still regressed by
  21.7 ms/turn, matching the earlier sticky movement-cost result: generation
  and direction-key lookup overhead outweigh the saved duplicate
  `movementCost` calls on this save. The helper, path-scope hooks, and
  Fast/ProfileFast build flags were removed; all three `PythonErr.log` files
  were empty; `AutoVerify.ini` was removed; no Civ/Wine/Wineskin process
  remained; the fast wrapper was restored to accepted ReleaseFast DLL hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`; the
  original/control wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`; and a
  clean `ReleaseFastVerify` rebuild without the flag succeeded with DLL SHA256
  `6586198ad608002e1b2847c9c52668d0e981d922622c39b80d9f3fb7e9ceb35f`.
- Build-list by improvement:
  `candidate-build-list-by-improvement-releaseverify-batch-20260706-115205`
  measured 574.7 ms/turn against the accepted 546.0 ms/turn batch. The
  `MNAI_OPT_BUILD_LIST_BY_IMPROVEMENT` candidate pre-indexed XML build actions
  by created improvement and used that list inside `AI_getImprovementValue` so
  the helper would skip builds for unrelated improvements while preserving the
  same `CvPlayer::canBuild` checks for relevant builds. The release-path median
  regressed by 28.7 ms/turn, so the source helper, city call-site, and
  Fast/ProfileFast build flags were removed. All three `PythonErr.log` files
  were empty; `AutoVerify.ini` was removed; no Civ/Wine/Wineskin process
  remained; the fast wrapper was restored to accepted ReleaseFast DLL hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`; the
  original/control wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`; and a
  clean `ReleaseFastVerify` rebuild without the flag succeeded with DLL SHA256
  `0d788da61dc9f6fdc9692c03129d69983d13dfd066fe314de64633662cbaf19c`.
- Upgrade-list precompute:
  `candidate-upgrade-list-precompute-releaseverify-batch-20260706-121552`
  measured 569.0 ms/turn against the accepted 546.0 ms/turn batch. The
  `MNAI_OPT_UPGRADE_LIST_PRECOMPUTE` candidate precomputed the sorted
  available-upgrade unit list per civilization/source-unit at XML cache init
  time, then reused it in `CvUnit::getUpgradeCity(bool)` instead of rebuilding
  and sorting the same list during turn processing. It preserved the existing
  candidate order and city legality checks, but the release-path median
  regressed by 23.0 ms/turn, so the source cache, accessor, call-site, and
  Fast/ProfileFast build flags were removed. All three `PythonErr.log` files
  were empty; `AutoVerify.ini` was removed; no Civ/Wine/Wineskin process
  remained; the fast wrapper was restored to accepted ReleaseFast DLL hash
  `73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`; the
  original/control wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`; and a
  clean `ReleaseFastVerify` rebuild without the flag succeeded with DLL SHA256
  `80e28e7a4eeb5495a9f5ef5ad2397f5116ae9252509f47c299040aa875fc435b`.
- City-emphasis batch assignment:
  `candidate-city-emphasize-batch-assign-releaseverify-batch-20260705-191906`
  measured 582.0 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_OPT_CITY_EMPHASIZE_BATCH_ASSIGN` candidate deferred repeated
  `AI_setEmphasize` citizen assignment inside `AI_doEmphasize` and reassigned
  once after all emphasis flags were updated. The release-path median
  regressed by 31.0 ms/turn, likely because intermediate assignment state feeds
  later emphasis decisions and yield multiplier state. The helper and
  Fast/ProfileFast build flags were removed, the fast wrapper was restored to
  accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  clean `ReleaseFastVerify` was rebuilt.
- Route-territory inline plot-valid:
  `candidate-route-territory-inline-valid-releaseverify-batch-20260705-161540`
  measured 563.3 ms/turn against the accepted 551.0 ms/turn batch. The
  `MNAI_OPT_ROUTE_TERRITORY_INLINE_PLOT_VALID` candidate inlined the exact
  `AI_plotValid` logic inside `AI_routeTerritory` while caching domain, area,
  all-terrain movement, and no-reveal-map invariants. Despite the ProfileFast
  route-territory evidence showing `plot_valid` call volume, the no-profiler
  median regressed by 12.3 ms/turn. The helper and `ReleaseFastVerify` build
  flag were removed, the fast wrapper was restored to accepted ReleaseFast DLL
  hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  `ReleaseFastVerify` was rebuilt without the flag.
- Earlier route-territory owned-plot cache attempt, superseded by the accepted
  runtime `CvPlayerAI` owned-plot service above:
  `candidate-route-territory-owned-plot-cache-releaseverify-batch-20260705-163006`
  measured 569.7 ms/turn against the accepted 551.0 ms/turn batch. The
  earlier candidate cached only the
  map-order list of plots owned by the worker's player, keyed by game turn,
  owner, map size, and `getTotalLand()`, while rechecking all live route,
  improvement, visibility, target-mission, and path conditions per worker.
  That keyed implementation regressed by 18.7 ms/turn and was removed before
  the later simpler runtime-owned-plot service was built and accepted. The
  fast wrapper was then restored to accepted ReleaseFast DLL hash
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, and
  `ReleaseFastVerify` was rebuilt without the flag.
- `AI_plotValue` final-improvement no-op shortcut.
- Broad base-bonus cache invalidation through `AI_makeAssignWorkDirty` and
  `AI_makeProductionDirty`.
- Scoped `AI_countGoodTiles` / `AI_countGoodSpecialists` cache inside
  `AI_assignWorkingPlots`: deterministic and compiled cleanly, but
  `candidate-city-goodtile-cache-releaseverify-batch-20260704-143705` measured
  602.7 ms/turn versus the restored clean release-path median of
  593.0 ms/turn. It is below the acceptance gate and likely noise.
- `MNAI_PROFILE_VOTE_SOURCE_RELIGION_ARRAY`: mirrored vote-source religion
  lookups from the serialized map into an array for `getVoteSourceReligion`.
  It compiled and completed cleanly, but
  `candidate-votesource-religion-array-releaseverify-batch-20260704-145936`
  measured 606.7 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed.
- `MNAI_PROFILE_HAS_VOTES_CACHE`: cached `CvPlayer::hasVotes` by player and
  religion, with invalidation on religion, population, and diplomatic-vote unit
  changes. It compiled and completed cleanly, but
  `candidate-hasvotes-cache-releaseverify-batch-20260704-151913` measured
  602.0 ms/turn versus the restored clean 593.0 ms/turn median, so it was
  removed.
- `MNAI_PROFILE_PATHCOST_ENEMY_CACHE`: cached per-path, per-unit enemy-adjacent
  path-cost classification for `MOVE_AVOID_ENEMY_WEIGHT_2/3`. It preserved the
  original integer weighting sequence and completed cleanly, but
  `candidate-pathcost-enemy-cache-releaseverify-batch-20260704-153831`
  measured 618.0 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed.
- `MNAI_PROFILE_PATHVALID_GROUP_CACHE`: cached per-path group-level
  `canFight()` and `alwaysInvisible()` values for the path danger check. It
  compiled and completed cleanly, but
  `candidate-pathvalid-group-cache-releaseverify-batch-20260704-154920`
  measured 606.7 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed.
- `MNAI_PROFILE_PATHVALID_DANGER_CACHE`: cached per-path
  `AI_getAnyPlotDanger` results during one `generatePath` call. It compiled and
  completed cleanly, but
  `candidate-pathvalid-danger-cache-releaseverify-batch-20260704-160958`
  measured 621.0 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed and the fast wrapper DLL was restored.
- `MNAI_PROFILE_EMPHASIZE_AVOID_ANGRY_SINGLE_SET`: folded the forced
  avoid-angry-citizens emphasis state into the first `AI_setEmphasize` call to
  avoid an immediate false/true reassignment pair during `AI_doEmphasize`. It
  compiled and completed cleanly, but
  `candidate-emphasize-avoid-angry-single-set-releaseverify-batch-20260704-162303`
  measured 603.3 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed and the fast wrapper DLL was restored.
- `MNAI_PROFILE_CITY_YIELD_SCAN_CONTEXT`: hoisted city/player invariants for
  the `AI_addBestCitizen` worker plot scan into a per-call context while
  preserving exact per-scan city state. It compiled and completed cleanly, but
  `candidate-city-yield-scan-context-releaseverify-batch-20260704-165907`
  measured 603.3 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed. This shape added enough context setup/code-size cost to lose on
  the release path; future city work should be narrower or move repeated
  scoring out of the repeated scan loop.
- `MNAI_PROFILE_FULL_MEMBER_CACHE`: cached `CvPlayer::isFullMember` by
  player/vote source with conservative invalidation on religion, population,
  civic, loyal-member, alive-state, and diplomatic-vote unit changes. It
  compiled and completed cleanly, but
  `candidate-full-member-cache-releaseverify-batch-20260704-172507` measured
  602.3 ms/turn versus the restored clean 593.0 ms/turn median, so it was
  removed. This confirms that a ProfileFast helper hotspot is not enough by
  itself; future work should remove repeated membership queries at the caller
  level or fold them into a broader per-turn vote context.
- `MNAI_PROFILE_CITY_PLOT_STATIC_CACHE`: built a transient per-city assignment
  cache for plot yields, final-improvement yields, discovery bonus value, and
  upgrade-time value, while still calling `AI_yieldValue` for current city
  state on every candidate. It compiled and completed cleanly, but
  `candidate-city-plot-static-cache-releaseverify-batch-20260704-174918`
  measured 608.0 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed. The cache construction and extra call indirection outweighed the
  saved plot bookkeeping; future city work needs to change the repeated scan
  shape itself rather than only hoist per-plot constants.
- `MNAI_PROFILE_CITY_YIELD_COUNT_CACHE`: scoped a small cache around citizen
  scans so repeated `AI_yieldValue` calls could reuse exact
  `AI_countGoodTiles`/`AI_countGoodSpecialists` results for the same stable
  city state. It compiled and completed cleanly, but
  `candidate-city-yield-count-cache-releaseverify-batch-20260704-181323`
  measured 597.7 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed. The city assignment bottleneck is not dominated by these
  secondary count scans; future work should avoid whole repeated
  add/remove/revalue passes or move to a broader incremental assignment model.
- `MNAI_PROFILE_FORCE_CIVIC_OPTION_CACHE`: cached `CvGame::isForceCivicOption`
  and changed civic legality checks to test the cached forced-option state
  before calling `CvPlayer::isFullMember`. It compiled and completed cleanly,
  but `candidate-force-civic-option-cache-releaseverify-batch-20260704-182928`
  measured 602.7 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed. This indicates the `isFullMember` flat-profile hotspot is not
  being profitably addressed from the force-civic-option path; future work
  needs caller-attribution instrumentation before another voting-membership
  optimization.
- `MNAI_OPT_NO_BONUS_CACHE`: cached the exact per-player/per-bonus result of
  "does any full-member vote source disable this bonus?" and invalidated it on
  loyalty, state religion, civic, city religion/population, vote-source
  religion, NoBonus resolution, and diplomatic-vote unit changes. It compiled
  and completed cleanly, but
  `candidate-no-bonus-votesource-cache-releaseverify-batch-20260704-190734`
  measured 606.7 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed. Caller attribution showed `CvCity::getNumBonuses` dominates
  `isFullMember` calls, but this cache added more release-path overhead than it
  removed. Future NoBonus work needs a broader turn-level vote/no-bonus context
  or a different city bonus query shape.
- `MNAI_OPT_NO_BONUS_CHECK_ORDER`: tested the cheaper exact loop order
  `isNoBonus(voteSource, bonus)` before `isFullMember(voteSource)` in
  `CvCity::getNumBonuses` and `CvPlayer::getNumAvailableBonuses`. It compiled
  and completed cleanly, but
  `candidate-no-bonus-check-order-releaseverify-batch-20260704-191856`
  measured 610.0 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed. The extra branch/order change did not translate into a
  release-path win on the primary save.
- `MNAI_PROFILE_ACTIVE_DANGER_NOMOVE_CACHE`: added a transient active-player
  no-danger cache for `AI_getAnyPlotDanger(..., bTestMoves=false)` safe results,
  keyed by cached range and cleared by the existing active-player danger-cache
  invalidation. It compiled and completed cleanly, but
  `candidate-active-danger-nomove-cache-releaseverify-batch-20260704-193244`
  measured 596.7 ms/turn versus the restored clean 593.0 ms/turn median, so it
  was removed. This suggests the current primary save does not repeat enough
  no-move danger checks for this extra plot state to pay for itself.
- `MNAI_OPT_SKIP_WRAPPED_SEARCH_DUPLICATES`: skipped duplicate `plotXY` results
  inside a few tactical search rectangles only when world wrapping could make
  the rectangle revisit the same map plot. It compiled and the clean
  `candidate-wrapped-search-dedupe-releaseverify-batch-20260705-005041`
  measured 568.3 ms/turn versus the restored clean 593.0 ms/turn median, but
  the required ProfileFast attribution run
  `profilefast-wrapped-search-dedupe-path-requests-batch-20260705-005809`
  showed no meaningful path-request movement: median path calls stayed at 3107,
  `path_request_callers.csv` rows were unchanged, and the profile median was
  flat at 654.7 ms/turn versus 653.0 ms/turn. The apparent release win was not
  accepted because it lacked profiler support, so the flag and guarded code
  were removed.
- `MNAI_OPT_CITY_BEST_ROUTE_CONTEXT`: hoisted one `CvPlayer::getBestRoute`
  result inside each valid `AI_getImprovementValue` call and reused it for the
  repeated `calculateImprovementYieldChange(..., bBestRoute=true)` yield
  calculations. It compiled and completed cleanly, but
  `candidate-city-best-route-context-releaseverify-batch-20260705-024616`
  measured 566.7 ms/turn versus the accepted 563.7 ms/turn median, so the
  helper, guarded calls, and build flag were removed.
- `MNAI_OPT_UNIT_CONSTRUCT_SCAN_CACHE`: cached the static
  unit/civilization XML answer for whether `AI_update::construct_scan` needs to
  call `AI_construct()`. It compiled and completed cleanly, but
  `candidate-unit-construct-scan-cache-releaseverify-batch-20260705-025723`
  measured 565.7 ms/turn versus the accepted 563.7 ms/turn median, so the
  helper, guarded scan, and build flag were removed.
- `MNAI_OPT_AI_PLOT_VALID_UPDATE_CACHE`: scoped an exact per-`CvUnitAI::AI_update`
  plot-valid memo to the active unit, resetting if the unit area changed. It
  compiled and completed cleanly with empty PythonErr logs, but
  `candidate-ai-plot-valid-update-cache-releaseverify-batch-20260705-032902`
  measured 565.3 ms/turn versus the accepted 559.0 ms/turn median, so the
  cache scope, guarded lookup, and build flag were removed. The simple helper
  is not worth an extra vector lookup on this primary save without a narrower
  caller-specific cache.
- `MNAI_OPT_CITY_CAN_WORK_ASSIGN_CACHE`: cached `CvCity::canWork` once per
  `AI_assignWorkingPlots` pass and reused that answer in repeated
  `AI_addBestCitizen` plot scans. It compiled and completed cleanly with empty
  PythonErr logs, but
  `candidate-city-can-work-assign-cache-releaseverify-batch-20260705-034300`
  measured 566.7 ms/turn versus the accepted 559.0 ms/turn median, so the
  cache, signature change, and build flag were removed. The vector setup and
  extra call plumbing outweighed the saved repeated `canWork` checks.
- `MNAI_OPT_WORKER_OWNERSHIP_BEFORE_PLOTVALID`: moved side-effect-free
  ownership/revealed filters before `AI_plotValid` in worker full-map territory
  scans. It compiled and completed cleanly with empty PythonErr logs, but
  `candidate-worker-ownership-before-plotvalid-releaseverify-batch-20260705-040246`
  measured 566.3 ms/turn versus the accepted 559.0 ms/turn median, so the
  guarded order changes and build flag were removed. The best single run was
  fast, but the 3-run median did not support accepting it.
- `MNAI_OPT_EMPHASIZE_AVOID_ANGRY_SINGLE_SET`: retested the old avoid-angry
  emphasis folding idea against the current accepted stack after
  `MNAI_PROFILE_CITY_EMPHASIZE_DETAIL` showed the reassignment path as hot. It
  compiled and completed cleanly with empty PythonErr logs, but
  `candidate-emphasize-avoid-angry-single-set-v2-releaseverify-batch-20260705-042730`
  measured 567.7 ms/turn versus the accepted 559.0 ms/turn median, so the
  guarded branch and build flag were removed. Future city-emphasis work should
  attack the broader repeated reassignment pattern, not just this single pair.
- `MNAI_OPT_EMPHASIZE_SKIP_UNCHANGED_SETTER`: skipped no-op
  `AI_setEmphasize` calls from `AI_doEmphasize` when the requested value
  already matched the current bit. It compiled and completed cleanly with empty
  PythonErr logs, but
  `candidate-emphasize-skip-unchanged-setter-releaseverify-batch-20260705-080706`
  measured 565.7 ms/turn versus the accepted 559.0 ms/turn median, so the
  guard and build flag were removed. The best single run was fast, but the
  3-run median did not support accepting a function-call-only micro-skip.
- `MNAI_OPT_EMPHASIZE_TAIL_ASSIGN_BATCH`: deferred repeated
  `AI_assignWorkingPlots` calls only after the Great People emphasis decision
  had read the current worked plots. It compiled and completed cleanly with
  empty PythonErr logs, but
  `candidate-emphasize-tail-assign-batch-releaseverify-batch-20260705-091110`
  measured 567.3 ms/turn versus the accepted 551.0 ms/turn median, so the
  guard, non-serialized state, and build flag were removed. Future
  city-emphasis work should avoid batching assignment order without a broader
  city-scoring service to offset the extra state and altered reassignment
  timing.
- `MNAI_OPT_WORKER_ROUTE_TERRITORY_IMPROVEMENT_PREFILTER`: skipped
  `CvPlayer::getBestRoute` during `AI_routeTerritory(true)` for plots whose
  current improvement has no positive route-yield change for any route. It
  compiled and completed cleanly with empty PythonErr logs, but
  `candidate-worker-route-improvement-prefilter-releaseverify-batch-20260705-093539`
  measured 566.0 ms/turn versus the accepted 551.0 ms/turn median, so the
  guarded prefilter and build flag were removed. Future worker work should
  target shared worker/explorer path and scoring services, not another narrow
  route-territory prefilter.
- `MNAI_OPT_PATHCOST_SINGLE_UNIT_COMPARE_SKIP`: skipped worst-unit comparison
  checks in `pathCost` when the selection group has exactly one unit. It
  compiled and completed cleanly with empty PythonErr logs, but
  `candidate-pathcost-single-unit-compare-skip-releaseverify-batch-20260705-094836`
  measured 573.0 ms/turn versus the accepted 551.0 ms/turn median, so the
  guarded comparison skips and build flag were removed. The callback-level
  branch did not offset its own added branch shape in this save.
- `MNAI_OPT_IMPROVEMENT_YIELD_SAME_FINAL_FOLD`: folded duplicate
  `calculateImprovementYieldChange` calls in `AI_getImprovementValue` when an
  improvement's final upgrade resolved to the same improvement. It was exact
  and compiled cleanly, but
  `candidate-improvement-yield-same-final-fold-releaseverify-batch-20260705-111819`
  measured 570.7 ms/turn versus the accepted 551.0 ms/turn median, so the
  guarded code and build flag were removed. The helper call reduction did not
  move enough release-path work and likely added branch overhead in the tight
  city build-value loop.
- `MNAI_OPT_CITY_YIELD_STATIC_CONTEXT`: extended the per-scan
  `MnaiCityYieldValueContext` so `AI_yieldValue` could reuse invariant
  city/player values such as commerce modifiers, emphasis flags, food state,
  anger timers, special-yield multipliers, and average yield multipliers. It
  compiled and completed cleanly with empty PythonErr logs, but
  `candidate-city-yield-static-context-releaseverify-batch-20260705-120502`
  measured 573.0 ms/turn versus the accepted 551.0 ms/turn median, so the
  build flag was removed from `ReleaseFast`, `ReleaseFastVerify`, and
  `ProfileFast`. The fast wrapper was restored to
  `9457469b122f09fc38bd47bddf05f83688e955f8fbc6b544b5de645790fcc892`, the
  original wrapper remained
  `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`, and a
  follow-up `ReleaseFastVerify` rebuild without the flag succeeded with DLL
  hash `8239661c8d06ea0954df22fe47a2af42d2aa7e964c8b106e4aa54f534cc21884`.
  Future city work should move to a broader shared city/worker scoring service
  instead of more caller-local yield-value hoisting.

## Build

Legacy profile build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-profile-wine.sh
```

ProfileFast build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-profilefast-wine.sh
```

Release build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-release-wine.sh
```

ReleaseFast build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-releasefast-wine.sh
```

`ReleaseFast` enables the safe gameplay optimization flags while excluding
`CUSTOM_PROFILER`, AutoVerify, and profiler-only marker skips. `ProfileFast`
uses the same gameplay flags plus profiler and AutoVerify hooks, but still
excludes `MNAI_PROFILE_SKIP_TINY_HELPERS` so marker-overhead wins stay separate.

ReleaseFastTrace build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-releasefasttrace-wine.sh
```

`ReleaseFastTrace` keeps the optimized release code path and AutoVerify hooks,
but replaces the map-based custom profiler with a fixed enum-indexed exclusive
phase timer. It writes `release_trace.csv`, `scheduler_trace.csv`, and a
deterministic `state_fingerprint.csv`. The benchmark batch automatically writes
`RELEASE_TRACE.md` / `release_trace.json` and fails when selected-turn
fingerprints differ across runs. Do not install this target as the playable
DLL.

ReleaseFastVerify build:

```sh
cd "$HOME/Applications/more-naval-ai-fast/CvGameCoreDLL"

export CIV4_APP="$HOME/Applications/More Naval AI Fast.app"
export CIV4_BUILD_WINEPREFIX="$HOME/Applications/civ4-mnai-build-wineprefix"

./build-releasefastverify-wine.sh
```

`ReleaseFastVerify` is for unattended benchmark automation only. It keeps the
no-profiler release code path but adds `MNAI_AUTOVERIFY_AUTO_END_TURN` and the
tiny `MNAI_AUTOVERIFY_TURN_CSV` one-row-per-turn marker so the fixed save can
advance and summarize without the full custom profiler. Do not treat it as a
replacement for the player-facing `ReleaseFast` artifact.

## Benchmark

Place a representative test save at:

```text
$HOME/Documents/my games/Beyond The Sword/Saves/single/PROFILE_TEST_niallohiggins TURN-0110.CivBeyondSwordSave
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

## 75% Target Roadmap

The primary 75% target has been exceeded: the fixed private AutoVerify save
moved from the original 848.7 ms/turn baseline to a 163.3 ms/turn no-profiler
median. The numeric thresholds for Gates A-D are complete. The service designs
below remain useful for reducing the remaining ~167 ms/turn of DLL work and
for pursuing Gate E, but they are no longer prerequisites for the sub-200
goal. Preserve the contiguous-update scheduler win while doing that work.

Primary benchmark command pattern:

```sh
scripts/benchmark_autoverify.sh \
  --label candidate-name \
  --runs 3 \
  --turns 3 \
  --turn-filter 111-113 \
  --save "$PRIMARY_MNAI_AUTOVERIFY_SAVE"
```

Use the local private niallohiggins turn-110 save for `$PRIMARY_MNAI_AUTOVERIFY_SAVE`.
Keep the save out of git. For public documentation, describe this as a private
fixed save passed with `--save`.

Acceptance rules for every milestone:

- Measure the candidate with `ProfileFast` for attribution and
  `ReleaseFastVerify` for no-profiler release-path timing.
- Prefer roadmap changes that improve the 3-run median by at least 8% or
  50 ms/turn. Smaller exact changes may be accepted when they remove a named
  repeated-work pattern with matching profiler movement, but they do not count
  as completing an aggressive milestone gate by themselves.
- Every accepted optimization must move a named hotspot aggregate in the
  expected direction; do not accept a wall-clock win that just shifts time into
  another top-level AI loop.
- Reject or feature-flag any change that relies on lower AI quality, bounded
  search, stale data, or route approximation.
- For city governor work, do not accept a stale per-assignment `AI_plotValue`
  table in the exact `ReleaseFast` path. `AI_yieldValue` reads current city
  food/yield state as citizens are added and removed, so exact optimizations
  should hoist invariant computation or use explicitly invalidated state rather
  than reusing plot scores across changed city state.
- Preserve empty `PythonErr.log`, restored INI state, no lingering Wine
  processes, and unchanged original control wrapper.
- Update this file with the exact batch path, metric source, median, and
  accepted or rejected reason.

More aggressive milestone gates:

1. Gate A: break 400 ms/turn release-path median — numeric target achieved.
   - Required gain from current release-path median: at least 146.0 ms/turn.
   - This is a 52.9% reduction from the original 848.7 ms/turn baseline.
   - Current route-territory-owned-plot ProfileFast median run
     `profilefast-route-territory-owned-plot-cache-r3-20260705-194614` shows
     the next dominant stacks as unit/pathing (`CvPlayerAI::AI_unitUpdate`
     265 ms, `CvUnitAI::AI_update` 263 ms, `CvSelectionGroup::generatePath`
     164 ms) plus city work (`CvCity::doTurn()` 196 ms,
     `CvCityAI::AI_doTurn` 154 ms, `AI_assignWorkingPlots` 87 ms). The next
     candidate should target a shared service-level cost, not another isolated
     helper cache.
   - Attack multiple major stacks in the same milestone: city citizen assignment,
     worker/build evaluation, repeated path request work, and high-volume
     player/team eligibility checks. Do not spend a full iteration on isolated
     helper caches unless the profiler shows a realistic 75 ms path to victory.
   - Build exact scoped services for `AI_assignWorkingPlots`, `AI_addBestCitizen`,
     `AI_plotValue`, `AI_bestPlotBuild`, `CvPlayer::canBuild`, and vote/team
     eligibility queries, and add the first turn-local path request dedupe keyed
     by group, destination, flags, and relevant movement state.
   - Next non-launch target selection after the 546.0 ms/turn accepted stack:
     the latest median ProfileFast run shows `AI_assignWorkingPlots` at
     87 ms / 247 calls, `AI_addBestCitizen::plot_scan` at 69 ms / 1073 calls,
     `AI_doTurn::update_best_build` at 66 ms / 75 calls, and
     `AI_bestPlotBuild::improvement_value_loop` at 63 ms / 1167 calls. Avoid
     retrying isolated `canWork` or static plot-value caches; the next
     candidate should be a city-turn work-planning service that shares exact
     city-radius candidate lists and build/improvement legality state while
     still recomputing state-sensitive `AI_yieldValue` after each citizen
     mutation.
   - A cross-turn `AI_updateBestBuild` skip was considered after this profile
     pass and left unimplemented. It needs an explicit invalidation/signature
     covering build legality, team tech, route/resource/feature/improvement
     state, city production, area war state, worker counts, city sites,
     financial trouble, food/yield multipliers, and plot ownership before it
     can preserve exact behavior. Do not add a best-build reuse shortcut
     without that coverage.
   - Exit criteria: `ReleaseFastVerify` median <= 400 ms/turn and at least two
     formerly dominant profile aggregates each fall by at least 35% without
     shifting more than 5% into another top-three AI aggregate.

2. Gate B: break 300 ms/turn release-path median — numeric target achieved.
   - Required gain from Gate A: at least 100 ms/turn.
   - This is a 64.7% reduction from the original baseline.
   - Replace repeated tactical path work with a shared turn-local path service.
     Deduplicate identical group/destination/flags requests within a turn and
     share cached `pathValid`, `pathCost`, `canMoveThrough`, and
     `canMoveOrAttackInto` callback results across callers.
   - Build exact per-team tactical maps for danger, visible threat, city threat,
     hostile border pressure, and influence checks. Dirty them on unit movement,
     ownership, visibility, war/peace, promotions that affect threat, map edits,
     and turn boundary.
   - Exit criteria: aggregate `generatePath` plus path callback samples fall by
     at least 70% from the current profile baseline, repeated plot-danger scans
     stop showing up as a dominant unit-AI cost, and release-path median
     <= 300 ms/turn.

3. Gate C: hit the 75% target, <= 212.2 ms/turn — achieved.
   - Required gain from Gate B: at least 87.8 ms/turn.
   - Convert city, worker, production, and role selection work into shared
     dirty-region services rather than caller-local caches.
   - Cache viable production candidates, role-specific unit lists, worker target
     queues, route value, final improvement value, city-radius score tables, and
     expensive player/team eligibility checks.
   - Add hierarchical long-distance pathing for naval, transport, exploration,
     and repeated long-route decisions, while preserving exact final movement
     legality in the local path.
   - Make all invalidation explicit: tech, civic, religion, resource, feature,
     route, improvement, ownership, happiness, health, yield, area, team state,
     visibility, war state, and turn boundary.
   - Exit criteria: release-path median <= 212.2 ms/turn on the primary fixed
     niallohiggins save, with a clean correctness run and documented hotspot
     movement.

4. Gate D: stretch target, <= 169.7 ms/turn release-path median — achieved.
   - Required gain from Gate C: at least 42.5 ms/turn.
   - This is an 80% reduction from the original baseline.
   - Prototype deterministic read-only parallel precompute for immutable tactical
     maps, city plot scores, production candidate scores, and path-region
     summaries. Apply all results on the main thread only.
   - Exit criteria: exact `ReleaseFast` behavior remains the default, the
     parallel/snapshot work is deterministic across repeated AutoVerify runs, and
     release-path median <= 169.7 ms/turn.

5. Gate E: extreme stretch target, <= 127.3 ms/turn release-path median.
   - Required gain from Gate D: at least 42.4 ms/turn.
   - This is an 85% reduction from the original baseline.
   - Start only after exact services and deterministic snapshot/precompute work
     have either hit the 80% target or stalled with evidence.
   - If exact work is exhausted, create an explicit experimental mode for AI
     quality tradeoffs: top-N pruning, stale-tolerant influence maps, bounded
     path attempts, or coarse path reuse. Keep exact `ReleaseFast` behavior
     separate and easy to disable.
   - Exit criteria: every AI-quality tradeoff is deliberate, measurable, and
     documented against the fixed niallohiggins AutoVerify save and at least one
     broader smoke-test save.

## 2026-07-10 Conquest Attribution and City Static-Service Trial

17. `MNAI_PROFILE_CONQUEST_DETAIL` and
    `MNAI_PROFILE_PICK_TARGET_CITY_DETAIL`
    - Retained ProfileFast-only stage scopes split `AI_ConquestMove` without
      changing either release target.
    - `profilefast-conquest-detail-batch-20260710-024002` measured a
      595.7 ms/turn median. In its median run, `AI_pickTargetCity` accounted
      for 15 ms / 12 conquest calls and the late fallback for 4 ms / 12 calls;
      every other measured conquest stage rounded to zero. ProfileFast DLL
      SHA256 was
      `9594324b32c1a4a906b8959515ddff51b575dd7751afeb880fcfee822c0c2482`.
    - `profilefast-pick-target-detail-batch-20260710-024651` measured a
      590.0 ms/turn median. Every run recorded 52 target-city path attempts,
      44 successes, and zero entries into the new-war scoring branch. The
      conquest path context itself recorded 36 successful, unique requests
      over turns 111-113 and about 15 ms. ProfileFast DLL SHA256 was
      `4a719755bd4e74b314a4e162bfe80ffc79e31f0229163f7dca078902960acd15`.
    - Do not add the proposed new-war eligibility-before-path reorder for this
      save: it has zero eligible calls, and moving randomized city scoring
      across failed paths would not preserve the global RNG sequence.

Rejected candidate:

- `MNAI_OPT_CITY_PLOT_STATIC_SERVICE`.
  - The exact candidate snapshotted only assignment-stable plot inputs once per
    `AI_assignWorkingPlots` pass: current/final yields, improvement chain,
    bonus-discovery constant, and upgrade-time constant. It deliberately
    recomputed state-sensitive `AI_yieldValue` after every citizen mutation.
  - `candidate-city-plot-static-service-releaseverify-batch-20260710-025747`
    measured 569.0 ms/turn (runs 569.0, 570.3, 564.3) with candidate DLL
    SHA256
    `4cb8fdfffe4396d04f57a89d75243323916c5a26a1d9c6a334d69636cf74f560`.
  - The paired flag-off control
    `clean-city-plot-static-service-control-releaseverify-batch-20260710-030352`
    measured 567.0 ms/turn (runs 564.3, 567.0, 568.3), so the service regressed
    by 2.0 ms/turn. Its eager per-used-assignment snapshot and extra plumbing
    outweighed the repeated static reads it removed.
  - The source, overloads, context, and build flag were removed and the original
    `CvCityAI.cpp` / `CvCityAI.h` bytes restored. A fully reverted clean Verify
    rebuild succeeded at SHA256
    `8adc0df26e377c365bb63a34184534d86bf175dcfaf9c759b29ff8263a4d0601`.
    Do not retry this whole-city-radius snapshot shape without evidence for a
    lazy, narrower service or a design shared with worker build evaluation.
  - Ignored backups:
    `profiling/dll-backups/city-plot-static-service-20260710-025743` and
    `profiling/dll-backups/clean-control-city-plot-static-service-20260710-030345`.
  - Across the two profile and two release batches, all 12 `PythonErr.log`
    files were empty, every `CivilizationIV.ini.before` matched restored state,
    transient `AutoVerify.ini` was absent, no fast-wrapper Wine process
    remained, and the control DLL stayed at
    `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
    The playable fast wrapper was restored to hook-free ReleaseFast SHA256
    `afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.

18. Sticky path-valid signature attribution
    - Added a retained ProfileFast-only timer around
      `mnaiPathValidGroupSignature`, the exact group/unit-state hash used by
      `MNAI_OPT_PATHVALID_STICKY_UPDATE_CACHE` before repeated path searches.
    - `profilefast-pathvalid-signature-detail-batch-20260710-032035` measured a
      595.3 ms/turn median (598.7, 589.7, 595.3). Every run computed 3,217
      signatures, but the aggregate rounded to 0 ms; all path requests together
      remained 115-117 ms / 3,271 calls.
    - ProfileFast DLL SHA256 was
      `cd2aae651698564b7bb663789a71d6d253a9fc33475da04de3d00d565b613aa1`.
      Do not replace the exact signature with a broad mutation-generation
      invalidation system without new evidence: its measured ceiling is below
      timer resolution while its correctness surface is large.
    - All three `PythonErr.log` files were empty, INI state was restored,
      transient `AutoVerify.ini` was absent, no wrapper process remained, the
      control DLL was unchanged, and the player DLL was restored to
      `afd0ff6e10f60ef29fa04da51326d7823b1ff73a5d8505b78a68e889eae2da7e`.

19. Cross-context path-request attribution
    - Extended `MNAI_PROFILE_PATH_REQUESTS` with
      `path_request_cross_context.csv`. Request identity remains exact owner,
      group, origin, destination, flags, and reuse mode; the new file reports
      request keys observed under two caller contexts in the same turn.
    - `profilefast-cross-context-paths-batch-20260710-033034` measured a
      589.0 ms/turn median. Every run found 103 shared request keys between
      `AI_exploreRange` flags 4 and `AI_explore` flags 4, plus 93 between
      `groupAttack` flags 0 and `groupPathTo` flags 0. ProfileFast DLL SHA256
      was
      `bf2dc85a52420000623f4c23efebf6acdaa35ba9674bb3e2d4ed1627b57993aa`.
    - These are turn-level overlap upper bounds, not safe whole-path cache hits:
      the same group/coordinates can recur after mutable FAStar or unit state
      changes. In particular, the full `AI_explore` duplicate calls already
      averaged about 1 microsecond under FAStar reuse. Retain the ledger for
      designing narrower sequence/state keys; do not infer permission for a
      stale turn-wide result cache.

20. `MNAI_OPT_WORKER_OWNED_PLOT_SERVICE`
    - Reuses the already accepted, explicitly invalidated, map-index-ordered
      `CvPlayerAI::AI_getOwnedPlots()` service in `AI_irrigateTerritory` and
      `AI_connectBonus`. Both routines previously scanned every map plot and
      then accepted only plots owned by the worker's player.
    - All original `AI_plotValid`, ownership, improvement, bonus, mission,
      path, scoring, and strict iteration/tie checks remain. The owned list is
      ordered by original map index and invalidated by ownership changes, so
      candidate coverage and decision order are unchanged.
    - Candidate release batch:
      `candidate-worker-owned-plot-service-releaseverify-batch-20260710-034041`
      measured 567.0 ms/turn (563.0, 573.0, 567.0) versus paired flag-off
      `clean-worker-owned-plot-service-control-releaseverify-batch-20260710-034650`
      at 567.3 ms/turn (567.3, 565.0, 568.0), a 0.3 ms/turn median improvement.
      Candidate Verify SHA256 was
      `a954283a72e24d9548ad29ebb6facbe8f1bbfc3b892607dd34cda686f9bb44a8`;
      control Verify SHA256 was
      `8551f6116b1425704a1ecbc6a819a405ec98617e08adf6c9bb836ad0fb2a29fd`.
    - Matching profile batch:
      `profilefast-worker-owned-plot-service-batch-20260710-035300` measured a
      587.7 ms/turn median. Against the cross-context profile median,
      `AI_irrigateTerritory` fell 9→0 ms, `AI_connectBonus` fell 9→0 ms,
      worker dispatch / `AI_workerMove` fell 70→51 ms, and total
      `AI_plotValid` calls fell 384,830→254,552 (-130,278 / -33.9%). ProfileFast
      DLL SHA256 was
      `a2a67e982999a60cd4d3f0bea471ffe2d87546e42bf5e2755e0898cfd922a250`.
    - The paired release gain is small, but the change is accepted under the
      documented exception for exact repeated-work removal with matching large
      profiler movement. It does not replace the historical 546.0 ms/turn
      absolute best or materially advance Gate A by itself.
    - Final hook-free ReleaseFast DLL SHA256 installed in the fast wrapper:
      `302d0f39dc162ec5b5c37443c7cd0f6c697a6f4327ab891fe98dcca23d302c59`.
      Wrapper `CvGameUtils.py` remained
      `587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c`.
    - Ignored backups include
      `profiling/dll-backups/worker-owned-plot-service-20260710-034032`,
      `profiling/dll-backups/clean-control-worker-owned-plot-service-20260710-034640`,
      `profiling/dll-backups/profilefast-worker-owned-plot-service-20260710-035254`,
      and
      `profiling/dll-backups/install-releasefast-worker-owned-plot-service-20260710-035919`.
    - Across all 12 relevant runs, `PythonErr.log` was empty, every INI backup
      matched restored state, transient `AutoVerify.ini` was absent, no
      fast-wrapper Wine process remained, and the control DLL stayed at
      `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

21. `MNAI_OPT_IMPROVE_BONUS_OWNED_PLOT_SERVICE`
    - Extends the ordered owned-plot service to `AI_improveBonus` without
      changing outside-border behavior. When Advanced Tactics is disabled,
      only the player's owned plots are possible candidates. When it is enabled
      with `bInsideBordersOrCurrentPlot`, the service merges the one permitted
      unowned current plot into the owned list at its original map index. The
      unrestricted outside-border mode retains the original full-map scan.
    - All original ownership/reveal, `AI_plotValid`, area, enemy, movement,
      bonus, build, route, mission, path, scoring, RNG, and strict tie logic is
      preserved. The service changes only which impossible plots are visited.
    - Attribution batch:
      `profilefast-improve-bonus-mode-batch-20260710-040628` measured a
      587.7 ms/turn median. Every run had 29 inside/current calls, Advanced
      Tactics disabled, 74,240 plot-filter visits, 811 eligible owned plots,
      and zero eligible unowned plots. Attribution ProfileFast SHA256 was
      `9cbc9c619ee2ee6cfb55ad7054a5b4219b5e08f81f9f9d6854a585a5c09d2c8a`.
    - Candidate release batch:
      `candidate-improve-bonus-owned-plot-service-releaseverify-batch-20260710-041346`
      measured 567.0 ms/turn (567.0, 565.3, 568.0) versus paired flag-off
      `clean-improve-bonus-owned-plot-service-control-releaseverify-batch-20260710-041953`
      at 568.0 ms/turn (568.0, 571.7, 566.0). The candidate improved the median
      by 1.0 ms/turn / 0.2% and the mean by 1.8 ms/turn. Candidate Verify SHA256
      was
      `e4ef1288388f04c25e1e9935e9b610ed671ce6043c2cd9c66caf80c016bc859d`;
      control Verify SHA256 was
      `115942d048f2bd3b9f9bf3d9b966be6452be5fffc10f1943d582ec8c375674d9`.
    - Matching profile batch:
      `profilefast-improve-bonus-owned-plot-service-batch-20260710-042602`
      measured a 583.7 ms/turn median. `AI_improveBonus` fell 16→5 ms,
      its plot-filter calls fell 74,240→1,710, and worker dispatch / move fell
      53→41 ms. ProfileFast SHA256 was
      `0fa14f8783945b6a66370988579feb8e67328bcc94a22f6f55f2bba0f5e3e91e`.
    - Final hook-free ReleaseFast DLL SHA256 installed in the fast wrapper:
      `898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.
      Wrapper `CvGameUtils.py` stayed at
      `587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c`.
    - Ignored backups include
      `profiling/dll-backups/profilefast-improve-bonus-mode-20260710-040620`,
      `profiling/dll-backups/improve-bonus-owned-plot-service-20260710-041340`,
      `profiling/dll-backups/clean-control-improve-bonus-owned-plot-service-20260710-041948`,
      `profiling/dll-backups/profilefast-improve-bonus-owned-plot-service-20260710-042553`,
      and
      `profiling/dll-backups/install-releasefast-improve-bonus-owned-plot-service-20260710-043218`.
    - Across all 12 runs, `PythonErr.log` was empty, every INI backup matched
      restored state, transient `AutoVerify.ini` was absent, no fast-wrapper
      Wine process remained, and the control DLL stayed at
      `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

Latest rejected follow-up: do not retry `MNAI_OPT_FORT_OWNED_PLOT_SERVICE`
without a different design or new evidence. ProfileFast batch
`profilefast-fort-mode-batch-20260710-044342` measured 587.7 ms/turn and found
Advanced Tactics disabled in all 32 `AI_fortTerritory` calls: 81,920
`AI_plotValid` checks produced 656 owned eligible plots. The exact conditional
owned-list candidate nevertheless regressed in the paired no-profiler test:
`candidate-fort-owned-plot-service-releaseverify-batch-20260710-045054`
measured 568.7 ms/turn (568.7, 566.0, 569.7) versus flag-off
`clean-fort-owned-plot-service-control-releaseverify-batch-20260710-045702` at
564.0 ms/turn (564.0, 564.3, 564.0). The candidate and build flag were removed;
the ProfileFast-only mode/ownership markers remain. DLL SHA256 values were
`d783d618085401bffa75eb5c96b03efc9534da18a5b0328d4f41779e99d761a2`
(attribution), `2fe15da79f0a8d46f10295e050cd0b6483028707095e8b7dfb930484159185f1`
(candidate), and
`c302c9f56922e18273809532cb5e56bd01eccf38a3cbf4a1c43baaa0140719eb`
(control). The fast wrapper was restored to hook-free ReleaseFast SHA256
`898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

22. Current-plot yield attribution inside `AI_getImprovementValue`
    - Retained ProfileFast-only scopes split the current plot's nature-yield
      and current-improvement-yield calculations from the broader
      `yield_diff_loop`. This tests a lazy, per-`AI_bestPlotBuild` context
      without retrying the rejected whole-city-radius snapshot services.
    - Batch:
      `profilefast-current-yield-attribution-batch-20260710-060653` measured a
      604.3 ms/turn median (604.3, 612.0, 594.3) with ProfileFast DLL SHA256
      `6a2858c3d179d0aafb95c69179e458882bba6e2332fec594747ea62c40c7f385`.
    - Every run recorded 2,196 current-nature calculations and 948
      current-improvement calculation scopes, but both aggregates rounded to
      0 ms. The complete yield-difference band was only 12 ms / 732 calls,
      while `AI_bestPlotBuild::improvement_value_loop` remained 70-73 ms.
    - No release candidate was built: the proposed context cannot explain the
      dominant build-loop cost and would add plumbing to remove an unmeasured
      aggregate. Future build work should target the 42 ms / 93,638-call build
      validity scan through a genuinely broader city/worker legality service,
      not another local yield cache or pre-indexed improvement build list.
    - Ignored backups:
      `profiling/dll-backups/profilefast-current-yield-attribution-20260710-060246`
      and
      `profiling/dll-backups/restore-player-after-current-yield-attribution-20260710-061029`.
      All three Python error logs were empty, INI state was restored,
      transient `AutoVerify.ini` was absent, no wrapper Wine process remained,
      and the playable wrapper was restored to hook-free ReleaseFast SHA256
      `898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

23. Explorer range/full sequence-overlap attribution
    - Added a ProfileFast-only sequence ledger around the exact
      `AI_exploreMove` range-3 → failed pillage-range-3 → full-search path.
      It records map plots visited by `AI_exploreRange(3)` and counts which
      plots the subsequent `AI_explore()` revisits, passes through
      `AI_plotValid`, and rescans for adjacent fog/contact. It does not cache
      or reuse pathfinder, score, RNG, mission, or plot-valid results.
    - Batch:
      `profilefast-explore-sequence-overlap-batch-20260710-061846` measured a
      594.0 ms/turn median (600.0, 589.0, 594.0). ProfileFast DLL SHA256 was
      `4dfd33efa7169c4a0619bd8f6d7bc49d4b2adede21712a6156e23bf3ecf466cf`.
    - Every run found 6,942 range/full plot overlaps, but only 1,464 overlap
      plots passed `AI_plotValid` and only 362 reached repeated adjacent
      scoring. Both overlap-specific aggregates rounded to 0 ms. The complete
      full-search adjacent scan was only 1-2 ms; prior path-request attribution
      already showed 103 cross-context path keys averaging about 1 microsecond
      in the full search.
    - No release cache was built. The exact reusable ceiling is too small to
      justify a sequence vector/context, and path reuse across the intervening
      pillage search would require mutable unit/FAStar state that the ledger
      deliberately does not assume stable. Do not add an explorer sequence
      cache without materially different evidence.
    - Ignored backups:
      `profiling/dll-backups/profilefast-explore-sequence-overlap-20260710-061459`
      and
      `profiling/dll-backups/restore-player-after-explore-sequence-overlap-20260710-062146`.
      All three Python error logs were empty, restored INI state matched each
      backup, transient `AutoVerify.ini` was absent, no wrapper process
      remained, and the fast wrapper was restored to hook-free ReleaseFast
      SHA256
      `898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

24. Turn-level `canBuild` duplicate attribution and rejected assignment route service
    - Added a retained ProfileFast-only ledger keyed by exact player, plot,
      build, era-test, and visibility-test request identity. It counts repeated
      identities within a game turn but does not cache results; plot, tech,
      unit, Python callback, plot-group, and economic state can mutate.
    - `profilefast-canbuild-turn-duplicates-batch-20260710-062751` measured a
      597.0 ms/turn median and found 17,933 duplicate identities among 55,521
      `CvPlayer::canBuild` calls (32.3%) in every run. ProfileFast SHA256 was
      `25b893701097d0d69ac3f3034326fb7405950932a007859b7376754f1c550916`.
    - Caller refinement batch
      `profilefast-canbuild-caller-duplicates-batch-20260710-063521` measured
      591.7 ms/turn and attributed 3,925 duplicates to
      `AI_bestPlotBuild`, only 130 to unit AI, and 13,878 to other callers.
      Its ProfileFast SHA256 was
      `5857cf8e5756ecb1a1a6e36b79b03d8cb792684f6489e21bb958b79f189d556c`.
      The `AI_bestPlotBuild` path-request context is retained for future
      caller attribution.
    - A distinct lazy `MNAI_OPT_ASSIGN_BEST_ROUTE_SERVICE` candidate cached
      `CvPlayer::getBestRoute(plot)` only during one `AI_assignWorkingPlots`
      scope. It used generation-stamped map-index storage and reset at every
      assignment, avoiding the rejected eager whole-radius snapshot and the
      rejected per-improvement route hoist.
    - Release candidate batch
      `candidate-assign-best-route-service-releaseverify-batch-20260710-064425`
      measured 569.0 ms/turn (572.0, 569.0, 566.7; mean 569.2) versus paired
      flag-off
      `clean-assign-best-route-service-control-releaseverify-batch-20260710-065038`
      at 571.0 ms/turn (573.3, 571.0, 566.7; mean 570.3). Candidate Verify
      SHA256 was
      `a42a61989eecec6df5b5766f083771c92c94c3960d3b71c2ad7f799a484f4284`;
      control Verify SHA256 was
      `c7d0122c2844cc9a54cfd6e6b6f88cf7af411c9e4d7e9db3ebe409debed43c5c`.
    - The release edge was rejected because matching ProfileFast batch
      `profilefast-assign-best-route-service-batch-20260710-065641` showed no
      movement: `getBestRoute` stayed 39 ms / 27,655 calls, `canBuild` stayed
      69-70 ms / 55,521 calls, `AI_assignWorkingPlots` stayed 89 ms, and
      `AI_plotValue` stayed 75 ms. ProfileFast median was 591.7 ms/turn and DLL
      SHA256 was
      `a4e2eb75cc80fa56512b29f90feebc42c53f51523a648f7f654f8c732f21c7ae`.
      The service, scope, header declarations, and build flags were removed.
      Do not retry assignment-scoped best-route caching without caller
      evidence showing the repeated route requests actually occur there.
    - Ignored backups include
      `profiling/dll-backups/profilefast-canbuild-turn-duplicates-20260710-062416`,
      `profiling/dll-backups/profilefast-canbuild-caller-duplicates-20260710-063521`,
      `profiling/dll-backups/assign-best-route-service-20260710-064425`,
      `profiling/dll-backups/clean-control-assign-best-route-service-20260710-065038`,
      `profiling/dll-backups/profilefast-assign-best-route-service-20260710-065641`,
      and
      `profiling/dll-backups/restore-player-after-assign-best-route-rejection-20260710-070013`.
      Across all 15 runs, Python error logs were empty, INI state was restored,
      transient `AutoVerify.ini` was absent, no wrapper process remained, and
      the player wrapper was restored to hook-free ReleaseFast SHA256
      `898071dcb021710f23f778dd18538ac8bca989c646c0dd18a4059fb97ca12976`.

25. Best-route leaf attribution and rejected plot-scoped route service
    - Retained ProfileFast-only caller contexts now distinguish
      `AI_updateBestBuild`, `AI_plotValue`, player-wide unimproved-bonus scans,
      city improvable-bonus scans, the `CvUnit::canBuild` gateway, and the
      `CvPlayer::getBestRoute` gateway. The turn-identity ledger still records
      observations only; it never reuses a legality result.
    - `profilefast-canbuild-updatebest-refinement-batch-20260710-070757`
      measured 605.0 ms/turn (605.0, 604.3, 632.0), and ruled out the outer
      `AI_updateBestBuild` and `AI_plotValue` labels. ProfileFast SHA256 was
      `3702abd38e04dde70231f0cdaf13d4ecd0c4f4fbae65984a5c4cc6acd5e83587`.
      `profilefast-canbuild-mapscan-refinement-batch-20260710-071553`
      attributed only 208 duplicate identities to
      `CvPlayer::countUnimprovedBonuses`; its noisy median was 692.0 ms/turn
      and SHA256 was
      `dbb77d98c7b202d8f138d3b6020bc14dff89351fb858dcfa1e2de7838cca27bf`.
    - The decisive leaf batch,
      `profilefast-canbuild-leaf-refinement-batch-20260710-072428`, measured
      606.3 ms/turn and split all 17,933 duplicates into 17,183 through
      `CvPlayer::getBestRoute` (95.8%), 446 remaining under
      `AI_bestPlotBuild`, 208 in the unimproved-bonus scan, and 96 through
      `CvUnit::canBuild`. ProfileFast SHA256 was
      `347c08910a38405b585be01f2329dc5346a2f3aaa9bde96b690fc135a322af81`.
    - New evidence justified a design distinct from the rejected
      per-improvement context: `MNAI_OPT_BEST_PLOT_ROUTE_SERVICE` computed one
      exact best-route answer for a complete `AI_bestPlotBuild` plot
      evaluation and passed it through every candidate-improvement yield
      comparison. No plot, tech, treasury, build-progress, or ownership state
      mutates inside that scope.
    - `profilefast-best-plot-route-service-batch-20260710-073441` confirmed
      movement at a 593.7 ms/turn median versus the 606.3 attribution batch:
      `getBestRoute` fell 27,655→22,526 calls and about 39→30-32 ms,
      `canBuild` fell 55,521→52,709 calls and about 71→63 ms, and
      `AI_bestPlotBuild` fell about 85→75 ms. Candidate ProfileFast SHA256 was
      `c354a9d0bd19fef8d6e6419c71f1a1d4f8e763580adbc5db09473cd773d9e93a`.
    - Release timing nevertheless rejected it. Candidate batch
      `candidate-best-plot-route-service-releaseverify-batch-20260710-074054`
      measured 571.7 ms/turn (576.0, 569.0, 571.7; mean 572.2) versus paired
      flag-off
      `clean-best-plot-route-service-control-releaseverify-batch-20260710-074706`
      at 567.7 ms/turn (576.3, 567.7, 565.3; mean 569.8). Candidate/control
      Verify SHA256 values were
      `189ff6cff4bd397d3c0a2540aa181f53fd547b25bcf63e7520ead0646c03466d`
      and
      `7a660cfdc1b228de7e5ef7ea1d012e9479217055929e3c60f3844d63e7566117`.
      The 4.0 ms median regression outweighed the profiler reduction, so all
      service code, overloads, and flags were removed. Do not retry the
      plot-scoped or older per-improvement route hoists without a different
      code shape or broader service evidence.
    - A clean hook-free `ReleaseFast` rebuild succeeded and was installed at
      SHA256
      `747e84bf21525cb3083ebf65dd83c5e9b0c84d51b9aeec74ddc7d1ba0d18a501`;
      string inspection found no AutoVerify/custom-profiler hooks. The control
      wrapper remained
      `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
      Across all 18 attribution/candidate/control runs, `PythonErr.log` was
      empty, INI state was restored, transient `AutoVerify.ini` was absent,
      and no wrapper Wine process remained.
    - Ignored backups:
      `profiling/dll-backups/profilefast-canbuild-updatebest-refinement-20260710-070752`,
      `profiling/dll-backups/profilefast-canbuild-mapscan-refinement-20260710-071546`,
      `profiling/dll-backups/profilefast-canbuild-leaf-refinement-20260710-072423`,
      `profiling/dll-backups/profilefast-best-plot-route-service-20260710-073435`,
      `profiling/dll-backups/candidate-best-plot-route-service-20260710-074048`,
      `profiling/dll-backups/clean-control-best-plot-route-service-20260710-074654`,
      `profiling/dll-backups/restore-player-after-best-plot-route-rejection-20260710-075027`,
      and
      `profiling/dll-backups/install-reverted-releasefast-after-best-plot-route-20260710-075425`.

26. `MNAI_OPT_CONTIGUOUS_TURN_UPDATES`
    - Added `ReleaseFastTrace`, a fixed-cost exclusive phase tracer plus
      scheduler snapshots and deterministic post-turn state fingerprints. The
      flag-off trace batch
      `releasefasttrace-scheduler-batch-20260710-205418` measured 582.0
      ms/turn with trace DLL SHA256
      `086b927640f8eab02cc6c05cdff9cfd22b466abee40cec1a9052f5f1577e17e4`.
      It accounted for 579.8 ms/turn: 412.8 ms/turn (71.2%) was spent between
      two engine-to-DLL update callbacks, while only about 167 ms/turn was
      executing the measured DLL call tree.
    - Scheduler snapshots found zero busy groups, mission timers, or combat
      groups at the continuation boundary. The active human's show-enemy,
      show-friendly, quick-move, quick-attack, and quick-defense options were
      all off. A no-focus-guard batch and a foreground-launch probe retained
      the same large callback gap, ruling out the harness focus guard as the
      cause.
    - The accepted candidate lets `CvGame::update` run another complete logical
      update slice immediately in single-player only when no active group is
      busy and no human is waiting for input. Every internal slice retains the
      original `sendPlayerOptions`, `gameUpdate` Python event, `doTurn`, score,
      war, movement, timer, assignment, alive-test, and turn-slice order. It
      yields on busy missions/combat, human input, multiplayer, WorldBuilder,
      game-over state, or after a 256-slice safety cap. It changes scheduling,
      not AI scoring, search coverage, RNG order, or decisions.
    - Candidate trace batch
      `candidate-contiguous-turn-updates-trace-batch-20260710-210856` measured
      170.7 ms/turn (172.7, 169.3, 170.7), down 411.3 ms/turn / 70.7% from the
      matching trace baseline. Accounted time was 168.5 ms/turn and callback
      gap time fell to zero. Candidate trace DLL SHA256 was
      `2274d59df38e9522a0a3715e4542da2337cc71ffcad2d1662df63ce33831ca4a`.
    - All three candidate trace runs produced identical baseline-matching
      fingerprints: turn 111 `136614134cac9f91` (29 cities / 129 units / 125
      groups), turn 112 `5968e631201a6a5a` (29 / 126 / 122), and turn 113
      `bb2966ba6eab8bc5` (29 / 130 / 127).
    - The accepted no-profiler batch
      `candidate-contiguous-turn-updates-releaseverify-batch-20260710-211556`
      measured a 163.3 ms/turn median (163.3, 162.3, 172.0; mean 165.9) with
      ReleaseFastVerify DLL SHA256
      `5f91d1908c777ed6a1f74a89e1adfd2c3cb3a733216ad588e40555ca933fe0c5`.
      This is 382.7 ms/turn / 70.1% below the previous documented 546.0
      ms/turn best and 685.4 ms/turn / 80.8% below the original 848.7
      ms/turn baseline. It clears Gates A-D and the requested sub-200 target.
    - Final hook-free player `ReleaseFast` DLL SHA256 installed in More Naval
      AI Fast.app:
      `9cbfb4cc162bd145d1533e1c63fd17ace94524cd11a3e0a090099eaf6ad82e1d`.
      String inspection found no AutoVerify, custom-profiler, release-trace,
      scheduler-trace, or fingerprint hooks. Wrapper `CvGameUtils.py` remained
      `587b1887ee587c0990754cea7e0bf8c500414db642fbf5346a67b516e0f31a3c`.
    - Ignored backups include
      `profiling/dll-backups/releasefasttrace-scheduler-20260710-205412`,
      `profiling/dll-backups/contiguous-turn-trace-20260710-210850`,
      `profiling/dll-backups/contiguous-turn-releaseverify-20260710-211551`,
      and
      `profiling/dll-backups/install-contiguous-turn-releasefast-20260710-212322`.
      Across the accepted trace and release batches, PythonErr logs were empty,
      INI state was restored, transient AutoVerify.ini was absent, no fast
      wrapper Wine process remained, and the control DLL stayed at
      `11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.
