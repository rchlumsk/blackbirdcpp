#ifndef BOUNDARYCONDITIONS_H
#define BOUNDARYCONDITIONS_H

#include <string>
#include <vector>
#include "BlackbirdInclude.h"

class CBoundaryConditions {
public:
  // Member variables
  std::string stationname;
  double station;
  std::string reach;
  std::string location;
  enum_bc_type bctype;
  double bcvalue;
  double init_WSL;

  // Constructor
  CBoundaryConditions();

  // Functions
  void pretty_print() const; // defined in StandardOutput.cpp
};

#endif