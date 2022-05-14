#include <zarks/math/Map.h>

#include <ccut/ccut_framework.h>

#include <random>
#include <iostream>

using namespace zmath;

TEST(test_s2d_indexing)
{
	std::default_random_engine eng;
	std::uniform_int_distribution<int> dist(0, 100);

	Map rand(1000, 1000);
	rand.Apply([&](){
		return dist(eng);
	});

	Indices le50 = rand <= 50;
	Indices gt50 = rand > 50;

	// Create copy of the original to do operations on
	Map cpy = rand;

	// Set lower half of cpy to -1, upper half to 1
	cpy(le50) = -1;
	cpy(gt50) = 1;
	cpy.ApplySample(rand, [](double self, double orig){
		if (orig > 50)
			ASSERT_EQUAL(self, 1);
		else if (orig <= 50)
			ASSERT_EQUAL(self, -1);
		return self;
	});

	// Union of these conditions should index everything
	cpy(gt50 || le50) = 0;
	cpy.Apply([](double self){
		ASSERT_EQUAL(self, 0);
		return self;
	});

	// Test this range
	cpy(rand < 25 || rand > 75) = 1;
	cpy.ApplySample(rand, [](double self, double orig){
		ASSERT_EQUAL((bool)self, (orig < 25 || orig > 75));
		return self;
	});

	// Test &&
	cpy.Clear(0);
	cpy(rand >= 25 && rand <= 75) = 1;
	cpy.ApplySample(rand, [](double self, double orig){
		ASSERT_EQUAL((bool)(self), (orig >= 25 && orig <= 75));
		return self;
	});

	// Test ^
	cpy.Clear(0);
	cpy((rand <= 25) ^ (rand <= 75)) = 1;
	cpy.ApplySample(rand, [](double self, double orig){
		ASSERT_EQUAL((bool)(self), (orig <= 25) != (orig <= 75));
		return self;
	});
}

TEST_MAIN()
