#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

APP="$HOME/Applications/More Naval AI Fast.app"
BTS_DOCS="$HOME/Documents/my games/Beyond The Sword"
DEFAULT_SAVE="$BTS_DOCS/Saves/single/PROFILE_TEST_niallohiggins TURN-0110.CivBeyondSwordSave"
SAVE_PATH="$DEFAULT_SAVE"
TURNS=3
TIMEOUT=900
RUN_ID="$(date +%Y%m%d-%H%M%S)"
COMPARE_CSV=""
KEEP_APP_RUNNING=0
KEYPRESS_FALLBACK=0
DIRECT_WINE_LAUNCH=1
BACKGROUND_LAUNCH=1
HIDE_BACKGROUND_LAUNCH=1
FOCUS_GUARD=1
FOCUS_GUARD_SECONDS=0
FOCUS_GUARD_PID=""

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
  --direct-wine-launch
                      Launch Civ directly with the wrapper Wine prefix; avoids Wineskin UI
  --app-launch        Launch the macOS app bundle through LaunchServices
  --keypress-fallback
                      Activate the app and press Return if a turn stalls
  --no-keypress-fallback
                      Do not press Return if the game stops at a human turn
  --foreground-launch Bring the app to the foreground while launching
  --no-hide-launch    Do not pass open --hide for background launches
  --no-focus-guard   Do not restore focus after a background launch
  --focus-guard-seconds N
                      Seconds to watch for the wrapper taking focus; 0 means the whole run
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
    --direct-wine-launch) DIRECT_WINE_LAUNCH=1; shift ;;
    --app-launch) DIRECT_WINE_LAUNCH=0; shift ;;
    --keypress-fallback) KEYPRESS_FALLBACK=1; shift ;;
    --no-keypress-fallback) KEYPRESS_FALLBACK=0; shift ;;
    --foreground-launch) DIRECT_WINE_LAUNCH=0; BACKGROUND_LAUNCH=0; HIDE_BACKGROUND_LAUNCH=0; FOCUS_GUARD=0; shift ;;
    --no-hide-launch) HIDE_BACKGROUND_LAUNCH=0; shift ;;
    --no-focus-guard) FOCUS_GUARD=0; shift ;;
    --focus-guard-seconds) FOCUS_GUARD_SECONDS="$2"; shift 2 ;;
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

app_bundle_id() {
  /usr/libexec/PlistBuddy -c 'Print :CFBundleIdentifier' "$APP/Contents/Info.plist" 2>/dev/null || true
}

front_bundle_id() {
  osascript -e 'tell application "System Events" to get bundle identifier of first application process whose frontmost is true' 2>/dev/null || true
}

front_app_name() {
  osascript -e 'tell application "System Events" to get name of first application process whose frontmost is true' 2>/dev/null || true
}

activate_bundle_id() {
  local bundle_id="$1"
  [[ -n "$bundle_id" ]] || return 0
  osascript -e "tell application id \"$bundle_id\" to activate" >/dev/null 2>&1 || true
}

activate_app_name() {
  local app_name="$1"
  [[ -n "$app_name" ]] || return 0
  osascript - "$app_name" >/dev/null 2>&1 <<'OSA' || true
on run argv
  tell application (item 1 of argv) to activate
end run
OSA
}

hide_app_name() {
  local app_name="$1"
  [[ -n "$app_name" ]] || return 0
  osascript - "$app_name" >/dev/null 2>&1 <<'OSA' || true
on run argv
  tell application "System Events"
    if exists application process (item 1 of argv) then
      set visible of application process (item 1 of argv) to false
    end if
  end tell
end run
OSA
}

hide_benchmark_apps() {
  local target_app_name="$1"
  osascript - "$target_app_name" >/dev/null 2>&1 <<'OSA' || true
on run argv
  set targetName to item 1 of argv
  tell application "System Events"
    repeat with proc in application processes
      set procName to name of proc as text
      ignoring case
        if procName is targetName or procName contains "wineskin" or procName contains "wine" or procName contains "civilization iv" or procName contains "civ iv" or procName contains "beyond the sword" or procName contains "civ4beyondsword" then
          set visible of proc to false
        end if
      end ignoring
    end repeat
  end tell
end run
OSA
}

is_benchmark_app_name() {
  local app_name="$1"
  local target_app_name="$2"
  [[ -n "$app_name" ]] || return 1
  [[ "$app_name" == "$target_app_name" ]] && return 0
  [[ "$app_name" == "Wineskin" ]] && return 0
  [[ "$app_name" == *"Wineskin"* ]] && return 0
  [[ "$app_name" == "Wine" ]] && return 0
  [[ "$app_name" == *"wine"* ]] && return 0
  [[ "$app_name" == *"Wine"* ]] && return 0
  [[ "$app_name" == *"Civ IV"* ]] && return 0
  [[ "$app_name" == *"Civilization IV"* ]] && return 0
  [[ "$app_name" == *"Beyond The Sword"* ]] && return 0
  return 1
}

activate_previous_app() {
  local bundle_id="$1"
  local app_name="$2"
  if [[ -n "$bundle_id" ]]; then
    activate_bundle_id "$bundle_id"
  elif [[ -n "$app_name" ]]; then
    activate_app_name "$app_name"
  fi
}

focus_guard() {
  local previous_bundle_id="$1"
  local previous_app_name="$2"
  local target_bundle_id="$3"
  local target_app_name="$4"
  local deadline=0
  if [[ "$FOCUS_GUARD_SECONDS" -gt 0 ]]; then
    deadline=$((SECONDS + FOCUS_GUARD_SECONDS))
  fi

  [[ -n "$previous_bundle_id" && "$previous_bundle_id" == "$target_bundle_id" ]] && return 0

  while [[ "$deadline" -eq 0 || "$SECONDS" -lt "$deadline" ]]; do
    local foreground_bundle_id foreground_app_name
    foreground_bundle_id="$(front_bundle_id)"
    foreground_app_name="$(front_app_name)"
    if [[ -n "$target_bundle_id" && "$foreground_bundle_id" == "$target_bundle_id" ]] ||
       is_benchmark_app_name "$foreground_app_name" "$target_app_name"; then
      hide_benchmark_apps "$target_app_name"
      hide_app_name "$foreground_app_name"
      hide_app_name "$target_app_name"
      hide_app_name "Wineskin"
      activate_previous_app "$previous_bundle_id" "$previous_app_name"
    fi
    sleep 0.5
  done
}

stop_focus_guard() {
  [[ -n "$FOCUS_GUARD_PID" ]] || return 0
  kill "$FOCUS_GUARD_PID" >/dev/null 2>&1 || true
  wait "$FOCUS_GUARD_PID" >/dev/null 2>&1 || true
  FOCUS_GUARD_PID=""
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

  local wine_prefix wine_server
  wine_prefix="$APP/Contents/SharedSupport/prefix"
  wine_server="$APP/Contents/SharedSupport/wine/bin/wineserver"
  if [[ -x "$wine_server" && -d "$wine_prefix" ]]; then
    DYLD_FALLBACK_LIBRARY_PATH="$APP/Contents/Frameworks:${DYLD_FALLBACK_LIBRARY_PATH:-}" WINEPREFIX="$wine_prefix" "$wine_server" -k >/dev/null 2>&1 || true
    sleep 1
  fi

  if [[ "$DIRECT_WINE_LAUNCH" -eq 1 ]]; then
    local wine_path
    wine_path="$APP/Contents/SharedSupport/wine"
    if pgrep -f "$wine_path" >/dev/null 2>&1; then
      pkill -f "$wine_path" >/dev/null 2>&1 || true
      sleep 1
    fi
    if pgrep -f "$wine_path" >/dev/null 2>&1; then
      pkill -9 -f "$wine_path" >/dev/null 2>&1 || true
    fi
    return 0
  fi

  local bundle_id
  bundle_id="$(app_bundle_id)"
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

launch_direct_wine() {
  local shared wine_prefix wine_bin exe exe_dir
  shared="$APP/Contents/SharedSupport"
  wine_prefix="$shared/prefix"
  wine_bin="$shared/wine/bin/wine64"
  exe="$wine_prefix/drive_c/GOG Games/Civilization IV Complete/Civ4/Beyond the Sword/Civ4BeyondSword.exe"
  exe_dir="$(dirname "$exe")"

  [[ -x "$wine_bin" ]] || die "wine64 not found: $wine_bin"
  [[ -d "$wine_prefix" ]] || die "Wine prefix not found: $wine_prefix"
  [[ -f "$exe" ]] || die "Civ4BeyondSword.exe not found: $exe"

  (
    export WINEPREFIX="$wine_prefix"
    export WINEDEBUG="${WINEDEBUG:--all}"
    export WINEESYNC="${WINEESYNC:-1}"
    export WINEMSYNC="${WINEMSYNC:-1}"
    export WINEDLLOVERRIDES="${WINEDLLOVERRIDES:-winemenubuilder.exe=d}"
    export DYLD_FALLBACK_LIBRARY_PATH="$APP/Contents/Frameworks:${DYLD_FALLBACK_LIBRARY_PATH:-}"
    export PATH="$shared/wine/bin:$PATH"
    cd "$exe_dir"
    "$wine_bin" "$exe" 'mod=\More Naval AI' &
    printf '%s\n' "$!" > "$OUT_DIR/wine.pid"
  )
}

archive_if_present() {
  local path="$1"
  local name
  name="$(basename "$path")"
  if [[ -e "$path" ]]; then
    mv "$path" "$OUT_DIR/preexisting-$name"
  fi
}

measurement_reached_target() {
  python3 - "$LOG_DIR/autoverify.csv" "$LOG_DIR/custom_profile.csv" "$LOG_DIR/autoverify_dll.csv" "$RUN_ID" <<'PY'
import csv
import sys
from pathlib import Path

autoverify = Path(sys.argv[1])
profile = Path(sys.argv[2])
dll_turns = Path(sys.argv[3])
run_id = sys.argv[4]

if not autoverify.exists():
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
if profile.exists():
    with profile.open(newline="") as handle:
        for row in csv.DictReader(handle):
            try:
                max_turn = max(max_turn, int(row.get("turn", "-1")))
            except ValueError:
                pass

if dll_turns.exists():
    with dll_turns.open(newline="") as handle:
        for row in csv.DictReader(handle):
            try:
                max_turn = max(max_turn, int(row.get("turn", "-1")))
            except ValueError:
                pass

if max_turn >= target_turn:
    wall_ms = int(__import__("time").time() * 1000)
    start_wall_ms = -1
    with autoverify.open(newline="") as handle:
        for row in csv.DictReader(handle):
            if row.get("run_id") != run_id:
                continue
            if row.get("event", "").startswith("start_"):
                try:
                    start_wall_ms = int(row.get("wall_ms", "-1"))
                except ValueError:
                    start_wall_ms = -1
                break
    elapsed_ms = wall_ms - start_wall_ms if start_wall_ms >= 0 else -1
    with autoverify.open("a", newline="") as handle:
        handle.write("%s,complete_inferred,%d,%d,%d,%d,%d\n" % (
            run_id,
            start_turn if start_turn is not None else -1,
            max_turn,
            target_turn,
            wall_ms,
            elapsed_ms,
        ))
    raise SystemExit(0)

raise SystemExit(1)
PY
}

run_turn_state() {
  python3 - "$LOG_DIR/autoverify.csv" "$LOG_DIR/custom_profile.csv" "$LOG_DIR/autoverify_dll.csv" "$RUN_ID" <<'PY'
import csv
import sys
from pathlib import Path

autoverify = Path(sys.argv[1])
profile = Path(sys.argv[2])
dll_turns = Path(sys.argv[3])
run_id = sys.argv[4]

if not autoverify.exists():
    raise SystemExit(1)

start_turn = None
target_turn = None
max_turn = -1
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
        if event.startswith("start_") or event == "turn" or event == "complete" or event == "complete_inferred":
            try:
                max_turn = max(max_turn, int(row.get("current_turn", "-1")))
            except ValueError:
                pass

if profile.exists():
    with profile.open(newline="") as handle:
        for row in csv.DictReader(handle):
            try:
                max_turn = max(max_turn, int(row.get("turn", "-1")))
            except ValueError:
                pass

if dll_turns.exists():
    with dll_turns.open(newline="") as handle:
        for row in csv.DictReader(handle):
            try:
                max_turn = max(max_turn, int(row.get("turn", "-1")))
            except ValueError:
                pass

if start_turn is None or target_turn is None or max_turn < 0:
    raise SystemExit(1)

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
require_int "$FOCUS_GUARD_SECONDS" "--focus-guard-seconds"
[[ "$TURNS" -gt 0 ]] || die "--turns must be greater than zero"

mkdir -p "$OUT_DIR" "$SETTINGS_DIR" "$LOG_DIR" "$SAVE_DIR"

INI_BACKUP="$OUT_DIR/CivilizationIV.ini.before"
cp "$INI" "$INI_BACKUP"

if [[ -f "$AUTOVERIFY_INI" ]]; then
  AUTOVERIFY_EXISTED=1
  AUTOVERIFY_BACKUP="$OUT_DIR/AutoVerify.ini.before"
  cp "$AUTOVERIFY_INI" "$AUTOVERIFY_BACKUP"
fi

trap 'status=$?; stop_focus_guard; quit_app; restore_files; exit $status' EXIT

cp "$SAVE_PATH" "$STAGED_SAVE"
archive_if_present "$LOG_DIR/custom_profile.csv"
archive_if_present "$LOG_DIR/custom_profile.log"
archive_if_present "$LOG_DIR/full_member_callers.csv"
archive_if_present "$LOG_DIR/path_request_callers.csv"
archive_if_present "$LOG_DIR/path_request_cross_context.csv"
archive_if_present "$LOG_DIR/autoverify.csv"
archive_if_present "$LOG_DIR/autoverify_dll.csv"
archive_if_present "$LOG_DIR/release_trace.csv"
archive_if_present "$LOG_DIR/scheduler_trace.csv"
archive_if_present "$LOG_DIR/state_fingerprint.csv"
archive_if_present "$LOG_DIR/PythonErr.log"
archive_if_present "$LOG_DIR/PythonDbg.log"

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
previous_front_bundle_id=""
previous_front_app_name=""
target_bundle_id=""
target_app_name=""
if [[ "$BACKGROUND_LAUNCH" -eq 1 && "$FOCUS_GUARD" -eq 1 ]]; then
  previous_front_bundle_id="$(front_bundle_id)"
  previous_front_app_name="$(front_app_name)"
  if [[ "$DIRECT_WINE_LAUNCH" -eq 0 ]]; then
    target_bundle_id="$(app_bundle_id)"
  fi
  target_app_name="$(basename "$APP" .app)"
  focus_guard "$previous_front_bundle_id" "$previous_front_app_name" "$target_bundle_id" "$target_app_name" &
  FOCUS_GUARD_PID="$!"
fi
if [[ "$DIRECT_WINE_LAUNCH" -eq 1 ]]; then
  launch_direct_wine
elif [[ "$BACKGROUND_LAUNCH" -eq 1 ]]; then
  if [[ "$HIDE_BACKGROUND_LAUNCH" -eq 1 ]]; then
    /usr/bin/open -g -j -n "$APP"
  else
    /usr/bin/open -g -n "$APP"
  fi
else
  /usr/bin/open -n "$APP"
fi
if [[ "$BACKGROUND_LAUNCH" -eq 1 && "$FOCUS_GUARD" -eq 1 ]]; then
  hide_benchmark_apps "$target_app_name"
  hide_app_name "$target_app_name"
  hide_app_name "Wineskin"
  hide_app_name "Wine"
  activate_previous_app "$previous_front_bundle_id" "$previous_front_app_name"
fi

deadline=$((SECONDS + TIMEOUT))
status="timeout"
last_seen_turn=-1
last_turn_change=$SECONDS
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
  if measurement_reached_target; then
    status="complete"
    break
  fi
  if [[ "$KEYPRESS_FALLBACK" -eq 1 ]]; then
    if state="$(run_turn_state 2>/dev/null)"; then
      read -r start_turn target_turn max_turn <<<"$state"
      if [[ "$max_turn" -gt "$last_seen_turn" ]]; then
        last_seen_turn="$max_turn"
        last_turn_change=$SECONDS
      fi
      if [[ "$max_turn" -ge "$start_turn" && "$max_turn" -lt "$target_turn" ]]; then
        if [[ $((SECONDS - last_turn_change)) -ge 8 && $((SECONDS - last_keypress)) -ge 8 ]]; then
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
stop_focus_guard
quit_app

for name in custom_profile.csv custom_profile.log full_member_callers.csv path_request_callers.csv path_request_cross_context.csv autoverify.csv autoverify_dll.csv release_trace.csv scheduler_trace.csv state_fingerprint.csv PythonErr.log PythonDbg.log; do
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
