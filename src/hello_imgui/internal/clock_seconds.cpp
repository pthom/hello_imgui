#include "hello_imgui/internal/clock_seconds.h"

#include <chrono>

namespace HelloImGui
{
    namespace Internal
    {

        class ClockSeconds_
        {
            // Typical C++ shamanic incantations to get a time in seconds
        private:
            using Clock = std::chrono::steady_clock;  // use a steady clock, i.e monotonic
            using second = std::chrono::duration<double, std::ratio<1>>;
            std::chrono::time_point<Clock> mStart;

        public:
            ClockSeconds_() : mStart(Clock::now()) {}

            double elapsed() const
            {
                return std::chrono::duration_cast<second>
                    (Clock::now() - mStart).count();
            }
        };

        double ClockSeconds()
        {
            static ClockSeconds_ watch;
            return watch.elapsed();
        }

    }
}
