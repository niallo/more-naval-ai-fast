# CUSTOM_PROFILER by lfgr #


CUSTOM_PROFILER is a replacement of the CIV4 profiler, because I couldn't get the latter
working reliably.


### RELEASE-FAST TRACE ###

The optimization workspace also provides a separate `ReleaseFastTrace` target.
It is intended for low-overhead release-path attribution, not as a replacement
for the detailed `CUSTOM_PROFILER` markers and not as a playable DLL.

`ReleaseFastTrace` uses a fixed enum-indexed nested timer, so it does not build
maps or register named samples in hot code. It writes `release_trace.csv` with
exclusive phase time, `scheduler_trace.csv` with callback-gap and busy-state
observations, and `state_fingerprint.csv` with deterministic post-turn hashes.
The AutoVerify batch runner summarizes these as `RELEASE_TRACE.md` and
`release_trace.json`, and treats differing selected-turn fingerprints as a
failed run.

Build it with:

```sh
./build-releasefasttrace-wine.sh
```

Use `ProfileFast`/`CUSTOM_PROFILER` for detailed hotspot attribution,
`ReleaseFastTrace` for low-overhead phase and scheduler attribution, and
`ReleaseFastVerify` for the final no-profiler acceptance median.


### USAGE ###

You need to compile with the /DCUSTOM_PROFILER as well as /DFP_PROFILE_ENABLE.
The "Profile" configuration of the included Makefile does this. Its output, found in
custom_profile.log in the Logs folder is slightly simpler than that of Firaxis' profiler.
The first column shows the total time spent in the profiled function/block in ms. The
second column shows the number of calls to the profiled function/block. The third column
shows the number of unclosed calls, i.e. the number of calls where the profiling wasn't
ended for that particular sample. The fourth column is the name of the profiled
function/block. The functions/blocks are sorted by total time. The special "Turn" sample
measures the whole time (between starting and stopping profiling in this turn).


### MERGING ###

All code is inside CUSTOM_PROFILER-#ifdefs. Make sure to also merge the #ifndef and #else
which disable the Firaxis profiler.

### CREDITS ###

CUSTOM_PROFILER was inspired by Koshling's custom C2C profiler. I also used some of
Koshing's code.
