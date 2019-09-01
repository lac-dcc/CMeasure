#include "../include/ControlChannel.hpp"

void ControlChannel::setState(float64 sample) {
  active = (sample > startLevel)? edge : !edge;
}

int ControlChannel::getBorder(float64 sample) {
  bool lastState = active;
  setState(sample);
  // Positive border
  if(this->edge) {
    // It is in level H
    if(lastState ) return (sample >startLevel)? 0 : -1;
    // It is in level L
    else return (sample < startLevel)? 0 : 1;
  // Neg border
  } else {
    // It is in level L
    if(lastState) return (sample < startLevel)? 0 : 1;
    // It is in level H
    else return (sample > startLevel)? 0 : -1;
  }
}

bool ControlChannel::trigger(float64 sample) {
  int borda = getBorder(sample);
  if ( (edge && (borda == 1)) ||
            (!edge && (borda == -1)) ) return true;
  else return false;
}

bool ControlChannel::untrigger(float64 sample) {
    int borda = getBorder(sample);
    return  ( (edge && (borda == -1)) ||
              (!edge &&(borda == 1)) );
}

