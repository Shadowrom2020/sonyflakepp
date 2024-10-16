#include "sonyflake.hpp"
#include <iostream>
#include <cassert>

void testNextID()
{
    sonyflakepp::Settings settings;
    settings.StartTime = std::chrono::system_clock::now();
    settings.MachineID = []()
        {
            return 0;
        };
    settings.CheckMachineID = [](uint16_t)
        {
            return true;
        };

    sonyflakepp::Sonyflake sf(settings);
    auto id1 = sf.NextID();
    auto id2 = sf.NextID();
    assert(id1 != id2);
    std::cout << "testNextID passed" << std::endl;
}

int main()
{
    testNextID();
    std::cout << "All tests passed" << std::endl;
    return 0;
}
