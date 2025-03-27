#include "Options.h"

// Default constructor
COptions::COptions()
  : version(PLACEHOLDER_STR),
  run_name(PLACEHOLDER_STR),
  bbi_filename(PLACEHOLDER_STR),
  bbb_filename(PLACEHOLDER_STR),
  bbg_filename(PLACEHOLDER_STR),
  main_output_dir(PLACEHOLDER_STR),
  working_dir(PLACEHOLDER_STR),
  gis_path(PLACEHOLDER_STR),
  modeltype(enum_mt_method::HAND_MANNING),
  regimetype(enum_rt_method::SUBCRITICAL),
  dx(PLACEHOLDER),
  extrapolate_depth_table(true),
  num_extrapolation_points(PLACEHOLDER),
  friction_slope_method(enum_fs_method::AVERAGE_CONVEYANCE),
  xsection_conveyance_method(enum_xsc_method::OVERBANK_CONVEYANCE),
  reach_conveyance_method(enum_rc_method::DISCRETIZED_CONVEYANCE_R),
  enforce_delta_Leff(false),
  delta_reachlength(PLACEHOLDER),
  tolerance_cp(PLACEHOLDER),
  iteration_limit_cp(PLACEHOLDER),
  next_WSL_split_cp(PLACEHOLDER),
  tolerance_nd(PLACEHOLDER),
  iteration_limit_nd(PLACEHOLDER),
  next_WSL_split_nd(PLACEHOLDER),
  max_RHSQ_ratio(PLACEHOLDER),
  min_RHSQ_ratio(PLACEHOLDER),
  manning_composite_method(enum_mc_method::EQUAL_FORCE),
  manning_enforce_values(false),
  reach_integration_method(enum_ri_method::EFFECTIVE_LENGTH),
  interpolation_postproc_method(enum_ppi_method::CATCHMENT_HAND),
  dhand_method(enum_dh_method::INTERPOLATE),
  postproc_elev_corr_threshold(PLACEHOLDER),
  roughness_multiplier(PLACEHOLDER),
  blended_conveyance_weights(PLACEHOLDER),
  blended_nc_weights(PLACEHOLDER),
  froude_threshold(PLACEHOLDER),
  silent_run(false),
  noisy_run(false),
  out_format(enum_output_format::RASTER),
  in_nc_name("bb_inputs.nc"),
  in_nc(false) {
}

//////////////////////////////////////////////////////////////////
/// \brief creates specified output directory, if needed
//
void COptions::PrepareOutputdirectory()
{
  if (this->main_output_dir != "")
  {
#if defined(_WIN32)
    _mkdir(this->main_output_dir.c_str());
#elif defined(__linux__)
    mkdir(this->main_output_dir.c_str(), 0777);
#elif defined(__APPLE__)
    mkdir(this->main_output_dir.c_str(), 0777);
#elif defined(__unix__)
    mkdir(this->main_output_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
  }
  g_output_directory = this->main_output_dir;//necessary evil
}

// Check options function
void COptions::check_options() {
}
