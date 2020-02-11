#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::ifstream;
using std::istringstream;

vector<string> Line2StrVector(string filename) {
  vector<string> vstr;
  string line;
  string value;
  ifstream filestream(filename);
  if (filestream.is_open()) {
    getline(filestream, line);

    // FOR TESTING:
    // string line{ "968 (Xorg) S 966 966 966 1025 966 4194560 1343231 83084 200
    // 5 86972 46384 269 45 20 0 4 0 2355 824307712 39867 18446744073709551615
    // 94218370744320 94218373105768 140721572940992 0 0 0 0 4096 1098933999 0 0
    // 0 17 0 0 0 157 0 0 94218375206576 94218375263552 94218382950400
    // 140721572945090 140721572945208 140721572945208 140721572945893 0"};
    // #14 => 86972
    // #15 => 46384
    // #16 => 269
    // #17 => 45
    // #22 => 2355

    istringstream linestream(line);
    while (linestream >> value) {
      vstr.push_back(value);
    }
  }
  filestream.close();
  return vstr;
}

int LinuxParser::ClkTPS(void) { return sysconf(_SC_CLK_TCK); }

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
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
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  auto total_memory = (float)LinuxParser::ReadProcMemInfo("MemTotal");
  auto avail_memory = (float)LinuxParser::ReadProcMemInfo("MemAvailable");
  return ( total_memory - avail_memory ) / total_memory;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  string str_value;
  long value = 0;
  ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    istringstream linestream(line);
    linestream >> str_value;
    value = stol(str_value);
  }
  // no error handling happening...
  return value;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return (ActiveJiffies() - IdleJiffies()); }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string filename = kProcDirectory + to_string(pid) + kStatFilename;
  vector<string> vstr = Line2StrVector(filename);

  if (vstr.size() >= 21) {
    return (stol(vstr[13]) + stol(vstr[14]) + stol(vstr[15]) + stol(vstr[16]));
  } else {
    // Fails quietly
    return 0;
  }
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpu_stats = LinuxParser::CpuUtilization();

  // normal processes executing in user mode
  long user = stol(cpu_stats[LinuxParser::kUser_]);

  // niced processes executing in user mode
  long nice = stol(cpu_stats[LinuxParser::kNice_]);

  // processes executing in kernel mode
  long system = stol(cpu_stats[LinuxParser::kSystem_]);

  // twiddling thumbs
  long idle = stol(cpu_stats[LinuxParser::kIdle_]);

  // waiting for I/O to complete
  long iowait = stol(cpu_stats[LinuxParser::kIOwait_]);

  // servicing interrupts
  long irq = stol(cpu_stats[LinuxParser::kIRQ_]);

  // servicing softirqs
  long softirq = stol(cpu_stats[LinuxParser::kSoftIRQ_]);

  // involuntary wait
  long steal = stol(cpu_stats[LinuxParser::kSteal_]);

  return (user + nice + system + idle + iowait + irq + softirq + steal);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpu_stats = LinuxParser::CpuUtilization();

  // twiddling thumbs
  long idle = stol(cpu_stats[LinuxParser::kIdle_]);

  // waiting for I/O to complete
  long iowait = stol(cpu_stats[LinuxParser::kIOwait_]);

  return (idle + iowait);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> temp_strings;
  string line, temp;
  ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> temp;
    // I have adding kEndCPUStates_ to end of enum CPUStates in linux_parser.h
    for (int ii = 0; ii < kEndCPUStates_; ii++) {
      linestream >> temp;
      temp_strings.push_back(temp);
    }
  }
#if 0
  // TESTING:
  std::vector<std::string> cpu{"4705","356","584","3699","23","23","0","0","0","0"};  
  return cpu;//temp_strings;
#else
  return temp_strings;
#endif
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  int value;
  ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return value;
        }
      }
    }
  }
  return 0;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int value;
  ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return value;
        }
      }
    }
  }
  // Fails quietly.
  return 0;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string line{"BADLINE"};
  ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
  }
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  int kB2mB = 1000;
  int mbMem = (ReadProcPIDStatus(pid, "VmRSS") / kB2mB);
  return to_string(mbMem);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  int uid = ReadProcPIDStatus(pid, "Uid");
  return to_string(uid);
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string uid_str_search = LinuxParser::Uid(pid);
  string line;
  string user{"NONE"};
  string uid_str;
  ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ' ', '_');
      replace(line.begin(), line.end(), 'x', ' ');
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      while (linestream >> user >> uid_str) {
        if (uid_str == uid_str_search) {
          return user;
        }
      }
    }
  }
  // Failing quietly. No clear instruction/requirement here.
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string filename = kProcDirectory + to_string(pid) + kStatFilename;
  vector<string> vstr = Line2StrVector(filename);

  if (vstr.size() >= 21) {
    long starttime = stol(vstr[21]);
    long pidUptime = UpTime() - (starttime / ClkTPS());
    return pidUptime;
  } else {
    // Failing quietly. No clear instruction/requirement here.
    return 0;
  }
}

long LinuxParser::ReadProcMemInfo(const std::string &search_key) {
  string line;
  string key;
  string str_value;
  long value = 0;
  ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      linestream >> key >> str_value;
      if (key == search_key) {
        value = stol(str_value);
        return value;
      }
    }
  }
  // no error handling taking place
  return value;
}

long LinuxParser::ReadProcPIDStatus(const int &pid,
                                    const std::string &search_key) {
  string line;
  string key;
  string str_value;
  long value = 0;
  ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      linestream >> key >> str_value;
      if (key == search_key) {
        value = stol(str_value);
        return value;
      }
    }
  }
  // no error handling taking place
  return value;
}

