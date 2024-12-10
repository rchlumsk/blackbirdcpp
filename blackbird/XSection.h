#ifndef XSECTION_H
#define XSECTION_H

#include "Streamnode.h"

class CXSection : public CStreamnode {
public:
  // Member variables specific to CXSection
  double xx;
  double zz;
  double manning;
  double manning_LOB;
  double manning_main;
  double manning_ROB;
  double lbs_xx;
  double rbs_xx;
  double ds_length_LOB;
  double ds_length_main;
  double ds_length_ROB;

  // Constructor
  CXSection();

  // Functions
  double calc_min_elev();
  hydraulic_output compute_basic_depth_properties();
  hydraulic_output compute_basic_flow_properties();
  double calculate_flow_area();
};

#endif