#include <gtest/gtest.h>
#include "stable_deque.h"

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
	sd.insert(iter1 + 2, 99);
	sd.erase(sd.begin());

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
