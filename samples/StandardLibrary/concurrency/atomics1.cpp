#include <atomic>
#include <future>
#include <thread>
#include <chrono>
#include <iostream>

long data;
std::atomic<bool> readyFlag(false);

void provider()
{
    // after reading a character
    std::cout << "<return>" << std::endl;
    std::cin.get();

    // provide some data
    data = 42;

    // and signal readiness
    readyFlag.store(true);
}

void consumer()
{
    // wait for readiness and do something else
    while (!readyFlag.load())
    {
        std::cout.put('.').flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // and process provided data
    std::cout << "\nvalue: " << data << std::endl;
}

// As a consequence, because the setting of data happens before the provider() stores true in the readyFlag
// and the processing of data happens after the consumer() has loaded true as value of the readyFlag,
// the processing of data is guaranteed to happen after the data was provided.
// This guarantee is provided because in all atomic operations, we use a default memory order named memory_order_seq_cst,
// which stands for sequential consistent memory order.
int sample_atomics1()
{
    // start provider and consumer
    auto p = std::async(std::launch::async, provider);
    auto c = std::async(std::launch::async, consumer);
    return 0;
}