#include "BlackbirdInclude.h"
#include "Model.h"
#include "Streamnode.h"
#include<string>

// Default constructor
CStreamnode::CStreamnode()
  : nodeID(PLACEHOLDER),
  nodetype(PLACEHOLDER_STR),
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
hydraulic_output CStreamnode::compute_normal_depth(double flow, double slope, double init_wsl, COptions *bbopt) {
  if (init_wsl == -99) {
    if (nodetype == "xsection") { // maybe change to type check
      double area_req = flow / 3;
      // brent optimization for init_wsl
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

      compute_profile();
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
        std::cout << "Norma depth estimated successfully." << std::endl;
      }
    }
  }

  //temp
  hydraulic_output output;
  return output;
}

// Compute basic depth properties with interpolation
hydraulic_output CStreamnode::compute_basic_depth_properties_interpolation() {
  hydraulic_output output;
  // Logic to compute basic depth properties with interpolation
  return output;
}

// Compute profile
hydraulic_output CStreamnode::compute_profile(double flow, double wsl, COptions *bbopt) {
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
  compute_basic_depth_properties_interpolation();
  compute_basic_flow_properties();

  //temp
  hydraulic_output output;
  return output;
}

// Compute next profile
hydraulic_output CStreamnode::compute_profile_next() {
  hydraulic_output output;
  // Logic to compute the next profile
  return output;
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
