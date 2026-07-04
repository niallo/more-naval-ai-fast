#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

APP="$HOME/Applications/More Naval AI Fast.app"
BTS_DOCS="$HOME/Documents/my games/Beyond The Sword"
DEFAULT_SAVE="$BTS_DOCS/Saves/single/PROFILE_TEST.CivBeyondSwordSave"
SAVE_PATH="$DEFAULT_SAVE"
TURNS=3
TIMEOUT=900
RUN_ID="$(date +%Y%m%d-%H%M%S)"
COMPARE_CSV=""
KEEP_APP_RUNNING=0
KEYPRESS_FALLBACK=1

usage() {
  cat <<'USAGE'
Usage:
  scripts/autoverify_mnai.sh [options]

Options:
  --app PATH          More Naval AI Fast.app path
  --bts-docs PATH     Beyond The Sword documents folder
  --save PATH         Save file to load
  --turns N           Number of turns to run under AI autoplay
  --timeout SECONDS   Maximum wall-clock wait
  --run-id ID         Output directory id
  --compare CSV       Optional baseline custom_profile.csv
  --keep-app-running  Do not ask the app to quit after completion
  --no-keypress-fallback
                      Do not press Return if the game stops at a human turn
  -h, --help          Show this help
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --app) APP="$2"; shift 2 ;;
    --bts-docs) BTS_DOCS="$2"; shift 2 ;;
    --save) SAVE_PATH="$2"; shift 2 ;;
    --turns) TURNS="$2"; shift 2 ;;
    --timeout) TIMEOUT="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --compare) COMPARE_CSV="$2"; shift 2 ;;
    --keep-app-running) KEEP_APP_RUNNING=1; shift ;;
    --no-keypress-fallback) KEYPRESS_FALLBACK=0; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

INI="$BTS_DOCS/CivilizationIV.ini"
SETTINGS_DIR="$BTS_DOCS/FFH - More Naval AI/Settings"
LOG_DIR="$BTS_DOCS/Logs"
SAVE_DIR="$BTS_DOCS/Saves/single"
STAGED_SAVE="$SAVE_DIR/AUTOVERIFY.CivBeyondSwordSave"
AUTOVERIFY_INI="$SETTINGS_DIR/AutoVerify.ini"
OUT_DIR="$REPO_ROOT/profiling/autoverify/$RUN_ID"

INI_BACKUP=""
AUTOVERIFY_BACKUP=""
AUTOVERIFY_EXISTED=0

die() {
  echo "autoverify: $*" >&2
  exit 1
}

restore_files() {
  if [[ -n "$INI_BACKUP" && -f "$INI_BACKUP" ]]; then
    cp "$INI_BACKUP" "$INI"
  fi

  if [[ "$AUTOVERIFY_EXISTED" -eq 1 && -n "$AUTOVERIFY_BACKUP" && -f "$AUTOVERIFY_BACKUP" ]]; then
    cp "$AUTOVERIFY_BACKUP" "$AUTOVERIFY_INI"
  elif [[ "$AUTOVERIFY_EXISTED" -eq 0 ]]; then
    rm -f "$AUTOVERIFY_INI"
  fi
}

quit_app() {
  [[ "$KEEP_APP_RUNNING" -eq 1 ]] && return 0
  [[ -d "$APP" ]] || return 0

  local bundle_id
  bundle_id="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleIdentifier' "$APP/Contents/Info.plist" 2>/dev/null || true)"
  if [[ -n "$bundle_id" ]]; then
    osascript -e "tell application id \"$bundle_id\" to quit" >/dev/null 2>&1 || true
  else
    osascript -e "tell application \"$(basename "$APP" .app)\" to quit" >/dev/null 2>&1 || true
  fi

  sleep 2

  local wine_path
  wine_path="$APP/Contents/SharedSupport/wine"
  if pgrep -f "$wine_path" >/dev/null 2>&1; then
    pkill -f "$wine_path" >/dev/null 2>&1 || true
    sleep 1
  fi
  if pgrep -f "$wine_path" >/dev/null 2>&1; then
    pkill -9 -f "$wine_path" >/dev/null 2>&1 || true
  fi
}

archive_if_present() {
  local path="$1"
  local name
  name="$(basename "$path")"
  if [[ -e "$path" ]]; then
    mv "$path" "$OUT_DIR/preexisting-$name"
  fi
}

profile_reached_target() {
  python3 - "$LOG_DIR/autoverify.csv" "$LOG_DIR/custom_profile.csv" "$RUN_ID" <<'PY'
import csv
import sys
from pathlib import Path

autoverify = Path(sys.argv[1])
profile = Path(sys.argv[2])
run_id = sys.argv[3]

if not autoverify.exists() or not profile.exists():
    raise SystemExit(1)

start_turn = None
target_turn = None
with autoverify.open(newline="") as handle:
    for row in csv.DictReader(handle):
        if row.get("run_id") != run_id:
            continue
        event = row.get("event", "")
        if event.startswith("start_"):
            try:
                start_turn = int(row.get("start_turn", ""))
                target_turn = int(row.get("target_turn", ""))
            except ValueError:
                pass

if target_turn is None:
    raise SystemExit(1)

max_turn = -1
with profile.open(newline="") as handle:
    for row in csv.DictReader(handle):
        try:
            max_turn = max(max_turn, int(row.get("turn", "-1")))
        except ValueError:
            pass

if max_turn >= target_turn:
    with autoverify.open("a", newline="") as handle:
        handle.write("%s,complete_inferred,%d,%d,%d\n" % (
            run_id,
            start_turn if start_turn is not None else -1,
            max_turn,
            target_turn,
        ))
    raise SystemExit(0)

raise SystemExit(1)
PY
}

profile_turn_state() {
  python3 - "$LOG_DIR/autoverify.csv" "$LOG_DIR/custom_profile.csv" "$RUN_ID" <<'PY'
import csv
import sys
from pathlib import Path

autoverify = Path(sys.argv[1])
profile = Path(sys.argv[2])
run_id = sys.argv[3]

if not autoverify.exists() or not profile.exists():
    raise SystemExit(1)

start_turn = None
target_turn = None
with autoverify.open(newline="") as handle:
    for row in csv.DictReader(handle):
        if row.get("run_id") != run_id:
            continue
        if row.get("event", "").startswith("start_"):
            try:
                start_turn = int(row.get("start_turn", ""))
                target_turn = int(row.get("target_turn", ""))
            except ValueError:
                pass

if start_turn is None or target_turn is None:
    raise SystemExit(1)

max_turn = -1
with profile.open(newline="") as handle:
    for row in csv.DictReader(handle):
        try:
            max_turn = max(max_turn, int(row.get("turn", "-1")))
        except ValueError:
            pass

print("%d %d %d" % (start_turn, target_turn, max_turn))
PY
}

press_end_turn() {
  [[ "$KEYPRESS_FALLBACK" -eq 1 ]] || return 0
  [[ -d "$APP" ]] || return 0

  local bundle_id app_name
  bundle_id="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleIdentifier' "$APP/Contents/Info.plist" 2>/dev/null || true)"
  app_name="$(basename "$APP" .app)"

  if [[ -n "$bundle_id" ]]; then
    osascript >/dev/null 2>&1 <<OSA || true
tell application id "$bundle_id" to activate
delay 0.2
tell application "System Events" to key code 36
OSA
  else
    osascript >/dev/null 2>&1 <<OSA || true
tell application "$app_name" to activate
delay 0.2
tell application "System Events" to key code 36
OSA
  fi
}

set_ini_value() {
  local section="$1"
  local key="$2"
  local value="$3"
  python3 - "$INI" "$section" "$key" "$value" <<'PY'
import sys
from pathlib import Path

path = Path(sys.argv[1])
section = sys.argv[2]
key = sys.argv[3]
value = sys.argv[4]

lines = path.read_text(errors="ignore").splitlines()
out = []
current = None
seen_section = False
set_key = False

for line in lines:
    stripped = line.strip()
    stripped_lower = stripped.lower()
    key_lower = key.lower()
    if stripped.startswith("[") and stripped.endswith("]"):
        if current == section and not set_key:
            out.append(f"{key} = {value}")
            set_key = True
        current = stripped[1:-1]
        if current == section:
            seen_section = True
    if current == section and (stripped_lower.startswith(key_lower + " ") or stripped_lower.startswith(key_lower + "=")):
        out.append(f"{key} = {value}")
        set_key = True
    else:
        out.append(line)

if not seen_section:
    out.append(f"[{section}]")
if not set_key:
    out.append(f"{key} = {value}")

path.write_text("\n".join(out) + "\n")
PY
}

require_int() {
  [[ "$1" =~ ^[0-9]+$ ]] || die "$2 must be a non-negative integer"
}

[[ -d "$APP" ]] || die "app not found: $APP"
[[ -f "$SAVE_PATH" ]] || die "save not found: $SAVE_PATH"
[[ -f "$INI" ]] || die "CivilizationIV.ini not found: $INI"
require_int "$TURNS" "--turns"
require_int "$TIMEOUT" "--timeout"
[[ "$TURNS" -gt 0 ]] || die "--turns must be greater than zero"

mkdir -p "$OUT_DIR" "$SETTINGS_DIR" "$LOG_DIR" "$SAVE_DIR"

INI_BACKUP="$OUT_DIR/CivilizationIV.ini.before"
cp "$INI" "$INI_BACKUP"

if [[ -f "$AUTOVERIFY_INI" ]]; then
  AUTOVERIFY_EXISTED=1
  AUTOVERIFY_BACKUP="$OUT_DIR/AutoVerify.ini.before"
  cp "$AUTOVERIFY_INI" "$AUTOVERIFY_BACKUP"
fi

trap 'status=$?; quit_app; restore_files; exit $status' EXIT

cp "$SAVE_PATH" "$STAGED_SAVE"
archive_if_present "$LOG_DIR/custom_profile.csv"
archive_if_present "$LOG_DIR/custom_profile.log"
archive_if_present "$LOG_DIR/autoverify.csv"

set_ini_value "CONFIG" "QuickStart" "1"
set_ini_value "CONFIG" "NoIntroMovie" "1"
set_ini_value "CONFIG" "AudioEnable" "0"
set_ini_value "CONFIG" "FullScreen" "0"
set_ini_value "GAME" "GameType" "spLoad"
set_ini_value "GAME" "FileName" "$STAGED_SAVE"

cat > "$AUTOVERIFY_INI" <<EOF
[AutoVerify]
Enabled = True
Turns = $TURNS
RunId = $RUN_ID
EOF

echo "autoverify: launching $(basename "$APP")"
echo "autoverify: run id $RUN_ID"
echo "autoverify: output $OUT_DIR"
/usr/bin/open -n "$APP"

deadline=$((SECONDS + TIMEOUT))
status="timeout"
last_profile_turn=-1
last_profile_change=$SECONDS
last_keypress=0

while [[ "$SECONDS" -lt "$deadline" ]]; do
  if [[ -f "$LOG_DIR/autoverify.csv" ]]; then
    if grep -q "^$RUN_ID,complete," "$LOG_DIR/autoverify.csv"; then
      status="complete"
      break
    fi
    if grep -q "^$RUN_ID,error_" "$LOG_DIR/autoverify.csv"; then
      status="error"
      break
    fi
  fi
  if profile_reached_target; then
    status="complete"
    break
  fi
  if [[ "$KEYPRESS_FALLBACK" -eq 1 ]]; then
    if state="$(profile_turn_state 2>/dev/null)"; then
      read -r start_turn target_turn max_turn <<<"$state"
      if [[ "$max_turn" -gt "$last_profile_turn" ]]; then
        last_profile_turn="$max_turn"
        last_profile_change=$SECONDS
      fi
      if [[ "$max_turn" -gt "$start_turn" && "$max_turn" -lt "$target_turn" ]]; then
        if [[ $((SECONDS - last_profile_change)) -ge 8 && $((SECONDS - last_keypress)) -ge 8 ]]; then
          echo "autoverify: pressing Return to continue from turn $max_turn"
          press_end_turn
          last_keypress=$SECONDS
        fi
      fi
    fi
  fi
  sleep 2
done

sleep 3
quit_app

for name in custom_profile.csv custom_profile.log autoverify.csv PythonErr.log PythonDbg.log; do
  if [[ -e "$LOG_DIR/$name" ]]; then
    cp "$LOG_DIR/$name" "$OUT_DIR/$name"
  fi
done

if [[ -f "$OUT_DIR/custom_profile.csv" ]]; then
  summary_args=("$OUT_DIR/custom_profile.csv" "--out" "$OUT_DIR/SUMMARY.md" "--run-id" "$RUN_ID")
  if [[ -n "$COMPARE_CSV" ]]; then
    summary_args+=("--compare" "$COMPARE_CSV" "--compare-label" "$(basename "$COMPARE_CSV")")
  fi
  "$REPO_ROOT/scripts/summarize_profile.py" "${summary_args[@]}"
fi

if [[ "$status" == "complete" ]]; then
  echo "autoverify: complete"
  echo "autoverify: artifacts in $OUT_DIR"
  exit 0
fi

echo "autoverify: $status; artifacts in $OUT_DIR" >&2
exit 1
