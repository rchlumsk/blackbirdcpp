#include "BlackbirdInclude.h"
#include "Model.h"
#include "Streamnode.h"
#include<string>

// Default constructor
CStreamnode::CStreamnode()
  : nodeID(PLACEHOLDER),
  nodetype(enum_nodetype::REACH),
  downnodeID(PLACEHOLDER),
  upnodeID1(PLACEHOLDER),
  upnodeID2(PLACEHOLDER),
  stationname(PLACEHOLDER_STR),
  station(PLACEHOLDER),
  reachID(PLACEHOLDER),
  ds_reach_length(PLACEHOLDER),
  us_reach_length1(PLACEHOLDER),
  us_reach_length2(PLACEHOLDER),
  contraction_coeff(PLACEHOLDER),
  expansion_coeff(PLACEHOLDER),
  min_elev(PLACEHOLDER),
  bed_slope(PLACEHOLDER),
  depthdf(new std::vector<hydraulic_output*>),
  upstream_flows(),
  flow_sources(),
  flow_sinks(),
  output_flows(),
  output_depths(),
  output_wsls(),
  mm(new hydraulic_output) {
}

// Copy constructor
CStreamnode::CStreamnode(const CStreamnode& other)
  : nodeID(other.nodeID),
  nodetype(other.nodetype),
  downnodeID(other.downnodeID),
  upnodeID1(other.upnodeID1),
  upnodeID2(other.upnodeID2),
  stationname(other.stationname),
  station(other.station),
  reachID(other.reachID),
  ds_reach_length(other.ds_reach_length),
  us_reach_length1(other.us_reach_length1),
  us_reach_length2(other.us_reach_length2),
  contraction_coeff(other.contraction_coeff),
  expansion_coeff(other.expansion_coeff),
  min_elev(other.min_elev),
  bed_slope(other.bed_slope),
  depthdf(new std::vector<hydraulic_output*>),
  upstream_flows(other.upstream_flows),
  flow_sources(other.flow_sources),
  flow_sinks(other.flow_sinks),
  output_flows(other.output_flows),
  output_depths(other.output_depths),
  output_wsls(other.output_wsls),
  mm(new hydraulic_output(*(other.mm))) {
  if (other.depthdf) {
    for (auto ptr : *other.depthdf) {
      depthdf->push_back(new hydraulic_output(*ptr));
    }
  }
}

// Copy assignment operator
CStreamnode &CStreamnode::operator=(const CStreamnode &other) {
  if (this == &other) {
    return *this; // Handle self-assignment
  }

  nodeID = other.nodeID;
  nodetype = other.nodetype;
  downnodeID = other.downnodeID;
  upnodeID1 = other.upnodeID1;
  upnodeID2 = other.upnodeID2;
  stationname = other.stationname;
  station = other.station;
  reachID = other.reachID;
  ds_reach_length = other.ds_reach_length;
  us_reach_length1 = other.us_reach_length1;
  us_reach_length2 = other.us_reach_length2;
  contraction_coeff = other.contraction_coeff;
  expansion_coeff = other.expansion_coeff;
  min_elev = other.min_elev;
  bed_slope = other.bed_slope;
  upstream_flows = other.upstream_flows;
  flow_sources = other.flow_sources;
  flow_sinks = other.flow_sinks;
  output_flows = other.output_flows;
  output_depths = other.output_depths;
  output_wsls = other.output_wsls;
  mm = other.mm;

  // Delete existing depthdf contents
  if (depthdf) {
    for (auto ptr : *depthdf) {
      delete ptr;
    }
    delete depthdf;
  }

  // Deep copy new depthdf contents
  if (other.depthdf) {
    depthdf = new std::vector<hydraulic_output *>();
    for (auto ptr : *other.depthdf) {
      depthdf->push_back(new hydraulic_output(*ptr));
    }
  } else {
    depthdf = nullptr;
  }

  return *this;
}

//////////////////////////////////////////////////////////////////
/// \brief Compute wsl based on parameters and streamnode member variables
///
/// \param flow [in] flow to be used in computations
/// \param slope [in] slope to be used in computations
/// \param init_wsl [in] initial water surface level to be used in computations
/// \param *&bbopt [in] Global model options information
/// \return wsl value computed
//
double CStreamnode::compute_normal_depth(double flow, double slope, double init_wsl, COptions *bbopt) {
  CStreamnode dupe = *this;
  if (init_wsl == -99) {
    init_wsl = dupe.mm->min_elev + 1;
  }
  
  dupe.compute_profile(flow, init_wsl, bbopt);
  dupe.mm->sf = slope;
  dupe.mm->sf_avg = slope;

  double err_lag1 = PLACEHOLDER,
         err_lag2 = PLACEHOLDER,
         prevWSL_lag1 = dupe.mm->wsl,
         prevWSL_lag2 = PLACEHOLDER;
  for (int i = 0; i < bbopt->iteration_limit_nd; i++) {
    if (!bbopt->silent_run && i % 10 == 0) {
      std::cout << "normal depth: iteration " << i + 1 << ": ----" << std::endl;
    }

    double max_depth_change;
    if (i == 0) {
      max_depth_change = 50;
    } else {
      prevWSL_lag2 = prevWSL_lag1;
      prevWSL_lag1 = dupe.mm->wsl;
      max_depth_change = 0.5 * (0.5 * (prevWSL_lag2 + prevWSL_lag1 - 2. * dupe.mm->min_elev));

      dupe.compute_profile(flow, dupe.mm->wsl, bbopt);
      dupe.mm->sf = slope;
      dupe.mm->sf_avg = slope;
    }

    double rhs = dupe.mm->k_total * std::sqrt(slope);
    double comp_wsl = dupe.mm->min_elev + ((dupe.mm->depth * flow) / rhs);

    if (comp_wsl <= dupe.mm->min_elev) {
      comp_wsl = dupe.mm->flow == 0
                    ? dupe.mm->min_elev
                    : std::max(dupe.mm->min_elev + 0.1, comp_wsl);
    }

    err_lag2 = err_lag1;
    err_lag1 = comp_wsl - prevWSL_lag1;
    double err_diff = err_lag2 == PLACEHOLDER ? PLACEHOLDER : err_lag2 - err_lag1;
    double assum_diff = prevWSL_lag2 == PLACEHOLDER ? PLACEHOLDER : prevWSL_lag2 - prevWSL_lag1;
    if (std::abs(err_lag1) > bbopt->tolerance_nd) {

      if (i + 1 == bbopt->iteration_limit_nd) {
        WriteWarning("Iteration limit on normal depth calculation exceeded, "
                     "terminating normal depth calculation.",
                     bbopt->noisy_run);
        break;
      }

      double proposed_wsl = PLACEHOLDER;
      if (i == 0) {
        proposed_wsl = dupe.mm->wsl + 0.7 * err_lag1;
        dupe.mm->wsl = std::abs(proposed_wsl - dupe.mm->wsl) > max_depth_change
                      ? dupe.mm->wsl + (proposed_wsl - dupe.mm->wsl) * (max_depth_change / std::abs(proposed_wsl - dupe.mm->wsl))
                      : proposed_wsl;
      } else {
        if (std::abs(err_diff) < 0.003 || i >= ((double)bbopt->iteration_limit_nd) / 2 || i >= 20) {
          proposed_wsl = std::abs(comp_wsl - prevWSL_lag1) > max_depth_change
                            ? prevWSL_lag1 + std::copysign(0.5 * max_depth_change, comp_wsl - prevWSL_lag1)
                            : (comp_wsl + prevWSL_lag1) / 2;
          dupe.mm->wsl = proposed_wsl;
        } else {
          proposed_wsl = prevWSL_lag2 - (err_lag2 * assum_diff) / err_diff;
          dupe.mm->wsl = std::abs(proposed_wsl - dupe.mm->wsl) > max_depth_change
                        ? dupe.mm->wsl + (proposed_wsl - dupe.mm->wsl) * (max_depth_change / std::abs(proposed_wsl - dupe.mm->wsl))
                        : proposed_wsl;
        }
      }
      dupe.mm->depth = dupe.mm->wsl - dupe.mm->min_elev;
    } else {
      if (!bbopt->silent_run) {
        std::cout << "Normal depth estimated successfully." << std::endl;
      }
      return dupe.mm->wsl;
    }
  }
  return dupe.mm->wsl;
}

//////////////////////////////////////////////////////////////////
/// \brief Compute basic depth properties with interpolation
///
/// \param wsl [in] wsl to be used in computations
/// \param *&bbopt [in] Global model options information
//
void CStreamnode::compute_basic_depth_properties_interpolation(double wsl, COptions*& bbopt) {
  ExitGracefullyIf(
      depthdf->size() == 0,
      "Streamnode.cpp: compute_basic_depth_properties_interpolation: depthdf "
      "has not been computed, please include it in a bbg file.",
      exitcode::BAD_DATA);
  ExitGracefullyIf(
      mm->nodeID != (*depthdf)[0]->nodeID ||
          mm->min_elev != (*depthdf)[0]->min_elev ||
          mm->reach_length_US1 != (*depthdf)[0]->reach_length_US1,
      "Streamnode.cpp: compute_basic_depth_properties_interpolation: check "
      "properties in interpolation, they do not match those in the provided mm "
      "structure",
      exitcode::BAD_DATA);
  std::vector<double> vec_depthdf_wsl = hyd_out_collect(&hydraulic_output::wsl, *depthdf);
  std::valarray<double> val_depthdf_wsl(vec_depthdf_wsl.data(), vec_depthdf_wsl.size());
  //std::cout << wsl << " | " << val_depthdf_wsl.min() << " | "
  //          << val_depthdf_wsl.max() << std::endl;
  if (wsl < val_depthdf_wsl.min() || wsl > val_depthdf_wsl.max()) {
    ExitGracefullyIf(
        !bbopt->extrapolate_depth_table,
        "Streamnode.cpp: compute_basic_depth_properties_interpolation: wsl "
        "provided is outside of the range in depthdf",
        exitcode::RUNTIME_ERR);
    WriteWarning(
        "Streamnode.cpp: compute_basic_depth_properties_interpolation: wsl "
        "provided (" +
            std::to_string(wsl) + ") is outside of the range in depthdf [" +
            std::to_string(val_depthdf_wsl.min()) + ", " +
            std::to_string(val_depthdf_wsl.max()) + "] in calculating flow " +
            std::to_string(mm->flow) +
            ",\nExtrapolating to continue computation.",
        bbopt->noisy_run);
    mm->depth = wsl - mm->min_elev;
    mm->k_total = extrapolate(wsl, &hydraulic_output::k_total, *depthdf);
    mm->alpha = extrapolate(wsl, &hydraulic_output::alpha, *depthdf);
    mm->area = extrapolate(wsl, &hydraulic_output::area, *depthdf);
    mm->hradius = extrapolate(wsl, &hydraulic_output::hradius, *depthdf);
    mm->wet_perimeter = extrapolate(wsl, &hydraulic_output::wet_perimeter, *depthdf);
    mm->manning_composite = extrapolate(wsl, &hydraulic_output::manning_composite, *depthdf);
    mm->length_effective = extrapolate(wsl, &hydraulic_output::length_effective, *depthdf);
    mm->hyd_depth = extrapolate(wsl, &hydraulic_output::hyd_depth, *depthdf);
    mm->top_width = extrapolate(wsl, &hydraulic_output::top_width, *depthdf);
    mm->k_total_areaconv = extrapolate(wsl, &hydraulic_output::k_total_areaconv, *depthdf);
    mm->k_total_disconv = extrapolate(wsl, &hydraulic_output::k_total_disconv, *depthdf);
    mm->k_total_roughconv = extrapolate(wsl, &hydraulic_output::k_total_roughconv, *depthdf);
    mm->alpha_areaconv = extrapolate(wsl, &hydraulic_output::alpha_areaconv, *depthdf);
    mm->alpha_disconv = extrapolate(wsl, &hydraulic_output::alpha_disconv, *depthdf);
    mm->alpha_roughconv = extrapolate(wsl, &hydraulic_output::alpha_roughconv, *depthdf);
    mm->nc_equalforce = extrapolate(wsl, &hydraulic_output::nc_equalforce, *depthdf);
    mm->nc_equalvelocity = extrapolate(wsl, &hydraulic_output::nc_equalvelocity, *depthdf);
    mm->nc_wavgwp = extrapolate(wsl, &hydraulic_output::nc_wavgwp, *depthdf);
    mm->nc_wavgarea = extrapolate(wsl, &hydraulic_output::nc_wavgarea, *depthdf);
    mm->nc_wavgconv = extrapolate(wsl, &hydraulic_output::nc_wavgconv, *depthdf);
  } else {
    mm->depth = wsl - mm->min_elev;
    mm->k_total = interpolate(wsl, &hydraulic_output::k_total, *depthdf);
    mm->alpha = interpolate(wsl, &hydraulic_output::alpha, *depthdf);
    mm->area = interpolate(wsl, &hydraulic_output::area, *depthdf);
    mm->hradius = interpolate(wsl, &hydraulic_output::hradius, *depthdf);
    mm->wet_perimeter = interpolate(wsl, &hydraulic_output::wet_perimeter, *depthdf);
    mm->manning_composite = interpolate(wsl, &hydraulic_output::manning_composite, *depthdf);
    mm->length_effective = interpolate(wsl, &hydraulic_output::length_effective, *depthdf);
    mm->hyd_depth = interpolate(wsl, &hydraulic_output::hyd_depth, *depthdf);
    mm->top_width = interpolate(wsl, &hydraulic_output::top_width, *depthdf);
    mm->k_total_areaconv = interpolate(wsl, &hydraulic_output::k_total_areaconv, *depthdf);
    mm->k_total_disconv = interpolate(wsl, &hydraulic_output::k_total_disconv, *depthdf);
    mm->k_total_roughconv = interpolate(wsl, &hydraulic_output::k_total_roughconv, *depthdf);
    mm->alpha_areaconv = interpolate(wsl, &hydraulic_output::alpha_areaconv, *depthdf);
    mm->alpha_disconv = interpolate(wsl, &hydraulic_output::alpha_disconv, *depthdf);
    mm->alpha_roughconv = interpolate(wsl, &hydraulic_output::alpha_roughconv, *depthdf);
    mm->nc_equalforce = interpolate(wsl, &hydraulic_output::nc_equalforce, *depthdf);
    mm->nc_equalvelocity = interpolate(wsl, &hydraulic_output::nc_equalvelocity, *depthdf);
    mm->nc_wavgwp = interpolate(wsl, &hydraulic_output::nc_wavgwp, *depthdf);
    mm->nc_wavgarea = interpolate(wsl, &hydraulic_output::nc_wavgarea, *depthdf);
    mm->nc_wavgconv = interpolate(wsl, &hydraulic_output::nc_wavgconv, *depthdf);
  }
  if (mm->length_effective <= 0) {
    ExitGracefully(
        ("Streamnode.cpp: compute_basic_depth_properties_interpolation: "
         "length_effective for " + std::to_string(nodeID) +
         " was computed to be non-positive. Cannot attain leff_ratio.")
            .c_str(),
        exitcode::BAD_DATA);
  }

  mm->length_effectiveadjusted = mm->length_effective;

  if (bbopt->roughness_multiplier != 1) {
    if (bbopt->roughness_multiplier == PLACEHOLDER ||
        bbopt->roughness_multiplier <= 0) {
      ExitGracefully("Streamnode.cpp: compute_basic_depth_properties_interpolation: "
                     "bbopt->roughness_multiplier must be a positive value.",
                     BAD_DATA);
    }
    mm->k_total /= bbopt->roughness_multiplier;
    mm->manning_composite *= bbopt->roughness_multiplier;
  }

  double reach_length = PLACEHOLDER;
  if (mm->reach_length_US2 != -99) {
    reach_length = mm->reach_length_US2;
  } else {
    reach_length = mm->reach_length_US1;
  }

  if (reach_length <= 0) {
    ExitGracefully(
        ("Streamnode.cpp: compute_basic_depth_properties_interpolation: "
         "reach_length for streamnode with nodeID " +
         std::to_string(nodeID) +
         " is a non-positive value. Cannot obtain depth properties")
            .c_str(),
        exitcode::BAD_DATA);
  }

  if (bbopt->enforce_delta_Leff) {
    if (mm->length_effective < reach_length * (1 - bbopt->delta_reachlength)) {
      std::cout << "Enforcing Leff on node with nodeID " + std::to_string(nodeID) << std::endl;
      mm->length_effectiveadjusted = reach_length * (1 - bbopt->delta_reachlength);
      double leff_ratio = mm->length_effectiveadjusted / mm->length_effective;

      if (bbopt->reach_integration_method == enum_ri_method::EFFECTIVE_LENGTH) {
        mm->area *= leff_ratio;
        mm->wet_perimeter *= leff_ratio;
        mm->k_total *= leff_ratio;
        mm->top_width *= leff_ratio;
        mm->hyd_depth *= leff_ratio;
      }
    } else if (mm->length_effective > reach_length * (1 + bbopt->delta_reachlength)) {
      std::cout << "Enforcing Leff on node with nodeID " + std::to_string(nodeID) << std::endl;
      mm->length_effectiveadjusted = reach_length * (1 + bbopt->delta_reachlength);
      double leff_ratio = mm->length_effectiveadjusted / mm->length_effective;

      if (bbopt->reach_integration_method == enum_ri_method::EFFECTIVE_LENGTH) {
        mm->area *= leff_ratio;
        mm->wet_perimeter *= leff_ratio;
        mm->k_total *= leff_ratio;
        mm->top_width *= leff_ratio;
        mm->hyd_depth *= leff_ratio;
      }
    }
  }

  if (bbopt->reach_integration_method == enum_ri_method::EFFECTIVE_LENGTH) {
    mm->area *= mm->length_effective / reach_length;
    mm->wet_perimeter *= mm->length_effective / reach_length;
    mm->k_total *= mm->length_effective / reach_length;
    mm->top_width *= mm->length_effective / reach_length;
    mm->hyd_depth *= mm->length_effective / reach_length;
  }

  return;
}

//////////////////////////////////////////////////////////////////
/// \brief Compute profile for streamnode
///
/// \param flow [in] flow to be used in computations
/// \param wsl [in] wsl to be used in computations
/// \param *&bbopt [in] Global model options information
//
void CStreamnode::compute_profile(double flow, double wsl, COptions *bbopt) {
  mm->flow = flow;
  mm->wsl = wsl;
  
  if (mm->flow <= bbopt->tolerance_nd) {
    WriteWarning("In computing hydraulic profile, flow is found to be zero or "
                 "within tolerance of zero. Setting depth estimate to zero.",
                 bbopt->noisy_run);
    mm->wsl = mm->min_elev;
  }

  if (mm->wsl < mm->min_elev) {
    mm->wsl = mm->min_elev;
  }

  mm->depth = mm->wsl - mm->min_elev;

  compute_basic_depth_properties_interpolation(mm->wsl, bbopt);
  compute_basic_flow_properties(mm->flow, bbopt);
}

//////////////////////////////////////////////////////////////////
/// \brief Compute profile for next streamnode
///
/// \param flow [in] flow to be used in computations
/// \param wsl [in] wsl to be used in computations
/// \param down_mm [in] mm of downstream node
/// \param *&bbopt [in] Global model options information
//
void CStreamnode::compute_profile_next(double flow, double wsl, hydraulic_output *down_mm, COptions *bbopt) {
  compute_profile(flow, wsl, bbopt);

  if (bbopt->reach_integration_method == enum_ri_method::EFFECTIVE_LENGTH) {
    double reach_length = 0;
    if (mm->reach_length_US2 != -99) {
      reach_length = mm->reach_length_US2;
    } else {
      reach_length = mm->reach_length_US1;
    }
    mm->sf *= std::pow(mm->length_effective / reach_length, 2.0);
  }

  if (bbopt->friction_slope_method == enum_fs_method::AVERAGE_CONVEYANCE) {
    if (mm->flow == PLACEHOLDER || down_mm->flow == PLACEHOLDER ||
        mm->k_total == PLACEHOLDER || down_mm->k_total == PLACEHOLDER) {
      mm->sf_avg = (mm->sf + down_mm->sf) / 2.;
    } else {
      mm->sf_avg = std::pow((mm->flow + down_mm->flow) / (mm->k_total + down_mm->k_total), 2.0);
    }
  } else if (bbopt->friction_slope_method == enum_fs_method::AVERAGE_FRICTION) {
    mm->sf_avg = (mm->sf + down_mm->sf) / 2.;
  } else if (bbopt->friction_slope_method == enum_fs_method::GEOMETRIC_FRICTION) {
    mm->sf_avg = std::sqrt(mm->sf * down_mm->sf);
  } else if (bbopt->friction_slope_method == enum_fs_method::HARMONIC_FRICTION) {
    mm->sf_avg = 2. * mm->sf * down_mm->sf / (mm->sf + down_mm->sf);
  } else if (bbopt->friction_slope_method == enum_fs_method::REACH_FRICTION) {
    if (bbopt->regimetype == enum_rt_method::SUBCRITICAL) {
      mm->sf_avg = down_mm->sf;
    } else {
      mm->sf_avg = mm->sf;
    }
  }

  double loss_coeff = 0;
  if (down_mm->velocity_head > mm->velocity_head) {
    loss_coeff = contraction_coeff;
  } else {
    loss_coeff = expansion_coeff;
  }

  if (bbopt->leff_method == enum_le_method::AVERAGE) {
    mm->length_energyloss = (mm->length_effectiveadjusted + down_mm->length_effectiveadjusted) / 2.;
  } else if (bbopt->leff_method == enum_le_method::DOWNSTREAM) {
    mm->length_energyloss = down_mm->length_effectiveadjusted;
  } else if (bbopt->leff_method == enum_le_method::UPSTREAM) {
    mm->length_energyloss = mm->length_effectiveadjusted;
  } else {
    ExitGracefully("Streamnode.cpp: compute_profile_next: unrecognized leff_method", exitcode::BAD_DATA);
  }

  mm->head_loss =
      mm->length_energyloss * mm->sf_avg +
      loss_coeff *
          std::abs(((mm->alpha * std::pow(mm->velocity, 2.) / 2.) / GRAVITY) -
                   ((down_mm->alpha * std::pow(down_mm->velocity, 2.) / 2.) / GRAVITY));
}


//////////////////////////////////////////////////////////////////
/// \brief Compute total energy for streamnode at junction
///
/// \param H [in] wsl value
/// \param *down_mm [in] mm of downstream node
/// \param *&bbopt [in] Global model options information
//
double CStreamnode::get_total_energy(double H, hydraulic_output *down_mm, COptions *&bbopt) {
  compute_profile_next(mm->flow, H, down_mm, bbopt);
  return energy_calc(mm->min_elev, mm->depth, mm->velocity, GRAVITY);
}

//////////////////////////////////////////////////////////////////
/// \brief Add row to depthdf
///
/// \param row [in] row to be added
//
void CStreamnode::add_depthdf_row(hydraulic_output*& row) {
  depthdf->push_back(row);
  depthdf_map[row->depth] = depthdf->size() - 1;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns row of depthdf with depth 'depth'
///
/// \param depth [in] depth of desired row
/// \return row of depthdf with depth 'depth'
//
hydraulic_output* CStreamnode::get_depthdf_row_from_depth(double depth) {
  return depthdf_map.find(depth) != depthdf_map.end() ? depthdf->at(depthdf_map[depth]) : NULL;
}

//////////////////////////////////////////////////////////////////
/// \brief Add steady flow to CStreamnode
///
/// \param flow [in] flow associated with flowprofile
//
void CStreamnode::add_steadyflow(double flow) {
  allocate_flowprofiles(output_flows.size() + 1);
  output_flows.back() = flow;
  upstream_flows.back() = HEADWATER;
}

//////////////////////////////////////////////////////////////////
/// \brief Adds source and sink at index
///
/// \param index [in] index at which to add source and sink
/// \param source [in] value of flow at source
/// \param sink [in] value of flow at sink
//
void CStreamnode::add_sourcesink(int index, double source, double sink) {
  allocate_flowprofiles(index + 1);
  flow_sources[index] = source;
  flow_sinks[index] = sink;
}

//////////////////////////////////////////////////////////////////
/// \brief Calculates the output flows of the node based on upstream flows, sources, and sinks
///
/// \param upflows [in] flow value contributed by upstream nodes
//
void CStreamnode::calc_output_flows(std::vector<double> upflows) {
  allocate_flowprofiles(upflows.size());
  for (int k = 0; k < upflows.size(); k++) {
    upstream_flows[k] = upflows[k];
    output_flows[k] = upflows[k] + flow_sources[k] - flow_sinks[k];
  }
}

//////////////////////////////////////////////////////////////////
/// \brief If necessary, allocates enough space in corresponding variables for num_fp flow profiles
///
/// \param num_fp [in] number of flow profiles needed
//
void CStreamnode::allocate_flowprofiles(int num_fp) {
  ExitGracefullyIf(num_fp == PLACEHOLDER, "Streamnode.cpp: ERROR num_fp was not assigned or is PLACEHOLDER", RUNTIME_ERR);
  while (upstream_flows.size() < num_fp) {
    upstream_flows.push_back(PLACEHOLDER);
  }
  while (flow_sources.size() < num_fp) {
    flow_sources.push_back(0);
  }
  while (flow_sinks.size() < num_fp) {
    flow_sinks.push_back(0);
  }
  while (output_flows.size() < num_fp) {
    output_flows.push_back(PLACEHOLDER);
  }
  while (output_depths.size() < num_fp) {
    output_depths.push_back(PLACEHOLDER);
  }
  while (output_wsls.size() < num_fp) {
    output_wsls.push_back(PLACEHOLDER);
  }
}

// Destructor
CStreamnode::~CStreamnode() {
  for (std::vector<hydraulic_output *>::iterator i = depthdf->begin(); i != depthdf->end();
       i++) {
    delete (*i);
    *i = nullptr;
  }
  for (auto ptr : *depthdf) {
    delete ptr;
  }
  delete depthdf;
  depthdf = nullptr;
  delete mm;
  mm = nullptr;
}