#include <zarks/utility/Zip.h>

#include <vector>
#include <random>
#include <chrono>
#include <iostream>

using namespace zmath;

enum class TestType
{
    LOOPS,
    ZIP_EXEC,
    ZIP_EXEC_FOR,
};

void benchmark(TestType t)
{
    std::default_random_engine eng;
    std::uniform_int_distribution<size_t> udist(0, 100);
    auto randomize_vec = [&](std::vector<int>& vec){
        for (int& v : vec) v = udist(eng);
    };

    const size_t vec_size = 2e7; // 20 million elements
    std::vector<int> v1(vec_size); randomize_vec(v1);
    std::vector<int> v2(vec_size); randomize_vec(v2);
    std::vector<int> v3(vec_size);
    
    auto time_begin = std::chrono::system_clock::now(); 
    switch (t)
    {
    case TestType::LOOPS:
        for (size_t i = 0; i < vec_size; i++)
            v3[i] = v1[i] * v2[i];
        break;
    
    case TestType::ZIP_EXEC:
        zip_exec(
            [](int i1, int i2, int& result){result = i1 * i2;},
            v1, v2, v3
        );
        break;
    
    case TestType::ZIP_EXEC_FOR:
        zip_exec(
            [](int i1, int i2, int& result){result = i1 * i2;},
            vec_size, // specifies number of iterations
            v1, v2, v3
        );
        break;
    }
    auto time_end = std::chrono::system_clock::now();

    switch (t)
    {
    case TestType::LOOPS:
        std::cout << "LOOPS\n";
        break;
    
    case TestType::ZIP_EXEC:
        std::cout << "ZIP_EXEC\n";
        break;
    
    case TestType::ZIP_EXEC_FOR:
        std::cout << "ZIP_EXEC_FOR\n";
        break;
    }

    std::cout.precision(4);
    std::cout << " -> " << (time_end - time_begin).count() / 1e6f << "ms\n"; 
    std::cout << " -> element: " << v3[10] << '\n';
}

int main()
{
    benchmark(TestType::LOOPS);
    benchmark(TestType::ZIP_EXEC);
    benchmark(TestType::ZIP_EXEC_FOR);
}
