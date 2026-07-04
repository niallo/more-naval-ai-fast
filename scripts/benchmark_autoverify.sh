#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

APP="$HOME/Applications/More Naval AI Fast.app"
BTS_DOCS="$HOME/Documents/my games/Beyond The Sword"
SAVE_PATH="$BTS_DOCS/Saves/single/PROFILE_TEST.CivBeyondSwordSave"
LABEL="benchmark"
RUNS=3
TURNS=3
TURN_FILTER="111-113"
TIMEOUT=900
COMPARE_JSON=""

usage() {
  cat <<'USAGE'
Usage:
  scripts/benchmark_autoverify.sh [options]

Options:
  --label NAME        Benchmark label and run-id prefix
  --runs N            Number of repeated AutoVerify runs
  --turns N           Number of in-game turns for each run
  --turn-filter SPEC  Turns to measure, e.g. 111-113
  --app PATH          More Naval AI Fast.app path
  --bts-docs PATH     Beyond The Sword documents folder
  --save PATH         Save file to load
  --timeout SECONDS   Maximum wall-clock wait per run
  --compare-json PATH Optional baseline benchmark.json for comparison
  -h, --help          Show this help
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --label) LABEL="$2"; shift 2 ;;
    --runs) RUNS="$2"; shift 2 ;;
    --turns) TURNS="$2"; shift 2 ;;
    --turn-filter) TURN_FILTER="$2"; shift 2 ;;
    --app) APP="$2"; shift 2 ;;
    --bts-docs) BTS_DOCS="$2"; shift 2 ;;
    --save) SAVE_PATH="$2"; shift 2 ;;
    --timeout) TIMEOUT="$2"; shift 2 ;;
    --compare-json) COMPARE_JSON="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

require_int() {
  [[ "$1" =~ ^[0-9]+$ ]] || {
    echo "benchmark: $2 must be a non-negative integer" >&2
    exit 2
  }
}

require_int "$RUNS" "--runs"
require_int "$TURNS" "--turns"
require_int "$TIMEOUT" "--timeout"
[[ "$RUNS" -gt 0 ]] || { echo "benchmark: --runs must be greater than zero" >&2; exit 2; }
[[ "$TURNS" -gt 0 ]] || { echo "benchmark: --turns must be greater than zero" >&2; exit 2; }

STAMP="$(date +%Y%m%d-%H%M%S)"
BATCH_DIR="$REPO_ROOT/profiling/autoverify/${LABEL}-batch-${STAMP}"
mkdir -p "$BATCH_DIR"

run_dirs=()
for i in $(seq 1 "$RUNS"); do
  run_id="${LABEL}-r${i}-${STAMP}"
  echo "benchmark: run $i/$RUNS: $run_id"
  "$SCRIPT_DIR/autoverify_mnai.sh" \
    --app "$APP" \
    --bts-docs "$BTS_DOCS" \
    --save "$SAVE_PATH" \
    --turns "$TURNS" \
    --timeout "$TIMEOUT" \
    --run-id "$run_id"
  run_dirs+=("$REPO_ROOT/profiling/autoverify/$run_id")

  if [[ -s "$REPO_ROOT/profiling/autoverify/$run_id/PythonErr.log" ]]; then
    echo "benchmark: PythonErr.log is not empty for $run_id" >&2
    exit 1
  fi
done

summary_args=(
  "${run_dirs[@]}"
  --label "$LABEL"
  --turns "$TURN_FILTER"
  --out "$BATCH_DIR/BENCHMARK.md"
  --json-out "$BATCH_DIR/benchmark.json"
)

if [[ -n "$COMPARE_JSON" ]]; then
  summary_args+=(--compare-json "$COMPARE_JSON")
fi

"$SCRIPT_DIR/summarize_benchmark_runs.py" "${summary_args[@]}"

{
  printf 'label=%s\n' "$LABEL"
  printf 'turns=%s\n' "$TURNS"
  printf 'turn_filter=%s\n' "$TURN_FILTER"
  printf 'runs=%s\n' "$RUNS"
  printf 'summary=%s\n' "$BATCH_DIR/BENCHMARK.md"
  printf 'json=%s\n' "$BATCH_DIR/benchmark.json"
  printf 'run_dirs=\n'
  printf '%s\n' "${run_dirs[@]}"
} > "$BATCH_DIR/runs.txt"

echo "benchmark: summary $BATCH_DIR/BENCHMARK.md"
echo "benchmark: json $BATCH_DIR/benchmark.json"
