#ifndef BLACKBIRDINCLUDE_H
#define BLACKBIRDINCLUDE_H

#include <string>
#include <iostream>
#include <sstream>

struct hydraulic_output {
  // Attributes
  int nodeID;
  int reachID;
  int downnodeID;
  int upnodeID1;
  int upnodeID2;
  std::string stationname;
  double station;
  double reach_length_DS;
  double reach_length_US1;
  double reach_length_US2;
  double flow;
  double flow_lob;
  double flow_main;
  double flow_rob;
  double min_elev;
  double wsl;
  double depth;
  double hyd_depth;
  double hyd_depth_lob;
  double hyd_depth_main;
  double hyd_depth_rob;
  double top_width;
  double top_width_lob;
  double top_width_main;
  double top_width_rob;
  double velocity;
  double velocity_lob;
  double velocity_main;
  double velocity_rob;
  double k_total;
  double k_lob;
  double k_main;
  double k_rob;
  double alpha;
  double area;
  double area_lob;
  double area_main;
  double area_rob;
  double hradius;
  double hradius_lob;
  double hradius_main;
  double hradius_rob;
  double wet_perimeter;
  double wet_perimeter_lob;
  double wet_perimeter_main;
  double wet_perimeter_rob;
  double energy_total;
  double velocity_head;
  double froude;
  double sf;
  double sf_avg;
  double sbed;
  double length_effective;
  double head_loss;
  double manning_lob;
  double manning_main;
  double manning_rob;
  double manning_composite;
  double k_total_areaconv;
  double k_total_roughconv;
  double k_total_disconv;
  double alpha_areaconv;
  double alpha_roughconv;
  double alpha_disconv;
  double nc_equalforce;
  double nc_equalvelocity;
  double nc_wavgwp;
  double nc_wavgarea;
  double nc_wavgconv;
  double depth_critical;
  int cp_iterations;
  double k_err;
  double ws_err;
  double length_energyloss;
  double length_effectiveadjusted;
};

enum enum_mt_method
{
  HAND_MANNING,
  STEADYFLOW
};

enum enum_rt_method
{
  SUBCRITICAL,
  SUPERCRITICAL,
  MIXED
};

enum enum_fs_method
{
  AVERAGE_CONVEYANCE,
  AVERAGE_FRICTION,
  GEOMETRIC_FRICTION,
  HARMONIC_FRICTION,
  REACH_FRICTION
};

enum enum_mc_method
{
  EQUAL_FORCE,
  WEIGHTED_AVERAGE_AREA,
  WEIGHTED_AVERAGE_WETPERIMETER,
  WEIGHTED_AVERAGE_CONVEYANCE,
  EQUAL_VELOCITY,
  BLENDED_NC
};

enum enum_xsc_method
{
  OVERBANK_CONVEYANCE,
  DEFAULT_CONVEYANCE,
  COORDINATE_CONVEYANCE,
  DISCRETIZED_CONVEYANCE,
  AREAWEIGHTED_CONVEYANCE_ONECALC,
  AREAWEIGHTED_CONVEYANCE
};

enum enum_rc_method
{
  DISCRETIZED_CONVEYANCE,
  AREAWEIGHTED_CONVEYANCE_ONECALC,
  ROUGHZONE_CONVEYANCE,
  BLENDED_CONVEYANCE
};

enum enum_ri_method
{
  EFFECTIVE_LENGTH,
  REACH_LENGTH
};

enum enum_ppi_method
{
  CATCHMENT_HAND,
  CATCHMENT_DHAND,
  INTERP_HAND,
  INTERP_DHAND,
  INTERP_DHAND_WSLCORR
};

#endif