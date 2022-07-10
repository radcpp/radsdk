#include <condition_variable>
#include <mutex>
#include <future>
#include <iostream>

// After including the necessary header files, we need three things to communicate between threads:
// 1. An object for the data provided to process or a flag signaling that the condition is indeed satisfied (readFlag)
// 2. A mutex (readyMutex)
// 3. A condition variable (readyCondVar)
bool readyFlag;
std::mutex readyMutex;
std::condition_variable readyCondVar;

void thread1()
{
    // do something thread2 needs as preparation
    std::cout << "<return>" << std::endl;
    std::cin.get();

    // signal that thread1 has prepared a condition
    {
        std::lock_guard<std::mutex> lg(readyMutex);
        readyFlag = true;
    } // release lock
    // Note that the notification itself does not have to be inside the protected area of the lock.
    readyCondVar.notify_one();
}

void thread2()
{
    // wait until thread1 is ready (readyFlag is true)
    {
        std::unique_lock<std::mutex> ul(readyMutex);
        // The effect is that wait() internally calls a loop until the passed callable returns true.
        // Again note that you have to use a unique_lock and can't use a lock_guard here,
        // because internally, wait() explicitly unlocks and locks the mutex
        readyCondVar.wait(ul, [] {return readyFlag; });
    } // release lock

    // do whatever shall happen after thread1 has prepared things
    std::cout << "done" << std::endl;
}

// You can argue that this is a bad example for using condition variable,
// because futures can be used for blocking until some data arrives.
int sample_condvar1()
{
    auto f1 = std::async(std::launch::async, thread1);
    auto f2 = std::async(std::launch::async, thread2);

    return 0;
}
