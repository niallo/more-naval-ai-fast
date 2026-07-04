#!/usr/bin/env python3
import argparse
import json
import statistics
from pathlib import Path

from summarize_profile import TRACKED, load_rows, parse_turns, summarize


def average_interval(summary):
    turns = summary["turns"]
    if not turns:
        return 0.0
    total = sum(summary["full_by_turn"].get(turn, 0) for turn in turns)
    return total / float(len(turns))


def run_result(path, turns):
    csv_path = path / "custom_profile.csv"
    if not csv_path.exists():
        raise SystemExit("Missing custom_profile.csv in %s" % path)
    summary = summarize(load_rows(csv_path), turns)
    return {
        "path": str(path),
        "run_id": path.name,
        "turns": summary["turns"],
        "avg_ms": average_interval(summary),
        "total_ms": sum(summary["full_by_turn"].get(turn, 0) for turn in summary["turns"]),
        "full_by_turn": summary["full_by_turn"],
        "samples": dict(summary["samples"]),
    }


def sample_rows(samples, names):
    rows = []
    for name in names:
        total_ms, calls = samples.get(name, [0, 0])
        if total_ms == 0 and calls == 0:
            continue
        per_call = (total_ms * 1000.0 / calls) if calls else 0.0
        rows.append((name, total_ms, calls, per_call))
    return rows


def write_markdown(path, label, turns, results, baseline_json=None):
    sorted_results = sorted(results, key=lambda item: item["avg_ms"])
    median_result = sorted_results[len(sorted_results) // 2]
    averages = [item["avg_ms"] for item in results]

    lines = []
    lines.append("# AutoVerify Benchmark: %s" % label)
    lines.append("")
    lines.append("Turn filter: %s" % ",".join(str(turn) for turn in turns))
    lines.append("Runs: %d" % len(results))
    lines.append("Median average interval: %.1f ms" % median_result["avg_ms"])
    lines.append("Best average interval: %.1f ms" % sorted_results[0]["avg_ms"])
    lines.append("Worst average interval: %.1f ms" % sorted_results[-1]["avg_ms"])
    if len(averages) > 1:
        lines.append("Mean average interval: %.1f ms" % statistics.mean(averages))
    lines.append("")

    if baseline_json:
        baseline = json.loads(Path(baseline_json).read_text())
        base_median = float(baseline["median_avg_ms"])
        delta = median_result["avg_ms"] - base_median
        pct = (100.0 * delta / base_median) if base_median else 0.0
        lines.append("Compared baseline: %s" % baseline.get("label", baseline_json))
        lines.append("Baseline median average interval: %.1f ms" % base_median)
        lines.append("Delta: %.1f ms / %.1f%%" % (delta, pct))
        lines.append("")

    lines.append("## Runs")
    lines.append("")
    lines.append("| Run | Avg ms | Total ms | Turns |")
    lines.append("| --- | ---: | ---: | --- |")
    for item in results:
        turn_text = ", ".join("%s:%s" % (turn, item["full_by_turn"].get(turn, 0)) for turn in item["turns"])
        lines.append("| %s | %.1f | %d | %s |" % (item["run_id"], item["avg_ms"], item["total_ms"], turn_text))
    lines.append("")

    lines.append("## Median Run Tracked Samples")
    lines.append("")
    lines.append("Median run: `%s`" % median_result["run_id"])
    lines.append("")
    lines.append("```text")
    tracked_rows = sample_rows(median_result["samples"], TRACKED)
    if tracked_rows:
        for name, total_ms, calls, per_call in tracked_rows:
            lines.append("%7d ms / %9d calls / %8.3f us  %s" % (total_ms, calls, per_call, name))
    else:
        lines.append("No tracked samples found.")
    lines.append("```")
    lines.append("")

    lines.append("## Median Run Top Samples")
    lines.append("")
    lines.append("```text")
    for name, (total_ms, calls) in sorted(median_result["samples"].items(), key=lambda item: item[1][0], reverse=True)[:30]:
        per_call = (total_ms * 1000.0 / calls) if calls else 0.0
        lines.append("%7d ms / %9d calls / %8.3f us  %s" % (total_ms, calls, per_call, name))
    lines.append("```")
    path.write_text("\n".join(lines) + "\n")


def write_json(path, label, turns, results):
    sorted_results = sorted(results, key=lambda item: item["avg_ms"])
    median_result = sorted_results[len(sorted_results) // 2]
    payload = {
        "label": label,
        "turns": turns,
        "median_avg_ms": median_result["avg_ms"],
        "median_run_id": median_result["run_id"],
        "runs": [
            {
                "run_id": item["run_id"],
                "path": item["path"],
                "avg_ms": item["avg_ms"],
                "total_ms": item["total_ms"],
                "turns": item["turns"],
                "full_by_turn": item["full_by_turn"],
            }
            for item in results
        ],
    }
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")


def main():
    parser = argparse.ArgumentParser(description="Summarize repeated AutoVerify runs.")
    parser.add_argument("run_dirs", nargs="+", type=Path)
    parser.add_argument("--label", default="benchmark")
    parser.add_argument("--turns", default="111-113")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--json-out", type=Path, default=None)
    parser.add_argument("--compare-json", type=Path, default=None)
    args = parser.parse_args()

    turns = parse_turns(args.turns)
    if not turns:
        raise SystemExit("--turns must select at least one turn")

    results = [run_result(path, turns) for path in args.run_dirs]
    if not results:
        raise SystemExit("No runs supplied")

    args.out.parent.mkdir(parents=True, exist_ok=True)
    write_markdown(args.out, args.label, turns, results, args.compare_json)
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        write_json(args.json_out, args.label, turns, results)


if __name__ == "__main__":
    main()
