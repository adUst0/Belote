#include "Utils.h"

void Utils::crashGame()
{
	*(unsigned int*)0 = 0xDEADBEAF;
}

int Utils::randRanged(int min, int max, std::mt19937* random_engine /*= nullptr*/)
{
	static std::mt19937 m_random_engine(std::random_device{}());

	std::uniform_int_distribution<int> dis(min, max);
	return dis(random_engine ? *random_engine : m_random_engine);
}