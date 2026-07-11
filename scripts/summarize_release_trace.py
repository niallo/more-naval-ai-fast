#!/usr/bin/env python3
"""Summarize fixed-cost ReleaseFastTrace output across an AutoVerify batch."""

import argparse
import csv
import json
import statistics
from collections import defaultdict
from pathlib import Path


def parse_turns(value):
    turns = []
    for part in value.split(","):
        part = part.strip()
        if not part:
            continue
        if "-" in part:
            start, end = part.split("-", 1)
            turns.extend(range(int(start), int(end) + 1))
        else:
            turns.append(int(part))
    return sorted(set(turns))


def read_trace(run_dir, turns):
    path = run_dir / "release_trace.csv"
    phase_us = defaultdict(int)
    phase_calls = defaultdict(int)
    with path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            turn = int(row["turn"])
            if turn not in turns:
                continue
            phase_us[row["phase"]] += int(row["exclusive_us"])
            phase_calls[row["phase"]] += int(row["calls"])
    return phase_us, phase_calls


def read_fingerprints(run_dir, turns):
    path = run_dir / "state_fingerprint.csv"
    if not path.exists():
        return None
    result = {}
    with path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            turn = int(row["turn"])
            if turn in turns:
                result[turn] = {
                    "hash": f"{row['hash_hi']}{row['hash_lo']}",
                    "cities": int(row["cities"]),
                    "units": int(row["units"]),
                    "groups": int(row["groups"]),
                }
    return result


def read_scheduler(run_dir, turns):
    path = run_dir / "scheduler_trace.csv"
    if not path.exists():
        return []
    rows = []
    with path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            if int(row["turn"]) in turns:
                rows.append({key: int(value) for key, value in row.items()})
    return rows


def median(values):
    return statistics.median(values) if values else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_dirs", nargs="+", type=Path)
    parser.add_argument("--label", default="release-trace")
    parser.add_argument("--turns", required=True)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--json-out", required=True, type=Path)
    args = parser.parse_args()

    turns = parse_turns(args.turns)
    if not turns:
        raise SystemExit("No turns selected")

    traces = []
    fingerprints = []
    schedulers = []
    for run_dir in args.run_dirs:
        if not (run_dir / "release_trace.csv").exists():
            raise SystemExit(f"Missing release_trace.csv: {run_dir}")
        phase_us, phase_calls = read_trace(run_dir, turns)
        traces.append((run_dir.name, phase_us, phase_calls))
        fingerprints.append((run_dir.name, read_fingerprints(run_dir, turns)))
        schedulers.append((run_dir.name, read_scheduler(run_dir, turns)))

    phases = sorted({phase for _, values, _ in traces for phase in values})
    phase_rows = []
    for phase in phases:
        totals_us = [values.get(phase, 0) for _, values, _ in traces]
        total_calls = [values.get(phase, 0) for _, _, values in traces]
        phase_rows.append(
            {
                "phase": phase,
                "median_total_us": median(totals_us),
                "median_us_per_turn": median(totals_us) / len(turns),
                "median_calls": median(total_calls),
                "median_calls_per_turn": median(total_calls) / len(turns),
                "run_total_us": dict(zip((name for name, _, _ in traces), totals_us)),
            }
        )
    phase_rows.sort(key=lambda row: row["median_total_us"], reverse=True)

    run_traced_us = {
        name: sum(values.values()) for name, values, _ in traces
    }
    median_traced_us = median(list(run_traced_us.values()))

    present_fingerprints = [value for _, value in fingerprints if value is not None]
    fingerprints_identical = bool(present_fingerprints) and all(
        value == present_fingerprints[0] for value in present_fingerprints[1:]
    )
    fingerprints_complete = len(present_fingerprints) == len(fingerprints) and all(
        set(value) == set(turns) for value in present_fingerprints
    )

    scheduler_rows = [row for _, rows in schedulers for row in rows]
    scheduler_summary = {}
    if scheduler_rows:
        keys = [
            "callback_gap_us",
            "active_players",
            "busy_groups",
            "mission_timer_groups",
            "combat_groups",
            "mission_timer_total",
        ]
        scheduler_summary = {
            "samples_per_run": median([len(rows) for _, rows in schedulers]),
            **{f"median_{key}": median([row[key] for row in scheduler_rows]) for key in keys},
            "options": {
                key: sorted({row[key] for row in scheduler_rows})
                for key in [
                    "show_enemy_moves",
                    "show_friendly_moves",
                    "quick_moves",
                    "quick_attack",
                    "quick_defense",
                ]
            },
        }

    result = {
        "label": args.label,
        "turns": turns,
        "runs": [name for name, _, _ in traces],
        "median_traced_us": median_traced_us,
        "median_traced_ms_per_turn": median_traced_us / 1000.0 / len(turns),
        "phases": phase_rows,
        "fingerprints_complete": fingerprints_complete,
        "fingerprints_identical": fingerprints_identical,
        "fingerprints": dict(fingerprints),
        "scheduler": scheduler_summary,
    }
    args.json_out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")

    lines = [
        f"# Release Trace Summary: {args.label}",
        "",
        f"Runs: {len(traces)}",
        f"Turns: {','.join(str(turn) for turn in turns)}",
        f"Median accounted wall time: {result['median_traced_ms_per_turn']:.1f} ms/turn",
        f"State fingerprints: {'identical' if fingerprints_identical and fingerprints_complete else 'MISMATCH OR INCOMPLETE'}",
        "",
        "## Exclusive Phases",
        "",
        "| Phase | Median ms/turn | Median calls/turn | Accounted share |",
        "| --- | ---: | ---: | ---: |",
    ]
    for row in phase_rows:
        share = 100.0 * row["median_total_us"] / median_traced_us if median_traced_us else 0.0
        lines.append(
            f"| {row['phase']} | {row['median_us_per_turn'] / 1000.0:.3f} | "
            f"{row['median_calls_per_turn']:.1f} | {share:.1f}% |"
        )

    if scheduler_summary:
        lines.extend(
            [
                "",
                "## Scheduler Boundary",
                "",
                f"Median samples/run: {scheduler_summary['samples_per_run']}",
                f"Median callback gap/sample: {scheduler_summary['median_callback_gap_us'] / 1000.0:.1f} ms",
                f"Median busy groups/sample: {scheduler_summary['median_busy_groups']}",
                f"Median mission-timer groups/sample: {scheduler_summary['median_mission_timer_groups']}",
                f"Median combat groups/sample: {scheduler_summary['median_combat_groups']}",
                "",
                "```json",
                json.dumps(scheduler_summary["options"], sort_keys=True),
                "```",
            ]
        )

    lines.extend(["", "## Fingerprints", ""])
    if fingerprints_complete:
        lines.extend(["| Turn | Hash | Cities | Units | Groups |", "| ---: | --- | ---: | ---: | ---: |"]) 
        for turn in turns:
            item = present_fingerprints[0][turn]
            lines.append(
                f"| {turn} | `{item['hash']}` | {item['cities']} | {item['units']} | {item['groups']} |"
            )
    else:
        lines.append("One or more runs lacked a complete fingerprint set.")

    args.out.write_text("\n".join(lines) + "\n")

    if not fingerprints_complete or not fingerprints_identical:
        raise SystemExit("Release trace state fingerprints differ or are incomplete")


if __name__ == "__main__":
    main()
