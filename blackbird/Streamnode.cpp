#include "Model.h"
#include "Streamnode.h"
#include "BlackbirdInclude.h"
#include<string>

// Default constructor
CStreamnode::CStreamnode()
  : nodeID(0), nodetype(""), downnodeID(0), upnodeID1(0), upnodeID2(0),
  stationname(""), station(0.0), reachID(0), contraction_coeff(0.0),
  expansion_coeff(0.0), ds_reach_length(0.0), us_reach_length1(0.0),
  us_reach_length2(0.0), min_elev(0.0), bed_slope(0.0), output_depth(0.0), output_flow(0.0) {
  // Default constructor implementation
}

// Generate initial hydraulic profile
hydraulic_output CStreamnode::generate_initial_hydraulic_profile() {
  hydraulic_output output;
  // Logic to generate initial hydraulic profile
  return output;
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
bool CStreamnode::add_depthdf_row(hydraulic_output*& row) {
  depthdf->push_back(row);
  depthdf_map[row->depth] = depthdf->size() - 1;
  return true;
}

// Returns row of depthdf with depth 'depth'
hydraulic_output* CStreamnode::get_depthdf_row_from_depth(double depth) {
  return depthdf_map.find(depth) != depthdf_map.end() ? depthdf->at(depthdf_map[depth]) : NULL;
}

// Function to add flowprofile
bool CStreamnode::add_steadyflow(double flow) {
  steady_flows.push_back(flow);
  flow_sources.push_back(0);
  flow_sinks.push_back(0);
  sourcesink_map[flow] = steady_flows.size();
  num_fp++;
  return true;
}

// Returns number of flowprofiles
int CStreamnode::get_num_steadyflows() {
  return num_fp;
}

bool CStreamnode::add_sourcesink(int index, double source, double sink) {
  if (index < num_fp) {
    flow_sources[index] = source;
    flow_sinks[index] = sink;
    return true;
  }
  else {
    return false;
  }
}

std::vector<double> CStreamnode::get_sourcesink_from_steadyflow(double steadyflow) {
  if (sourcesink_map.find(steadyflow) == sourcesink_map.end()) {
    return std::vector<double>(NULL);
  }
  double source = flow_sources.at(sourcesink_map[steadyflow]);
  double sink = flow_sinks.at(sourcesink_map[steadyflow]);
  return std::vector<double>({ source, sink });
}