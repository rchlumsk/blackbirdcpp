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
  output_depth(PLACEHOLDER),
  output_flows(),
  mm(new hydraulic_output) {
}

// Compute preprocessed depthdf
void CStreamnode::compute_preprocessed_depthdf() {
  // Logic to compute preprocessed depthdf
}

// Compute normal depth
void CStreamnode::compute_normal_depth(double flow, double slope, double init_wsl, COptions *bbopt) {
  if (init_wsl == -99) {
    if (nodetype == enum_nodetype::XSECTION) {
      double area_req = flow / 3;
      // optimization placeholder
      init_wsl = mm->min_elev + 1;
    } else {
      init_wsl = mm->min_elev + 1;
    }
  }

  compute_profile(flow, init_wsl, bbopt);
  mm->sf = slope;
  mm->sf_avg = slope;

  double err_lag1 = PLACEHOLDER,
         err_lag2 = PLACEHOLDER,
         prevWSL_lag1 = mm->wsl,
         prevWSL_lag2 = PLACEHOLDER;
  for (int i = 0; i < bbopt->iteration_limit_nd; i++) {
    if (!bbopt->silent_cp && i % 10 == 0) {
      std::cout << "normal depth: iteration " << i + 1 << ": ----" << std::endl;
    }

    double max_depth_change;
    if (i == 0) {
      max_depth_change = 50;
    } else {
      prevWSL_lag2 = prevWSL_lag1;
      prevWSL_lag1 = mm->wsl;
      max_depth_change = 0.5 * (0.5 * (prevWSL_lag2 + prevWSL_lag1 - 2 * mm->min_elev));

      compute_profile(flow, init_wsl, bbopt);
      mm->sf = slope;
      mm->sf_avg = slope;
    }

    double rhs = mm->k_total * std::sqrt(slope);
    double comp_wsl = mm->min_elev + ((mm->depth * flow) / rhs);

    if (comp_wsl <= mm->min_elev) {
      comp_wsl = mm->flow == 0
                    ? mm->min_elev
                    : std::max(mm->min_elev + 0.1, comp_wsl);
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
        proposed_wsl = mm->wsl + 0.7 * err_lag1;
        mm->wsl = std::abs(proposed_wsl - mm->wsl) > max_depth_change
                      ? mm->wsl + (proposed_wsl - mm->wsl) * (max_depth_change / std::abs(proposed_wsl - mm->wsl))
                      : proposed_wsl;
      } else {
        if (std::abs(err_diff) < 0.003 || i >= ((double)bbopt->iteration_limit_nd) / 2 || i >= 20) {
          proposed_wsl = std::abs(comp_wsl - prevWSL_lag1) > max_depth_change
                            ? prevWSL_lag1 + std::copysign(0.5 * max_depth_change, comp_wsl - prevWSL_lag1)
                            : (comp_wsl + prevWSL_lag1) / 2;
          mm->wsl = proposed_wsl;
        } else {
          proposed_wsl = prevWSL_lag2 - (err_lag2 * assum_diff) / err_diff;
          mm->wsl = std::abs(proposed_wsl - mm->wsl) > max_depth_change
                        ? mm->wsl + (proposed_wsl - mm->wsl) * (max_depth_change / std::abs(proposed_wsl - mm->wsl))
                        : proposed_wsl;
        }
      }
      mm->depth = mm->wsl - mm->min_elev;
    } else {
      if (!bbopt->silent_nd) {
        std::cout << "Normal depth estimated successfully." << std::endl;
      }
    }
  }
}

// Compute basic depth properties with interpolation
void CStreamnode::compute_basic_depth_properties_interpolation(double wsl, COptions*& bbopt) {
  ExitGracefullyIf(
      depthdf->size() == 0,
      "Streamnode.cpp: compute_basic_depth_properties_interpolation: depthdf "
      "has not been computed, please run compute_preprocessed_depthdf first.",
      exitcode::BAD_DATA);
  ExitGracefullyIf(
      mm->stationname != (*depthdf)[0]->stationname,
      "Streamnode.cpp: compute_basic_depth_properties_interpolation: check "
      "properties in interpolation, they do not match those in the provided mm "
      "structure",
      exitcode::BAD_DATA);
  std::vector<double> vec_depthdf_wsl = hyd_out_collect(&hydraulic_output::wsl, *depthdf);
  std::valarray<double> val_depthdf_wsl(vec_depthdf_wsl.data(), vec_depthdf_wsl.size());
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
    mm->depth = wsl - mm->min_elev; // pointless?
    mm->depth = extrapolate(wsl, &hydraulic_output::depth, *depthdf);
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
    mm->depth = wsl - mm->min_elev; // pointless?
    mm->depth = interpolate(wsl, &hydraulic_output::depth, *depthdf);
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

  mm->length_effectiveadjusted = mm->length_effective;

  return;
}

// Compute profile
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

  // assuming always use_preproc ?
  compute_basic_depth_properties_interpolation(mm->wsl, bbopt);
  compute_basic_flow_properties(mm->flow, bbopt);
}

// Compute next profile
void CStreamnode::compute_profile_next(double flow, double wsl, hydraulic_output *down_mm, COptions *bbopt) {
  compute_profile(flow, wsl, bbopt);

  if (bbopt->reach_integration_method == enum_ri_method::EFFECTIVE_LENGTH) {
    double reach_length = 0;
    if (mm->reach_length_US2 != PLACEHOLDER) {
      reach_length = mm->reach_length_US2;
    } else {
      reach_length = mm->reach_length_US1;
    }
    mm->sf *= std::pow(mm->length_effective / 2.0, 2.0);
  }

  if (bbopt->friction_slope_method == enum_fs_method::AVERAGE_CONVEYANCE) {
    if (mm->flow == PLACEHOLDER || down_mm->flow == PLACEHOLDER ||
        mm->k_total == PLACEHOLDER || down_mm->k_total == PLACEHOLDER) {
      mm->sf_avg = (mm->sf + down_mm->sf) / 2;
    } else {
      mm->sf_avg = std::pow((mm->flow + down_mm->flow) / (mm->k_total + down_mm->k_total), 2.0);
    }
  } else if (bbopt->friction_slope_method == enum_fs_method::AVERAGE_FRICTION) {
    mm->sf_avg = (mm->sf + down_mm->sf) / 2;
  } else if (bbopt->friction_slope_method == enum_fs_method::GEOMETRIC_FRICTION) {
    mm->sf_avg = std::sqrt(mm->sf * down_mm->sf);
  } else if (bbopt->friction_slope_method == enum_fs_method::HARMONIC_FRICTION) {
    mm->sf_avg = 2 * mm->sf * down_mm->sf / (mm->sf + down_mm->sf);
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

  // no Leff method check? assuming us_leff (default)
  mm->length_energyloss = mm->length_effectiveadjusted;

  mm->head_loss =
      mm->length_energyloss * mm->sf_avg +
      loss_coeff *
          std::abs(((mm->alpha * std::pow(mm->velocity, 2) / 2) / GRAVITY) -
                   ((down_mm->alpha * std::pow(down_mm->velocity, 2) / 2) / GRAVITY));
}

// Function to add row to depthdf
void CStreamnode::add_depthdf_row(hydraulic_output*& row) {
  depthdf->push_back(row);
  depthdf_map[row->depth] = depthdf->size() - 1;
}

// Returns row of depthdf with depth 'depth'
hydraulic_output* CStreamnode::get_depthdf_row_from_depth(double depth) {
  return depthdf_map.find(depth) != depthdf_map.end() ? depthdf->at(depthdf_map[depth]) : NULL;
}

// Function to add flowprofile
void CStreamnode::add_steadyflow(double flow) {
  output_flows.push_back(flow);
  upstream_flows.push_back(HEADWATER);
}

// Adds source and sink at index
void CStreamnode::add_sourcesink(int index, double source, double sink) {
  allocate_flowprofiles(index + 1);
  flow_sources[index] = source;
  flow_sinks[index] = sink;
}

// Calculates the output flows of the node
void CStreamnode::calc_output_flows(std::vector<double> upflows) {
  allocate_flowprofiles(upflows.size());
  for (int k = 0; k < upflows.size(); k++) {
    upstream_flows[k] = upflows[k];
    output_flows[k] = upflows[k] + flow_sources[k] - flow_sinks[k];
  }
}

// If necessary, allocates enough space in corresponding variables for num_fp flow profiles
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
}

// Destructor
CStreamnode::~CStreamnode() {
  for (std::vector<hydraulic_output *>::iterator i = depthdf->begin(); i != depthdf->end();
       i++) {
    delete (*i);
    *i = nullptr;
  }
  depthdf->clear();
  depthdf->shrink_to_fit();
  delete depthdf;
  depthdf = nullptr;
  delete mm;
  mm = nullptr;
}