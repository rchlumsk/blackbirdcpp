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
  double contraction_coeff;
  double expansion_coeff;
  double ds_reach_length;
  double us_reach_length1;
  double us_reach_length2;
  std::vector<hydraulic_output*> *depthdf;
  double min_elev;
  double bed_slope;
  double flow_source;
  double flow_sink;
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
};

#endif