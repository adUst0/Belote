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

	template <typename Container, typename T>
	T& emplace_back_unique(Container& container, T&& t)
	{
		auto iter = std::find(container.begin(), container.end(), t);
		if (iter == container.end())
		{
			return container.emplace_back(std::forward<T>(t));
		}

		return *iter;
	}

	void crashGame();
};

