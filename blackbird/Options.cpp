#include "Options.h"

// Default constructor
COptions::COptions()
  : workingfolder(""),
  modeltype(""),
  regimetype(""),
  dx(0.0),
  interp_extrapolation_points(""),
  num_extrapolation_points(0.0),
  xs_use_obcalcs(false),
  friction_slope_method(""),
  xsection_conveyance_method(""),
  catchment_conveyance_method(""),
  enforce_delta_Leff(false),
  delta_reachlength(0.0),
  tolerance_cp(0.0),
  iteration_limit_cp(0.0),
  next_WSL_split_cp(0.0),
  tolerance_nd(0.0),
  iteration_limit_nd(0.0),
  next_WSL_split_nd(0.0),
  silent_cp(false),
  silent_nd(false),
  max_RHSQ_ratio(0.0),
  min_RHSQ_ratio(0.0),
  use_dhand(false),
  dhand_Hseq(0.0),
  manning_composite_method(""),
  manning_enforce_values(false),
  catchment_integration_method(""),
  interpolation_postproc_method(""),
  postproc_elev_corr_threshold(0.0),
  roughness_multiplier(0.0),
  blended_conveyance_weights(0.0),
  blended_nc_weights(0.0) {
  // Default constructor implementation
}

// Check options function
void COptions::check_options() {
}