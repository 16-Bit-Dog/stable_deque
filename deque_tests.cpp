#include <gtest/gtest.h>
#include "stable_deque.h"
#include "old_stable_deque.h"
#include <boost/container/stable_vector.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <array>
#include <typeinfo>

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
	std::cout << "(n = " << std::to_string(count) << ") " << __FUNCTION__ << "_profile<" << type_prompt << ">(...): " << elapsed.count() << '\n';

template<typename T, typename Container>
void push_front_profile(std::string type_prompt)
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
void push_back_profile(std::string type_prompt)
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
void erase_front_profile(std::string type_prompt)
{
	PREAMBLE(5000)
	for (auto i = 0; i < count; i++)
	{
		container.push_back(magicData);
	}

	START_PROFILE()
	for (auto i = 0; i < count; i++)
	{
		container.erase(container.begin());
	}
	END_PROFILE()
}

template<typename T, typename Container>
void erase_back_profile(std::string type_prompt)
{
	// Skip deque<T> since `end()-1` hits an assert (a MSVC bug?)
	if constexpr (std::same_as<std::deque<T>, Container>)
		return;

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


#define str(chars) std::string(chars)
TEST(StableDequeTest, Perf)
{
#define PROFILE_FUNC(func_name, T) \
	std::cout << "\n" << #func_name << "<" << typeid(T).name() << ">:\n"; \
	func_name<T, std::deque<T>>(str("deque<")+str(typeid(T).name())+">"); \
	func_name<T, stable_deque<T>>(str("stable_deque<")+str(typeid(T).name())+">"); \
	func_name<T, old_stable_deque<T>>(str("old_stable_deque<")+str(typeid(T).name())+">"); \
	func_name<T, std::vector<T>>("vector<"+str(typeid(T).name())+">"); \
	func_name<T, stable_vector<T>>(str("stable_vector<")+str(typeid(T).name())+">"); 

	PROFILE_FUNC(push_front_profile, int);
	PROFILE_FUNC(push_front_profile, BigData);

	PROFILE_FUNC(push_back_profile, int);
	PROFILE_FUNC(push_back_profile, BigData);

	PROFILE_FUNC(erase_front_profile, int);
	PROFILE_FUNC(erase_front_profile, BigData);

	PROFILE_FUNC(erase_back_profile, int);
	PROFILE_FUNC(erase_back_profile, BigData);
}