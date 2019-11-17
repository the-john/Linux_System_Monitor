#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iomanip>          // To set the precision of the RAM memory size (and keep the display out clean)
#include <iostream>         // For debugging

#include "linux_parser.h"

// NOTE: The below line is a great way to do some debugging, but seems only useful when used in the main.cpp file
// #define printVariableNameAndValue(x) std::cout << "The name of variable **" << (#x) << "** and the value of variable is =>" << x << "\n"

//using std::string;
//using std::to_string;
//using std::vector;

// DONE: An example of how to read data from the filesystem ... key:value setup
std::string LinuxParser::OperatingSystem() {
  std::string line;
  std::string key;
  std::string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          filestream.close();
          return value;
        }
      }
    }
    filestream.close();
  }
  return value;
}

// DONE: An example of how to read data from the filesystem ... pull a token
std::string LinuxParser::Kernel() {
  std::string os;
  std::string kernel;
  std::string number;
  std::string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel >> number;
    stream.close();
  }
  return number;
}

// BONUS: Update this to use std::filesystem
std::vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
  // .c_str is used to convert a C++ string into a C string (for passing to a function)
  // I think this is to get things to work with ncurses_display (but don't quote me on that)
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      std::string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = std::stoi(filename);
        // NOTE:  Using emplace_back here instead of push_back to enhance efficiency
        pids.emplace_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
// Pull the total memory and the amount of memory that is still free and calculate the percentage of memory being used.
float LinuxParser::MemoryUtilization() { 
  std::string line;
  std::string key;
  std::string value;
  float memTotal = 0.0;
  float memFree = 0.0;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      (linestream >> key >> value);
      if (key == "MemTotal:") {
          memTotal = std::stof(value);
        }
      if (key == "MemFree:") {
        memFree = std::stof(value);
      }
    }
    filestream.close();
    return (memTotal - memFree) / memTotal;
  }
  return 0.0; 
}

// TODO: Read and return the system uptime
// Pull the system uptime (including time spent in suspend)  
// The second value in this file is the amount of time spent in the idle process.
long LinuxParser::UpTime() { 
  std::string line;
  std::string upTime;
  //std::string idleTime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> upTime; // >> idleTime;
    stream.close();
    return std::stol(upTime);
  }
  return 0; 
}

// TODO: Read and return the number of jiffies for the system
// Get the aggrigate CPU Jiffies in the /proc/stat file using the CpuUtilization() function and add them up
// Jiffies = the kernel-internal value.  It is dependent upon the system HZ.
// kUser is the 1st number after "cpu", kSteal is the 8th number after "cpu"; 
// refer to http://man7.org/linux/man-pages/man5/proc.5.html (about 7/10 down the page)
// But remember, the values are stored in a vector called cpuUtilization.  In that vector, the first value is stored at location 0 (not 1 like in the man5)
long LinuxParser::Jiffies() { 
  long jiffy = 0;
  // cpuUtilization is our vector containing all of the values in the first line of /proc/stat
  std::vector<std::string> cpuUtilization = LinuxParser::CpuUtilization();
  for (int i = kUser_; i <= kGuestNice_; i++) {
    jiffy += std::stol(cpuUtilization[i]);
  }
  return jiffy; 
}

// TODO: Read and return the number of active jiffies for a PID
// Get the user mode, kernel mode, user mode with childs, and kernel mode with childs Jiffies 
long LinuxParser::ActiveJiffies(int pid) { 
  std::string line;
  std::string value;
  long pidJiffy = 0;
  int fieldNum = 0;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (fieldNum >= 13 && fieldNum <= 16) {  //***********************************************************************************14 & 17
        pidJiffy += std::stol(value);
      }
    fieldNum++;
    }
    stream.close();
  }
  return pidJiffy; 
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  return LinuxParser::Jiffies() - LinuxParser::IdleJiffies(); 
}

// TODO: Read and return the number of idle jiffies for the system
// Go get the two values kIdle_ and kIOwait_ from /proc/stat using the CpuUtilization function, and add up for total idle Jiffies
// kIdle_ is the 4th number, kIOwait_ is the 5th number; see http://man7.org/linux/man-pages/man5/proc.5.html (about 7/10ths of the way down)
// But because we store these values in a vector called cpuUtilization, thier count starts at 0 (not 1)
long LinuxParser::IdleJiffies() { 
  long idleJiffies = 0;
  std::vector<std::string> cpuUtilization = LinuxParser::CpuUtilization();
  for (int i = kIdle_; i <= kIOwait_; i++) {
    idleJiffies += std::stol(cpuUtilization[i]);
  }
  return idleJiffies; 
}

// TODO: Read and return CPU utilization
// Strip off the first line of aggrigate values from the /proc/stat file
std::vector<std::string> LinuxParser::CpuUtilization() {
  std::string line;
  std::string value;
  std::vector<std::string> cpuNumbers;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (value != "cpu") {             // Don't include the first value "cpu" in the cpuNumbers vector; values are elements 0 through 9
        // NOTE: Using emplace_back here instead of push_back to enhance efficiency
        cpuNumbers.emplace_back(value);
      }
    }
    stream.close();
  }
  return cpuNumbers; 
}

// TODO: Read and return the total number of processes
// Pull the number of process from the /proc/stat file using the "processes" key and return the value
int LinuxParser::TotalProcesses() { 
  std::string line;
  std::string key;
  std::string value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "processes") {
        if (value != "") {
          stream.close();
          return std::stoi(value);
        }
      }
    }
    stream.close();
  }
  
  return 0; // If I put NULL here, I get a warning when compiling (because the return type needs to be int).  Trying to make warning free code.
}

// TODO: Read and return the number of running processes
// Pull the number of processes running from the /proc/stat file using the "procs_running" key and return the value
int LinuxParser::RunningProcesses() { 
  std::string line;
  std::string key;
  std::string value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "procs_running") {  // Why do I need an underline between procs and running in order for this to work properly????????????????????????????
        if (value != "") {
          stream.close();
          return std::stoi(value);
        }
      }
    }
    stream.close();
  }
  return 0; 
}

// TODO: Read and return the command associated with a process
// Go pull the command line information for a particular PID from /proc/pid#/cmdline file
std::string LinuxParser::Command(int pid) { 
  std::string command;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, command);
    stream.close();
    return command;
  }
  return std::string();
}

// TODO: Read and return the memory used by a process
// Pull the value of the memory size from /proc/status and convert it from kB to MB
// VmSize gives the entire size of VIRTUAL memory allocated ... not the amount of physical memory being used.
// Use VmData to get the actual size of physical memory used in RAM
std::string LinuxParser::Ram(int pid) { 
  std::string line;
  std::string key;
  std::string value;
  std::stringstream ram_mem;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;  // the "value" is in kB
      if (key == "VmData:") {
        // we set the float value of VmSize to a fixed precison of one decimal places to keep the display clean
        ram_mem << std::fixed << std::setprecision(1) << (std::stof(value) / 1000);  // will need to include iomanip for setprecision to work
        //ram_mem << std::stof(value) / 1000;
        stream.close();
        return ram_mem.str();  // Force the size of Ram to be a string
      }
    }
    stream.close();
  }
  return "0"; 
}

// TODO: Read and return the user ID associated with a process
// Pull the user ID number from a specific PID and return it
std::string LinuxParser::Uid(int pid) { 
  std::string line;
  std::string key;
  std::string value;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "Uid:") {
        stream.close();
        return value;
      }
    }
    stream.close();
  }
  return std::string(); 
}

// TODO: Read and return the user associated with a process
// Using the User ID for a specific PID, get the user's name
// Use the /etc/passwd/ file to get the name
std::string LinuxParser::User(int pid) { 
  std::string uid = LinuxParser::Uid(pid);
  std::string line;
  std::string key;
  std::string value;
  std::string name;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> name >> value;
      if (value == uid) {
        stream.close();
        return key;
      }
    }
    stream.close();
  }
  return std::string(); 
}

// TODO: Read and return the uptime of a process
// pull the time the process started after system boot (in Jiffies for Linux 2.6 or older, in clock ticks since Linux 2.6) Field 22 ###################
// Reference http://man7.org/linux/man-pages/man5/proc.5.html (about 1/3 of the way down the page)
long LinuxParser::UpTime(int pid) { 
  std::string line;
  std::string value;
  int field = 0;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (field == 21) {  // This should be field 22 versus 21, but TIME+ goes nuts when I do 22 ... why ???????????????????????????????????????????????????????
        stream.close();
        return std::stol(value) / sysconf(_SC_CLK_TCK);  // see definition at http://man7.org/linux/man-pages/man5/proc.5.html (about 1/3 of the way down)
      }
      field++;
    }
    stream.close();
  }
  return 0; 
}