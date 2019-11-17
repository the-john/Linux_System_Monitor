#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>  // For debugging

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Need a constructor or this won't build
Process::Process(int pid) {
    Process::pid_ = pid;
}

// TODO: Return this process's ID
int Process::Pid() { 
    return pid_; 
}

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() { 
    long activeTime = LinuxParser::ActiveJiffies(pid_);
    long startTime = LinuxParser::UpTime(pid_);
    long upTime = LinuxParser::UpTime();
    long seconds = upTime - (startTime / sysconf(_SC_CLK_TCK));

    // NOTE:  must turn type long (two of them) into float; (return long/long to a type float will be zero!!!)
    return ((float)activeTime / sysconf(_SC_CLK_TCK)) / (float)seconds; 
}

// TODO: Return the command that generated this process
string Process::Command() { 
    return LinuxParser::Command(pid_); 
}

// TODO: Return this process's memory utilization
string Process::Ram() { 
    return LinuxParser::Ram(pid_); 
}

// TODO: Return the user (name) that generated this process
string Process::User() { 
    return LinuxParser::User(pid_);
}

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { 
    return LinuxParser::UpTime(pid_); 
}

// TODO: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { 
    return std::stol(LinuxParser::Ram(pid_)) > std::stol(LinuxParser::Ram(a.pid_)); 
}