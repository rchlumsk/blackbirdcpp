#include "XSection.h"

// Default constructor
CXSection::CXSection()
  : CStreamnode(),
  xx(),
  zz(),
  manning(),
  manning_LOB(PLACEHOLDER),
  manning_main(PLACEHOLDER),
  manning_ROB(PLACEHOLDER),
  lbs_xx(PLACEHOLDER),
  rbs_xx(PLACEHOLDER),
  ds_length_LOB(PLACEHOLDER),
  ds_length_main(PLACEHOLDER),
  ds_length_ROB(PLACEHOLDER) {
}

// Calculate minimum elevation
double CXSection::calc_min_elev() {
  return min_elev;
}

// Compute basic depth properties
void CXSection::compute_basic_depth_properties(double wsl, COptions *&bbopt) {
  if (bbopt->manning_enforce_values && (manning_LOB == PLACEHOLDER || manning_main == PLACEHOLDER || manning_ROB == PLACEHOLDER)) {
    WriteWarning("XSection.cpp: compute_basic_depth_properties: "
                 "bbopt->manning_enforce_values is TRUE but one or more Manning "
                 "values for bank sections not defined.\nSetting "
                 "bbopt->manning_enforce_values to FALSE", bbopt->noisy_run);
    bbopt->manning_enforce_values = false;
  }
  if (bbopt->xsection_conveyance_method == enum_xsc_method::AREAWEIGHTED_CONVEYANCE &&
      bbopt->manning_composite_method != enum_mc_method::WEIGHTED_AVERAGE_AREA) {
    WriteWarning(
        "XSection.cpp: compute_basic_depth_properties: "
        "conveyance method 'areaweighted_conveyance' should be used with the "
        "Manning composite method 'weighted_average_area' for consistency.",
        bbopt->noisy_run);
  }
  if (bbopt->roughness_multiplier == PLACEHOLDER ||
      bbopt->roughness_multiplier <= 0) {
    ExitGracefully("XSection.cpp: compute_basic_depth_properties: "
                   "bbopt->roughness_multiplier must be a positive value.", BAD_DATA);
  }

  mm->wsl = wsl;
  mm->depth = mm->wsl - mm->min_elev;

  std::valarray<double> t_xx, t_zz, t_nn, depth;
  std::valarray<bool> ind_lob, ind_main, ind_rob;

  if (bbopt->xsection_conveyance_method !=
      enum_xsc_method::DISCRETIZED_CONVEYANCE_XS) {
    t_xx = xx;
    t_zz = zz;
    t_nn = manning;

    ind_lob.resize(t_xx.size());
    ind_main.resize(t_xx.size());
    ind_rob.resize(t_xx.size());

    depth = mm->wsl - zz;

    //maybe needs revision
    for (int i = 0; i < t_xx.size(); i++) {
      if (depth[i] > 0 || (i + 1 < t_xx.size() && depth[i + 1] > 0))
      if (t_xx[i] < lbs_xx) {
        ind_lob[i] = true;
      } else if (t_xx[i] < rbs_xx) {
        ind_main[i] = true;
      } else {
        ind_rob[i] = true;
      }
    }

  } else {
    double min_elev = mm->min_elev;
    double min_dist = xx.min();
    double max_dist = xx.max();
    //more logic
  }
  
  if (bbopt->manning_enforce_values && manning_LOB != PLACEHOLDER &&
      manning_main != PLACEHOLDER && manning_ROB != PLACEHOLDER) {
    t_nn[ind_lob] = manning_LOB;
    t_nn[ind_main] = manning_main;
    t_nn[ind_rob] = manning_ROB;
  }

  if (bbopt->roughness_multiplier != 1) {
    t_nn *= bbopt->roughness_multiplier;
  }

  if (bbopt->xsection_conveyance_method !=
      enum_xsc_method::DISCRETIZED_CONVEYANCE_XS) {
    // left off here
  }
}

// Compute basic flow properties
void CXSection::compute_basic_flow_properties(double flow, COptions *&bbopt) {
  mm->flow = flow;
  mm->flow_lob = mm->flow * mm->k_lob / mm->k_total;
  mm->flow_main = mm->flow * mm->k_main / mm->k_total;
  mm->flow_rob = mm->flow * mm->k_rob / mm->k_total;
  
  mm->velocity = mm->flow / mm->area != DBL_MAX ? mm->flow / mm->area : 0; // maybe needs revision
  mm->velocity_lob = mm->area_lob != 0 && mm->flow_lob / mm->area_lob
                         ? mm->flow_lob / mm->area_lob
                         : 0;
  mm->velocity_main = mm->area_main != 0 && mm->flow_main / mm->area_main
                          ? mm->flow_main / mm->area_main
                          : 0;
  mm->velocity_rob = mm->area_rob != 0 && mm->flow_rob / mm->area_rob
                         ? mm->flow_rob / mm->area_rob
                         : 0;

  mm->velocity_head = (mm->alpha * pow(mm->velocity, 2) / 2) / GRAVITY;
  mm->energy_total = mm->velocity_head + mm->wsl;
  mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
  mm->sf = pow(mm->flow / mm->k_total, 2) != DBL_MAX // maybe needs revision
               ? pow(mm->flow / mm->k_total, 2)
               : 0;
}

// Calculate flow area
double CXSection::calculate_flow_area() {
  return xx * zz;
}