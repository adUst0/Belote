#pragma once
#include <format>
#include <iostream>

namespace Utils
{
	template <typename... Args>
	void log(std::string_view format, Args... args)
	{
#ifndef DEBUG
		return;
#endif
		static std::string lastLog;

		std::string currentLog = std::format(format, std::forward<Args>(args)...);
		if (lastLog == currentLog)
		{
			return;
		}

		std::cout << currentLog;
		lastLog = std::move(currentLog);
	}

	int randRanged(int min, int max); //range : [min, max]
};

