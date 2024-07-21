#pragma once
#include <format>
#include <iostream>
#include <vector>
#include <random>
#include <numeric>

namespace Utils
{
	template <typename... Args>
	void log(std::string_view format, Args... args);

	void crashGame();

	int randRanged(int min, int max, std::mt19937* random_engine = nullptr); //range : [min, max]

	template <typename Container, typename T>
	T& emplaceBackUnique(Container& container, T&& t);

	template <typename Vector>
	void unorderedVectorErase(Vector& v, typename Vector::iterator it);

	template <typename Iterator>
	Iterator weightedRandomSelect(Iterator begin, Iterator end, const std::vector<int> weights, std::mt19937* random_engine = nullptr);
}

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

	template <typename Container, typename T>
	T& emplaceBackUnique(Container& container, T&& t)
	{
		auto iter = std::find(container.begin(), container.end(), t);
		if (iter == container.end())
		{
			return container.emplace_back(std::forward<T>(t));
		}

		return *iter;
	}

	template <typename Vector>
	void unorderedVectorErase(Vector& v, typename Vector::iterator it)
	{
		*it = std::move(v.back());
		v.pop_back();
	}

	template <typename Iterator>
	Iterator weightedRandomSelect(Iterator begin, Iterator end, const std::vector<int> weights, std::mt19937* random_engine /*= nullptr*/)
	{
		if (weights.size() != std::distance(begin, end) || begin == end)
		{
			__debugbreak();
			return end;
		}

		int weightsSum = std::accumulate(weights.begin(), weights.end(), 0);
		if (weightsSum <= 0)
		{
			return end;
		}

		int rnd = randRanged(0, weightsSum - 1, random_engine);
		for (size_t i = 0; i < weights.size(); ++i)
		{
			if (rnd < weights[i])
			{
				return begin + i;
			}
			
			rnd -= weights[i];
		}

		crashGame(); // Should never get here
	}
}

