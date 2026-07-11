#include "CvGameCoreDLL.h"

#include "CvGameCoreDLLUndefNew.h"

#include <algorithm> // CUSTOM_PROFILER
#include <new>

#include "CvGlobals.h"
#include "FProfiler.h"
#include "CvDLLInterfaceIFaceBase.h"
#ifdef MNAI_PROFILE_PATH_REQUESTS
#include "CvSelectionGroup.h"
#include "CvPlot.h"
#endif

//
// operator global new and delete override for gamecore DLL 
//
void *__cdecl operator new(size_t size)
{
	if (gDLL)
	{
		return gDLL->newMem(size, __FILE__, __LINE__);
	}
	return malloc(size);
}

void __cdecl operator delete (void *p)
{
	if (gDLL)
	{
		gDLL->delMem(p, __FILE__, __LINE__);
	}
	else
	{
		free(p);
	}
}

void* operator new[](size_t size)
{
	if (gDLL)
		return gDLL->newMemArray(size, __FILE__, __LINE__);
	return malloc(size);
}

void operator delete[](void* pvMem)
{
	if (gDLL)
	{
		gDLL->delMemArray(pvMem, __FILE__, __LINE__);
	}
	else
	{
		free(pvMem);
	}
}

void *__cdecl operator new(size_t size, char* pcFile, int iLine)
{
	return gDLL->newMem(size, pcFile, iLine);
}

void *__cdecl operator new[](size_t size, char* pcFile, int iLine)
{
	return gDLL->newMem(size, pcFile, iLine);
}

void __cdecl operator delete(void* pvMem, char* pcFile, int iLine)
{
	gDLL->delMem(pvMem, pcFile, iLine);
}

void __cdecl operator delete[](void* pvMem, char* pcFile, int iLine)
{
	gDLL->delMem(pvMem, pcFile, iLine);
}


void* reallocMem(void* a, unsigned int uiBytes, const char* pcFile, int iLine)
{
	return gDLL->reallocMem(a, uiBytes, pcFile, iLine);
}

unsigned int memSize(void* a)
{
	return gDLL->memSize(a);
}

// BUG - EXE/DLL Paths - start
HANDLE dllModule = NULL;
// BUG - EXE/DLL Paths - end

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved)
{
	switch( ul_reason_for_call ) {
	case DLL_PROCESS_ATTACH:
		{

// BUG - EXE/DLL Paths - start
		dllModule = hModule;

#ifdef _DEBUG
/************************************************************************************************/
/* Afforess	                  Start		 07/30/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
//Irritating, and meaningless.
//	MessageBox(0, "DLL attached", "Message", 0);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
#endif
// BUG - EXE/DLL Paths - end

		// The DLL is being loaded into the virtual address space of the current process as a result of the process starting up 
		OutputDebugString("DLL_PROCESS_ATTACH\n");

		// set timer precision
		MMRESULT iTimeSet = timeBeginPeriod(1);		// set timeGetTime and sleep resolution to 1 ms, otherwise it's 10-16ms
		FAssertMsg(iTimeSet==TIMERR_NOERROR, "failed setting timer resolution to 1 ms");
		}
		break;
	case DLL_THREAD_ATTACH:
		// OutputDebugString("DLL_THREAD_ATTACH\n");
		break;
	case DLL_THREAD_DETACH:
		// OutputDebugString("DLL_THREAD_DETACH\n");
		break;
	case DLL_PROCESS_DETACH:

// BUG - EXE/DLL Paths - start
		dllModule = NULL;
// BUG - EXE/DLL Paths - end

		OutputDebugString("DLL_PROCESS_DETACH\n");
		timeEndPeriod(1);
		GC.setDLLIFace(NULL);
		break;
	}
	
	return TRUE;	// success
}

#ifdef CUSTOM_PROFILER
namespace custom_profiler
{
	std::map<std::string, ProfileSample*> samples;
	LARGE_INTEGER measurementFreq;
	bool bCsvHeaderWritten = false;
	ProfileSample turnSample( "Full turn or sample interval" );

	// Start measurement for with the sample
	void beginSample( ProfileSample* pSample )
	{
		if( ! pSample->bRegistered )
		{
			samples[std::string( pSample->Name )] = pSample;
			pSample->ProfileInstances = 0;
			pSample->OpenProfiles = 0;
			pSample->StartTime = 0;
			pSample->Accumulator = 0;
			pSample->bRegistered = true;
		}

		pSample->ProfileInstances++;
		pSample->OpenProfiles++;
	
		LARGE_INTEGER time;
		QueryPerformanceCounter( &time );
		pSample->StartTime = time.QuadPart;
	}

	void endSample( ProfileSample* pSample )
	{
		pSample->OpenProfiles--;
	
		LARGE_INTEGER time;
		QueryPerformanceCounter( &time );
		pSample->Accumulator += time.QuadPart - pSample->StartTime;
	}

	void startProfiling()
	{
		for( std::map<std::string, ProfileSample*>::iterator it = samples.begin();
			it != samples.end(); it++ )
		{
			it->second->ProfileInstances = 0;
			it->second->OpenProfiles = 0;
			it->second->Accumulator = 0;
		}
	}

	void stopProfiling()
	{
		CvString buffer; // For formatting strings
		
		// Get tick frequency.
		QueryPerformanceFrequency( &measurementFreq );
		if( measurementFreq.QuadPart == 0 )
		{
			measurementFreq.QuadPart = 1;
			gDLL->logMsg( "custom_profile.log", "Unknown frequency!", false, false );
		}
		
		std::vector<ProfileSample*> vpSamples;
		for( std::map<std::string, ProfileSample*>::iterator it = samples.begin();
			it != samples.end(); it++ )
		{
			vpSamples.push_back( it->second );
		}
		
		// Sort vpSamples by time (descending) with insertionsort
		for( size_t i = 1; i < vpSamples.size(); i++ )
		{
			for( size_t j = i; j > 0; j-- )
			{
				if( vpSamples.at( j-1 )->Accumulator < vpSamples.at( j )->Accumulator )
				{
					std::swap( vpSamples.at(j-1), vpSamples.at(j) );
				}
				else
				{
					break;
				}
			}		
		}

		gDLL->logMsg( "custom_profile.log", "-----------------------------------", false, false );
		gDLL->logMsg( "custom_profile.log", "total time - # called - # unclosed calls - name", false, false );
		buffer.Format( "%d samples", samples.size() );
		gDLL->logMsg( "custom_profile.log", buffer.c_str() );

		if( ! bCsvHeaderWritten )
		{
			gDLL->logMsg( "custom_profile.csv", "turn,interval_ms,sample,total_ms,calls,open_profiles", false, false );
			bCsvHeaderWritten = true;
		}

		const int iTurn = GC.getGameINLINE().getGameTurn();
		const long iIntervalMs = (long) (turnSample.Accumulator * 1000 / measurementFreq.QuadPart);

		for( size_t i = 0; i < vpSamples.size(); i++ )
		{
			if( vpSamples.at(i)->ProfileInstances >= 1 ) {
				const long iSampleMs = (long) (vpSamples.at(i)->Accumulator * 1000 / measurementFreq.QuadPart);
				buffer.Format( "%7ld\t%7u\t%3d\t%s",
					iSampleMs, // ms
					vpSamples.at(i)->ProfileInstances,
					vpSamples.at(i)->OpenProfiles,
					vpSamples.at(i)->Name );
				gDLL->logMsg( "custom_profile.log", buffer.c_str(), false, false );

				buffer.Format( "%d,%ld,%s,%ld,%u,%d",
					iTurn,
					iIntervalMs,
					vpSamples.at(i)->Name,
					iSampleMs,
					vpSamples.at(i)->ProfileInstances,
					vpSamples.at(i)->OpenProfiles );
				gDLL->logMsg( "custom_profile.csv", buffer.c_str(), false, false );
			}
		}
	}
	
} // end namespace custom_profiler
#endif // CUSTOM_PROFILER

#ifdef MNAI_AUTOVERIFY_TURN_CSV
namespace mnai_autoverify_turn_csv
{
	bool bHeaderWritten = false;
	bool bStarted = false;
	LARGE_INTEGER startTime;
}
#endif // MNAI_AUTOVERIFY_TURN_CSV

#ifdef MNAI_RELEASE_TRACE
namespace mnai_release_trace
{
	const char* const aszPhaseNames[MNAI_TRACE_PHASE_COUNT] =
	{
		"game_update",
		"callback_gap",
		"update_slice",
		"update_moves",
		"set_turn_active",
		"game_do_turn",
		"player_do_turn",
		"player_do_turn_units",
		"city_do_turn",
		"city_ai_do_turn",
		"city_assign",
		"city_best_build",
		"city_plot_value",
		"player_unit_update",
		"group_ai_update",
		"unit_ai_update",
		"unit_dispatch",
		"generate_path",
		"danger"
	};

	__int64 aiExclusiveTicks[MNAI_TRACE_PHASE_COUNT];
	unsigned int aiCalls[MNAI_TRACE_PHASE_COUNT];
	MnaiReleaseTraceScope* pCurrentScope = NULL;
	bool bActive = false;
	bool bTraceHeaderWritten = false;
	bool bFingerprintHeaderWritten = false;
	bool bSchedulerHeaderWritten = false;
	__int64 iLastGameUpdateExitTicks = 0;
	__int64 iCurrentCallbackGapTicks = 0;

	struct SchedulerSample
	{
		__int64 iCallbackGapTicks;
		int iActivePlayers;
		int iBusyGroups;
		int iMissionTimerGroups;
		int iCombatGroups;
		int iMissionTimerTotal;
	};

	const int MAX_SCHEDULER_SAMPLES = 16;
	SchedulerSample aSchedulerSamples[MAX_SCHEDULER_SAMPLES];
	int iSchedulerSampleCount = 0;

	class Fingerprint
	{
	public:
		Fingerprint() : m_iValue(14695981039346656037ui64) {}

		void addInt(int iValue)
		{
			addUnsigned((unsigned int)iValue);
		}

		void addUnsigned(unsigned int iValue)
		{
			for (int iByte = 0; iByte < 4; ++iByte)
			{
				m_iValue ^= (unsigned char)((iValue >> (iByte * 8)) & 0xff);
				m_iValue *= 1099511628211ui64;
			}
		}

		unsigned __int64 value() const
		{
			return m_iValue;
		}

	private:
		unsigned __int64 m_iValue;
	};

	void reset()
	{
		for (int i = 0; i < MNAI_TRACE_PHASE_COUNT; ++i)
		{
			aiExclusiveTicks[i] = 0;
			aiCalls[i] = 0;
		}
		pCurrentScope = NULL;
		iLastGameUpdateExitTicks = 0;
		iCurrentCallbackGapTicks = 0;
		iSchedulerSampleCount = 0;
		bActive = true;
	}

	void finishOpenScopes()
	{
		LARGE_INTEGER kNow;
		QueryPerformanceCounter(&kNow);
		while (pCurrentScope != NULL)
		{
			MnaiReleaseTraceScope* pScope = pCurrentScope;
			pCurrentScope = pScope->getParent();
			pScope->finish(kNow.QuadPart);
		}
		bActive = false;
	}

	void logTrace()
	{
		LARGE_INTEGER kFrequency;
		QueryPerformanceFrequency(&kFrequency);
		if (kFrequency.QuadPart == 0)
		{
			kFrequency.QuadPart = 1;
		}
		if (!bTraceHeaderWritten)
		{
			gDLL->logMsg("release_trace.csv", "turn,phase,exclusive_us,calls", false, false);
			bTraceHeaderWritten = true;
		}

		CvString szBuffer;
		for (int i = 0; i < MNAI_TRACE_PHASE_COUNT; ++i)
		{
			if (aiCalls[i] == 0)
			{
				continue;
			}
			const long iMicroseconds = (long)(aiExclusiveTicks[i] * 1000000 / kFrequency.QuadPart);
			szBuffer.Format("%d,%s,%ld,%u",
				GC.getGameINLINE().getGameTurn(),
				aszPhaseNames[i],
				iMicroseconds,
				aiCalls[i]);
			gDLL->logMsg("release_trace.csv", szBuffer.c_str(), false, false);
		}
	}

	void logSchedulerTrace()
	{
		if (iSchedulerSampleCount == 0)
		{
			return;
		}
		LARGE_INTEGER kFrequency;
		QueryPerformanceFrequency(&kFrequency);
		if (kFrequency.QuadPart == 0)
		{
			kFrequency.QuadPart = 1;
		}
		if (!bSchedulerHeaderWritten)
		{
			gDLL->logMsg("scheduler_trace.csv", "turn,sample,callback_gap_us,active_players,busy_groups,mission_timer_groups,combat_groups,mission_timer_total,show_enemy_moves,show_friendly_moves,quick_moves,quick_attack,quick_defense", false, false);
			bSchedulerHeaderWritten = true;
		}

		CvGame& kGame = GC.getGameINLINE();
		const PlayerTypes eActivePlayer = kGame.getActivePlayer();
		const CvPlayer* pActivePlayer = (eActivePlayer != NO_PLAYER) ? &GET_PLAYER(eActivePlayer) : NULL;
		CvString szBuffer;
		for (int i = 0; i < iSchedulerSampleCount; ++i)
		{
			const SchedulerSample& kSample = aSchedulerSamples[i];
			szBuffer.Format("%d,%d,%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				kGame.getGameTurn(),
				i + 1,
				(long)(kSample.iCallbackGapTicks * 1000000 / kFrequency.QuadPart),
				kSample.iActivePlayers,
				kSample.iBusyGroups,
				kSample.iMissionTimerGroups,
				kSample.iCombatGroups,
				kSample.iMissionTimerTotal,
				(pActivePlayer != NULL && pActivePlayer->isOption(PLAYEROPTION_SHOW_ENEMY_MOVES)) ? 1 : 0,
				(pActivePlayer != NULL && pActivePlayer->isOption(PLAYEROPTION_SHOW_FRIENDLY_MOVES)) ? 1 : 0,
				(pActivePlayer != NULL && pActivePlayer->isOption(PLAYEROPTION_QUICK_MOVES)) ? 1 : 0,
				(pActivePlayer != NULL && pActivePlayer->isOption(PLAYEROPTION_QUICK_ATTACK)) ? 1 : 0,
				(pActivePlayer != NULL && pActivePlayer->isOption(PLAYEROPTION_QUICK_DEFENSE)) ? 1 : 0);
			gDLL->logMsg("scheduler_trace.csv", szBuffer.c_str(), false, false);
		}
	}

	void addCity(Fingerprint& kHash, CvCity* pCity)
	{
		kHash.addInt(pCity->getID());
		kHash.addInt(pCity->getX_INLINE());
		kHash.addInt(pCity->getY_INLINE());
		kHash.addInt(pCity->getPopulation());
		kHash.addInt(pCity->getFood());
		kHash.addInt(pCity->getProduction());
		kHash.addInt(pCity->getOccupationTimer());
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
		{
			kHash.addInt(pCity->getYieldRate((YieldTypes)iYield));
		}
		for (int iPlot = 0; iPlot < pCity->getNumCityPlots(); ++iPlot)
		{
			kHash.addInt(pCity->isWorkingPlot(iPlot) ? 1 : 0);
			kHash.addInt(pCity->AI_getBestBuild(iPlot));
			kHash.addInt(pCity->AI_getBestBuildValue(iPlot));
		}
		for (int iSpecialist = 0; iSpecialist < GC.getNumSpecialistInfos(); ++iSpecialist)
		{
			kHash.addInt(pCity->getSpecialistCount((SpecialistTypes)iSpecialist));
		}
		for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); ++iBuilding)
		{
			kHash.addInt(pCity->getNumBuilding((BuildingTypes)iBuilding));
		}
	}

	void addUnit(Fingerprint& kHash, CvUnit* pUnit)
	{
		kHash.addInt(pUnit->getID());
		kHash.addInt(pUnit->getUnitType());
		kHash.addInt(pUnit->getX_INLINE());
		kHash.addInt(pUnit->getY_INLINE());
		kHash.addInt(pUnit->getDamage());
		kHash.addInt(pUnit->getMoves());
		kHash.addInt(pUnit->getExperience());
		kHash.addInt(pUnit->getLevel());
		kHash.addInt(pUnit->getGroupID());
		kHash.addInt(pUnit->AI_getUnitAIType());
		kHash.addInt(pUnit->AI_getGroupflag());
		CvUnit* pTransport = pUnit->getTransportUnit();
		kHash.addInt((pTransport != NULL) ? pTransport->getOwnerINLINE() : -1);
		kHash.addInt((pTransport != NULL) ? pTransport->getID() : -1);
	}

	void addGroup(Fingerprint& kHash, CvSelectionGroup* pGroup)
	{
		kHash.addInt(pGroup->getID());
		kHash.addInt(pGroup->getActivityType());
		kHash.addInt(pGroup->getNumUnits());
		kHash.addInt(pGroup->AI_getGroupflag());
		kHash.addInt(pGroup->getLengthMissionQueue());
		for (CLLNode<MissionData>* pNode = pGroup->headMissionQueueNode(); pNode != NULL; pNode = pGroup->nextMissionQueueNode(pNode))
		{
			kHash.addInt(pNode->m_data.eMissionType);
			kHash.addInt(pNode->m_data.iData1);
			kHash.addInt(pNode->m_data.iData2);
			kHash.addInt(pNode->m_data.iFlags);
			kHash.addInt(pNode->m_data.iPushTurn);
		}
	}

	void logFingerprint()
	{
		Fingerprint kHash;
		CvGame& kGame = GC.getGameINLINE();
		kHash.addInt(kGame.getGameTurn());
		kHash.addInt(kGame.getElapsedGameTurns());
		kHash.addUnsigned(kGame.getSorenRand().getSeed());
		kHash.addUnsigned(kGame.getMapRand().getSeed());

		int iCityCount = 0;
		int iUnitCount = 0;
		int iGroupCount = 0;
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);
			kHash.addInt(iPlayer);
			kHash.addInt(kPlayer.isAlive() ? 1 : 0);
			if (!kPlayer.isEverAlive())
			{
				continue;
			}
			kHash.addInt(kPlayer.getTeam());
			kHash.addInt(kPlayer.getGold());
			kHash.addInt(kPlayer.getTotalPopulation());
			kHash.addInt(kPlayer.getNumCities());
			kHash.addInt(kPlayer.getNumUnits());
			kHash.addInt(kPlayer.getPower());
			kHash.addInt(kPlayer.getStateReligion());
			for (int iCivic = 0; iCivic < GC.getNumCivicOptionInfos(); ++iCivic)
			{
				kHash.addInt(kPlayer.getCivics((CivicOptionTypes)iCivic));
			}

			int iLoop = 0;
			for (CvCity* pCity = kPlayer.firstCity(&iLoop); pCity != NULL; pCity = kPlayer.nextCity(&iLoop))
			{
				addCity(kHash, pCity);
				++iCityCount;
			}
			for (CvUnit* pUnit = kPlayer.firstUnit(&iLoop); pUnit != NULL; pUnit = kPlayer.nextUnit(&iLoop))
			{
				addUnit(kHash, pUnit);
				++iUnitCount;
			}
			for (CvSelectionGroup* pGroup = kPlayer.firstSelectionGroup(&iLoop); pGroup != NULL; pGroup = kPlayer.nextSelectionGroup(&iLoop))
			{
				addGroup(kHash, pGroup);
				++iGroupCount;
			}
		}

		for (int iTeam = 0; iTeam < MAX_TEAMS; ++iTeam)
		{
			CvTeam& kTeam = GET_TEAM((TeamTypes)iTeam);
			kHash.addInt(iTeam);
			kHash.addInt(kTeam.isAlive() ? 1 : 0);
			if (!kTeam.isEverAlive())
			{
				continue;
			}
			for (int iTech = 0; iTech < GC.getNumTechInfos(); ++iTech)
			{
				kHash.addInt(kTeam.isHasTech((TechTypes)iTech) ? 1 : 0);
			}
			for (int iOtherTeam = 0; iOtherTeam < MAX_TEAMS; ++iOtherTeam)
			{
				kHash.addInt(kTeam.isAtWar((TeamTypes)iOtherTeam) ? 1 : 0);
			}
		}

		for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); ++iPlot)
		{
			CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
			kHash.addInt(pPlot->getOwnerINLINE());
			kHash.addInt(pPlot->getPlotType());
			kHash.addInt(pPlot->getTerrainType());
			kHash.addInt(pPlot->getFeatureType());
			kHash.addInt(pPlot->getBonusType(NO_TEAM));
			kHash.addInt(pPlot->getImprovementType());
			kHash.addInt(pPlot->getRouteType());
			for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
			{
				if (GET_PLAYER((PlayerTypes)iPlayer).isEverAlive())
				{
					kHash.addInt(pPlot->getCulture((PlayerTypes)iPlayer));
				}
			}
		}

		if (iCityCount == 0)
		{
			return;
		}

		if (!bFingerprintHeaderWritten)
		{
			gDLL->logMsg("state_fingerprint.csv", "turn,hash_hi,hash_lo,cities,units,groups", false, false);
			bFingerprintHeaderWritten = true;
		}
		const unsigned __int64 iHash = kHash.value();
		CvString szBuffer;
		szBuffer.Format("%d,%08x,%08x,%d,%d,%d",
			kGame.getGameTurn(),
			(unsigned int)(iHash >> 32),
			(unsigned int)(iHash & 0xffffffffui64),
			iCityCount,
			iUnitCount,
			iGroupCount);
		gDLL->logMsg("state_fingerprint.csv", szBuffer.c_str(), false, false);
	}
}

MnaiReleaseTraceScope::MnaiReleaseTraceScope(MnaiReleaseTracePhase ePhase) :
	m_ePhase(ePhase),
	m_iStartTicks(0),
	m_iChildTicks(0),
	m_pParent(mnai_release_trace::pCurrentScope),
	m_bActive(mnai_release_trace::bActive)
{
	LARGE_INTEGER kNow;
	QueryPerformanceCounter(&kNow);
	if (m_ePhase == MNAI_TRACE_GAME_UPDATE && mnai_release_trace::bActive)
	{
		mnai_release_trace::iCurrentCallbackGapTicks = 0;
		if (mnai_release_trace::iLastGameUpdateExitTicks != 0)
		{
			mnai_release_trace::iCurrentCallbackGapTicks = kNow.QuadPart - mnai_release_trace::iLastGameUpdateExitTicks;
			mnai_release_trace::aiExclusiveTicks[MNAI_TRACE_CALLBACK_GAP] += mnai_release_trace::iCurrentCallbackGapTicks;
			mnai_release_trace::aiCalls[MNAI_TRACE_CALLBACK_GAP]++;
		}
	}
	if (!m_bActive)
	{
		return;
	}
	m_iStartTicks = kNow.QuadPart;
	mnai_release_trace::pCurrentScope = this;
}

MnaiReleaseTraceScope::~MnaiReleaseTraceScope()
{
	LARGE_INTEGER kNow;
	QueryPerformanceCounter(&kNow);
	if (!m_bActive)
	{
		if (m_ePhase == MNAI_TRACE_GAME_UPDATE && mnai_release_trace::bActive)
		{
			mnai_release_trace::iLastGameUpdateExitTicks = kNow.QuadPart;
		}
		return;
	}
	finish(kNow.QuadPart);
	mnai_release_trace::pCurrentScope = m_pParent;
	if (m_ePhase == MNAI_TRACE_GAME_UPDATE && mnai_release_trace::bActive)
	{
		mnai_release_trace::iLastGameUpdateExitTicks = kNow.QuadPart;
	}
}

void MnaiReleaseTraceScope::finish(__int64 iNowTicks)
{
	if (!m_bActive)
	{
		return;
	}
	const __int64 iElapsed = iNowTicks - m_iStartTicks;
	const __int64 iExclusive = (iElapsed > m_iChildTicks) ? (iElapsed - m_iChildTicks) : 0;
	mnai_release_trace::aiExclusiveTicks[m_ePhase] += iExclusive;
	mnai_release_trace::aiCalls[m_ePhase]++;
	if (m_pParent != NULL)
	{
		m_pParent->m_iChildTicks += iElapsed;
	}
	m_bActive = false;
}

MnaiReleaseTraceScope* MnaiReleaseTraceScope::getParent() const
{
	return m_pParent;
}

void mnaiReleaseTraceSampleScheduler()
{
	if (!mnai_release_trace::bActive || mnai_release_trace::iSchedulerSampleCount >= mnai_release_trace::MAX_SCHEDULER_SAMPLES)
	{
		return;
	}

	mnai_release_trace::SchedulerSample& kSample = mnai_release_trace::aSchedulerSamples[mnai_release_trace::iSchedulerSampleCount++];
	kSample.iCallbackGapTicks = mnai_release_trace::iCurrentCallbackGapTicks;
	kSample.iActivePlayers = 0;
	kSample.iBusyGroups = 0;
	kSample.iMissionTimerGroups = 0;
	kSample.iCombatGroups = 0;
	kSample.iMissionTimerTotal = 0;

	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (!kPlayer.isAlive() || !kPlayer.isTurnActive())
		{
			continue;
		}
		++kSample.iActivePlayers;
		int iLoop = 0;
		for (CvSelectionGroup* pGroup = kPlayer.firstSelectionGroup(&iLoop); pGroup != NULL; pGroup = kPlayer.nextSelectionGroup(&iLoop))
		{
			if (pGroup->isBusy())
			{
				++kSample.iBusyGroups;
			}
			if (pGroup->getMissionTimer() > 0)
			{
				++kSample.iMissionTimerGroups;
				kSample.iMissionTimerTotal += pGroup->getMissionTimer();
			}
			bool bCombat = false;
			for (CLLNode<IDInfo>* pNode = pGroup->headUnitNode(); pNode != NULL && !bCombat; pNode = pGroup->nextUnitNode(pNode))
			{
				CvUnit* pUnit = ::getUnit(pNode->m_data);
				bCombat = (pUnit != NULL && pUnit->isCombat());
			}
			if (bCombat)
			{
				++kSample.iCombatGroups;
			}
		}
	}
}
#endif // MNAI_RELEASE_TRACE

#ifdef MNAI_PROFILE_PATH_REQUESTS
namespace mnai_path_requests
{
	struct PathRequestStats
	{
		PathRequestStats() :
			iFlags(0),
			iCalls(0),
			iSuccesses(0),
			iReuseCalls(0),
			iPathTurnSamples(0),
			iPathTurnSum(0),
			iMaxPathTurns(0),
			iElapsedTicks(0)
		{
		}

		std::string szContext;
		int iFlags;
		int iCalls;
		int iSuccesses;
		int iReuseCalls;
		int iPathTurnSamples;
		int iPathTurnSum;
		int iMaxPathTurns;
		__int64 iElapsedTicks;
		std::map<std::string, int> mapRequestCounts;
	};

	struct CrossContextStats
	{
		CrossContextStats() : iSharedRequests(0), iDuplicateCalls(0), iCallsA(0), iCallsB(0) {}
		int iSharedRequests;
		int iDuplicateCalls;
		int iCallsA;
		int iCallsB;
	};

	std::map<std::string, PathRequestStats> mapStats;
	std::map<std::string, std::map<std::string, int> > mapRequestContextCounts;
	const char* pszCurrentContext = "unscoped";
	bool bCsvHeaderWritten = false;
	bool bCrossCsvHeaderWritten = false;

	bool comparePathRequestStats(const PathRequestStats* pA, const PathRequestStats* pB)
	{
		if (pA->iElapsedTicks != pB->iElapsedTicks)
		{
			return pA->iElapsedTicks > pB->iElapsedTicks;
		}
		return pA->iCalls > pB->iCalls;
	}
}

void mnaiResetPathRequestStats()
{
	mnai_path_requests::mapStats.clear();
	mnai_path_requests::mapRequestContextCounts.clear();
}

const char* mnaiGetPathRequestContext()
{
	return mnai_path_requests::pszCurrentContext;
}

MnaiPathRequestContextScope::MnaiPathRequestContextScope(const char* pszContext)
{
	m_pszPreviousContext = mnai_path_requests::pszCurrentContext;
	mnai_path_requests::pszCurrentContext = (pszContext != NULL) ? pszContext : "unscoped";
}

MnaiPathRequestContextScope::~MnaiPathRequestContextScope()
{
	mnai_path_requests::pszCurrentContext = m_pszPreviousContext;
}

void mnaiRecordPathRequest(const char* pszContext, const CvSelectionGroup* pGroup, const CvPlot* pFromPlot, const CvPlot* pToPlot, int iFlags, bool bReuse, bool bSuccess, int iPathTurns, __int64 iElapsedTicks)
{
	CvString szContextKey;
	szContextKey.Format("%s:%d", (pszContext != NULL) ? pszContext : "unscoped", iFlags);
	mnai_path_requests::PathRequestStats& kStats = mnai_path_requests::mapStats[std::string(szContextKey.c_str())];

	if (kStats.iCalls == 0)
	{
		kStats.szContext = (pszContext != NULL) ? pszContext : "unscoped";
		kStats.iFlags = iFlags;
	}

	kStats.iCalls++;
	if (bSuccess)
	{
		kStats.iSuccesses++;
	}
	if (bReuse)
	{
		kStats.iReuseCalls++;
	}
	if (bSuccess && iPathTurns != MAX_INT)
	{
		kStats.iPathTurnSamples++;
		kStats.iPathTurnSum += iPathTurns;
		kStats.iMaxPathTurns = std::max(kStats.iMaxPathTurns, iPathTurns);
	}
	kStats.iElapsedTicks += iElapsedTicks;

	const int iOwner = (pGroup != NULL) ? pGroup->getOwnerINLINE() : -1;
	const int iGroup = (pGroup != NULL) ? pGroup->getID() : -1;
	const int iFromX = (pFromPlot != NULL) ? pFromPlot->getX_INLINE() : -1;
	const int iFromY = (pFromPlot != NULL) ? pFromPlot->getY_INLINE() : -1;
	const int iToX = (pToPlot != NULL) ? pToPlot->getX_INLINE() : -1;
	const int iToY = (pToPlot != NULL) ? pToPlot->getY_INLINE() : -1;

	CvString szRequest;
	szRequest.Format("%d:%d:%d:%d:%d:%d:%d:%d",
		iOwner,
		iGroup,
		iFromX,
		iFromY,
		iToX,
		iToY,
		iFlags,
		bReuse ? 1 : 0);
	kStats.mapRequestCounts[std::string(szRequest.c_str())]++;
	mnai_path_requests::mapRequestContextCounts[std::string(szRequest.c_str())][std::string(szContextKey.c_str())]++;
}

void mnaiLogPathRequestStats()
{
	LARGE_INTEGER measurementFreq;
	QueryPerformanceFrequency(&measurementFreq);
	if (measurementFreq.QuadPart == 0)
	{
		measurementFreq.QuadPart = 1;
	}

	if (!mnai_path_requests::bCsvHeaderWritten)
	{
		gDLL->logMsg("path_request_callers.csv", "turn,context,flags,calls,successes,reuse_calls,total_ms,avg_us,unique_requests,duplicate_calls,top_request_calls,avg_path_turns,max_path_turns", false, false);
		mnai_path_requests::bCsvHeaderWritten = true;
	}
	if (!mnai_path_requests::bCrossCsvHeaderWritten)
	{
		gDLL->logMsg("path_request_cross_context.csv", "turn,context_a,context_b,shared_requests,cross_duplicate_calls,calls_a,calls_b", false, false);
		mnai_path_requests::bCrossCsvHeaderWritten = true;
	}

	std::vector<mnai_path_requests::PathRequestStats*> apStats;
	for (std::map<std::string, mnai_path_requests::PathRequestStats>::iterator it = mnai_path_requests::mapStats.begin(); it != mnai_path_requests::mapStats.end(); ++it)
	{
		apStats.push_back(&it->second);
	}
	std::sort(apStats.begin(), apStats.end(), mnai_path_requests::comparePathRequestStats);

	const int iTurn = GC.getGameINLINE().getGameTurn();
	CvString szBuffer;
	for (std::vector<mnai_path_requests::PathRequestStats*>::iterator it = apStats.begin(); it != apStats.end(); ++it)
	{
		const mnai_path_requests::PathRequestStats& kStats = **it;
		int iDuplicateCalls = 0;
		int iTopRequestCalls = 0;
		for (std::map<std::string, int>::const_iterator itReq = kStats.mapRequestCounts.begin(); itReq != kStats.mapRequestCounts.end(); ++itReq)
		{
			if (itReq->second > 1)
			{
				iDuplicateCalls += itReq->second - 1;
			}
			iTopRequestCalls = std::max(iTopRequestCalls, itReq->second);
		}

		const long iTotalMs = (long)(kStats.iElapsedTicks * 1000 / measurementFreq.QuadPart);
		const long iAvgUs = (kStats.iCalls > 0) ? (long)(kStats.iElapsedTicks * 1000000 / measurementFreq.QuadPart / kStats.iCalls) : 0;
		const int iAvgPathTurns = (kStats.iPathTurnSamples > 0) ? (kStats.iPathTurnSum / kStats.iPathTurnSamples) : -1;

		szBuffer.Format("%d,%s,%d,%d,%d,%d,%ld,%ld,%d,%d,%d,%d,%d",
			iTurn,
			kStats.szContext.c_str(),
			kStats.iFlags,
			kStats.iCalls,
			kStats.iSuccesses,
			kStats.iReuseCalls,
			iTotalMs,
			iAvgUs,
			(int)kStats.mapRequestCounts.size(),
			iDuplicateCalls,
			iTopRequestCalls,
			iAvgPathTurns,
			kStats.iMaxPathTurns);
		gDLL->logMsg("path_request_callers.csv", szBuffer.c_str(), false, false);
	}

	std::map<std::string, mnai_path_requests::CrossContextStats> mapCrossStats;
	for (std::map<std::string, std::map<std::string, int> >::const_iterator itRequest = mnai_path_requests::mapRequestContextCounts.begin(); itRequest != mnai_path_requests::mapRequestContextCounts.end(); ++itRequest)
	{
		const std::map<std::string, int>& kContexts = itRequest->second;
		for (std::map<std::string, int>::const_iterator itA = kContexts.begin(); itA != kContexts.end(); ++itA)
		{
			std::map<std::string, int>::const_iterator itB = itA;
			++itB;
			for (; itB != kContexts.end(); ++itB)
			{
				CvString szPair;
				szPair.Format("%s|%s", itA->first.c_str(), itB->first.c_str());
				mnai_path_requests::CrossContextStats& kCross = mapCrossStats[std::string(szPair.c_str())];
				kCross.iSharedRequests++;
				kCross.iDuplicateCalls += std::min(itA->second, itB->second);
				kCross.iCallsA += itA->second;
				kCross.iCallsB += itB->second;
			}
		}
	}
	for (std::map<std::string, mnai_path_requests::CrossContextStats>::const_iterator itCross = mapCrossStats.begin(); itCross != mapCrossStats.end(); ++itCross)
	{
		const std::string& szPair = itCross->first;
		const std::string::size_type iSeparator = szPair.find('|');
		if (iSeparator == std::string::npos)
		{
			continue;
		}
		const mnai_path_requests::CrossContextStats& kCross = itCross->second;
		szBuffer.Format("%d,%s,%s,%d,%d,%d,%d",
			iTurn,
			szPair.substr(0, iSeparator).c_str(),
			szPair.substr(iSeparator + 1).c_str(),
			kCross.iSharedRequests,
			kCross.iDuplicateCalls,
			kCross.iCallsA,
			kCross.iCallsB);
		gDLL->logMsg("path_request_cross_context.csv", szBuffer.c_str(), false, false);
	}
}
#endif // MNAI_PROFILE_PATH_REQUESTS

//
// enable dll profiler if necessary, clear history
//
void startProfilingDLL()
{
#ifdef MNAI_RELEASE_TRACE
	mnai_release_trace::reset();
#endif
#ifdef MNAI_AUTOVERIFY_TURN_CSV
	QueryPerformanceCounter(&mnai_autoverify_turn_csv::startTime);
	mnai_autoverify_turn_csv::bStarted = true;
#endif

#ifdef CUSTOM_PROFILER
	if( GC.isDLLProfilerEnabled() || GC.getDefineINT( "FORCE_ENABLE_CUSTOM_PROFILER", 1 ) )
	{
#ifdef MNAI_PROFILE_FULL_MEMBER_CALLERS
		mnaiResetFullMemberCallerStats();
#endif
#ifdef MNAI_PROFILE_PATH_REQUESTS
		mnaiResetPathRequestStats();
#endif
		custom_profiler::startProfiling();
		custom_profiler::beginSample( &custom_profiler::turnSample );
	}
#else
	if (GC.isDLLProfilerEnabled())
	{
		gDLL->ProfilerBegin();
	}
#endif
}

//
// dump profile stats on-screen
// CUSTOM_PROFILER logs profile stats to custom_profile.log
//
void stopProfilingDLL()
{
#ifdef CUSTOM_PROFILER
	if( GC.isDLLProfilerEnabled() || GC.getDefineINT( "FORCE_ENABLE_CUSTOM_PROFILER", 1 ) )
	{
		custom_profiler::endSample( &custom_profiler::turnSample );
		custom_profiler::stopProfiling();
#ifdef MNAI_PROFILE_FULL_MEMBER_CALLERS
		mnaiLogFullMemberCallerStats();
#endif
#ifdef MNAI_PROFILE_PATH_REQUESTS
		mnaiLogPathRequestStats();
#endif
	}
#else
	if (GC.isDLLProfilerEnabled())
	{
		gDLL->ProfilerEnd();
	}
#endif

#ifdef MNAI_AUTOVERIFY_TURN_CSV
	if (mnai_autoverify_turn_csv::bStarted)
	{
		LARGE_INTEGER endTime;
		LARGE_INTEGER freq;
		QueryPerformanceCounter(&endTime);
		QueryPerformanceFrequency(&freq);
		if (freq.QuadPart == 0)
		{
			freq.QuadPart = 1;
		}
		if (!mnai_autoverify_turn_csv::bHeaderWritten)
		{
			gDLL->logMsg("autoverify_dll.csv", "turn,interval_ms", false, false);
			mnai_autoverify_turn_csv::bHeaderWritten = true;
		}
		CvString buffer;
		buffer.Format("%d,%ld",
			GC.getGameINLINE().getGameTurn(),
			(long)((endTime.QuadPart - mnai_autoverify_turn_csv::startTime.QuadPart) * 1000 / freq.QuadPart));
		gDLL->logMsg("autoverify_dll.csv", buffer.c_str(), false, false);
		mnai_autoverify_turn_csv::bStarted = false;
	}
#endif

#ifdef MNAI_RELEASE_TRACE
	mnai_release_trace::finishOpenScopes();
	mnai_release_trace::logTrace();
	mnai_release_trace::logSchedulerTrace();
	mnai_release_trace::logFingerprint();
#endif
}
