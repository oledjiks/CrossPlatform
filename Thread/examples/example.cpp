#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>

#include "../ThreadPool.h"

int main()
{

    ThreadPool pool(4);
    std::vector< std::future<int> > results;

    for (int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] () {
                    std::stringstream ss;
                    // std::cout << "hello " << i << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    // std::cout << "world " << i << std::endl;
                    ss << "hello world " << i << std::endl;
                    std::cout << ss.str();
                    return i*i;
                })
            );
    }

    for (auto && result : results)
        std::cout << "result = " << result.get() << ' ';
    std::cout << std::endl;

    return 0;
}
