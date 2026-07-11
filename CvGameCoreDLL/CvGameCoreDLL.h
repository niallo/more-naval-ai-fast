#pragma once

#ifndef CvGameCoreDLL_h
#define CvGameCoreDLL_h

//
// includes (pch) for gamecore dll files
// Author - Mustafa Thamer
//

//
// WINDOWS
//
#pragma warning( disable: 4530 )	// C++ exception handler used, but unwind semantics are not enabled

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <MMSystem.h>
#if defined _DEBUG && !defined USE_MEMMANAGER
#define USE_MEMMANAGER
#include <crtdbg.h>
#endif
#include <vector>
#include <list>
#include <tchar.h>
#include <math.h>
#include <assert.h>
#include <map>
#include <hash_map>

#define DllExport   __declspec( dllexport ) 

//
// GameBryo
//
class NiColor
{
public:
	float r, g, b;
};
class NiColorA 
{
public:
	NiColorA(float fr, float fg, float fb, float fa) : r(fr), g(fg), b(fb), a(fa) {}
	NiColorA() {}
	float r, g, b, a;
};
class NiPoint2
{
public:
	NiPoint2() {}
	NiPoint2(float fx, float fy) : x(fx),y(fy) {}

	float x, y;
};
class NiPoint3
{
public:
	NiPoint3() {}
	NiPoint3(float fx, float fy, float fz) : x(fx),y(fy),z(fz) {} 

	bool NiPoint3::operator== (const NiPoint3& pt) const
	{	return (x == pt.x && y == pt.y && z == pt.z);	}

	inline NiPoint3 NiPoint3::operator+ (const NiPoint3& pt) const
	{	return NiPoint3(x+pt.x,y+pt.y,z+pt.z);	}

	inline NiPoint3 NiPoint3::operator- (const NiPoint3& pt) const
	{	return NiPoint3(x-pt.x,y-pt.y,z-pt.z);	}

	inline float NiPoint3::operator* (const NiPoint3& pt) const
	{	return x*pt.x+y*pt.y+z*pt.z;	}

	inline NiPoint3 NiPoint3::operator* (float fScalar) const
	{	return NiPoint3(fScalar*x,fScalar*y,fScalar*z);	}

	inline NiPoint3 NiPoint3::operator/ (float fScalar) const
	{
		float fInvScalar = 1.0f/fScalar;
		return NiPoint3(fInvScalar*x,fInvScalar*y,fInvScalar*z);
	}

	inline NiPoint3 NiPoint3::operator- () const
	{	return NiPoint3(-x,-y,-z);	}

	inline float Length() const
	{ return sqrt(x * x + y * y + z * z); }

	inline float Unitize()
	{
		float length = Length();
		if(length != 0)
		{
			x /= length;
			y /= length;
			z /= length;
		}
		return length;
	}

//	inline NiPoint3 operator* (float fScalar, const NiPoint3& pt)
//	{	return NiPoint3(fScalar*pt.x,fScalar*pt.y,fScalar*pt.z);	}
	float x, y, z;
};

namespace NiAnimationKey
{
	enum KeyType
	{
		NOINTERP,
		LINKEY,
		BEZKEY,
		TCBKEY,
		EULERKEY,
		STEPKEY,
		NUMKEYTYPES
	};
};

typedef unsigned char    byte;
typedef unsigned short   word;
typedef unsigned int     uint;
typedef unsigned long    dword;
typedef unsigned __int64 qword;
typedef wchar_t          wchar;

#define MAX_CHAR                            (0x7f)
#define MIN_CHAR                            (0x80)
#define MAX_SHORT                           (0x7fff)
#define MIN_SHORT                           (0x8000)
#define MAX_INT                             (0x7fffffff)
#define MIN_INT                             (0x80000000)
#define MAX_UNSIGNED_CHAR                   (0xff)
#define MIN_UNSIGNED_CHAR                   (0x00)
#define MAX_UNSIGNED_SHORT                  (0xffff)
#define MIN_UNSIGNED_SHORT                  (0x0000)
#define MAX_UNSIGNED_INT                    (0xffffffff)
#define MIN_UNSIGNED_INT                    (0x00000000)

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#define SQR(x)      ( (x) * (x) )
#define DEGTORAD(x) ( (float)( (x) * (M_PI / 180) ))
#define LIMIT_RANGE(low, value, high) value = (value < low ? low : (value > high ? high : value));
#define M_PI       3.14159265358979323846
#define fM_PI		3.141592654f		//!< Pi (float)

__forceinline DWORD FtoDW( float f ) { return *(DWORD*)&f; }
__forceinline float DWtoF( dword n ) { return *(float*)&n; }
__forceinline float MaxFloat() { return DWtoF(0x7f7fffff); }

void startProfilingDLL();
void stopProfilingDLL();
#ifdef MNAI_RELEASE_TRACE
enum MnaiReleaseTracePhase
{
	MNAI_TRACE_GAME_UPDATE = 0,
	MNAI_TRACE_CALLBACK_GAP,
	MNAI_TRACE_UPDATE_SLICE,
	MNAI_TRACE_UPDATE_MOVES,
	MNAI_TRACE_SET_TURN_ACTIVE,
	MNAI_TRACE_GAME_DO_TURN,
	MNAI_TRACE_PLAYER_DO_TURN,
	MNAI_TRACE_PLAYER_DO_TURN_UNITS,
	MNAI_TRACE_CITY_DO_TURN,
	MNAI_TRACE_CITY_AI_DO_TURN,
	MNAI_TRACE_CITY_ASSIGN,
	MNAI_TRACE_CITY_BEST_BUILD,
	MNAI_TRACE_CITY_PLOT_VALUE,
	MNAI_TRACE_PLAYER_UNIT_UPDATE,
	MNAI_TRACE_GROUP_AI_UPDATE,
	MNAI_TRACE_UNIT_AI_UPDATE,
	MNAI_TRACE_UNIT_DISPATCH,
	MNAI_TRACE_GENERATE_PATH,
	MNAI_TRACE_DANGER,
	MNAI_TRACE_PHASE_COUNT
};

class MnaiReleaseTraceScope
{
public:
	MnaiReleaseTraceScope(MnaiReleaseTracePhase ePhase);
	~MnaiReleaseTraceScope();
	void finish(__int64 iNowTicks);
	MnaiReleaseTraceScope* getParent() const;

private:
	MnaiReleaseTracePhase m_ePhase;
	__int64 m_iStartTicks;
	__int64 m_iChildTicks;
	MnaiReleaseTraceScope* m_pParent;
	bool m_bActive;
};

void mnaiReleaseTraceSampleScheduler();

#define MNAI_RELEASE_TRACE_JOIN_INNER(a, b) a##b
#define MNAI_RELEASE_TRACE_JOIN(a, b) MNAI_RELEASE_TRACE_JOIN_INNER(a, b)
#define MNAI_RELEASE_TRACE_SCOPE(phase) MnaiReleaseTraceScope MNAI_RELEASE_TRACE_JOIN(mnaiReleaseTraceScope_, __LINE__)(phase)
#define MNAI_RELEASE_TRACE_SCHEDULER_SAMPLE() mnaiReleaseTraceSampleScheduler()
#else
#define MNAI_RELEASE_TRACE_SCOPE(phase)
#define MNAI_RELEASE_TRACE_SCHEDULER_SAMPLE()
#endif
#ifdef MNAI_PROFILE_FULL_MEMBER_CALLERS
void mnaiResetFullMemberCallerStats();
void mnaiLogFullMemberCallerStats();
#endif
#ifdef MNAI_PROFILE_PATH_REQUESTS
class CvSelectionGroup;
class CvPlot;
class MnaiPathRequestContextScope
{
public:
	MnaiPathRequestContextScope(const char* pszContext);
	~MnaiPathRequestContextScope();

private:
	const char* m_pszPreviousContext;
};
#define MNAI_PATH_REQUEST_CONTEXT(name) MnaiPathRequestContextScope mnaiPathRequestContextScope(name)
void mnaiResetPathRequestStats();
void mnaiLogPathRequestStats();
const char* mnaiGetPathRequestContext();
void mnaiRecordPathRequest(const char* pszContext, const CvSelectionGroup* pGroup, const CvPlot* pFromPlot, const CvPlot* pToPlot, int iFlags, bool bReuse, bool bSuccess, int iPathTurns, __int64 iElapsedTicks);
#else
#define MNAI_PATH_REQUEST_CONTEXT(name)
#endif

//
// Boost Python
//
# include <boost/python/list.hpp>
# include <boost/python/tuple.hpp>
# include <boost/python/class.hpp>
# include <boost/python/manage_new_object.hpp>
# include <boost/python/return_value_policy.hpp>
# include <boost/python/object.hpp>
# include <boost/python/def.hpp>

namespace python = boost::python;

#include "FAssert.h"
#include "CvGameCoreDLLDefNew.h"
#include "FDataStreamBase.h"
#include "FFreeListArrayBase.h"
#include "FFreeListTrashArray.h"
#include "FFreeListArray.h"
//#include "FVariableSystem.h"
#include "CvString.h"
#include "CvEnums.h"
#include "CvStructs.h"
#include "CvDLLUtilityIFaceBase.h"

//jason tests
#include "CvPlayerAI.h"
#include "CvGameCoreUtils.h"
#include "CvMap.h"
#include "CvGameAI.h"
#include "CvPlot.h"
#include "CvUnit.h"
#include "CvGlobals.h"
#include "CvCity.h"
#include "FProfiler.h"
#include "CyCity.h"
#include "CvInfos.h"
#include "CvTeamAI.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvRandom.h"
#include "CvArea.h"
#include "CvDllEntity.h"
#include "CvDeal.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvGame.h"
#include "CyGlobalContext.h"
#include "CvSelectionGroup.h"
#include "CvTalkingHeadMessage.h"
#include "CvPlotGroup.h"
#include "CvCityAI.h"
#include "CvSelectionGroupAI.h"
#include "CvUnitAI.h"

#ifdef FINAL_RELEASE
// Undefine OutputDebugString in final release builds
#undef OutputDebugString
#define OutputDebugString(x)
#endif //FINAL_RELEASE

#endif	// CvGameCoreDLL_h
