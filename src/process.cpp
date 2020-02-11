#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;


Process::Process(const int pidint) : pid_(pidint) {
  user_ = "";
  command_ = "";
  ram_ = "";
  up_time_ = 0;
  cpu_utilization_ = 0.0;
};

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() { return cpu_utilization_; }

// TODO: Return the command that generated this process
string Process::Command() { return command_; }

// TODO: Return this process's memory utilization
string Process::Ram() { return ram_; }

// TODO: Return the user (name) that generated this process
string Process::User() { return user_; }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return up_time_; }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
  return (cpu_utilization_ > a.cpu_utilization_ ? true : false);
}

bool Process::UpdateInfo(void) {
  bool validUpdate = false;
  // if we have a valid PID, then let's update the private fields
  if (pid_ > 0) {
    float processUpTime = (float)LinuxParser::UpTime(pid_);

    // concerned about divide by zero ...
    if ((processUpTime) < 0.00000000000001) {
      cpu_utilization_ = 0.0;
    } else {
      cpu_utilization_ =
          ((float)LinuxParser::ActiveJiffies(pid_) / LinuxParser::ClkTPS()) /
          processUpTime;
    }

    // should never happen that cpu_utilization_ > 100.0 ... but in case...
    if (cpu_utilization_ > 100.0) cpu_utilization_ = 100.0;

    command_ = LinuxParser::Command(pid_);
    ram_ = LinuxParser::Ram(pid_);
    user_ = LinuxParser::User(pid_);
    up_time_ = LinuxParser::UpTime(pid_);

    validUpdate = true;
  };
  return validUpdate;
};
