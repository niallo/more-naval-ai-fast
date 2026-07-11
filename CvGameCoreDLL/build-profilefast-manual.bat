@echo off
setlocal

set TOOLKIT=C:\Program Files\Civ4SDK\Microsoft Visual C++ Toolkit 2003
set PSDK=C:\Program Files\Civ4SDK\WindowsSDK
set CIV4_PATH=%CIV4_LIB_INSTALL_PATH%
if "%CIV4_PATH%"=="" set CIV4_PATH=C:\GOG Games\Civilization IV Complete\Civ4\Beyond the Sword\CvGameCoreDLL

set TARGET=ProfileFast
set OUT=temp_files\%TARGET%
set BIN=%TARGET%

set PATH=%TOOLKIT%\bin;%PSDK%\bin;%PATH%

set DEFINES=/DWIN32 /D_WINDOWS /D_USRDLL /DCVGAMECOREDLL_EXPORTS /DNDEBUG /DLOG_AI /DFINAL_RELEASE /DCUSTOM_PROFILER /DFP_PROFILE_ENABLE /DMNAI_PROFILE_AI_UNIT_VALUE_DETAIL /DMNAI_PROFILE_UNIT_UPDATE_DETAIL /DMNAI_PROFILE_UNIT_DISPATCH_DETAIL /DMNAI_PROFILE_UNIT_PREP_DETAIL /DMNAI_PROFILE_PLAYER_TURN_DETAIL /DMNAI_PROFILE_CITY_TURN_DETAIL /DMNAI_PROFILE_CITY_ASSIGN_DETAIL /DMNAI_PROFILE_CITY_YIELD_DETAIL /DMNAI_PROFILE_BEST_PLOT_BUILD_DETAIL /DMNAI_PROFILE_FULL_MEMBER_CALLERS /DMNAI_PROFILE_PATH_REQUESTS /DMNAI_PROFILE_WORKER_DETAIL /DMNAI_PROFILE_ROUTE_TERRITORY_DETAIL /DMNAI_PROFILE_CITY_EMPHASIZE_DETAIL /DMNAI_PROFILE_EXPLORE_DETAIL /DMNAI_PROFILE_PATROL_DETAIL /DMNAI_PROFILE_CONQUEST_DETAIL /DMNAI_PROFILE_PICK_TARGET_CITY_DETAIL /DMNAI_PROFILE_FOUND_VALUE_DETAIL /DMNAI_PROFILE_DANGER_DETAIL /DMNAI_PROFILE_PATHVALID_ORDER_DETAIL /DMNAI_PROFILE_CANBUILD_ORDER_DETAIL /DMNAI_AUTOVERIFY_AUTO_END_TURN /DMNAI_PROFILE_TRUE_COMBAT_CACHE /DMNAI_PROFILE_UPGRADE_LIST_CACHE /DMNAI_PROFILE_PATHVALID_MOVE_CACHE /DMNAI_PROFILE_BASE_BONUS_CACHE /DMNAI_PROFILE_ACTIVE_DANGER_CACHE_SKIP_CLEAN /DMNAI_OPT_CITY_YIELD_DELTA_CONTEXT /DMNAI_OPT_NOBONUS_VOTE_SOURCE_FIRST /DMNAI_OPT_ATTACK_ODDS_BEFORE_PATH /DMNAI_OPT_PATHCOST_SINGLE_UNIT_NEXT_SKIP /DMNAI_OPT_PATHVALID_STICKY_UPDATE_CACHE /DMNAI_OPT_ROUTE_TERRITORY_OWNED_PLOT_CACHE /DMNAI_OPT_TOWER_MANA_OWNED_PLOT_SERVICE /DMNAI_OPT_PILLAGE_VALUE_BEFORE_PATH /DMNAI_OPT_PATHVALID_MOVE_BEFORE_DANGER /DMNAI_OPT_WORKER_OWNED_PLOT_SERVICE /DMNAI_OPT_IMPROVE_BONUS_OWNED_PLOT_SERVICE /DMNAI_OPT_CONTIGUOUS_TURN_UPDATES
set INCS=/I"build-support\include" /I"%CIV4_PATH%\Boost-1.32.0\include" /I"%CIV4_PATH%\Python24\include" /I"%TOOLKIT%\include" /I"%PSDK%\Include" /I"%PSDK%\Include\mfc"
set COMMON_CFLAGS=/GR /Gy /W3 /EHsc /Gd /Gm- /MD /O2 /Oy /Oi /G7 %DEFINES%
set PCH_CFLAGS=/Yu"CvGameCoreDLL.h" /Fp"%OUT%\CvGameCoreDLL.pch" /Fd"%OUT%\CvGameCoreDLL.pdb" /GL

if not exist "%OUT%\." mkdir "%OUT%"
if not exist "%BIN%\." mkdir "%BIN%"
if exist "%OUT%\*.obj" del /q "%OUT%\*.obj"
if exist "%OUT%\*.pch" del /q "%OUT%\*.pch"
if exist "%OUT%\*.pdb" del /q "%OUT%\*.pdb"
if exist "%OUT%\*.res" del /q "%OUT%\*.res"
if exist "%OUT%\*.dll" del /q "%OUT%\*.dll"
if exist "%OUT%\*.lib" del /q "%OUT%\*.lib"
if exist "%OUT%\*.exp" del /q "%OUT%\*.exp"

echo Precompiling header
"%TOOLKIT%\bin\cl.exe" /nologo %COMMON_CFLAGS% %PCH_CFLAGS% %INCS% /YcCvGameCoreDLL.h /Fo"%OUT%\_precompile.obj" /c _precompile.cpp
if errorlevel 1 exit /b 1
if not exist "%OUT%\CvGameCoreDLL.pch" exit /b 1

for %%F in (*.cpp) do (
  if /I not "%%F"=="_precompile.cpp" if /I not "%%F"=="CvTextScreens.cpp" (
    echo Compiling %%F
    "%TOOLKIT%\bin\cl.exe" /nologo %COMMON_CFLAGS% %PCH_CFLAGS% %INCS% /Fo"%OUT%\%%~nF.obj" /c "%%F"
    if errorlevel 1 exit /b 1
    if not exist "%OUT%\%%~nF.obj" exit /b 1
  )
)

echo Compiling CvTextScreens dummy
type nul > "%OUT%\CvTextScreens-dummy.cpp"
"%TOOLKIT%\bin\cl.exe" /nologo %COMMON_CFLAGS% /Y- /Fo"%OUT%\CvTextScreens.obj" /c "%OUT%\CvTextScreens-dummy.cpp"
if errorlevel 1 exit /b 1
if not exist "%OUT%\CvTextScreens.obj" exit /b 1
del "%OUT%\CvTextScreens-dummy.cpp"

set RESOURCE_ARG=
if exist CvGameCoreDLL.rc (
  echo Compiling resources
  "%PSDK%\bin\RC.Exe" /Fo"%OUT%\CvGameCoreDLL.res" %INCS% CvGameCoreDLL.rc
  if errorlevel 1 exit /b 1
  if not exist "%OUT%\CvGameCoreDLL.res" exit /b 1
  set RESOURCE_ARG="%OUT%\CvGameCoreDLL.res"
) else (
  echo Skipping resources
)

if exist "%OUT%\objects.rsp" del "%OUT%\objects.rsp"
for %%O in (%OUT%\*.obj) do echo "%%O" >> "%OUT%\objects.rsp"

echo Linking DLL
"%TOOLKIT%\bin\link.exe" /out:"%OUT%\CvGameCoreDLL.dll" /DLL /NOLOGO /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE /TLBID:1 /PDB:"%OUT%\CvGameCoreDLL.pdb" /INCREMENTAL:NO /OPT:REF /OPT:ICF /LTCG /IMPLIB:"%OUT%\CvGameCoreDLL.lib" /LIBPATH:"%CIV4_PATH%\Python24\libs" /LIBPATH:"%CIV4_PATH%\Boost-1.32.0\libs" /LIBPATH:"%TOOLKIT%\lib" /LIBPATH:"%PSDK%\Lib" @"%OUT%\objects.rsp" %RESOURCE_ARG% boost_python-vc71-mt-1_32.lib winmm.lib user32.lib
if errorlevel 1 exit /b 1
if not exist "%OUT%\CvGameCoreDLL.dll" exit /b 1

copy "%OUT%\CvGameCoreDLL.dll" "%BIN%\CvGameCoreDLL.dll"
if errorlevel 1 exit /b 1

echo Built %BIN%\CvGameCoreDLL.dll
