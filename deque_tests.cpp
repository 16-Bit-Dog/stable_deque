#include "stable_deque.h"
#include "vector_stable_deque.h"

#include <boost/container/stable_vector.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <array>
#include <chrono>
#include <deque>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

using namespace boost::container;
using namespace std::chrono;
using namespace boost;

// Ensure gtest works
TEST(StableDequeTest, GTest)
{
	EXPECT_EQ(7 * 6, 42);
}

TEST(StableDequeTest, Smoke)
{
	stable_deque<int> sd;
	sd.push_front(1);
	auto iter1 = sd.begin();
	sd.push_back(3);
	auto iter3 = sd.end()-1;
	sd.insert(iter3, 2);
	auto iter2 = iter1+1;

	// Try to break the iterators
	sd.push_front(99);
	sd.push_front(99);
	sd.push_back(99);
	sd.push_back(99);
	sd.push_back(99);
	sd.insert(iter1 + 2, 99);
	sd.erase(sd.begin());
	sd.erase(sd.end()-1);

	EXPECT_EQ(*iter1, 1);
	EXPECT_EQ(*iter2, 2);
	EXPECT_EQ(*iter3, 3);
	EXPECT_EQ(sd.size(), 7);

	for (int i = 0; i < sd.size(); i++)
	{
		switch (i)
		{
		case 1:
			EXPECT_EQ(sd[i], 1);
			break;
		case 2:
			EXPECT_EQ(sd[i], 2);
			break;
		case 4:
			EXPECT_EQ(sd[i], 3);
			break;
		default:
			EXPECT_EQ(sd[i], 99);
			break;
		}
	}

	EXPECT_EQ(iter1[2], 99);
	EXPECT_EQ(iter1[3], 3);

	EXPECT_GT(iter3, iter2);
	EXPECT_GT(iter3, iter1);

	EXPECT_LT(iter1, iter3);
	EXPECT_LT(iter1, iter2);
}

#define PREAMBLE(N) \
	std::ifstream stream("magic_data.txt"); \
	char firstChar{}; \
	stream.read(&firstChar, 1); \
	std::string firstCharString{}; \
	firstCharString += firstChar; \
	T magicData = T(std::stoi(firstCharString)); \
	Container container{}; \
	constexpr std::size_t count = N;

#define START_PROFILE() \
	auto start = std::chrono::high_resolution_clock::now(); 


#define END_PROFILE() \
	auto end = std::chrono::high_resolution_clock::now(); \
	auto elapsed = end - start; \
	std::cout << "(n = " << std::to_string(count) << ") " << __FUNCTION__ << "_profile<" << type_prompt << ">(...): " << elapsed.count() << '\n'; \
	return (int64_t)elapsed.count();

template<typename T, typename Container>
int64_t push_front_profile(std::string type_prompt)
{
	PREAMBLE(5000)
	START_PROFILE()
	for (auto i = 0; i < count; i++)
	{
		container.insert(container.begin(), magicData);
	}
	END_PROFILE()
}

template<typename T, typename Container>
int64_t push_back_profile(std::string type_prompt)
{
	PREAMBLE(5000)
	START_PROFILE()
	for (auto i = 0; i < count; i++)
	{
		container.push_back(magicData);
	}
	END_PROFILE()
}

template<typename T, typename Container>
int64_t erase_front_profile(std::string type_prompt)
{
	PREAMBLE(5000)
	for (auto i = 0; i < count; i++)
	{
		container.insert(container.begin(), magicData);
	}

	START_PROFILE()
	for (auto i = 0; i < count; i++)
	{
		container.erase(container.begin());
	}
	END_PROFILE()
}

template<typename T, typename Container>
int64_t erase_back_profile(std::string type_prompt)
{
	// Skip deque<T> since `end()-1` hits an assert (a MSVC bug?)
	if constexpr (std::same_as<std::deque<T>, Container>)
		return 0;

	PREAMBLE(5000)
	for (auto i = 0; i < count; i++)
	{
		container.push_back(magicData);
	}

	START_PROFILE()
	for (int64_t i = count-1; i >= 0; i--)
	{
		container.erase(container.end() - 1);
	}
	END_PROFILE()
}

struct BigData
{
	constexpr static std::size_t size = 512;
	std::array<int, size> data{};
	BigData(int val)
	{
		std::fill(data.begin(), data.end(), val);
	}
};

struct ASCIIBarChartGenerator
{
private:
	std::vector<std::pair<std::string, int64_t>> title_and_val{};
	int64_t step_count = 100;
public:
	ASCIIBarChartGenerator()
	{
	}
	ASCIIBarChartGenerator(std::string title, int64_t timepoint)
	{
		title_and_val.push_back({ title, timepoint });
	}
	ASCIIBarChartGenerator& operator()(std::string title, int64_t timepoint)
	{
		title_and_val.push_back({ title, timepoint });
		return *this;
	}
	void emitChart(std::string title)
	{
		if (title_and_val.size() == 0)
			return;

		int64_t longest_name = -1;
		int64_t max_val = -1;
		for (auto i : title_and_val)
		{
			if (max_val < i.second)
				max_val = i.second;
			if (longest_name < (int64_t)i.first.size())
				longest_name = (int64_t)i.first.size();
		}
		
		std::cout << "----[" << title << "]----\n";
		
		auto dstep_count = (double)step_count;
		// Logic to draw the chart
		for (const auto& item : title_and_val)
		{
			// 1. Print the label, padded to the longest name
			std::cout << std::setw(longest_name) << std::right << item.first << " | ";

			// 2. Calculate bar length relative to max_val and step_count
			int64_t bar_length = 0;
			if (max_val > 0) {
				// Cast to double for precision, then back to int
				bar_length = (int64_t)(((double)item.second / (double)max_val) * dstep_count);
			}

			// 3. Draw the bar
			for (int k = 0; k < bar_length; ++k) {
				std::cout << "#";
			}

			// 4. Print the actual numeric value for clarity
			std::cout << " (" << item.second << ")\n";
		}

		std::cout << "-----" << title << "-----\n\n\n";
	}
};

#define str(chars) std::string(chars)
TEST(StableDequeTest, Perf)
{
	// Note: removed `vector_stable_deque` because it is such a large diff. in performance that it wreaks the graphs scaling
#define PROFILE_FUNC(func_name, T) \
	{ \
		std::cout << "\n" << #func_name << "<" << typeid(T).name() << ">:\n"; \
		std::string TName = str(typeid(T).name()); \
		std::string deque_name = str("deque<") + TName + ">"; \
		std::string stable_deque_name = str("stable_deque<") + TName + ">"; \
		std::string stable_vector_name = str("stable_vector<") + TName + ">"; \
		std::string vector_name = str("vector<") + TName + ">"; \
		ASCIIBarChartGenerator() \
		(deque_name, func_name<T, std::deque<T>>(deque_name)) \
		(stable_deque_name, func_name<T, stable_deque<T>>(stable_deque_name)) \
		(stable_vector_name, func_name<T, stable_vector<T>>(stable_vector_name)) \
		(vector_name, func_name<T, std::vector<T>>(vector_name)) \
		.emitChart(#func_name); \
	}

	PROFILE_FUNC(push_front_profile, int);
	PROFILE_FUNC(push_front_profile, BigData);

	PROFILE_FUNC(push_back_profile, int);
	PROFILE_FUNC(push_back_profile, BigData);

	PROFILE_FUNC(erase_front_profile, int);
	PROFILE_FUNC(erase_front_profile, BigData);

	PROFILE_FUNC(erase_back_profile, int);
	PROFILE_FUNC(erase_back_profile, BigData);
}