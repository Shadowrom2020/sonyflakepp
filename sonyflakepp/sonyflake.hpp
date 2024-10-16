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
        uint16_t MachineID; ///< Unique machine ID
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
            machineID_ = settings.MachineID;
        }

        /**
         * @brief Generates the next unique ID.
         * @return The next unique ID.
         */
        uint64_t NextID()
        {
            std::lock_guard<std::mutex> lock(mutex_);

            int64_t current = (std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()
                ).count() / sonyflakeTimeUnit) - startTime_;

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
                    std::this_thread::sleep_for(std::chrono::nanoseconds(overtime * sonyflakeTimeUnit) -
                                                std::chrono::nanoseconds(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                                std::chrono::system_clock::now().time_since_epoch()
                    ).count() % sonyflakeTimeUnit));
                }
            }
            if( elapsedTime_ >= (static_cast<uint64_t>(1) << BitLenTime) )
            {
                throw std::overflow_error("over the time limit");
            }
            return (static_cast<uint64_t>(elapsedTime_) << (BitLenSequence + BitLenMachineID)) |
                (static_cast<uint64_t>(sequence_) << BitLenMachineID) |
                static_cast<uint64_t>(machineID_);
        }

    private:
        std::mutex mutex_; ///< Mutex to ensure thread safety
        int64_t startTime_; ///< Start time in Sonyflake time units
        int64_t elapsedTime_; ///< Elapsed time since the start time
        uint16_t sequence_; ///< Sequence number
        uint16_t machineID_; ///< Machine ID

        /**
         * @brief Converts a time point to Sonyflake time units.
         * @param t The time point to convert.
         * @return The time in Sonyflake time units.
         */
        static int64_t toSonyflakeTime(const std::chrono::time_point<std::chrono::system_clock>& t)
        {
            return t.time_since_epoch().count() / sonyflakeTimeUnit;
        }
    };

} // namespace sonyflakepp

#endif // SONYFLAKE_H
