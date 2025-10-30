#pragma once

#include <CCINIClass.h>
#include <vector>
#include <string>

class ThemeExt
{
public:
	std::string Name;
	bool Repeat;

	int Next;
	int Previous;
	int Side;
	std::vector<int> Sides;
	std::vector<int> Houses;
	std::vector<int> NegHouses;

	ThemeExt(const char* name) :
		Name{ name }
		, Repeat{ false }

		, Next{ -1 }
		, Previous{ -1 }
		, Side{ -1 }
		, Sides{}
		, Houses{}
		, NegHouses{}
	{ }

	~ThemeExt() = default;

	void LoadINI(CCINIClass* pINI);
	static std::vector<std::unique_ptr<ThemeExt>> Array;
};
