#include <condition_variable>
#include <mutex>
#include <future>
#include <thread>
#include <iostream>
#include <queue>

std::queue<int> queue;
std::mutex queueMutex;
std::condition_variable queueCondVar;

void provider(int val)
{
    // push different values (val til val+5 with timeouts of val milliseconds into the queue)
    for (int i = 0; i < 6; ++i)
    {
        {
            std::lock_guard<std::mutex> lg(queueMutex);
            queue.push(val + i);
        } // release lock

        // Note that the order in which concurrent waiting threads are notified is not defined.
        // In the same way, you can call notify_all() if multiple consumers will have to process
        // the same data. A typical example would be an event-driven system,
        // where an event has to get published to all registered consumers.
        queueCondVar.notify_one();

        std::this_thread::sleep_for(std::chrono::milliseconds(val));
    }
}

void consumer(int num)
{
    // pop values if available (num identifies the consumer)
    while (true)
    {
        int val;
        {
            std::unique_lock<std::mutex> ul(queueMutex);
            queueCondVar.wait(ul, [] {return !queue.empty(); });
            val = queue.front();
            queue.pop();
        } // release lock

        // Note that the output of the two consumers is not synchronized, so characters might interleave.
        std::cout << "consumer " << num << ": " << val << std::endl;
    }
}

int sample_condvar2()
{
    // start three providers for values 100+, 300+ and 500+
    auto f1 = std::async(std::launch::async, provider, 100);
    auto f2 = std::async(std::launch::async, provider, 300);
    auto f3 = std::async(std::launch::async, provider, 500);

    // start two consumers printing the values
    auto c1 = std::async(std::launch::async, consumer, 1);
    auto c2 = std::async(std::launch::async, consumer, 2);

    return 0;
}
