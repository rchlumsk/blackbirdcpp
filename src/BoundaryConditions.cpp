#include "BlackbirdInclude.h"
#include "BoundaryConditions.h"

// Default constructor
CBoundaryConditions::CBoundaryConditions()
  : stationname(PLACEHOLDER_STR),
  station(PLACEHOLDER),
  reach(PLACEHOLDER_STR),
  location(PLACEHOLDER_STR),
  bctype(PLACEHOLDER_STR),
  bcvalue(PLACEHOLDER),
  init_WSL(PLACEHOLDER) {
  // Default constructor implementation
}