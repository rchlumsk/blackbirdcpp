#include "Reach.h"

// Default constructor
CReach::CReach()
  : CStreamnode() {
  // Default constructor implementation (inherits from CStreamnode)
}

//////////////////////////////////////////////////////////////////
/// \brief Compute basic flow properties
///
/// \param flow [in] flow to be used in computations
/// \param *&bbopt [in] Global model options information
//
void CReach::compute_basic_flow_properties(double flow, COptions *&bbopt) {
  mm->flow = flow;
  mm->velocity = mm->area != 0 && mm->flow / mm->area != DBL_MAX
                     ? mm->flow / mm->area
                     : 0;
  mm->velocity_head = (mm->alpha * pow(mm->velocity, 2.) / 2.) / GRAVITY;
  mm->energy_total = mm->velocity_head + mm->wsl;
  mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
  mm->sf = mm->k_total != 0 && pow(mm->flow / mm->k_total, 2.) != DBL_MAX
               ? pow(mm->flow / mm->k_total, 2.)
               : 0;
}
