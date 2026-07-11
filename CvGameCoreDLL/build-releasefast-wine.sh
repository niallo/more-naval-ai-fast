#!/bin/sh
set -eu

APP_PATH="${CIV4_APP:-${MNAI_APP:-$HOME/Applications/More Naval AI Fast.app}}"
export WINEPREFIX="${CIV4_BUILD_WINEPREFIX:-${MNAI_BUILD_WINEPREFIX:-$HOME/Applications/civ4-mnai-build-wineprefix}}"
export DYLD_FALLBACK_LIBRARY_PATH="${DYLD_FALLBACK_LIBRARY_PATH:-$APP_PATH/Contents/Frameworks}"

CIV4_LIBS_UNIX="${CIV4_LIBS_UNIX:-$APP_PATH/Contents/SharedSupport/prefix/drive_c/GOG Games/Civilization IV Complete/Civ4/Beyond the Sword/CvGameCoreDLL}"
CIV4_LIBS_WIN="$(printf 'Z:%s' "$CIV4_LIBS_UNIX" | sed 's#/#\\#g')"
export CIV4_LIB_INSTALL_PATH="${CIV4_LIB_INSTALL_PATH:-$CIV4_LIBS_WIN}"

cd "$(dirname "$0")"
exec "$APP_PATH/Contents/SharedSupport/wine/bin/wine64" cmd /c build-releasefast-manual.bat
