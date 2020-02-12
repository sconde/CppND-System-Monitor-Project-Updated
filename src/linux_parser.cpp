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

int LinuxParser::ClkTPS() { return sysconf(_SC_CLK_TCK); }

std::vector<std::string> Line2StrVector(string filename) {
  std::vector<string> result;
  std::string line, value;
  std::ifstream f_stream(filename);

  if (f_stream) {
    getline(f_stream, line);

    std::istringstream linestream(line);
    while (linestream >> value) 
      result.push_back(value);

  }
  
  return result;
}


// An example of how to read data from the filesystem
std::string LinuxParser::OperatingSystem() {
  string line, key, value;

  std::ifstream f_stream(kOSPath);
  if (f_stream) {
    while (std::getline(f_stream, line)) {

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

//  An example of how to read data from the filesystem
std::string LinuxParser::Kernel() {
  std::string os, kernel;
  std::string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
std::vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
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

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  auto total_memory = static_cast<float>(LinuxParser::ReadProcMemInfo("MemTotal") );
  auto avail_memory = static_cast<float>(LinuxParser::ReadProcMemInfo("MemAvailable") );
  return ( total_memory - avail_memory ) / total_memory;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line, str_value;
  long value = 0;

  std::ifstream filestream(kProcDirectory + kUptimeFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> str_value;
    value = stol(str_value);
  }
  return value;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return (ActiveJiffies() - IdleJiffies()); }

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::string filename = kProcDirectory + to_string(pid) + kStatFilename;
  std::vector<string> vstr = Line2StrVector(filename);

  if (vstr.size() >= 21) 
    return (stol(vstr[13]) + stol(vstr[14]) + stol(vstr[15]) + stol(vstr[16]));

  return 0;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  const auto cpu_stats = LinuxParser::CpuUtilization();
  const auto user      = stol(cpu_stats[LinuxParser::kUser_]);
  const auto nice      = stol(cpu_stats[LinuxParser::kNice_]);
  const auto system    = stol(cpu_stats[LinuxParser::kSystem_]);
  const auto idle      = stol(cpu_stats[LinuxParser::kIdle_]);
  const auto iowait    = stol(cpu_stats[LinuxParser::kIOwait_]);
  const auto irq       = stol(cpu_stats[LinuxParser::kIRQ_]);
  const auto softirq   = stol(cpu_stats[LinuxParser::kSoftIRQ_]);
  const auto steal     = stol(cpu_stats[LinuxParser::kSteal_]);

  return (user + nice + system + idle + iowait + irq + softirq + steal);
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  const auto cpu_stats = LinuxParser::CpuUtilization();
  const auto idle      = stol(cpu_stats[LinuxParser::kIdle_]);
  const auto iowait    = stol(cpu_stats[LinuxParser::kIOwait_]);

  return (idle + iowait);
}

// Read and return CPU utilization
std::vector<std::string> LinuxParser::CpuUtilization() {
  std::vector<string> result;
  std::string line, temp;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream) {
    getline(stream, line);

    std::istringstream linestream(line);
    linestream >> temp;

    // I have adding kEndCPUStates_ to end of enum CPUStates in linux_parser.h
    for (int ii = 0; ii < kEndCPUStates_; ii++) {
      linestream >> temp;
      result.push_back(temp);
    }
  }

  return result;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::string line, key;

  int value;
  std::ifstream f_stream(kProcDirectory + kStatFilename);

  if (f_stream) {
    while (getline(f_stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes")  return value;
      }
    }
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int value;
  ifstream f_stream(kProcDirectory + kStatFilename);

  if (f_stream) {
    while (getline(f_stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running")  return value;
      }
    }
  }

  return 0;
}

// Read and return the command associated with a process
std::string LinuxParser::Command(int pid) {
  std::string line{"BADLINE"};

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);

  if (filestream) 
    getline(filestream, line);

  return line;
}

// Read and return the memory used by a process
std::string LinuxParser::Ram(int pid) {
  const int kB2mB = 1024;
  const auto mbMem = (ReadProcPIDStatus(pid, "VmRSS") / kB2mB);
  return to_string(mbMem);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  const auto uid = ReadProcPIDStatus(pid, "Uid");
  return to_string(uid);
}

// Read and return the user associated with a process
std::string LinuxParser::User(int pid) {

  std::string key = LinuxParser::Uid(pid);
  std::string line, uid_str, user;

  std::ifstream f_stream(kPasswordPath);

  if (f_stream) {
    while (getline(f_stream, line)) {

      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), 'x', ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      while (linestream >> user >> uid_str) {
        if (uid_str == key )  return user;
      }
    }
  }

  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {

  std::vector<string> result = Line2StrVector(kProcDirectory + to_string(pid) + kStatFilename);

  if (result.size() >= 21) {
    const auto starttime = stol(result[21]);
    return  UpTime() - (starttime / ClkTPS());
  }

    return 0;
}

long ReadProcInfo(const std::string filename, const std::string &search_key) {
  std::string line, key, str_value;
  std::ifstream f_stream(filename);

  if (f_stream) {
    while (getline(f_stream, line)) {

      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      while( linestream >> key >> str_value){
        if (key == search_key)  return stol(str_value);
      }
    }
  }
  return 0;
}

long LinuxParser::ReadProcMemInfo(const std::string &search_key) {
  std::string filename = kProcDirectory + kMeminfoFilename;
  return ReadProcInfo( filename, search_key); 
}

long LinuxParser::ReadProcPIDStatus(const int &pid, const std::string &search_key) {
  std::string filename = kProcDirectory + std::to_string(pid) + kStatusFilename;
  return ReadProcInfo(filename, search_key);
}
