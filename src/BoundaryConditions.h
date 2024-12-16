#ifndef BOUNDARYCONDITIONS_H
#define BOUNDARYCONDITIONS_H

#include <string>
#include <vector>

class CBoundaryConditions {
public:
  // Member variables
  std::string stationname;
  double station;
  std::string reach;
  std::string location;
  std::string bctype; // change to ENUM -> NORMAL_DEPTH, SET_WSL, and SET_DEPTH
  double bcvalue;
  double init_WSL;

  // Constructor
  CBoundaryConditions();

  // Functions
  void pretty_print() const; // defined in StandardOutput.cpp
};

#endif