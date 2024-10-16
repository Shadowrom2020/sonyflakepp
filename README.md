# sonyflakepp
Sonyflakepp is a C++ implementation of [Sonyflake](https://github.com/sony/sonyflake) (c) 2015 Sony Corporation

Sonyflake is a distributed unique ID generator inspired by [Twitter's Snowflake](https://blog.twitter.com/2010/announcing-snowflake).  

Sonyflake focuses on lifetime and performance on many host/core environment.
So it has a different bit assignment from Snowflake.
A Sonyflake ID is composed of

    39 bits for time in units of 10 msec
     8 bits for a sequence number
    16 bits for a machine id

As a result, Sonyflake has the following advantages and disadvantages:

- The lifetime (174 years) is longer than that of Snowflake (69 years)
- It can work in more distributed machines (2^16) than Snowflake (2^10)
- It can generate 2^8 IDs per 10 msec at most in a single machine/thread (slower than Snowflake)

## Installation:

Configure, build and install with CMake

## Usage:

Integrate with CMake:

```cmake
project("Sonyflakepp-Example")

find_package(sonyflakepp CONFIG REQUIRED)

add_executable(sonyflakepp-example src/main.cc)
target_link_libraries(sonyflakepp-example sonyflakepp::sonyflakepp)
```

Example usage in c++:

```c++
#include "sonyflake.hpp"
#include <iostream>

int main() {
    // Create Instance with machineID 1
    sonyflakepp::Sonyflake sf(1);

    // Generate Sonyflake ID
    uint64_t id = sf.NextID();
    std::cout << "ID: " << id << std::endl;

    // convert Id to base64
    std::string base64ID = sf.IDToBase64(id);
    std::cout << "Base64 ID: " << base64ID << std::endl;

    return 0;
}
```