#include <cstdint>
#include <ranges>
#include <vcs/CRunningScript.hh>
#include <vcs/CStreaming.hh>
#include <vcs/CVehicle.hh>
#include <vcs/CModelInfo.hh>

#include <core/Logger.hh>
#include <core/Randomizer.hh>
#include <core/Config.hh>

#include <hooks/Hooks.hh>

#include "Common.hh"
#include "core/ThreadUtils.hh"
#include "memory/GameAddress.hh"
#include "ppsspp/KeyCodes.h"
#include "ppsspp/Keyboard.hh"
#include "psploadcore.h"
#include "scm/Command.hh"
#include "scm/Opcodes.hh"
#include "vehicle/VehiclePatterns.hh"

#include <pspsdk.h>
#include <psputility.h>

class ScriptVehicleRandomizer : public Randomizer<ScriptVehicleRandomizer>
{
    VehiclePatternManager m_Patterns;
    int ForcedVehicle = -1;

    template <auto &CollectParams>
    int
    RandomizeVehicle (CRunningScript *scr, int *p2, int p3, int *params)
    {
        int ret = CollectParams (scr, p2, p3, params);

        PatternResult result{params[0]};
        m_Patterns.GetRandomVehicle (eVehicle (params[0]), scr, result);

        int originalVehicle = params[0];
        int newVehicle = ForcedVehicle == -1 ? result.vehId : ForcedVehicle;

        if (!VehicleCommon::AttemptToLoadVehicle (newVehicle))
            newVehicle = originalVehicle;

        params[0] = newVehicle;
        if (result.coords)
            {
                auto coords = reinterpret_cast<float *> (params);
                coords[1] += result.coords->x;
                coords[2] += result.coords->y;
                coords[3] += result.coords->z;
            }

        Rainbomizer::Logger::LogMessage (
            "Vehicle spawn: [%s]:%x (mission = %d) (%f %f %f): %d -> %d",
            scr->m_szName, scr->m_pCurrentIP,
            ThreadUtils::GetMissionIdFromThread (scr),
            std::bit_cast<float> (params[1]), std::bit_cast<float> (params[2]),
            std::bit_cast<float> (params[3]), originalVehicle, params[0]);

        return ret;
    }

    template <auto &CRunningScript__Process>
    static void
    ReloadPatternsCheck (CRunningScript *scr)
    {
        if (scr->m_bIsMission)
            CallCommand<PRINT_WITH_NUMBER_NOW>("NUMBER", CStreaming::sm_Instance->m_numVehiclesLoaded, 100, 1);

        if (PPSSPPUtils::CheckKeyUp<NKCODE_F5> ())
            {
                CallCommand<PRINT_BIG>("AU_ST2", 1000, 8);
                Get ().m_Patterns.ReadPatterns ("VehiclePatterns.txt");
            }

        CRunningScript__Process (scr);
    }

    template <auto &op_REQUEST_MODEL>
    static int
    RequestModelHook (CRunningScript *script)
    {
        int      model     = 10;
        uint32_t currentIp = script->m_pCurrentIP;

        script->CollectParams (&currentIp, 1, &model);

        if (model > 0
            && ModelInfo::GetModelInfo<CBaseModelInfo> (model)->type == 6)
            {
                script->CollectParams (&script->m_pCurrentIP, 1, &model);
                return 0;
            }

        return op_REQUEST_MODEL (script);
    }

    template <auto &op_HAS_MODEL_LOADED>
    static int
    HasModelLoadedHook (CRunningScript *script)
    {
        int      model;
        uint32_t currentIp = script->m_pCurrentIP;

        script->CollectParams (&currentIp, 1, &model);

        if (model > 0
            && ModelInfo::GetModelInfo<CBaseModelInfo> (model)->type == 6)
            {
                script->m_bNotFlag = !script->m_bNotFlag;
                return CTheScripts::ScriptCommands[BUILD_WORLD_GEOMETRY]
                    .handler (script);
            }

        return op_HAS_MODEL_LOADED (script);
    }

public:
    ScriptVehicleRandomizer ()
    {
        RB_C_DO_CONFIG("ScriptVehicleRandomizer", ForcedVehicle);

        m_Patterns.ReadPatterns ("VehiclePatterns.txt");
        HOOK_MEMBER (Jal, (0x08aec324), RandomizeVehicle,
                     int (class CRunningScript *, int *, int, int *));

        HOOK (Opcode, REQUEST_MODEL, RequestModelHook, int (CRunningScript *));
        HOOK (Opcode, HAS_MODEL_LOADED, HasModelLoadedHook,
              int (CRunningScript *));

        HOOK (Jal, 0x08869b00, ReloadPatternsCheck,
              void (class CRunningScript *));

        // Remove vehicle checks in several missions (hopefully no side-effects BlessRNG)
        GameAddress<0x8ae4efc>::WriteInstructions (li (a0, 1));


        // We don't want pop boot to crash the game in case car has no boot
        GameAddress<0x08832e04>::WriteInstructions (jr (ra));

        // Unlock scripted police vehicles
        GameAddress<0x8835298>::Write (li (a0, 1));

        ThreadUtils::Initialise ();
    }
};
