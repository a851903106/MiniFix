#pragma once

#include <CCINIClass.h>
#include <vector>

class ThemeExt
{
public:
	char NextText[128];
	char PreviousText[128];
	char HousesText[512];
	char NegHousesText[512];

	char UIName[128];
	bool Normal;
	bool Repeat;
	int Side;

	ThemeExt() :
		NextText{}
		, PreviousText{}
		, HousesText{}
		, NegHousesText{}

		, UIName {}
		, Normal { true }
		, Repeat { false }
		, Side { -1 }
	{ }

	~ThemeExt() = default;

	void LoadINI(CCINIClass* pINI, const char* pSection);
	static std::vector<std::unique_ptr<ThemeExt>> Array;
};
