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

  // SET_ALL_DEPTHS boundary condition data
  std::vector<int> nodeIDs;  // List of nodes affected by this BC
  std::vector<double> min_elevs; // minimum elevations for each node
  std::vector< std::vector<double> > specified_depths; // depth profiles as specified_depths[p][i], p=flow profile, i=node index

  // Constructor
  CBoundaryCondition();

  // Functions
  void pretty_print() const; // defined in StandardOutput.cpp
};

#endif
