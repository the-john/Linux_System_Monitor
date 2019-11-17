#include <string>

#include "format.h"

using std::string;
using std::to_string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS

string Format::ElapsedTime(long seconds) { 
    string output;

    int hour = seconds / (60 * 60);
    int min = (seconds / 60) % 60;
    long sec = seconds % 60; // clean up seconds

    // Make a string for the time
    if (hour < 10) {
        output += "0";
    }
    output += std::to_string(hour) + ":";
    
    if (min < 10) {
        output = "0";
    }
    output += std::to_string(min) + ":";
    
    if (sec < 10) {
        output += "0";
    }
    output += std::to_string(sec);

    return output; 
}


/*
// Alternative time methodology from reviewer
std::string Format::Pad(string s, char c) {
  s.insert(s.begin(), 2 - s.size(), c);
  return s;
}

std::string Format::ElapsedTime(long int time) {
  int hours = time / (60 * 60);
  int minutes = (time / 60) % 60;
  long seconds = time % 60;
  return std::string(Format::Pad(to_string(hours), '0') + ":" +
                     Format::Pad(to_string(minutes), '0') + ":" +
                     Format::Pad(to_string(seconds), '0'));
}
*/