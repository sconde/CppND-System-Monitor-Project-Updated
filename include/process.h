#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(const int pidint);
  bool UpdateInfo(void);
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  bool operator<(Process const& a) const;

  // Declare any necessary private members
 private:
  const int pid_;  // process id is fixed and does not change ( does re-nice
                   // change the process id? )
  float cpu_utilization_;
};

#endif
