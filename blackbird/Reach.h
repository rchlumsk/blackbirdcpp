#ifndef REACH_H
#define REACH_H

#include "Streamnode.h"

class CReach : public CStreamnode {
public:
  // Constructor
  CReach();

  // Functions
  hydraulic_output compute_basic_flow_properties();
};

#endif
