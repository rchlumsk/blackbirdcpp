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

// Copy constructor
CXSection::CXSection(const CXSection &other)
    : CStreamnode(other), xx(other.xx), zz(other.zz), manning(other.manning),
      manning_LOB(other.manning_LOB), manning_main(other.manning_main),
      manning_ROB(other.manning_ROB), lbs_xx(other.lbs_xx),
      rbs_xx(other.rbs_xx), ds_length_LOB(other.ds_length_LOB),
      ds_length_main(other.ds_length_main), ds_length_ROB(other.ds_length_ROB) {
}

// Copy assignment operator
CXSection &CXSection::operator=(const CXSection &other) {
  if (this == &other) {
    return *this; // Handle self-assignment
  }

  CStreamnode::operator=(other); // Copy base class members
  xx = other.xx;
  zz = other.zz;
  manning = other.manning;
  manning_LOB = other.manning_LOB;
  manning_main = other.manning_main;
  manning_ROB = other.manning_ROB;
  lbs_xx = other.lbs_xx;
  rbs_xx = other.rbs_xx;
  ds_length_LOB = other.ds_length_LOB;
  ds_length_main = other.ds_length_main;
  ds_length_ROB = other.ds_length_ROB;

  return *this;
}

//////////////////////////////////////////////////////////////////
/// \brief Compute minimum elevation
///
/// \return minimum elevation
//
double CXSection::calc_min_elev() {
  return min_elev;
}

//////////////////////////////////////////////////////////////////
/// \brief Compute basic depth properties
///
/// \param wsl [in] water surface level to be used in computations
/// \param *&bbopt [in] Global model options information
//
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

  std::valarray<double> t_xx, t_zz, t_nn, depth, wet_per, temp_t_nn;
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
    ind = depth.shift(1) > 0 || depth > 0;
    notind = !ind;

    ind_lob = ind && t_xx <= lbs_xx;
    ind_main = ind && t_xx > lbs_xx && xx < rbs_xx;
    ind_rob = ind && t_xx >= rbs_xx;
  } else { // skip this for now and write warning
    WriteWarning(
        "XSection.cpp: compute_basic_depth_properties: "
        "xsection_conveyance_method DISCRETIZED_CONVEYANCE_XS is not available",
        bbopt->noisy_run);
    //double min_elev = mm->min_elev;
    //double min_dist = xx.min();
    //double max_dist = xx.max();

    //t_xx.resize(floor((max_dist - min_dist) / bbopt->dx) + 1);
    //for (int i = 0; i < t_xx.size(); i++) {
    //  t_xx[i] = min_dist + i * bbopt->dx;
    //}
    //int n = t_xx.size();
    ////more logic 1273-1280 approx and manning interp method
    //
    //depth = mm->wsl - t_zz;
    //ind = depth > 0;
    //notind = !ind;

    //ind_lob = ind && t_xx <= lbs_xx;
    //ind_main = ind && t_xx > lbs_xx && xx < rbs_xx;
    //ind_rob = ind && t_xx >= rbs_xx;
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
  if (bbopt->xsection_conveyance_method != enum_xsc_method::DISCRETIZED_CONVEYANCE_XS) {
    area.resize(t_xx.size());
    area[t_xx.size() - 1] = 0;
    for (int i = 0; i < t_xx.size() - 1; i++) {
      if (mm->wsl < std::min(t_zz[i], t_zz[i + 1])) {
        area[i] = 0;
      } else {
        area[i] = std::max(mm->wsl - std::max(t_zz[i], t_zz[i + 1]) * (t_xx[i + 1] - t_xx[i]), 0.);
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
        top_width[i] = t_xx[i + 1] - t_xx[i];
        if (top_width[i] < 0) {
          WriteWarning("XSection.cpp: compute_basic_depth_properties: "
                       "top_width was negative",
                       bbopt->noisy_run);
        }
        wet_per[i] = std::sqrt(std::pow(t_xx[i + 1] - t_xx[i], 2.) +
                               std::pow(t_zz[i + 1] - t_zz[i], 2.));
      } else {
        top_width[i] = std::abs(t_xx[i + 1] - t_xx[i]) *
                       (mm->wsl - std::min(t_zz[i], t_zz[i + 1]) /
                                      (t_zz[i + 1] - t_zz[i]));
        if (top_width[i] < 0) {
          WriteWarning("XSection.cpp: compute_basic_depth_properties: "
                       "top_width was negative",
                       bbopt->noisy_run);
        }
        wet_per[i] =
            std::sqrt(std::pow(top_width[i], 2.) +
                      std::pow(mm->wsl - std::min(t_zz[i], t_zz[i + 1]), 2.));
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
  } else { // skip and throw warning for now
    WriteWarning(
        "XSection.cpp: compute_basic_depth_properties: "
        "xsection_conveyance_method DISCRETIZED_CONVEYANCE_XS is not available",
        bbopt->noisy_run);
    //int len(0), len_l(0), len_m(0), len_r(0);
    //for (int i = 0; i < ind.size(); i++) {
    //  if (ind[i]) {
    //    len++;
    //  }

    //  if (ind_lob[i]) {
    //    len_l++;
    //  } else if (ind_main[i]) {
    //    len_m++;
    //  } else if (ind_rob[i]) {
    //    len_r++;
    //  }
    //}

    //mm->top_width = len * bbopt->dx;
    //mm->top_width_lob = len_l * bbopt->dx;
    //mm->top_width_main = len_m * bbopt->dx;
    //mm->top_width_rob = len_r * bbopt->dx;

    //// assign wet perimeter logic. "wetted_perimeter" , "get_breakpoints" , etc. 1401


    //if (t_zz.size() > 0 && mm->wsl > t_zz[0]) {
    //  wet_per[0] += mm->wsl - t_zz[0];
    //  mm->wet_perimeter_lob += mm->wsl - t_zz[0];
    //}
    //if (t_zz.size() > 0 && mm->wsl > t_zz[t_zz.size() - 1]) {
    //  wet_per[wet_per.size() - 1] += mm->wsl - t_zz[wet_per.size() - 1];
    //  mm->wet_perimeter_rob += mm->wsl - t_zz[wet_per.size() - 1];
    //}
  }
  mm->wet_perimeter = mm->wet_perimeter_lob + mm->wet_perimeter_main + mm->wet_perimeter_rob;
  mm->hradius = mm->wet_perimeter > 0 ? std::max(mm->area / mm->wet_perimeter, 0.) : 0;
  mm->hradius_main = mm->wet_perimeter_main > 0 ? std::max(mm->area_main / mm->wet_perimeter_main, 0.) : 0;
  mm->hradius_lob = mm->wet_perimeter_lob > 0 ? std::max(mm->area_lob / mm->wet_perimeter_lob, 0.) : 0;
  mm->hradius_rob = mm->wet_perimeter_rob > 0 ? std::max(mm->area_rob / mm->wet_perimeter_rob, 0.) : 0;
  std::valarray<double> hradius = area / wet_per;
  mm->hyd_depth = mm->area / mm->top_width;
  mm->hyd_depth_lob = mm->area_lob / mm->top_width_lob;
  mm->hyd_depth_main = mm->area_main / mm->top_width_main;
  mm->hyd_depth_rob = mm->area_rob / mm->top_width_rob;

  std::valarray<double> k = (1. / t_nn) * area * std::pow(hradius, 2. / 3.);
  k[notind] = 0;
  k[k < 0] = 0;

  if (bbopt->xsection_conveyance_method == enum_xsc_method::OVERBANK_CONVEYANCE) {
    if (manning_LOB == PLACEHOLDER || manning_LOB == 0 ||
        manning_main == PLACEHOLDER || manning_main == 0 ||
        manning_ROB == PLACEHOLDER || manning_ROB == PLACEHOLDER) {
      ExitGracefully(
          "XSection.cpp: compute_basic_depth_properties: manning_LOB, "
          "manning_main, and manning_ROB must be set and non 0 when using "
          "xsection conveyance method: OVERBANK_CONVEYANCE",
          exitcode::BAD_DATA);
    }
    mm->k_lob = (1. / manning_LOB) * mm->area_lob * std::pow(mm->hradius_lob, 2. / 3.);
    mm->k_main = (1. / manning_main) * mm->area_main * std::pow(mm->hradius_main, 2. / 3.);
    mm->k_rob = (1. / manning_ROB) * mm->area_rob * std::pow(mm->hradius_rob, 2. / 3.);
    mm->k_total = mm->k_lob + mm->k_main + mm->k_rob;
    double fac1 = mm->area_lob != 0
                      ? std::pow(mm->k_lob, 3.) / std::pow(mm->area_lob, 2.)
                      : 0;
    double fac2 = mm->area_main != 0
                      ? std::pow(mm->k_main, 3.) / std::pow(mm->area_main, 2.)
                      : 0;
    double fac3 = mm->area_rob != 0
                      ? std::pow(mm->k_rob, 3.) / std::pow(mm->area_rob, 2.)
                      : 0;
    mm->alpha = (std::pow(mm->area, 2.) / std::pow(mm->k_total, 3.)) * (fac1 + fac2 + fac3);
  } else if (bbopt->xsection_conveyance_method == enum_xsc_method::DEFAULT_CONVEYANCE){
    std::valarray<int> roughness_zones(t_xx.size());
    roughness_zones[0] = 1;
    for (int i = 1; i < t_xx.size(); i++) {
      if (t_nn[i] != t_nn[i - 1]) {
        roughness_zones[i] = roughness_zones[i - 1] + 1;
      } else {
        roughness_zones[i] = roughness_zones[i - 1];
      }
    }
    roughness_zones[ind_main || ind_rob] = (std::valarray<int>)roughness_zones[ind_main || ind_rob] + 1;
    roughness_zones[ind_rob] = (std::valarray<int>)roughness_zones[ind_rob] + 1;

    std::valarray<double> conv(t_xx.size());
    temp_t_nn = t_nn;
    std::valarray<double> temp_area = area;
    std::valarray<double> temp_wet_per = wet_per;

    std::vector<int> unique_roughness_zones;
    for (auto rz : roughness_zones) {
      if (std::find(unique_roughness_zones.begin(),
                    unique_roughness_zones.end(),
                    rz) != unique_roughness_zones.end()) {
        unique_roughness_zones.push_back(rz);
      }
    }
    for (auto rz : unique_roughness_zones) {
      std::valarray<bool> allzones = roughness_zones == rz;
      int firstind = -1;
      for (int i = 0; i < allzones.size(); i++) {
        if (allzones[i]) {
          firstind = i;
          break;
        }
      }
      std::valarray<bool> otherzones = allzones;
      otherzones[firstind] = false;

      temp_area[firstind] = ((std::valarray<double>)temp_area[allzones]).sum();
      temp_area[otherzones] = 0;

      temp_wet_per[firstind] = ((std::valarray<double>)temp_wet_per[allzones]).sum();
      temp_wet_per[otherzones] = 0;
      conv[firstind] =
          temp_t_nn[firstind] != 0 && temp_wet_per[firstind] != 0
              ? ConvCalc(temp_t_nn[firstind], temp_area[firstind],
                         temp_area[firstind] / temp_wet_per[firstind])
              : 0;
    }

    wet_per = temp_wet_per;
    k = conv;
    area = temp_area;
    hradius = area / wet_per;
    hradius[wet_per == 0] = 0;

    mm->k_lob = ((std::valarray<double>)conv[ind_lob]).sum();
    mm->k_main = ((std::valarray<double>)conv[ind_main]).sum();
    mm->k_rob = ((std::valarray<double>)conv[ind_rob]).sum();
    mm->k_total = mm->k_lob + mm->k_main + mm->k_rob;
    double fac1 = mm->area_lob != 0
                      ? std::pow(mm->k_lob, 3.) / std::pow(mm->area_lob, 2.)
                      : 0;
    double fac2 = mm->area_main != 0
                      ? std::pow(mm->k_main, 3.) / std::pow(mm->area_main, 2.)
                      : 0;
    double fac3 = mm->area_rob != 0
                      ? std::pow(mm->k_rob, 3.) / std::pow(mm->area_rob, 2.)
                      : 0;
    mm->alpha = (std::pow(mm->area, 2.) / std::pow(mm->k_total, 3.)) * (fac1 + fac2 + fac3);
  } else if (bbopt->xsection_conveyance_method == enum_xsc_method::COORDINATE_CONVEYANCE){
    std::valarray<double> conv(t_xx.size());
    temp_t_nn = t_nn;
    conv = ConvCalc(manning, area, area / wet_per);
    conv[wet_per == 0 || manning == 0] = 0;
    conv[notind] = 0;
    conv[xx.size() - 1] = 0;

    mm->k_lob = ((std::valarray<double>)conv[ind_lob]).sum();
    mm->k_main = ((std::valarray<double>)conv[ind_main]).sum();
    mm->k_rob = ((std::valarray<double>)conv[ind_rob]).sum();
    mm->k_total = mm->k_lob + mm->k_main + mm->k_rob;
    double fac1 = mm->area_lob != 0
                      ? std::pow(mm->k_lob, 3.) / std::pow(mm->area_lob, 2.)
                      : 0;
    double fac2 = mm->area_main != 0
                      ? std::pow(mm->k_main, 3.) / std::pow(mm->area_main, 2.)
                      : 0;
    double fac3 = mm->area_rob != 0
                      ? std::pow(mm->k_rob, 3.) / std::pow(mm->area_rob, 2.)
                      : 0;
    mm->alpha = (std::pow(mm->area, 2.) / std::pow(mm->k_total, 3.)) * (fac1 + fac2 + fac3);
  } else if (bbopt->xsection_conveyance_method == enum_xsc_method::AREAWEIGHTED_CONVEYANCE){
    std::valarray<double> sum = area / t_nn;
    temp_t_nn = t_nn;
    mm->k_lob = mm->wet_perimeter_lob != 0
                    ? ((std::valarray<double>)sum[ind_lob]).sum() *
                          std::pow(mm->area_lob / mm->wet_perimeter_lob, 2. / 3.)
                    : 0;
    mm->k_main = mm->wet_perimeter_main != 0
                     ? ((std::valarray<double>)sum[ind_main]).sum() *
                           std::pow(mm->area_main / mm->wet_perimeter_main, 2. / 3.)
                     : 0;
    mm->k_rob = mm->wet_perimeter_rob != 0
                    ? ((std::valarray<double>)sum[ind_rob]).sum() *
                          std::pow(mm->area_rob / mm->wet_perimeter_rob, 2. / 3.)
                    : 0;
    mm->k_total = mm->k_lob + mm->k_main + mm->k_rob;
    double fac1 = mm->area_lob != 0
                      ? std::pow(mm->k_lob, 3.) / std::pow(mm->area_lob, 2.)
                      : 0;
    double fac2 = mm->area_main != 0
                      ? std::pow(mm->k_main, 3.) / std::pow(mm->area_main, 2.)
                      : 0;
    double fac3 = mm->area_rob != 0
                      ? std::pow(mm->k_rob, 3.) / std::pow(mm->area_rob, 2.)
                      : 0;
    mm->alpha = (std::pow(mm->area, 2.) / std::pow(mm->k_total, 3.)) * (fac1 + fac2 + fac3);
  } else if (bbopt->xsection_conveyance_method == enum_xsc_method::AREAWEIGHTED_CONVEYANCE_ONECALC_XS){
    std::valarray<double> sum = area / t_nn;
    temp_t_nn = t_nn;
    mm->k_total = mm->wet_perimeter != 0
                      ? ((std::valarray<double>)sum[ind]).sum() *
                            std::pow(mm->area / mm->wet_perimeter, 2. / 3.)
                      : 0;
    mm->k_lob = mm->k_total * mm->area_lob / mm->area;
    mm->k_main = mm->k_total * mm->area_main / mm->area;
    mm->k_rob = mm->k_total * mm->area_rob / mm->area;
    mm->alpha = 1;
  } else if (bbopt->xsection_conveyance_method == enum_xsc_method::DISCRETIZED_CONVEYANCE_XS){
    mm->k_lob = ((std::valarray<double>)k[ind_lob]).sum();
    mm->k_main = ((std::valarray<double>)k[ind_main]).sum();
    mm->k_rob = ((std::valarray<double>)k[ind_rob]).sum();
    mm->k_total = mm->k_lob + mm->k_main + mm->k_rob;
    temp_t_nn = t_nn;
    double fac1 = mm->area_lob != 0
                      ? std::pow(mm->k_lob, 3.) / std::pow(mm->area_lob, 2.)
                      : 0;
    double fac2 = mm->area_main != 0
                      ? std::pow(mm->k_main, 3.) / std::pow(mm->area_main, 2.)
                      : 0;
    double fac3 = mm->area_rob != 0
                      ? std::pow(mm->k_rob, 3.) / std::pow(mm->area_rob, 2.)
                      : 0;
    mm->alpha = (std::pow(mm->area, 2.) / std::pow(mm->k_total, 3.)) * (fac1 + fac2 + fac3);
  }

  mm->length_effective = (ds_length_LOB * mm->k_lob + ds_length_main * mm->k_main + ds_length_ROB * mm->k_rob) / mm->k_total;
  mm->length_effectiveadjusted = mm->length_effective;

  if (bbopt->xsection_conveyance_method == enum_xsc_method::OVERBANK_CONVEYANCE) {
    mm->manning_lob = manning_LOB * bbopt->roughness_multiplier;
    mm->manning_main = manning_main * bbopt->roughness_multiplier;
    mm->manning_rob = manning_ROB * bbopt->roughness_multiplier;

    if (bbopt->manning_composite_method == enum_mc_method::EQUAL_FORCE) {
      mm->manning_composite =
          std::sqrt((1. / mm->wet_perimeter) *
                    (mm->wet_perimeter_lob * std::pow(mm->manning_lob, 2.) +
                     mm->wet_perimeter_main * std::pow(mm->manning_main, 2.) +
                     mm->wet_perimeter_rob * std::pow(mm->manning_rob, 2.)));
    } else if (bbopt->manning_composite_method == enum_mc_method::WEIGHTED_AVERAGE_AREA) {
      mm->manning_composite = (mm->area_lob * mm->manning_lob +
                               mm->area_main * mm->manning_main +
                               mm->area_rob * mm->manning_rob) /
                              mm->area;
    } else if (bbopt->manning_composite_method == enum_mc_method::WEIGHTED_AVERAGE_WETPERIMETER) {
      mm->manning_composite = (mm->wet_perimeter_lob * mm->manning_lob +
                               mm->wet_perimeter_main * mm->manning_main +
                               mm->wet_perimeter_rob * mm->manning_rob) /
                              mm->area;
    } else if (bbopt->manning_composite_method == enum_mc_method::WEIGHTED_AVERAGE_CONVEYANCE) {
      mm->manning_composite = (mm->k_lob * mm->manning_lob +
                               mm->k_main * mm->manning_main +
                               mm->k_rob * mm->manning_rob) /
                              mm->k_total;
    } else if (bbopt->manning_composite_method == enum_mc_method::EQUAL_VELOCITY) {
      mm->manning_composite = std::pow(
          (1. / mm->wet_perimeter) *
              (mm->wet_perimeter_lob * std::pow(mm->manning_lob, 3. / 2.) +
               mm->wet_perimeter_main * std::pow(mm->manning_main, 3. / 2.) +
               mm->wet_perimeter_rob * std::pow(mm->manning_rob, 3. / 2.)),
          2. / 3.);
    }
  } else {
    if (bbopt->manning_composite_method == enum_mc_method::EQUAL_FORCE) {
      std::valarray<double> temp_coeff = wet_per * std::pow(temp_t_nn, 2.);
      mm->manning_lob = std::sqrt((1. / mm->wet_perimeter_lob) * ((std::valarray<double>)temp_coeff[ind_lob]).sum());
      mm->manning_main = std::sqrt((1. / mm->wet_perimeter_main) * ((std::valarray<double>)temp_coeff[ind_main]).sum());
      mm->manning_rob = std::sqrt((1. / mm->wet_perimeter_rob) * ((std::valarray<double>)temp_coeff[ind_rob]).sum());
      mm->manning_composite = std::sqrt((1. / mm->wet_perimeter) * ((std::valarray<double>)temp_coeff[ind]).sum());
    } else if (bbopt->manning_composite_method == enum_mc_method::WEIGHTED_AVERAGE_AREA) {
      std::valarray<double> temp_coeff = area * temp_t_nn;
      mm->manning_lob = ((std::valarray<double>)temp_coeff[ind_lob]).sum() /
                        ((std::valarray<double>)area[ind_lob]).sum();
      mm->manning_main = ((std::valarray<double>)temp_coeff[ind_main]).sum() /
                         ((std::valarray<double>)area[ind_main]).sum();
      mm->manning_rob = ((std::valarray<double>)temp_coeff[ind_rob]).sum() /
                        ((std::valarray<double>)area[ind_rob]).sum();
      mm->manning_composite = ((std::valarray<double>)temp_coeff[ind]).sum() /
                              ((std::valarray<double>)area[ind]).sum();
    } else if (bbopt->manning_composite_method == enum_mc_method::WEIGHTED_AVERAGE_WETPERIMETER) {
      std::valarray<double> temp_coeff = wet_per * temp_t_nn;
      mm->manning_lob = ((std::valarray<double>)temp_coeff[ind_lob]).sum() /
                        ((std::valarray<double>)wet_per[ind_lob]).sum();
      mm->manning_main = ((std::valarray<double>)temp_coeff[ind_main]).sum() /
                         ((std::valarray<double>)wet_per[ind_main]).sum();
      mm->manning_rob = ((std::valarray<double>)temp_coeff[ind_rob]).sum() /
                        ((std::valarray<double>)wet_per[ind_rob]).sum();
      mm->manning_composite = ((std::valarray<double>)temp_coeff[ind]).sum() /
                              ((std::valarray<double>)wet_per[ind]).sum();
    } else if (bbopt->manning_composite_method == enum_mc_method::WEIGHTED_AVERAGE_CONVEYANCE) {
      std::valarray<double> temp_coeff = k * temp_t_nn;
      mm->manning_lob = ((std::valarray<double>)temp_coeff[ind_lob]).sum() /
                        ((std::valarray<double>)k[ind_lob]).sum();
      mm->manning_main = ((std::valarray<double>)temp_coeff[ind_main]).sum() /
                         ((std::valarray<double>)k[ind_main]).sum();
      mm->manning_rob = ((std::valarray<double>)temp_coeff[ind_rob]).sum() /
                        ((std::valarray<double>)k[ind_rob]).sum();
      mm->manning_composite = ((std::valarray<double>)temp_coeff[ind]).sum() /
                              ((std::valarray<double>)k[ind]).sum();
    } else if (bbopt->manning_composite_method == enum_mc_method::EQUAL_VELOCITY) {
      std::valarray<double> temp_coeff = wet_per * std::pow(temp_t_nn, 3. / 2.);
      mm->manning_lob = std::pow((1. / mm->wet_perimeter_lob) * ((std::valarray<double>)temp_coeff[ind_lob]).sum(), 2. / 3.);
      mm->manning_main = std::pow((1. / mm->wet_perimeter_main) * ((std::valarray<double>)temp_coeff[ind_main]).sum(), 2. / 3.);
      mm->manning_rob = std::pow((1. / mm->wet_perimeter_rob) * ((std::valarray<double>)temp_coeff[ind_rob]).sum(), 2. / 3.);
      mm->manning_composite = std::pow((1. / mm->wet_perimeter) * ((std::valarray<double>)temp_coeff[ind]).sum(), 2. / 3.);
    }
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Compute basic flow properties
///
/// \param flow [in] flow to be used in computations
/// \param *&bbopt [in] Global model options information
//
void CXSection::compute_basic_flow_properties(double flow, COptions *&bbopt) {
  mm->flow = flow;
  mm->flow_lob = mm->flow * mm->k_lob / mm->k_total;
  mm->flow_main = mm->flow * mm->k_main / mm->k_total;
  mm->flow_rob = mm->flow * mm->k_rob / mm->k_total;
  
  mm->velocity = mm->area != 0 ? mm->flow / mm->area : 0;
  mm->velocity_lob = mm->area_lob != 0 && mm->flow_lob / mm->area_lob
                         ? mm->flow_lob / mm->area_lob
                         : 0;
  mm->velocity_main = mm->area_main != 0 && mm->flow_main / mm->area_main
                          ? mm->flow_main / mm->area_main
                          : 0;
  mm->velocity_rob = mm->area_rob != 0 && mm->flow_rob / mm->area_rob
                         ? mm->flow_rob / mm->area_rob
                         : 0;

  mm->velocity_head = (mm->alpha * pow(mm->velocity, 2.) / 2.) / GRAVITY;
  mm->energy_total = mm->velocity_head + mm->wsl;
  mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
  mm->sf = mm->k_total != 0 ? pow(mm->flow / mm->k_total, 2.) : 0;
}