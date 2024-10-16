#include "sonyflake.hpp"
#include <iostream>
#include <cassert>

void testNextID()
{
    sonyflakepp::Settings settings;
    settings.StartTime = std::chrono::system_clock::now();
    settings.MachineID = 1;

    sonyflakepp::Sonyflake sf(settings);

    auto id1 = sf.NextID();
    auto id2 = sf.NextID();
    assert(id1 != id2);
    std::cout << "testNextID passed" << std::endl;
}

void testHighFrequencyIDGeneration()
{
    sonyflakepp::Settings settings;
    settings.StartTime = std::chrono::system_clock::now();
    settings.MachineID = 1;

    sonyflakepp::Sonyflake sf(settings);
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
    sonyflakepp::Settings settings;
    settings.StartTime = std::chrono::system_clock::now();
    settings.MachineID = 1;

    sonyflakepp::Sonyflake sf(settings);
    auto id = sf.NextID();

    uint64_t timePart = id >> (sonyflakepp::BitLenSequence + sonyflakepp::BitLenMachineID);
    uint64_t seqPart = (id >> sonyflakepp::BitLenMachineID) & ((1 << sonyflakepp::BitLenSequence) - 1);
    uint64_t machinePart = id & ((1 << sonyflakepp::BitLenMachineID) - 1);

    assert(machinePart == settings.MachineID);
    std::cout << "testIDComposition passed" << std::endl;
}

int main()
{
    testNextID();
    testHighFrequencyIDGeneration();
    testIDComposition();
    std::cout << "All tests passed" << std::endl;
    return 0;
}
