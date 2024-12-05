#ifndef OPTIONS_H
#define OPTIONS_H

#include "BlackbirdInclude.h"

class COptions {
public:
  // Member variables
  std::string version;
  std::string run_name;
  std::string workingfolder;
  std::string bbi_filename;
  std::string bbp_filename;
  std::string bbb_filename;
  std::string bbg_filename;

  std::string main_output_dir;
  std::string modelname;

  enum_mt_method modeltype;
  enum_rt_method regimetype;
  double dx;
  bool extrapolate_depth_table; // On extrapolation beyond depth range of the table, true -> throw warning, false -> throw error
  double num_extrapolation_points;
  enum_fs_method friction_slope_method;
  enum_xsc_method xsection_conveyance_method;
  enum_rc_method reach_conveyance_method;
  bool enforce_delta_Leff;
  double delta_reachlength;
  double tolerance_cp;
  double iteration_limit_cp;
  double next_WSL_split_cp;
  double tolerance_nd;
  double iteration_limit_nd;
  double next_WSL_split_nd;
  bool silent_cp;
  bool silent_nd;
  double max_RHSQ_ratio;
  double min_RHSQ_ratio;
  bool use_dhand;
  double dhand_Hseq;
  enum_mc_method manning_composite_method;
  bool manning_enforce_values;
  enum_ri_method reach_integration_method;
  enum_ppi_method interpolation_postproc_method;
  double postproc_elev_corr_threshold;
  double roughness_multiplier;
  double blended_conveyance_weights;
  double blended_nc_weights;

  bool silent_run;
  bool noisy_run;
  std::string working_dir;

  // Constructor
  COptions();

  // Functions
  void check_options();
};

#endif