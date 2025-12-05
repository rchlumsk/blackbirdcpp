#ifndef OPTIONS_H
#define OPTIONS_H

#include "BlackbirdInclude.h"

class COptions {
public:
  // Member variables
  std::string version;                              // blackbird version
  std::string run_name;                             // name of current run
  std::string bbi_filename;                         // name of input bbi file
  std::string bbb_filename;                         // name of input bbb file
  std::string bbg_filename;                         // name of input bbg file

  std::string main_output_dir;                      // path to output directory
  std::string working_dir;                          // path to working directory
  std::string gis_path;                             // path to gis files (rasters, netcdf, shapefiles, etc.)

  enum_mt_method modeltype;                         // type of model. options: HAND_MANNING, STEADYFLOW
  enum_rt_method regimetype;                        // type of regime. options: SUBCRITICAL, SUPERCRITICAL, MIXED
  double dx;                                        // dx to use for cross section calculations
  bool extrapolate_depth_table;                     // on extrapolation beyond depth range of the table, true -> throw warning but extrapolate, false -> throw error and stop
  double num_extrapolation_points;                  // number of extrapolation points. unused?
  enum_fs_method friction_slope_method;             // friction slope method. options: AVERAGE_CONVEYANCE, AVERAGE_FRICTION, GEOMETRIC_FRICTON, HARMONIC_FRICTION, REACH_FRICTION
  enum_xsc_method xsection_conveyance_method;       // xsection conveyance method. options: OVERBANK_CONVEYANCE, DEFAULT_CONVEYANCE, COORDINATE_CONVEYANCE, DISCRETIZED_CONVEYANCE_XS, AREAWEIGHTED_CONVEYANCE_ONECALC_XS, AREAWEIGHTED_CONVEYANCE
  bool enforce_delta_Leff;                          // boolean for whether to enforce length effective when change in depth is too large
  double delta_reachlength;                         // maximum change in depth allowed before enforcing length effective
  double tolerance_cp;                              // tolerance error in metres for standard step depth calculations
  double iteration_limit_cp;                        // iteration limit for standard step depth calculations
  double next_WSL_split_cp;                         // wsl split for standard step depth calculations. unused?
  double tolerance_nd;                              // tolerance error in metres for normal depth calculations
  double iteration_limit_nd;                        // iteration limit for normal depth calculations
  double next_WSL_split_nd;                         // wsl split for normal depth calculations. unused?
  double max_RHSQ_ratio;                            // unused?
  double min_RHSQ_ratio;                            // unused?
  enum_mc_method manning_composite_method;          // manning composite method. options: EQUAL_FORCE, WEIGHTED_AVERAGE_AREA, WEIGHTED_AVERAGE_WETPERIMETER, WEIGHTED_AVERAGE_CONVEYANCE, EQUAL_VELOCITY, BLENDED_NC
  bool manning_enforce_values;                      // enforcing manning's n values to be populated
  enum_ri_method reach_integration_method;          // reach integration method. options: EFFECTIVE_LENGTH, REACH_LENGTH
  enum_le_method leff_method;                       // length effective method. options: AVERAGE, DOWNSTREAM, UPSTREAM
  enum_ppi_method interpolation_postproc_method;    // post-processing interpolation method. options: NONE, CATCHMENT_HAND, CATCHMENT_DHAND, INTERP_HAND, INTERP_DHAND, INTERP_DHAND_WSLCORR
  enum_dh_method dhand_method;                      // post-processing dhand method. options: INTERPOLATE, FLOOR
  double postproc_elev_corr_threshold;              // post-processing elevation correction threshold
  double roughness_multiplier;                      // roughness multiplier applied to all manning's n values
  double blended_conveyance_weights;                // unused?
  double blended_nc_weights;                        // unused?
  double froude_threshold;                          // froude threshold for computing depth properties
  enum_gridded_format in_format;                    // format of input gridded data. options: RASTER, NETCDF
  enum_gridded_format out_format;                   // format of output gridded data. options: RASTER, NETCDF, PNG
  std::string in_nc_name;                           // name of input netcdf file
  bool write_catchment_json;                        // for integration with BlackbirdView. boolean representing whether or not to modify the input catchments from streamnodes json file and write it to the output folder

  bool silent_run;                                  // true -> print less logs
  bool noisy_run;                                   // true -> print more logs

  // Constructor
  COptions();

  // Functions
  void PrepareOutputdirectory();  // ensures output directory exists
  void check_options();           // checks validity of option variables
  void pretty_print() const;      // defined in StandardOutput.cpp
};

#endif
