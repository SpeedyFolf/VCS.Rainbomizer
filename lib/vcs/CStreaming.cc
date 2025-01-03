#include "CStreaming.hh"
#include "memory/Memory.hh"
#include "vcs/CRunningScript.hh"

void
CStreaming::RequestModel (int id, int flags)
{
    return GameFunction<0x8ad3660, void (int, int)>::Call (id, flags);
}

void
CStreaming::LoadAllRequestedModels (bool p1)
{
    return GameFunction<0x8ad39f0, void (bool)>::Call (p1);
}

bool
CStreaming::HasModelLoaded(int id)
{
    return GameFunction<0x8ad3a78, bool (int)>::Call (id);
}

void
CStreaming::RequestModel (CRunningScript *script, int id)
{
    if (script->m_bUseMissionCleanup)
        RequestModel (id, 0xFFFFFFFF);
    else
        RequestModel (id, 0xFFFFFFFF);

    CStreaming::sm_Instance->ms_aInfoForModel[id].m_cleanupThread = script->id;
}
