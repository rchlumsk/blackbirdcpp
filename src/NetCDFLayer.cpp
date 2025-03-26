#include "BlackbirdInclude.h"
#include "NetCDFLayer.h"

// Default constructor
CNetCDF::CNetCDF()
  : CGriddedData() {
}

// Copy constructor
CNetCDF::CNetCDF(const CNetCDF &other)
  : CGriddedData(other) {
}

// Copy assignment operator
CNetCDF& CNetCDF::operator=(const CNetCDF &other) {
  if (this == &other) {
    return *this; // Handle self-assignment
  }

  CGriddedData::operator=(other); // Copy base class members

  return *this;
}