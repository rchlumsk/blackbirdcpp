#include "BlackbirdInclude.h"
#include "BoundaryConditions.h"

// Default constructor
CBoundaryConditions::CBoundaryConditions()
  : stationname(PLACEHOLDER_STR),
  station(PLACEHOLDER),
  reach(PLACEHOLDER_STR),
  location(PLACEHOLDER_STR),
  bctype(enum_bc_type::NORMAL_DEPTH),
  bcvalue(PLACEHOLDER),
  init_WSL(PLACEHOLDER) {
  // Default constructor implementation
}