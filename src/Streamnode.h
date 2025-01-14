#ifndef STREAMNODE_H
#define STREAMNODE_H

#include <string>
#include "BlackbirdInclude.h"
#include "Options.h"

class CStreamnode {
public:
  // Member variables
  int nodeID;
  enum_nodetype nodetype;
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
  std::vector<hydraulic_output*> *depthdf;  // contains data from the depthdf extracted from input files
  std::vector<double> upstream_flows;       // combined flows from upstream nodes w/o source/sink
  std::vector<double> flow_sources;         // flow sources to be added to upstream_flows
  std::vector<double> flow_sinks;           // flow sinks to be subtracted from upstream_flows
  double output_depth;
  std::vector<double> output_flows;         // flow of streamnode w/ source/sink included

  // For internal use
  hydraulic_output *mm;

  // Constructor and destructor
  CStreamnode();
  ~CStreamnode();

  // Functions
  void compute_preprocessed_depthdf();
  void compute_normal_depth(double flow, double slope, double init_wsl, COptions *bbopt);
  void compute_basic_depth_properties_interpolation(double wsl, COptions*& bbopt);
  void compute_profile(double flow, double wsl, COptions *bbopt);
  void compute_profile_next(double flow, double wsl, hydraulic_output *down_mm, COptions *bbopt);

  void add_depthdf_row(hydraulic_output*& row);
  hydraulic_output* get_depthdf_row_from_depth(double depth);

  void add_steadyflow(double flow);
  void add_sourcesink(int index, double source, double sink);

  void calc_output_flows(std::vector<double> upflows);

  void pretty_print() const; // defined in StandardOutput.cpp

  // Virtual Functions
  virtual void compute_basic_depth_properties(double wsl, COptions *&bbopt) {};
  virtual void compute_basic_flow_properties(double flow, COptions *&bbopt) {}; 


protected:
  // Private variables
  std::unordered_map<double, int> depthdf_map;

  // Private functions
  void allocate_flowprofiles(int num_fp);   // if needed, allocates space in flowprofile related variables
};

#endif