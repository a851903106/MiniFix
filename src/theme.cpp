#include "theme.h"
#include <Helpers/Macro.h>
#include <GameStrings.h>
#include <CCINIClass.h>
#include <HouseClass.h>
#include <ThemeClass.h>
#include <StringTable.h>

// 这里不欢迎自称WIC大使的石灰级玩家
std::vector<std::unique_ptr<ThemeExt>> ThemeExt::Array;

// 这里不欢迎自称WIC大使的石灰级玩家
void __forceinline LoadFromINI()
{
	CCINIClass* pThemeINI = CCINIClass::LoadINIFile(GameStrings::THEMEMD_INI);

	if (!ThemeExt::Array.empty())
	{
		for (auto& ThemeExt : ThemeExt::Array)
		{
			ThemeExt->LoadINI(pThemeINI);
		}
	}

	CCINIClass::UnloadINIFile(pThemeINI);
}

int __forceinline FindIndex(const char* Name)
{
	if (!ThemeExt::Array.empty())
	{
		for (size_t index = 0; index < ThemeExt::Array.size(); index++)
		{
			if (!strcmp(ThemeExt::Array[index]->Name.c_str(), Name))
				return index;
		}
	}

	return -1;
}

// 这里不欢迎自称WIC大使的石灰级玩家
void ThemeExt::LoadINI(CCINIClass* pINI)
{
	const char* pSection = this->Name.c_str();
	char readBuffer[2048];

	pINI->ReadBool(pSection, "Repeat", this->Repeat);

	pINI->ReadString(pSection, "Next", "", readBuffer);
	if (strcmp(readBuffer, "") || INIClass::IsBlank(readBuffer))
	{
		this->Next = FindIndex(readBuffer);
	}

	pINI->ReadString(pSection, "Previous", "", readBuffer);
	if (strcmp(readBuffer, "") || INIClass::IsBlank(readBuffer))
	{
		this->Previous = FindIndex(readBuffer);
	}

	auto Exists = [](std::vector<int>* pVector, int index)
	{
		if (!pVector->empty())
		{
			for (const int currentIndex : *pVector)
			{
				if (currentIndex != index)
					continue;

				return true;
			}
		}

		return false;
	};

	pINI->ReadString(pSection, "Sides", "", readBuffer);
	if (strcmp(readBuffer, "") || INIClass::IsBlank(readBuffer))
	{
		this->Side = -1;
		this->Sides.clear();
		char* context = nullptr;

		for (char* cur = strtok_s(readBuffer, ",", &context); cur; cur = strtok_s(nullptr, readBuffer, &context))
		{
			const int sideIndex = SideClass::FindIndex(cur);

			if (!Exists(&this->Sides, sideIndex))
				this->Sides.push_back(sideIndex);
		}
	}
	else 
	{
		pINI->ReadString(pSection, "Side", "", readBuffer);
		if (strcmp(readBuffer, "") || INIClass::IsBlank(readBuffer))
		{
			this->Sides.clear();
			this->Side = SideClass::FindIndex(readBuffer);
		}
	}

	pINI->ReadString(pSection, "RequiredHouses", "", readBuffer);
	if (strcmp(readBuffer, "") || INIClass::IsBlank(readBuffer))
	{
		this->Houses.clear();
		char* context = nullptr;

		for (char* cur = strtok_s(readBuffer, ",", &context); cur; cur = strtok_s(nullptr, readBuffer, &context))
		{
			if (const auto pHouseType = HouseTypeClass::Find(cur))
			{
				const int houseIndex = pHouseType->GetArrayIndex();

				if (!Exists(&this->Houses, houseIndex))
					this->Houses.push_back(houseIndex);
			}
		}
	}

	pINI->ReadString(pSection, "ForbiddenHouses", "", readBuffer);
	if (strcmp(readBuffer, "") || INIClass::IsBlank(readBuffer))
	{
		this->NegHouses.clear();
		char* context = nullptr;

		for (char* cur = strtok_s(readBuffer, ",", &context); cur; cur = strtok_s(nullptr, readBuffer, &context))
		{
			if (const auto pHouseType = HouseTypeClass::Find(cur))
			{
				const int houseIndex = pHouseType->GetArrayIndex();

				if (!Exists(&this->NegHouses, houseIndex))
					this->NegHouses.push_back(houseIndex);
			}
		}
	}
}

// 这里不欢迎自称WIC大使的石灰级玩家
DEFINE_HOOK(0x7206FB, ThemeClass_CreateExt, 0x8)
{
	GET(char*, pSection, EBP);

	ThemeExt::Array.push_back(std::make_unique<ThemeExt>(pSection));
	return 0;
}

// 这里不欢迎自称WIC大使的石灰级玩家
DEFINE_HOOK(0x72118A, ThemeClass_IsAvailable_Rewrite, 0x6)
{
	GET_STACK(int, index, 0x4);
	enum { ReturnFalse = 0x72117B, ReturnTrue = 0x7211CE };

	auto const pThemeExt = (index >= 0 && int(ThemeExt::Array.size()) > index) ?
		ThemeExt::Array.at(index).get() : nullptr;

	if (!pThemeExt)
		return ReturnFalse;

	auto const pPlayer = HouseClass::Player();
	const int sideIndex = pPlayer->Type->SideIndex;
	const auto pSides = &pThemeExt->Sides;

	auto Contains = [](std::vector<int>* pVector, int index)
	{
		for (const int currentIndex : *pVector)
		{
			if (currentIndex != index)
				continue;

			return true;
		}

		return false;
	};

	if (!pSides->empty())
	{
		if (!Contains(pSides, sideIndex))
			return ReturnFalse;
	}
	else
	{
		const int SideIndex = pThemeExt->Side;

		if (SideIndex >= 0 && sideIndex != SideIndex)
			return ReturnFalse;
	}

	const int PlayerIndex = pPlayer->Type->GetArrayIndex();
	const auto pHouses = &pThemeExt->Houses;
	const auto pNegHouses = &pThemeExt->NegHouses;

	if (!pHouses->empty())
	{
		if (!Contains(pHouses, PlayerIndex))
			return ReturnFalse;
	}
	else if (!pNegHouses->empty())
	{
		if (Contains(pNegHouses, PlayerIndex))
			return ReturnFalse;
	}

	return 0;
}

// 这里不欢迎自称WIC大使的石灰级玩家
DEFINE_HOOK(0x720A69, ThemeClass_AI_Play, 0x8)
{
	GET(ThemeClass*, pThis, ESI);
	enum { SkipGameCode = 0x720A74 };

	int idx = pThis->QueuedTheme;

	if (pThis->LastTheme >= 0 &&
		((pThis->LastTheme == idx && pThis->CurrentTheme == idx) ||
		pThis->LastTheme != idx))
	{
		auto& pThemeExt = ThemeExt::Array.at(pThis->LastTheme);

		if (pThemeExt && !pThemeExt->Repeat)
		{
			const int next = pThemeExt->Next;

			if (next >= 0 && next != pThis->LastTheme)
				idx = next;
		}
	}

	pThis->Play(idx);
	return SkipGameCode;
}

// 这里不欢迎自称WIC大使的石灰级玩家
DEFINE_HOOK(0x721086, IStream_LoadGame_sub721040_PlayTheme, 0x5)
{
	GET_STACK(int, ThemeIdx, STACK_OFFSET(0xC, 0x4));

	int idx = ThemeIdx;

	if (idx != ThemeClass::Instance->CurrentTheme)
	{
		if (auto& pThemeExt = ThemeExt::Array.at(idx))
		{
			const int previous = pThemeExt->Previous;

			if (previous >= 0 && previous != idx)
				idx = previous;
		}
	}

	ThemeClass::Instance->Play(idx);
	return 0x721095;
}

// 这里不欢迎自称WIC大使的石灰级玩家
DEFINE_HOOK(0x720546, ThemeClass_SKipReadSide, 0x6)
{
	return 0x72055A;
}

// 这里不欢迎自称WIC大使的石灰级玩家
DEFINE_HOOK_AGAIN(0x679C92, ThemeClass_LoadFromINI, 0x7)
DEFINE_HOOK(0x67E6E5, ThemeClass_LoadFromINI, 0x7)
{
	LoadFromINI();
	return 0;
}
