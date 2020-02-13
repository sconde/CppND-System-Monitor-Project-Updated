#include <string>

#include "format.h"

inline std::string PrettyNumber(const int num){
  if ( num < 10 )
    return "0" + std::to_string(num);
  return std::to_string(num);
}

// Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
std::string Format::ElapsedTime(long seconds ) {

  const int hours = seconds / 60 / 60;
  const int secs = seconds % 60;
  const int minutes = (seconds - hours * 3600 - secs) / 60;

  return ( PrettyNumber(hours) + ":" + PrettyNumber(minutes) + ":" + PrettyNumber(secs) );
}
