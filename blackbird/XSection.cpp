#include "XSection.h"

// Default constructor
CXSection::CXSection()
  : CStreamnode(),
  xx(PLACEHOLDER),
  zz(PLACEHOLDER),
  manning(PLACEHOLDER),
  manning_LOB(PLACEHOLDER),
  manning_main(PLACEHOLDER),
  manning_ROB(PLACEHOLDER),
  lbs_xx(PLACEHOLDER),
  rbs_xx(PLACEHOLDER),
  ds_length_LOB(PLACEHOLDER),
  ds_length_main(PLACEHOLDER),
  ds_length_ROB(PLACEHOLDER) {
}

// Calculate minimum elevation
double CXSection::calc_min_elev() {
  return min_elev;
}

// Compute basic depth properties
hydraulic_output CXSection::compute_basic_depth_properties() {
  hydraulic_output output;
  return output;
}

// Compute basic flow properties
hydraulic_output CXSection::compute_basic_flow_properties() {
  hydraulic_output output;
  return output;
}

// Calculate flow area
double CXSection::calculate_flow_area() {
  return xx * zz;
}