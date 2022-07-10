#include <future>
#include <mutex>
#include <iostream>
#include <string>

std::mutex printMutex;  // enable synchronized output with print()

void print(const std::string& s)
{
    std::lock_guard<std::mutex> lg(printMutex);
    for (char c : s)
    {
        std::cout.put(c);
    }
    std::cout << std::endl;
}

int sample_mutex1()
{
    auto f1 = std::async(std::launch::async, print, "Hello from the first thread");
    auto f2 = std::async(std::launch::async, print, "Hello from the second thread");
    print("Hello from the main thread");

    return 0;
}
