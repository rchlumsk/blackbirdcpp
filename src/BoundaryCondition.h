#ifndef BOUNDARYCONDITIONS_H
#define BOUNDARYCONDITIONS_H

#include "BlackbirdInclude.h"

class CBoundaryCondition {
public:
  // Member variables
  int nodeID;
  enum_bc_type bctype; // boundary condition type
  double bcvalue; // boundary condition value
  double init_WSL; // initial water surface level

  // Constructor
  CBoundaryCondition();

  // Functions
  void pretty_print() const; // defined in StandardOutput.cpp
};

#endif
