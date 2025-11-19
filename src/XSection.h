#ifndef XSECTION_H
#define XSECTION_H

#include "Streamnode.h"

class CXSection : public CStreamnode {
public:
  // Member variables specific to CXSection
  std::valarray <double> xx;
  std::valarray <double> zz;
  std::valarray <double> manning;
  double manning_LOB;
  double manning_main;
  double manning_ROB;
  double lbs_xx;
  double rbs_xx;
  double ds_length_LOB;
  double ds_length_main;
  double ds_length_ROB;

  // Constructors
  CXSection();
  CXSection(const CXSection &other);

  // Copy assignment operator
  CXSection &operator=(const CXSection &other);

  // Functions
  double calc_min_elev();
  void compute_basic_depth_properties(double wsl, COptions *&bbopt);
  void compute_basic_flow_properties(double flow, COptions *&bbopt);

  void pretty_print() const; // defined in StandardOutput.cpp
};

#endif