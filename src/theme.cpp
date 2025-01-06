#include "theme.h"
#include <Helpers/Macro.h>

#include <CCINIClass.h>
#include <HouseClass.h>
#include <ThemeClass.h>
#include <StringTable.h>

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
std::vector<std::unique_ptr<ThemeExt>> ThemeExt::Array;

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
void ThemeExt::LoadINI(CCINIClass* pINI, const char* pSection)
{
	pINI->ReadString(pSection, "Next", "", this->NextText);
	pINI->ReadString(pSection, "Previous", "", this->PreviousText);
	pINI->ReadString(pSection, "RequiredHouses", "", this->HousesText);
	pINI->ReadString(pSection, "ForbiddenHouses", "", this->NegHousesText);
	this->Side = pINI->ReadSide(pSection, "Side", -1);

	pINI->ReadString(pSection, "Name", "", this->UIName);
	this->Normal = pINI->ReadBool(pSection, "Normal", this->Normal);
	this->Repeat = pINI->ReadBool(pSection, "Repeat", this->Repeat);
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x406FC6, sub_406F70_ThemeClass_AI_Skip, 0x5)
{
	return 0x406FD0;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x406FE2, sub_406F70_ThemeClass_AI, 0xA)
{
	ThemeClass::Instance->AI();
	return 0;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x7206FB, ThemeClass_CreateExt, 0x8)
{
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x38, 0x4));
	GET(char*, pSection, EBP);

	ThemeExt::Array.push_back(std::make_unique<ThemeExt>());
	ThemeExt::Array.at(int(ThemeExt::Array.size() - 1))->LoadINI(pINI, pSection);

	return 0;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x721171, ThemeClass_IsAvailable_Rewrite, 0x6)
{
	GET_STACK(int, index, 0x4);
	enum { ReturnFalse = 0x72117B, ReturnTrue = 0x7211CE };

	auto const ThemeExt = (index >= 0 && int(ThemeExt::Array.size()) > index) ?
		&ThemeExt::Array.at(index) : nullptr;

	if (!ThemeExt)
		return ReturnFalse;

	auto const& pThemeExt = *ThemeExt;

	if (!pThemeExt->Normal)
		return ReturnFalse;

	auto const pPlayer = HouseClass::Player();
	if (!pPlayer)
		return ReturnFalse;

	if (pThemeExt->Side >= 0 && pPlayer->Type->SideIndex != pThemeExt->Side)
		return ReturnFalse;

	if (strcmp(pThemeExt->HousesText, ""))
	{
		char* context = nullptr;
		char readBuffer[2048];
		strcpy(readBuffer, pThemeExt->HousesText);

		bool getHouse = false;
		for (char* cur = strtok_s(readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context))
		{
			if (!strcmp(pPlayer->Type->get_ID(), cur))
			{
				getHouse = true;
				break;
			}
		}

		if (!getHouse)
			return ReturnFalse;
	}
	
	if (strcmp(pThemeExt->NegHousesText, ""))
	{
		char* context = nullptr;
		char readBuffer[2048];
		strcpy(readBuffer, pThemeExt->NegHousesText);

		bool getHouse = false;
		for (char* cur = strtok_s(readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context))
		{
			if (!strcmp(pPlayer->Type->get_ID(), cur))
			{
				getHouse = true;
				break;
			}
		}

		if (getHouse)
			return ReturnFalse;
	}

	return 0;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x7209B0, ThemeClass_GetUIName, 0x7)
{
	GET_STACK(int, index, 0x4);
	enum { ReturnValue = 0x7209C6 };

	auto const ThemeExt = (index >= 0 && int(ThemeExt::Array.size()) > index) ?
		&ThemeExt::Array.at(index) : nullptr;

	if (!ThemeExt)
	{
		R->EAX(L"\0");
		return ReturnValue;
	}

	auto const& pThemeExt = *ThemeExt;
	R->EAX(StringTable::LoadStringA(pThemeExt->UIName));
	return ReturnValue;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
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

		if (pThemeExt && !pThemeExt->Repeat &&
			strcmp(pThemeExt->NextText, ""))
		{
			int next = ThemeClass::Instance->FindIndex(pThemeExt->NextText);

			if (next >= 0 && next != pThis->LastTheme)
				idx = next;
		}
	}

	pThis->Play(idx);
	return SkipGameCode;
}

// 这里不欢迎名为邻座艾莉同学的石灰级玩家
DEFINE_HOOK(0x721086, IStream_LoadGame_sub721040_PlayTheme, 0x5)
{
	GET_STACK(int, ThemeIdx, STACK_OFFSET(0xC, 0x4));

	int idx = ThemeIdx;

	if (idx != ThemeClass::Instance->CurrentTheme)
	{
		if (auto& pThemeExt = ThemeExt::Array.at(idx))
		{
			if (strcmp(pThemeExt->PreviousText, ""))
			{
				int previous = ThemeClass::Instance->FindIndex(pThemeExt->PreviousText);

				if (previous >= 0 && previous != idx)
					idx = previous;
			}
		}
	}

	ThemeClass::Instance->Play(idx);
	return 0x721095;
}
