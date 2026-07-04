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
  --save "$HOME/Documents/my games/Beyond The Sword/Saves/single/PROFILE_TEST.CivBeyondSwordSave" \
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

The batch summary is written to:

```text
profiling/autoverify/<label>-batch-<stamp>/BENCHMARK.md
profiling/autoverify/<label>-batch-<stamp>/benchmark.json
```

Per-run outputs are written under:

```text
profiling/autoverify/<run-id>/
```

Expected artifacts:

```text
autoverify.csv
custom_profile.csv
custom_profile.log
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
5. Launch More Naval AI Fast.app.
6. Wait for AutoVerify.py to write event=complete, or infer completion once
   custom_profile.csv reaches the target turn.
7. Quit the fast app, fall back to killing only this wrapper's Wine processes if
   normal quit leaves helpers alive, copy logs, summarize custom_profile.csv,
   restore settings.
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

Current best local batch: candidate-pathvalid-detail-tinyhelper
Current best median: 652.3 ms/turn (-23.1%)
Current best DLL SHA256: ed8d967ca7a5a6e6a15b4001a861019a4b13e2c9438217c56518f91dcdf13445
Current accepted delta: CvPlayerAI::AI_unitUpdate dropped to about 156 ms,
CvSelectionGroup::generatePath dropped to about 93 ms, and the benchmark median
beat the 20% target by about 26.6 ms/turn.

Gate check: 3-run AutoVerify batch completed, PythonErr logs were empty,
CivilizationIV.ini was restored, transient AutoVerify.ini was removed, no test
wrapper Wine processes remained, and the control wrapper DLL stayed unchanged.

Caveat: the latest stack mixes gameplay-code optimizations with profile-build
instrumentation reductions. The tiny-helper marker skips improve the profiled
DLL benchmark, but a normal release DLL does not compile CUSTOM_PROFILER
markers, so validate release-player speed separately before claiming that full
23.1% as user-visible turn-time improvement.

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
```
