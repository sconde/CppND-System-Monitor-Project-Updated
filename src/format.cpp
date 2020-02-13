#include <string>

#include "format.h"

using std::string;

// Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds ) {
  const auto sec = (seconds % 60);
  const auto hrs = (seconds / 3600);
  const auto min = (seconds - hrs * 3600 - sec) / 60;

  std::string str_hrs = (hrs < 10 ? ("0" + std::to_string(hrs)) : (std::to_string(hrs)));
  std::string str_min = (min < 10 ? ("0" + std::to_string(min)) : (std::to_string(min)));
  std::string str_sec = (sec < 10 ? ("0" + std::to_string(sec)) : (std::to_string(sec)));

  return (str_hrs + ":" + str_min + ":" + str_sec);
}
