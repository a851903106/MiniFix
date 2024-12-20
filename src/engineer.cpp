#include <Helpers/Macro.h>
#include <InfantryClass.h>
#include <HouseClass.h>
#include <CellClass.h>
#include <SpawnManagerClass.h>
#include <InputManagerClass.h>

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
bool EngineerAllowEnterBuilding(InfantryClass* pThis, BuildingClass* pBuilding)
{
	if (!pBuilding)
		return false;

	bool isAllies = pThis->Owner->IsAlliedWith(pBuilding->Owner);

	if (pBuilding->Type->BridgeRepairHut ||
		(pBuilding->Type->Capturable && !isAllies))
		return false;

	if (!InputManagerClass::Instance->IsForceMoveKeyPressed())
	{
		if (pBuilding->AttachedBomb)
		{
			int Index = pThis->SelectWeapon(pBuilding);
			auto const fireError = pThis->GetFireErrorWithoutRange(pBuilding, Index);

			if (fireError == FireError::ILLEGAL ||
				fireError == FireError::CANT)
				return false;

			auto const pWeaponType = pThis->GetWeapon(Index)->WeaponType;
			if (pWeaponType->Warhead->BombDisarm)
				return false;
		}

		bool canRepairable = pBuilding->Type->Repairable;
		bool needRepair = canRepairable && pBuilding->Health < pBuilding->Type->Strength;

		if ((isAllies && !pBuilding->Type->InfantryAbsorb || needRepair) &&
			((pThis->Owner != pBuilding->Owner && pBuilding->Type->TechLevel >= 0) ||
				!pBuilding->Type->CanBeOccupied || needRepair))
			return false;
	}

	return true;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
bool EngineerAllow(InfantryClass* pThis, TechnoClass* pTechno, bool allowEnter)
{
	bool canCapture = false;
	bool canC4 = false;
	bool isBombDisarm = false;
	BuildingClass* pBuilding = abstract_cast<BuildingClass*>(pTechno);

	if (pBuilding)
	{
		if (pBuilding->Type->BridgeRepairHut)
			return false;

		canCapture = pBuilding->Type->Capturable &&
			!pThis->Owner->IsAlliedWith(pBuilding->Owner);

		if (pBuilding->Type->CanC4 && pThis->Type->C4)
			canC4 = true;
	}

	int index = pThis->SelectWeapon(pTechno);
	FireError fireError = pThis->GetFireErrorWithoutRange(pTechno, index);
	bool canFire = (fireError != FireError::NONE &&
		fireError != FireError::ILLEGAL &&
		fireError != FireError::CANT);

	if (pTechno->AttachedBomb && canFire)
	{
		auto const pWeapon = pThis->GetWeapon(index)->WeaponType;
		isBombDisarm = (pWeapon && pWeapon->Warhead && pWeapon->Warhead->BombDisarm);
	}

	if (!InputManagerClass::Instance->IsForceFireKeyPressed() &&
		!InputManagerClass::Instance->IsForceMoveKeyPressed() &&
		!InputManagerClass::Instance->IsForceSelectKeyPressed())
	{
		if (isBombDisarm || canCapture)
			return false;

		return (!allowEnter || EngineerAllowEnterBuilding(pThis, pBuilding)) && (canC4 || canFire);
	}

	return true;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
bool EngineerCanTargetObject(TechnoClass* pThis, TechnoClass* pTarget)
{
	bool isAllies = pThis->Owner->IsAlliedWith(pTarget);
	int index = pThis->SelectWeapon(pTarget);
	FireError fireError = pThis->GetFireErrorWithoutRange(pTarget, index);

	bool canFire = (fireError != FireError::NONE &&
		fireError != FireError::ILLEGAL &&
		fireError != FireError::CANT);

	bool inRange = pThis->DistanceFrom(pTarget) <= pThis->GetTechnoType()->GuardRange ||
		pThis->IsCloseEnough(pTarget, index);

	auto const pWeaponType = pThis->GetWeapon(index)->WeaponType;
	bool needRepair = false;

	if (isAllies && pTarget->Health < pTarget->GetTechnoType()->Strength)
	{
		if (pWeaponType && pWeaponType->Damage < 0)
		{
			needRepair = true;
		}
		else if (pTarget->WhatAmI() == AbstractType::Building)
		{
			auto const pBld = abstract_cast<BuildingClass*>(pTarget);

			if (!pBld->Type->BridgeRepairHut && pBld->Type->Repairable)
				needRepair = true;
		}
	}

	if (canFire && inRange)
	{
		if (isAllies)
		{
			if (pTarget->AttachedBomb &&
				pWeaponType &&
				pWeaponType->Warhead &&
				pWeaponType->Warhead->BombDisarm)
				return true;

			if (needRepair || pTarget->GetTechnoType()->AttackFriendlies)
				return true;
		}
		else if (!pTarget->Disguised ||
			pThis->GetTechnoType()->DetectDisguise ||
			!pThis->Owner->IsAlliedWith(pTarget->DisguisedAsHouse))
		{
			if (!pTarget->GetTechnoType()->Insignificant &&
				!pTarget->Owner->Type->MultiplayPassive)
			{
				if (pTarget->WhatAmI() == AbstractType::Building &&
					pTarget->GetWeapon(0)->WeaponType &&
					pTarget->GetThreatValue() > 0)
					return true;

				if (pTarget->WhatAmI() != AbstractType::Building)
					return true;
			}
		}
	}

	return needRepair && inRange;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x4389BD, BombClass_Disarm_Engineer, 0x6)
{
	GET(ObjectClass*, pTarget, EAX);

	for (const auto pTechno : *TechnoClass::Array())
	{
		if (!pTechno || pTechno == pTarget ||
			!pTechno->Owner->IsAlliedWith(pTarget))
			continue;

		auto const index = pTechno->SelectWeapon(pTarget);
		auto const pWeaponType = pTechno->GetWeapon(index) ?
			pTechno->GetWeapon(index)->WeaponType : nullptr;

		if (pWeaponType && pWeaponType->Warhead && pWeaponType->Warhead->BombDisarm)
		{
			auto const pFoot = abstract_cast<FootClass*>(pTechno);

			if (pFoot && (pFoot->Target == pTarget || pFoot->Destination == pTarget))
			{
				if (auto const pTeam = pFoot->Team)
				{
					if (pTeam->Focus == pTarget)
					{
						pTeam->SetFocus(nullptr);
						pTeam->StepCompleted = true;
					}
				}
			}

			if (pTechno->Target == pTarget)
			{
				if (pTechno->Passengers.NumPassengers > 0 &&
					pTechno->GetTechnoType()->OpenTopped)
					pTechno->SetTargetForPassengers(nullptr);

				if (pTechno->SpawnManager)
					pTechno->SpawnManager->ResetTarget();

				pTechno->SetTarget(nullptr);
			}

			if (pFoot && pFoot->Destination == pTarget)
				pFoot->SetDestination(nullptr, true);
		}
	}

	pTarget->AttachedBomb = nullptr;
	pTarget->BombVisible = false;

	return 0x4389C6;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x51B2BD, InfantryClass_UpdateTarget_Enigneer, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0xC, 0x4));
	enum { SkipGameCode = 0x51B33F, Continue = 0x51B2CB };

	if (!pThis || !pTarget ||
		pThis == pTarget ||
		pThis->Owner->ControlledByHuman())
		return SkipGameCode;

	if (pThis->Type->Engineer &&
		pTarget->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = abstract_cast<BuildingClass*>(pTarget);

		if (pThis->Owner->IsAlliedWith(pBuilding) &&
			pBuilding->Health < pBuilding->Type->Strength &&
			!pBuilding->Type->BridgeRepairHut &&
			pBuilding->Type->Repairable)
		{
			pThis->QueueMission(Mission::Capture, false);
			pThis->NextMission();
			pThis->SetDestination(pBuilding, true);
			return SkipGameCode;
		}
	}

	return !pThis->Type->Infiltrate ? SkipGameCode : Continue;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
bool EngineerSkip = false;
DEFINE_HOOK(0x51E462, InfantryClass_MouseOverObject_EnigneerSkip, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET_STACK(ObjectClass*, pObject, STACK_OFFSET(0x38, 0x4));
	enum { SkipGameCode = 0x51E668 };

	auto const pTechno = abstract_cast<TechnoClass*>(pObject);
	EngineerSkip = false;

	if (!pTechno || pThis == pTechno ||
		!pThis->Type->Engineer)
		return 0;

	bool IsForceKeyPressed = InputManagerClass::Instance->IsForceFireKeyPressed() ||
		InputManagerClass::Instance->IsForceSelectKeyPressed() ||
		InputManagerClass::Instance->IsForceMoveKeyPressed();

	EngineerSkip = (IsForceKeyPressed || EngineerAllow(pThis, pTechno, false));
	return  (IsForceKeyPressed || EngineerAllow(pThis, pTechno, true)) ? SkipGameCode : 0;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x51E6B4, InfantryClass_MouseOverObject_EnigneerSkip2, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(Action, pAction, EBP);
	enum { SkipGameCode = 0x51E6D8 };

	if (pThis->Type->Engineer && pAction == Action::Attack)
		return EngineerSkip ? SkipGameCode : 0;

	return 0;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x6F7E39, TechnoClass_CanAutoTarget_Enigneer, 0x5)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	enum { ReturnValue = 0x6F7FDF };

	if (!pTarget)
	{
		R->AL(false);
		return ReturnValue;
	}

	if (pThis && pThis->IsEngineer() &&
		pThis->GetWeapon(0)->WeaponType &&
		pThis->GetWeapon(1)->WeaponType &&
		(!pThis->GetWeapon(0)->WeaponType->Warhead->BombDisarm ||
		!pThis->GetWeapon(1)->WeaponType->Warhead->BombDisarm))
	{
		bool value = EngineerCanTargetObject(pThis, pTarget);

		R->AL(value);
		return ReturnValue;
	}

	return 0;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x70924B, sub_7091D0_EngineerAttack, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	enum { GoNext = 0x709266, SkipGameCode = 0x70927D };

	if (pThis->IsEngineer() && pThis->Owner->ControlledByHuman())
	{
		if (pThis->GetWeapon(0)->WeaponType &&
			pThis->GetWeapon(1)->WeaponType &&
			(!pThis->GetWeapon(0)->WeaponType->Warhead->BombDisarm ||
			!pThis->GetWeapon(1)->WeaponType->Warhead->BombDisarm))
			return GoNext;
		else
			return SkipGameCode;
	}

	return GoNext;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x6F37AF, TecnoClass_SelectWeapon_EngineerAttack, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);

	if (pTarget && pTarget->AbstractFlags & AbstractFlags::Techno)
	{
		if (auto const pWeaponType = pThis->GetWeapon(0)->WeaponType)
		{
			if (pWeaponType && pWeaponType->Warhead && pWeaponType->Warhead->BombDisarm &&
				!abstract_cast<TechnoClass*>(pTarget)->AttachedBomb)
			{
				if (pThis->GetWeapon(1)->WeaponType)
					R->EAX(1);
			}
		}
	}

	return 0;
}
