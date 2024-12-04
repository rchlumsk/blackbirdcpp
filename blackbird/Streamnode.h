#ifndef STREAMNODE_H
#define STREAMNODE_H

#include <string>
#include "BlackbirdInclude.h"

class CStreamnode {
public:
  // Member variables
  int nodeID;
  std::string nodetype;
  int downnodeID;
  int upnodeID1;
  int upnodeID2;
  std::string stationname;
  double station;
  int reachID;
  double ds_reach_length;
  double us_reach_length1;
  double us_reach_length2;
  double contraction_coeff;
  double expansion_coeff;
  double min_elev;
  double bed_slope;
  std::vector<hydraulic_output*> *depthdf;
  std::vector<double> steady_flows;
  std::vector<double> flow_sources;
  std::vector<double> flow_sinks;
  double output_depth;
  double output_flow;

  // Constructor
  CStreamnode();

  // Functions
  hydraulic_output generate_initial_hydraulic_profile();
  void compute_preprocessed_depthdf();
  hydraulic_output compute_normal_depth();
  hydraulic_output compute_basic_depth_properties_interpolation();
  hydraulic_output compute_profile();
  hydraulic_output compute_profile_next();

  bool add_depthdf_row(hydraulic_output*& row);
  hydraulic_output* get_depthdf_row_from_depth(double depth);

  bool add_steadyflow(double flow);
  int get_num_steadyflows();
  bool add_sourcesink(int index, double source, double sink);
  std::vector<double> get_sourcesink_from_steadyflow(double steadyflow);


private:
  // Private variables
  std::unordered_map<double, int> depthdf_map;
  int num_fp;
  std::unordered_map<double, double> sourcesink_map;
};

#endif