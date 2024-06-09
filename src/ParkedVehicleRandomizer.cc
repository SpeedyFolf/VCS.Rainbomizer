#include "Hooks.hh"
#include "core/Randomizer.hh"
#include "log.h"
#include "CCarGenerator.hh"
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <set>
#include <string>

class ParkedVehicleRandomizer
{
    BEGIN_RANDOMIZER(ParkedVehicleRandomizer)

public:
template <auto &DoInternalProcessing>
static void
RandomizeParkedVehicle (CCarGenerator* gen)
{
    int origModel = gen->m_nModelId;
    int newModel = 244;

    gen->m_nModelId = newModel;
    gen->DoInternalProcessing(gen);
    gen->m_nModelId = origModel;
}

    ParkedVehicleRandomizer()
    {
        REGISTER_HOOK_ADDR(0x8aed784, RandomizeParkedVehicle, CCarGenerator);
    }

    END_RANDOMIZER(ParkedVehicleRandomizer)
};