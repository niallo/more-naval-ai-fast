#!/usr/bin/env python3
import argparse
import csv
from collections import defaultdict
from pathlib import Path


TRACKED = [
    "CvGame::update",
    "CvPlayer::setTurnActive",
    "CvPlayerAI::AI_unitUpdate",
    "CvPlayerAI::AI_unitUpdate::force_separate",
    "CvPlayerAI::AI_unitUpdate::copy_group_cycle",
    "CvPlayerAI::AI_unitUpdate::movement_priority_sort",
    "CvPlayerAI::AI_unitUpdate::group_updates",
    "CvSelectionGroupAI::AI_update",
    "CvSelectionGroupAI::AI_update::group_attack",
    "CvSelectionGroupAI::AI_update::head_unit_update",
    "CvSelectionGroupAI::AI_update::follow_units",
    "CvUnitAI::AI_update",
    "CvUnitAI::AI_update::barbarian_python",
    "CvUnitAI::AI_update::ffh_pre",
    "CvUnitAI::AI_update::early_guards",
    "CvUnitAI::AI_update::construct_scan",
    "CvUnitAI::AI_update::ai_control",
    "CvUnitAI::AI_update::automated",
    "CvUnitAI::AI_update::non_automated_prep",
    "CvUnitAI::AI_update::unitai_dispatch",
    "CvSelectionGroup::generatePath()",
    "CvPlayerAI::AI_unitValue",
    "CvPlayerAI::AI_unitValue::score",
]


def load_rows(path):
    rows = []
    with path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            try:
                row["turn"] = int(row["turn"])
                row["interval_ms"] = int(row["interval_ms"])
                row["total_ms"] = int(row["total_ms"])
                row["calls"] = int(row["calls"])
            except (KeyError, ValueError) as exc:
                raise SystemExit(f"Invalid profile CSV row in {path}: {row!r}: {exc}")
            rows.append(row)
    return rows


def summarize(rows, turns=None):
    rows = [row for row in rows if row["turn"] != 0]
    if turns is not None:
        turns = set(turns)
        rows = [row for row in rows if row["turn"] in turns]

    turn_values = sorted({row["turn"] for row in rows})
    full_by_turn = {}
    samples = defaultdict(lambda: [0, 0])

    for row in rows:
        if row["sample"] == "Full turn or sample interval":
            full_by_turn[row["turn"]] = full_by_turn.get(row["turn"], 0) + row["interval_ms"]
        else:
            samples[row["sample"]][0] += row["total_ms"]
            samples[row["sample"]][1] += row["calls"]

    return {
        "turns": turn_values,
        "full_by_turn": full_by_turn,
        "samples": samples,
    }


def parse_turns(value):
    if not value:
        return None
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
    return turns


def table_lines(samples, names):
    lines = []
    for name in names:
        total_ms, calls = samples.get(name, [0, 0])
        if total_ms == 0 and calls == 0:
            continue
        per_call = (total_ms * 1000.0 / calls) if calls else 0.0
        lines.append(f"{total_ms:7d} ms / {calls:9d} calls / {per_call:8.3f} us  {name}")
    return lines


def write_summary(path, run_id, current, baseline=None, baseline_label=None):
    lines = []
    lines.append(f"# AutoVerify Profile Summary: {run_id}")
    lines.append("")
    lines.append("## Turn Intervals")
    lines.append("")
    if current["turns"]:
        total = sum(current["full_by_turn"].get(turn, 0) for turn in current["turns"])
        avg = total / float(len(current["turns"]))
        lines.append(f"Turns: {current['turns'][0]}-{current['turns'][-1]}")
        lines.append(f"Total interval: {total:.0f} ms")
        lines.append(f"Average interval: {avg:.1f} ms")
        lines.append("")
        lines.append("```text")
        for turn in current["turns"]:
            lines.append(f"Turn {turn}: {current['full_by_turn'].get(turn, 0)} ms")
        lines.append("```")
    else:
        lines.append("No non-load turns found.")
    lines.append("")

    lines.append("## Tracked Samples")
    lines.append("")
    lines.append("```text")
    tracked_lines = table_lines(current["samples"], TRACKED)
    lines.extend(tracked_lines if tracked_lines else ["No tracked samples found."])
    lines.append("```")
    lines.append("")

    lines.append("## Top Samples")
    lines.append("")
    lines.append("```text")
    for name, (total_ms, calls) in sorted(current["samples"].items(), key=lambda item: item[1][0], reverse=True)[:30]:
        per_call = (total_ms * 1000.0 / calls) if calls else 0.0
        lines.append(f"{total_ms:7d} ms / {calls:9d} calls / {per_call:8.3f} us  {name}")
    lines.append("```")

    if baseline is not None:
        lines.append("")
        lines.append("## Comparison")
        lines.append("")
        label = baseline_label or "baseline"
        lines.append(f"Compared against: {label}")
        lines.append("")
        lines.append("```text")
        for name in TRACKED:
            cur_ms, cur_calls = current["samples"].get(name, [0, 0])
            base_ms, base_calls = baseline["samples"].get(name, [0, 0])
            if cur_ms == 0 and base_ms == 0 and cur_calls == 0 and base_calls == 0:
                continue
            delta = cur_ms - base_ms
            pct = (100.0 * delta / base_ms) if base_ms else 0.0
            lines.append(f"{delta:7d} ms / {pct:7.1f}%  {name}  ({base_ms} -> {cur_ms} ms)")
        lines.append("```")

    path.write_text("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser(description="Summarize Civ4 custom_profile.csv output.")
    parser.add_argument("csv", type=Path)
    parser.add_argument("--out", type=Path, default=None)
    parser.add_argument("--run-id", default="profile")
    parser.add_argument("--turns", default=None, help="Comma/range turn filter, e.g. 111-113 or 111,112,113")
    parser.add_argument("--compare", type=Path, default=None)
    parser.add_argument("--compare-label", default=None)
    args = parser.parse_args()

    turns = parse_turns(args.turns)
    current = summarize(load_rows(args.csv), turns)
    baseline = summarize(load_rows(args.compare), turns) if args.compare else None
    out = args.out or args.csv.with_name("SUMMARY.md")
    write_summary(out, args.run_id, current, baseline, args.compare_label)


if __name__ == "__main__":
    main()
