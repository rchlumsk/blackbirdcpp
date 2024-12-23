#include "Reach.h"

// Default constructor
CReach::CReach()
  : CStreamnode() {
  // Default constructor implementation (inherits from CStreamnode)
}

// Compute basic flow properties
void CReach::compute_basic_flow_properties(double flow, COptions *&bbopt) {
  mm->flow = flow;
  mm->velocity = mm->flow / mm->area != DBL_MAX ? mm->flow / mm->area : 0; // maybe needs revision
  mm->velocity_head = (mm->alpha * pow(mm->velocity, 2) / 2) / GRAVITY;
  mm->energy_total = mm->velocity_head + mm->wsl;
  mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
  mm->sf = pow(mm->flow / mm->k_total, 2) != DBL_MAX // maybe needs revision
               ? pow(mm->flow / mm->k_total, 2)
               : 0;
}