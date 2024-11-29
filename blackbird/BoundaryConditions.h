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
  std::string bctype;
  double bcvalue;
  double init_WSL;
  std::vector<int> headwater_nodes;
  std::vector<double> initial_flows;

  // Constructor
  CBoundaryConditions();
};

#endif