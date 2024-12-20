#include <Helpers/Macro.h>
#include <Syringe.h>
#include <BuildingClass.h>
#include <HouseClass.h>

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x730B09, DeployCommandClass_Execute_BuildingDeploy, 0x5)
{
	for (auto const pObject : ObjectClass::CurrentObjects())
	{
		if (!pObject || pObject->WhatAmI() != AbstractType::Building)
			continue;

		auto const pBuilding = abstract_cast<BuildingClass*>(pObject);

		if (!pBuilding->Owner || pBuilding->Owner->Defeated ||
			pBuilding->Owner->IsObserver() ||
			!pBuilding->Owner->ControlledByPlayer() ||
			pBuilding->IsUnderEMP() ||
			pBuilding->IsDeactivated())
			continue;

		if (pBuilding->GetOccupantCount() > 0 ||
			pBuilding->Type->GapGenerator ||
			(pBuilding->Passengers.NumPassengers > 0 &&
			(pBuilding->Type->UnitAbsorb || pBuilding->Type->InfantryAbsorb)))
		{
			pBuilding->ClickedMission(Mission::Unload, pBuilding->Type->GapGenerator ? pBuilding : nullptr, nullptr, nullptr);
			continue;
		}
	}

	return 0;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x44D889, BuildingClass_Mi_Unload_FreezeOccupants, 0x9)
{
	GET(BuildingClass*, pThis, EBP);

	if (pThis->GetOccupantCount() > 0)
	{
		pThis->FreezeOccupants(false, false);
		pThis->NeedsRedraw = true;
	}

	R->EDI(0);
	return 0x44D8A1;
}
