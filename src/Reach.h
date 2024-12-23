#ifndef REACH_H
#define REACH_H

#include "Streamnode.h"

class CReach : public CStreamnode {
public:
  // Constructor
  CReach();

  // Functions
  void compute_basic_flow_properties(double flow, COptions *&bbopt);
};

#endif
