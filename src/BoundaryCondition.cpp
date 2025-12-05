#include "BlackbirdInclude.h"
#include "BoundaryCondition.h"

// Default constructor
CBoundaryCondition::CBoundaryCondition()
  : nodeID(PLACEHOLDER),
  bctype(enum_bc_type::NORMAL_DEPTH),
  bcvalue(PLACEHOLDER),
  init_WSL(PLACEHOLDER) {
  // Default constructor implementation
}
