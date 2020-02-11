#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(const int pidint);  // NICOL ADDS constructor
  bool UpdateInfo(void);      // NICOL ADDS Populate / Update fields
  int Pid();                               // TODO: See src/process.cpp
  std::string User();                      // TODO: See src/process.cpp
  std::string Command();                   // TODO: See src/process.cpp
  float CpuUtilization();                  // TODO: See src/process.cpp
  std::string Ram();                       // TODO: See src/process.cpp
  long int UpTime();                       // TODO: See src/process.cpp
  bool operator<(Process const& a) const;  // TODO: See src/process.cpp

  // TODO: Declare any necessary private members
 private:
  int pid_;                // NICOL ADDS
  std::string user_;       // NICOL ADDS
  std::string command_;    // NICOL ADDS
  float cpu_utilization_;  // NICOL ADDS
  std::string ram_;        // NICOL ADDS
  long int up_time_;       // NICOL ADDS
};

#endif
