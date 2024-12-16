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
  output_flows() {
}

// Compute preprocessed depthdf
void CStreamnode::compute_preprocessed_depthdf() {
  // Logic to compute preprocessed depthdf
}

// Compute normal depth
hydraulic_output CStreamnode::compute_normal_depth() {
  hydraulic_output output;
  // Logic to compute normal depth
  return output;
}

// Compute basic depth properties with interpolation
hydraulic_output CStreamnode::compute_basic_depth_properties_interpolation() {
  hydraulic_output output;
  // Logic to compute basic depth properties with interpolation
  return output;
}

// Compute profile
hydraulic_output CStreamnode::compute_profile() {
  hydraulic_output output;
  // Logic to compute profile
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
