#ifndef CCHANNEL_H
#define CCHANNEL_H

#include "Channel.hpp"

class ControlChannel : public Channel {
public:
  float64 startLevel;
  bool edge;
  bool active;
  
  ControlChannel(string chName, string chId,  float64 maxVoltage, 
		 float64 startLevel, bool edge) : Channel (chName, chId,  maxVoltage) {
    this->startLevel = startLevel;
    this->active = false;
    this->edge = edge;
  }

  void setState(float64 sample);

  int getBorder(float64 sample);

  bool trigger(float64 sample);

  bool untrigger(float64 sample);

};

#endif