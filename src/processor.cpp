#include "processor.h"
#include "linux_parser.h"
#include <unistd.h>         // So I can insert a fixed waiting time
#include <iostream>         // for debugging

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { 
    
    totalJiffyStart = LinuxParser::Jiffies();
    activeJiffyStart = LinuxParser::ActiveJiffies();
    
    // Now pause for a short moment to give the system time to slightly change
    usleep(100000); // waite for 100,000 microseconds (must be less than 1M); 100,000 us = 100 ms = 0.1 sec

    totalJiffyStop = LinuxParser::Jiffies();
    activeJiffyStop = LinuxParser::ActiveJiffies();
    
    long totalTime = totalJiffyStop - totalJiffyStart;
    long activeTime = activeJiffyStop - activeJiffyStart;

    if (totalTime == 0) {  // just in case usleep() fails
        return 0.0;
    }
    
    return float(activeTime) / float(totalTime);
}