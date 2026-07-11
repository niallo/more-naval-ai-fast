# AutoVerify Benchmark Matrix

The primary benchmark remains the fixed private turn-110 save over turns
111-113. Secondary matrix saves are local-only and must not be committed.

Matrix roles:

| Label | Role | Purpose | Status |
| --- | --- | --- | --- |
| `primary-turn-0110` | primary | Authoritative niallohiggins turns 111-113 gate. | Accepted release-path baseline is 546.0 ms/turn. |
| `midgame-worker-turn-0152` | city/worker-heavy | Exercises worker and city build/assignment work beyond the primary window. | 3-run baseline: 846.0 ms/turn. |
| `late-large-turn-0228` | late-game large-map | Exercises larger save size and later-turn AI workload. | 3-run baseline: 1407.0 ms/turn. |
| `early-pathing-turn-0079` | pathing-heavy | Exercises earlier exploration/pathing behavior without using the primary save. | 3-run baseline: 954.0 ms/turn. |

Latest baseline summary:

```text
profiling/autoverify/matrix-run-20260706-105554.md
```

The 2026-07-06 baseline used `ReleaseFastVerify` in the fast wrapper, then
restored the player-facing `ReleaseFast` DLL. All three 3-run/one-turn rows
completed through the direct-Wine path, all `PythonErr.log` files were empty,
transient `AutoVerify.ini` was removed, no Civ/Wine/Wineskin process remained,
the fast wrapper was restored to `ReleaseFast`
`73a90079eb7350a6ba2edd2f5182b691abbd81ba06189ab1996acb60293f818a`, and the
original/control wrapper DLL hash stayed
`11d16cd60f5f7973dfc28aff18d95f6caab2041cd001438f05da7e2dcf4a9ab6`.

Earlier one-run smoke summary:

```text
profiling/autoverify/matrix-run-20260706-103515.md
```

Run matrix rows with an untracked TSV:

```sh
scripts/benchmark_autoverify_matrix.sh --config /path/to/local-matrix.tsv
```

TSV columns:

```text
label	role	save_path	turn_filter	turns	runs	timeout	notes
```

Use `ReleaseFastVerify` in the fast wrapper for unattended matrix timing, and
restore the player-facing `ReleaseFast` DLL afterward. Store only sanitized
summary rows here; keep raw save files, raw logs, and absolute private paths out
of git.
