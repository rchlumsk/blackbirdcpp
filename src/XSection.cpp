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

  std::valarray<double> t_xx, t_zz, t_nn, depth, wet_per;
  std::valarray<bool> ind, notind, ind_lob, ind_main, ind_rob;

  if (bbopt->xsection_conveyance_method !=
      enum_xsc_method::DISCRETIZED_CONVEYANCE_XS) {
    t_xx = xx;
    t_zz = zz;
    t_nn = manning;

    ind_lob.resize(t_xx.size());
    ind_main.resize(t_xx.size());
    ind_rob.resize(t_xx.size());

    depth = mm->wsl - t_zz;
    ind = depth.shift(1) > 0 || depth > 0; // check w/ rob on logic
    notind = !ind;

    ind_lob = ind && t_xx <= lbs_xx;
    ind_main = ind && t_xx > lbs_xx && xx < rbs_xx;
    ind_rob = ind && t_xx >= rbs_xx;
  } else {
    double min_elev = mm->min_elev;
    double min_dist = xx.min();
    double max_dist = xx.max();

    t_xx.resize(floor((max_dist - min_dist) / bbopt->dx) + 1);
    for (int i = 0; i < t_xx.size(); i++) {
      t_xx[i] = min_dist + i * bbopt->dx;
    }
    int n = t_xx.size();
    //more logic 1273-1280 approx? and manning interp method?
    
    depth = mm->wsl - t_zz;
    ind = depth > 0;
    notind = !ind;

    ind_lob = ind && t_xx <= lbs_xx;
    ind_main = ind && t_xx > lbs_xx && xx < rbs_xx;
    ind_rob = ind && t_xx >= rbs_xx;
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

  std::valarray<double> area;
  if (bbopt->xsection_conveyance_method !=
      enum_xsc_method::DISCRETIZED_CONVEYANCE_XS) {
    area.resize(t_xx.size());
    area[t_xx.size() - 1] = 0;
    for (int i = 0; i < t_xx.size() - 1; i++) {
      if (mm->wsl < std::min(t_zz[i], t_zz[i + 1])) {
        area[i] = 0;
      } else {
        area[i] = std::max(mm->wsl - std::max(t_zz[i], t_zz[i + 1]) * (t_xx[i + 1] - t_xx[i]), 0.0);
        if (mm->wsl > std::max(t_zz[i + 1], t_zz[i])) {
          area[i] += 0.5 * (t_xx[i + 1] - t_xx[i]) * std::abs(t_zz[i + 1] - t_zz[i]);
        } else {
          area[i] += 0.5 * (t_xx[i + 1] - t_xx[i]) *
                         pow(mm->wsl - std::min(t_zz[i], t_zz[i + 1]), 2) /
                         std::abs(t_zz[i] - t_zz[i + 1]);
        }
      }
    }
    area[notind] = 0;
    area[area < 0] = 0;
  } else {
    area = depth * bbopt->dx;
    area[notind] = 0;
    area[area < 0] = 0;
  }
  mm->area_lob = ((std::valarray<double>)area[ind_lob]).sum();
  mm->area_main = ((std::valarray<double>)area[ind_main]).sum();
  mm->area_rob = ((std::valarray<double>)area[ind_rob]).sum();
  mm->area = mm->area_lob + mm->area_main + mm->area_rob;

  if (bbopt->xsection_conveyance_method != enum_xsc_method::DISCRETIZED_CONVEYANCE_XS) {
    std::valarray<double> top_width(t_xx.size());
    wet_per.resize(t_xx.size());
    for (int i = 0; i < t_xx.size() - 1; i++) {
      if (mm->wsl < std::min(zz[i], zz[i + 1])) {
        top_width[i] = 0;
        wet_per[i] = 0;
      } else if (mm->wsl > std::max(zz[i + 1], zz[i])) {
        // finiteorzero?
        top_width[i] = t_xx[i + 1] - t_xx[i];
        wet_per[i] = std::sqrt(std::pow(t_xx[i + 1] - t_xx[i], 2) +
                               std::pow(t_zz[i + 1] - t_zz[i], 2));
      } else {
        // finiteorzero?
        top_width[i] = std::abs(t_xx[i + 1] - t_xx[i]) *
                       (mm->wsl - std::min(t_zz[i], t_zz[i + 1]) /
                                      (t_zz[i + 1] - t_zz[i]));
        wet_per[i] =
            std::sqrt(std::pow(top_width[i], 2) +
                      std::pow(mm->wsl - std::min(t_zz[i], t_zz[i + 1]), 2));
      }
    }
    top_width[notind] = 0;
    top_width[top_width < 0] = 0;
    wet_per[notind] = 0;
    wet_per[wet_per < 0] = 0;

    if (t_zz.size() > 0 && mm->wsl > t_zz[0]) {
      wet_per[0] += mm->wsl - t_zz[0];
    }
    if (t_zz.size() > 0 && mm->wsl > t_zz[t_zz.size() - 1]) {
      wet_per[wet_per.size() - 1] += mm->wsl - t_zz[wet_per.size() - 1];
    }

    mm->top_width = ((std::valarray<double>)top_width[ind]).sum();
    mm->top_width_lob = ((std::valarray<double>)top_width[ind_lob]).sum();
    mm->top_width_main = ((std::valarray<double>)top_width[ind_main]).sum();
    mm->top_width_rob = ((std::valarray<double>)top_width[ind_rob]).sum();

    mm->wet_perimeter_lob = ((std::valarray<double>)wet_per[ind_lob]).sum();
    mm->wet_perimeter_main = ((std::valarray<double>)wet_per[ind_main]).sum();
    mm->wet_perimeter_rob = ((std::valarray<double>)wet_per[ind_rob]).sum();
  } else {
    int len(0), len_l(0), len_m(0), len_r(0);
    for (int i = 0; i < ind.size(); i++) {
      if (ind[i]) {
        len++;
      }

      if (ind_lob[i]) {
        len_l++;
      } else if (ind_main[i]) {
        len_m++;
      } else if (ind_rob[i]) {
        len_r++;
      }
    }

    mm->top_width = len * bbopt->dx;
    mm->top_width_lob = len_l * bbopt->dx;
    mm->top_width_main = len_m * bbopt->dx;
    mm->top_width_rob = len_r * bbopt->dx;

    // assign wet perimeter logic. "wetted_perimeter" , "get_breakpoints" , etc. ?

    if (t_zz.size() > 0 && mm->wsl > t_zz[0]) {
      wet_per[0] += mm->wsl - t_zz[0];
      mm->wet_perimeter_lob += mm->wsl - t_zz[0];
    }
    if (t_zz.size() > 0 && mm->wsl > t_zz[t_zz.size() - 1]) {
      wet_per[wet_per.size() - 1] += mm->wsl - t_zz[wet_per.size() - 1];
      mm->wet_perimeter_rob += mm->wsl - t_zz[wet_per.size() - 1];
    }
  }
  mm->wet_perimeter = mm->wet_perimeter_lob + mm->wet_perimeter_main + mm->wet_perimeter_rob;
  // might need na.rm ?
  mm->hradius = std::max(mm->area / mm->wet_perimeter, 0.0);
  mm->hradius_main = std::max(mm->area_main / mm->wet_perimeter_main, 0.0);
  mm->hradius_lob = std::max(mm->area_lob / mm->wet_perimeter_lob, 0.0);
  mm->hradius_rob = std::max(mm->area_rob / mm->wet_perimeter_rob, 0.0);
  std::valarray<double> hradius = area / wet_per;
  mm->hyd_depth = mm->area / mm->top_width;
  mm->hyd_depth_lob = mm->area_lob / mm->top_width_lob;
  mm->hyd_depth_main = mm->area_main / mm->top_width_main;
  mm->hyd_depth_rob = mm->area_rob / mm->top_width_rob;

  // left off here
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
  return 0;
}