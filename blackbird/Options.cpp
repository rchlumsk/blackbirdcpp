#include "Options.h"

// Default constructor
COptions::COptions()
  : run_name(""),
  workingfolder(""),
  bbi_filename(""),
  bbg_filename(""),
  bbp_filename(""),
  bbb_filename(""),
  main_output_dir(""),
  modelname(""),
  modeltype(HAND_MANNING),
  regimetype(SUBCRITICAL),
  dx(0.0),
  extrapolate_depth_table(true),
  num_extrapolation_points(0.0),
  friction_slope_method(AVERAGE_CONVEYANCE),
  xsection_conveyance_method(OVERBANK_CONVEYANCE),
  reach_conveyance_method(DISCRETIZED_CONVEYANCE_R),
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
  manning_composite_method(EQUAL_FORCE),
  manning_enforce_values(false),
  reach_integration_method(EFFECTIVE_LENGTH),
  interpolation_postproc_method(CATCHMENT_HAND),
  postproc_elev_corr_threshold(0.0),
  roughness_multiplier(0.0),
  blended_conveyance_weights(0.0),
  blended_nc_weights(0.0),
  silent_run(false),
  noisy_run(false) {
  // Default constructor implementation
}

// Check options function
void COptions::check_options() {
}