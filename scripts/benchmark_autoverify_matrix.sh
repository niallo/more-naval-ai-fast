#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

CONFIG=""
APP="$HOME/Applications/More Naval AI Fast.app"
BTS_DOCS="$HOME/Documents/my games/Beyond The Sword"
DEFAULT_RUNS=1
DEFAULT_TURNS=1
DEFAULT_TIMEOUT=900

usage() {
  cat <<'USAGE'
Usage:
  scripts/benchmark_autoverify_matrix.sh --config matrix.tsv [options]

Matrix TSV columns, with a header row:
  label<TAB>role<TAB>save_path<TAB>turn_filter<TAB>turns<TAB>runs<TAB>timeout<TAB>notes

Only label, role, save_path, and turn_filter are required. Empty turns/runs/timeout
fields use defaults. Keep matrix TSV files local and untracked if they contain
private save paths.

Options:
  --config PATH       TSV matrix config
  --app PATH          More Naval AI Fast.app path
  --bts-docs PATH     Beyond The Sword documents folder
  --runs N            Default run count for rows with empty runs
  --turns N           Default turn count for rows with empty turns
  --timeout SECONDS   Default timeout for rows with empty timeout
  -h, --help          Show this help
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --config) CONFIG="$2"; shift 2 ;;
    --app) APP="$2"; shift 2 ;;
    --bts-docs) BTS_DOCS="$2"; shift 2 ;;
    --runs) DEFAULT_RUNS="$2"; shift 2 ;;
    --turns) DEFAULT_TURNS="$2"; shift 2 ;;
    --timeout) DEFAULT_TIMEOUT="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

[[ -n "$CONFIG" ]] || { usage >&2; exit 2; }
[[ -f "$CONFIG" ]] || { echo "matrix: config not found: $CONFIG" >&2; exit 2; }

STAMP="$(date +%Y%m%d-%H%M%S)"
OUT="$REPO_ROOT/profiling/autoverify/matrix-run-$STAMP.md"

{
  printf '# AutoVerify Matrix Run: %s\n\n' "$STAMP"
  printf '| Label | Role | Runs | Turns | Turn filter | Median ms/turn | Batch |\n'
  printf '| --- | --- | ---: | ---: | --- | ---: | --- |\n'
} > "$OUT"

tail -n +2 "$CONFIG" | while IFS=$'\t' read -r label role save_path turn_filter turns runs timeout notes; do
  [[ -n "${label:-}" ]] || continue
  [[ "${label:0:1}" == "#" ]] && continue

  turns="${turns:-$DEFAULT_TURNS}"
  runs="${runs:-$DEFAULT_RUNS}"
  timeout="${timeout:-$DEFAULT_TIMEOUT}"

  if [[ ! -f "$save_path" ]]; then
    echo "matrix: save not found for $label: $save_path" >&2
    exit 1
  fi

  run_label="matrix-${label}"
  echo "matrix: running $run_label ($role)"
  output="$("$SCRIPT_DIR/benchmark_autoverify.sh" \
    --label "$run_label" \
    --runs "$runs" \
    --turns "$turns" \
    --turn-filter "$turn_filter" \
    --save "$save_path" \
    --app "$APP" \
    --bts-docs "$BTS_DOCS" \
    --timeout "$timeout")"
  echo "$output"

  json_path="$(printf '%s\n' "$output" | awk '/^benchmark: json / {print $3}' | tail -1)"
  batch_dir="$(dirname "$json_path")"
  median="$(python3 - "$json_path" <<'PY'
import json, sys
with open(sys.argv[1]) as f:
    data = json.load(f)
print("%.1f" % data["median_avg_ms"])
PY
)"

  {
    printf '| `%s` | %s | %s | %s | `%s` | %s | `%s` |\n' \
      "$label" "$role" "$runs" "$turns" "$turn_filter" "$median" "${batch_dir#$REPO_ROOT/}"
  } >> "$OUT"
done

echo "matrix: sanitized summary $OUT"
