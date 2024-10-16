/**
MIT License

Copyright (c) 2024 Jürgen Herrmann

Copyright (c) 2015 Sony Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef SONYFLAKE_H
#define SONYFLAKE_H

#include <cstdint>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <functional>

namespace sonyflakepp
{

    constexpr int BitLenTime = 39;        ///< Bit length of time
    constexpr int BitLenSequence = 8;     ///< Bit length of sequence number
    constexpr int BitLenMachineID = 63 - BitLenTime - BitLenSequence; ///< Bit length of machine ID
    constexpr int64_t sonyflakeTimeUnit = 10000000; ///< Time unit in nanoseconds (10 milliseconds)

    /**
     * @brief Settings configures the Sonyflake generator.
     */
    struct Settings
    {
        std::chrono::time_point<std::chrono::system_clock> StartTime; ///< Start time for the Sonyflake
        std::function<uint16_t()> MachineID; ///< Function to retrieve the unique machine ID
        std::function<bool(uint16_t)> CheckMachineID; ///< Function to validate the machine ID
    };

    /**
     * @brief Sonyflake is a distributed unique ID generator.
     */
    class Sonyflake
    {
    public:
        /**
         * @brief Constructs a new Sonyflake object with the given settings.
         * @param settings Configuration settings for the Sonyflake generator.
         */
        Sonyflake(const Settings& settings)
        {
            if( settings.StartTime > std::chrono::system_clock::now() )
            {
                throw std::invalid_argument("start time is ahead of now");
            }

            startTime_ = toSonyflakeTime(settings.StartTime);
            elapsedTime_ = 0;
            sequence_ = (1 << BitLenSequence) - 1;

            if( !settings.MachineID )
            {
                throw std::runtime_error("MachineID function is null");
            }

            machineID_ = settings.MachineID();
            if( settings.CheckMachineID && !settings.CheckMachineID(machineID_) )
            {
                throw std::runtime_error("invalid machine id");
            }
        }

        /**
         * @brief Generates the next unique ID.
         * @return The next unique ID.
         */
        uint64_t NextID()
        {
            std::lock_guard<std::mutex> lock(mutex_);

            int64_t current = currentElapsedTime();
            if( elapsedTime_ < current )
            {
                elapsedTime_ = current;
                sequence_ = 0;
            }
            else
            {
                sequence_ = (sequence_ + 1) & ((1 << BitLenSequence) - 1);
                if( sequence_ == 0 )
                {
                    elapsedTime_++;
                    int64_t overtime = elapsedTime_ - current;
                    sleepTime(overtime);
                }
            }
            return toID();
        }

    private:
        std::mutex mutex_; ///< Mutex to ensure thread safety
        int64_t startTime_; ///< Start time in Sonyflake time units
        int64_t elapsedTime_; ///< Elapsed time since the start time
        uint16_t sequence_; ///< Sequence number
        uint16_t machineID_; ///< Machine ID

        /**
         * @brief Gets the current time in nanoseconds.
         * @return The current time in nanoseconds.
         */
        uint64_t currentTime()
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }

        /**
         * @brief Gets the elapsed time since the start time.
         * @return The elapsed time in Sonyflake time units.
         */
        int64_t currentElapsedTime()
        {
            return toSonyflakeTime(std::chrono::system_clock::now()) - startTime_;
        }

        /**
         * @brief Sleeps for the specified overtime duration.
         * @param overtime The overtime duration to sleep for in Sonyflake time units.
         */
        void sleepTime(int64_t overtime)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(overtime * sonyflakeTimeUnit) -
                                        std::chrono::nanoseconds(currentTime() % sonyflakeTimeUnit));
        }

        /**
         * @brief Converts a time point to Sonyflake time units.
         * @param t The time point to convert.
         * @return The time in Sonyflake time units.
         */
        int64_t toSonyflakeTime(const std::chrono::time_point<std::chrono::system_clock>& t)
        {
            return t.time_since_epoch().count() / sonyflakeTimeUnit;
        }

        /**
         * @brief Converts the elapsed time, sequence number, and machine ID to a unique ID.
         * @return The unique ID.
         */
        uint64_t toID() const
        {
            if( elapsedTime_ >= (static_cast<long long>(1) << BitLenTime) )
            {
                throw std::overflow_error("over the time limit");
            }
            return (static_cast<uint64_t>(elapsedTime_) << (BitLenSequence + BitLenMachineID)) |
                (static_cast<uint64_t>(sequence_) << BitLenMachineID) |
                static_cast<uint64_t>(machineID_);
        }
    };

} // namespace sonyflakepp

#endif // SONYFLAKE_H
