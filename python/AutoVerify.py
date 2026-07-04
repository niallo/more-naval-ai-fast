# AutoVerify
#
# Test-only unattended turn runner for profiling builds. This module is inert
# unless FFH - More Naval AI/Settings/AutoVerify.ini enables it.

from CvPythonExtensions import *
import CvUtil
import os

try:
	import BugPath
except:
	BugPath = None

gc = CyGlobalContext()
game = gc.getGame()

g_started = False
g_completed = False
g_startTurn = -1
g_targetTurn = -1
g_turns = 0
g_runId = ""
g_player = -1
g_lastLoggedTurn = -1
g_lastResumeTurn = -1


def _boolValue(value):
	value = str(value).strip().lower()
	return value in ("1", "true", "yes", "on")


def _clean(value):
	return str(value).replace(",", "_").replace("\r", " ").replace("\n", " ").strip()


def _settingsDir():
	if BugPath:
		try:
			path = BugPath.getSettingsDir()
			if path:
				return path
		except:
			pass
		try:
			root = BugPath.getRootDir()
			if root:
				return os.path.join(root, "FFH - More Naval AI", "Settings")
		except:
			pass
	return ""


def _logsDir():
	if BugPath:
		try:
			root = BugPath.getRootDir()
			if root:
				return os.path.join(root, "Logs")
		except:
			pass
	return ""


def _readConfig():
	path = _settingsDir()
	if not path:
		return {}

	configPath = os.path.join(path, "AutoVerify.ini")
	if not os.path.isfile(configPath):
		return {}

	config = {}
	try:
		configFile = open(configPath, "r")
		for rawLine in configFile.readlines():
			line = rawLine.strip()
			if not line or line[0] in ("#", ";", "["):
				continue
			if "=" not in line:
				continue
			key, value = line.split("=", 1)
			config[key.strip()] = value.strip()
		configFile.close()
	except:
		CvUtil.pyPrint("AutoVerify: failed to read %s" % configPath)
		return {}

	return config


def _enabled(config):
	return _boolValue(config.get("Enabled", "False"))


def _writeLog(event, currentTurn=None):
	logsDir = _logsDir()
	if not logsDir:
		return
	try:
		if not os.path.isdir(logsDir):
			os.makedirs(logsDir)
	except:
		return

	path = os.path.join(logsDir, "autoverify.csv")
	writeHeader = not os.path.isfile(path)
	try:
		logFile = open(path, "a")
		if writeHeader:
			logFile.write("run_id,event,start_turn,current_turn,target_turn\n")
		if currentTurn is None:
			currentTurn = game.getGameTurn()
		logFile.write("%s,%s,%d,%d,%d\n" % (_clean(g_runId), _clean(event), g_startTurn, currentTurn, g_targetTurn))
		logFile.close()
	except:
		CvUtil.pyPrint("AutoVerify: failed to write %s" % path)


def _setAutoPlay(turns):
	try:
		game.setForcedAIAutoPlay(g_player, turns, True)
	except:
		game.setAIAutoPlay(g_player, turns)


def _start(source):
	global g_started, g_completed, g_startTurn, g_targetTurn, g_turns, g_runId, g_player, g_lastLoggedTurn, g_lastResumeTurn

	if g_started or g_completed:
		return

	config = _readConfig()
	if not _enabled(config):
		return

	try:
		g_turns = int(config.get("Turns", "0"))
	except:
		g_turns = 0

	if g_turns <= 0:
		g_runId = config.get("RunId", "autoverify")
		_writeLog("error_invalid_turns")
		return

	g_runId = config.get("RunId", "autoverify")
	g_player = game.getActivePlayer()
	g_startTurn = game.getGameTurn()
	g_targetTurn = g_startTurn + g_turns
	g_lastLoggedTurn = -1
	g_lastResumeTurn = -1

	if g_player < 0:
		_writeLog("error_no_active_player")
		return

	g_started = True
	g_completed = False
	_writeLog("start_%s" % source, g_startTurn)
	_setAutoPlay(g_turns)


def _resumeIfNeeded(source, currentTurn):
	global g_lastResumeTurn

	if not g_started or g_completed:
		return

	_checkComplete(currentTurn)
	if g_completed:
		return

	remainingTurns = g_targetTurn - currentTurn
	if remainingTurns <= 0:
		return

	if currentTurn != g_lastResumeTurn:
		_writeLog("resume_%s" % source, currentTurn)
		g_lastResumeTurn = currentTurn

	_setAutoPlay(remainingTurns)


def _checkComplete(currentTurn):
	global g_completed, g_started, g_lastLoggedTurn

	if not g_started or g_completed:
		return

	if currentTurn != g_lastLoggedTurn:
		_writeLog("turn", currentTurn)
		g_lastLoggedTurn = currentTurn

	if currentTurn >= g_targetTurn:
		_setAutoPlay(0)
		g_completed = True
		g_started = False
		_writeLog("complete", currentTurn)


def onLoadGame(argsList):
	_start("load")


def onGameStart(argsList):
	_start("start")


def onBeginGameTurn(argsList):
	_start("begin_turn")
	if len(argsList) > 0:
		_checkComplete(argsList[0])
	else:
		_checkComplete(game.getGameTurn())


def onEndGameTurn(argsList):
	if len(argsList) > 0:
		_checkComplete(argsList[0])
	else:
		_checkComplete(game.getGameTurn())


def onBeginPlayerTurn(argsList):
	if len(argsList) > 0:
		_checkComplete(argsList[0])
	else:
		_checkComplete(game.getGameTurn())


def onEndPlayerTurn(argsList):
	if len(argsList) > 0:
		_checkComplete(argsList[0])
	else:
		_checkComplete(game.getGameTurn())


def onEndTurnReady(argsList):
	if len(argsList) > 0:
		_resumeIfNeeded("end_turn_ready", argsList[0])
	else:
		_resumeIfNeeded("end_turn_ready", game.getGameTurn())


def onGameEnd(argsList):
	if g_started and not g_completed:
		_writeLog("game_end", game.getGameTurn())
