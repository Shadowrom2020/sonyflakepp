#include "sonyflake.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <unordered_set>
#include <mutex>

void testNextID()
{
    sonyflakepp::Sonyflake sf(1);

    auto id1 = sf.NextID();
    auto id2 = sf.NextID();
    assert(id1 != id2);
    std::cout << "testNextID passed" << std::endl;
}

void testHighFrequencyIDGeneration()
{
    sonyflakepp::Sonyflake sf(1);
    for( int i = 0; i < 10000; ++i )
    {
        auto id1 = sf.NextID();
        auto id2 = sf.NextID();
        assert(id1 != id2);
    }
    std::cout << "testHighFrequencyIDGeneration passed" << std::endl;
}

void testIDComposition()
{
    sonyflakepp::Sonyflake sf(1);
    auto id = sf.NextID();

    uint64_t timePart = id >> (sonyflakepp::BitLenSequence + sonyflakepp::BitLenMachineID);
    uint64_t seqPart = (id >> sonyflakepp::BitLenMachineID) & ((1 << sonyflakepp::BitLenSequence) - 1);
    uint64_t machinePart = id & ((1 << sonyflakepp::BitLenMachineID) - 1);

    assert(machinePart == 1);
    std::cout << "testIDComposition passed" << std::endl;
}

void testIDToBase64()
{
    sonyflakepp::Sonyflake sf(1);

    auto id = sf.NextID();
    std::string base64ID = sf.IDToBase64(id);

    // Check if Base64 string is not empty
    assert(!base64ID.empty());

    // Check if Base64 string contains only valid Base64 characters
    for( char c : base64ID )
    {
        assert((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '/' || c == '=');
    }

    std::cout << "testIDToBase64 passed" << std::endl;
}

void testThreadSafety()
{
    sonyflakepp::Sonyflake sf(1);

    std::unordered_set<uint64_t> ids;
    std::mutex mutex;

    auto generateIDs = [&sf, &ids, &mutex]()
        {
            for( int i = 0; i < 1000; ++i )
            {
                uint64_t id = sf.NextID();
                std::lock_guard<std::mutex> lock(mutex);
                assert(ids.find(id) == ids.end()); // Check for uniqueness
                ids.insert(id);
            }
        };

    std::thread t1(generateIDs);
    std::thread t2(generateIDs);
    std::thread t3(generateIDs);
    std::thread t4(generateIDs);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    std::cout << "testThreadSafety passed" << std::endl;
}

int main()
{
    testNextID();
    testHighFrequencyIDGeneration();
    testIDComposition();
    testIDToBase64();
    testThreadSafety();
    std::cout << "All tests passed" << std::endl;
    return 0;
}
