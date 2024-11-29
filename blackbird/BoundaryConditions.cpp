#include "BoundaryConditions.h"

// Default constructor
CBoundaryConditions::CBoundaryConditions()
  : stationname(""), station(0.0), reach(""), location(""), bctype(""),
  bcvalue(0.0), init_WSL(0.0), headwater_nodes(), initial_flows() {
  // Default constructor implementation
}