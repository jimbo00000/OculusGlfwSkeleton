// Timer.h

#ifndef _TIMER_H_
#define _TIMER_H_

#define PRINT_TIMING_INFORMATION

#ifdef _WIN32
// Thank you http://www.mindcontrol.org/~hplus/misc/simple-timer.html
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>

/// Create a Timer, which will immediately begin counting
/// up from 0.0 seconds.
/// You can call reset() to make it start over.
class Timer {
  public:
    Timer() {
      reset();
    }
    /// reset() makes the timer start over counting from 0.0 seconds.
    void reset() {
      unsigned __int64 pf;
      QueryPerformanceFrequency( (LARGE_INTEGER *)&pf );
      freq_ = 1.0 / (double)pf;
      QueryPerformanceCounter( (LARGE_INTEGER *)&baseTime_ );
    }
    /// seconds() returns the number of seconds (to very high resolution)
    /// elapsed since the timer was last created or reset().
    double seconds() const {
      unsigned __int64 val;
      QueryPerformanceCounter( (LARGE_INTEGER *)&val );
      return (val - baseTime_) * freq_;
    }
    /// seconds() returns the number of milliseconds (to very high resolution)
    /// elapsed since the timer was last created or reset().
    double milliseconds() const {
      return seconds() * 1000.0;
    }
  private:
    double freq_;
    unsigned __int64 baseTime_;
};
#endif //_WIN32


#ifdef _LINUX
#include <time.h>

// http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
timespec diff(const timespec& start, const timespec& end);

class Timer {
  public:
    Timer() {
      reset();
    }
    /// reset() makes the timer start over counting from 0.0 seconds.
    void reset() {
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1_);
    }
    /// seconds() returns the number of seconds (to very high resolution)
    /// elapsed since the timer was last created or reset().
    double seconds() const {
      timespec time2;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
      timespec ts = diff(time1_, time2);
      double dt = ts.tv_sec + (1.0/1000000000) * ts.tv_nsec;
      return dt;
    }
    /// seconds() returns the number of milliseconds (to very high resolution)
    /// elapsed since the timer was last created or reset().
    double milliseconds() const {
      return seconds() * 1000.0;
    }
  private:
    timespec time1_;
};
#endif // _LINUX

#endif //_TIMER_H_

