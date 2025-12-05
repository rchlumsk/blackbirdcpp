#ifndef XSECTION_H
#define XSECTION_H

#include "Streamnode.h"

class CXSection : public CStreamnode {
public:
  // Member variables specific to CXSection
  std::valarray <double> xx;
  std::valarray <double> zz;
  std::valarray <double> manning;
  double manning_LOB;                 // manning's n value for left overbank
  double manning_main;                // manning's n value for main channel
  double manning_ROB;                 // manning's n value for right overbank
  double lbs_xx;
  double rbs_xx;
  double ds_length_LOB;               // downstream length along left overbank
  double ds_length_main;              // downstream length along main channel
  double ds_length_ROB;               // downstream length along right overbank

  // Constructors
  CXSection();
  CXSection(const CXSection &other);

  // Copy assignment operator
  CXSection &operator=(const CXSection &other);

  // Functions
  void compute_basic_depth_properties(double wsl, COptions *&bbopt); // computes depth properties for xsection
  void compute_basic_flow_properties(double flow, COptions *&bbopt); // computes flow properties for xsection

  void pretty_print() const; // defined in StandardOutput.cpp
};

#endif
