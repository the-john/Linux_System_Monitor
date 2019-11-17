#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  int Pid();                         
  std::string User();                  
  std::string Command();               
  float CpuUtilization();               
  std::string Ram();                    
  long int UpTime();                      
  bool operator<(Process const& a) const;  
  Process(int pid);

  // TODO: Declare any necessary private members
 private:
  int pid_;
};

#endif