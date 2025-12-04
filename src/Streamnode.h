#ifndef STREAMNODE_H
#define STREAMNODE_H

#include <string>
#include "BlackbirdInclude.h"
#include "Options.h"

class CStreamnode {
public:
  // Member variables
  int nodeID;                               // nodeID of streamnode
  enum_nodetype nodetype;                   // type of streamnode. options: REACH, XSECTION
  int downnodeID;                           // nodeID of downstream node
  int upnodeID1;                            // nodeID of first upstream node
  int upnodeID2;                            // nodeID of second upstream node (if applicable)
  std::string stationname;                  // string containing name of station name of streamnode
  double station;                           // station number
  int reachID;                              // reachID of reach containing streamnode
  double ds_reach_length;                   // length of downstream river section towards downnodeID
  double us_reach_length1;                  // length of upstreamn river section towards upnodeID1
  double us_reach_length2;                  // length of upstream river section towards upnodeID2 (if applicable)
  double contraction_coeff;                 // contraction coefficient for use in compouting streamnode properties
  double expansion_coeff;                   // expansion coefficent for use in computing streamnode properties
  double min_elev;                          // minimum elevation of streamnode
  double bed_slope;                         // bed slope of streamnode
  std::vector<hydraulic_output*> *depthdf;  // contains data from the depthdf extracted from input files
  std::vector<double> upstream_flows;       // combined flows from upstream nodes w/o source/sink
  std::vector<double> flow_sources;         // flow sources to be added to upstream_flows
  std::vector<double> flow_sinks;           // flow sinks to be subtracted from upstream_flows
  std::vector<double> output_flows;         // flows of streamnode w/ source/sink included for each flow profile
  std::vector<double> output_depths;        // depths of streamnode for each flow profile
  std::vector<double> output_wsls;          // water surface levels of streamnode for each flow profile

  // For internal use
  hydraulic_output *mm;

  // Constructors and destructor
  CStreamnode();
  CStreamnode(const CStreamnode &other);
  ~CStreamnode();

  // Copy assignment operator
  CStreamnode &operator=(const CStreamnode &other);

  // Functions
  double compute_normal_depth(double flow, double slope, double init_wsl, COptions *bbopt);           // compute wsl based on parameters and streamnode member variables
  void compute_basic_depth_properties_interpolation(double wsl, COptions*& bbopt);                    // compute basic depth properties with interpolation
  void compute_profile(double flow, double wsl, COptions *bbopt);                                     // compute profile for streamnode
  void compute_profile_next(double flow, double wsl, hydraulic_output *down_mm, COptions *bbopt);     // compute profile for next streamnode
  double get_total_energy(double H, hydraulic_output *down_mm, COptions *&bbopt);                     // compute total energy for streamnode at junction

  void add_depthdf_row(hydraulic_output*& row);                                                       // add hydraulic_output row to depthdf
  hydraulic_output* get_depthdf_row_from_depth(double depth);                                         // get row of depthdf using the depth of the row

  void add_steadyflow(double flow);                                                                   // add a steadyflow condition to streamnode
  void add_sourcesink(int index, double source, double sink);                                         // add source and sink pair to streamnode

  void calc_output_flows(std::vector<double> upflows);                                                // calculate output flows of streamnode

  void pretty_print() const; // defined in StandardOutput.cpp

  // Virtual Functions
  virtual void compute_basic_depth_properties(double wsl, COptions *&bbopt) {};                       // compute basic streamnode depth properties
  virtual void compute_basic_flow_properties(double flow, COptions *&bbopt) {};                       // compute basic streamnode flow properties


protected:
  // Private variables
  std::unordered_map<double, int> depthdf_map;  // unordered map relating depths to row indexes of depthdf

  // Private functions
  void allocate_flowprofiles(int num_fp);   // if needed, allocates space in flowprofile related variables
};

#endif