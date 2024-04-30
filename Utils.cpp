#include "Utils.h"

int Utils::randRanged(int min, int max)
{
	static bool first = true;
	if (first)
	{
		srand((unsigned int)(time(nullptr)));
		first = false;
	}
	return min + rand() % ((max + 1) - min);
}

void Utils::crashGame()
{
	*(unsigned int*)0 = 0xDEADBEAF;
}
