#include "BlackbirdInclude.h"
#include "Raster.h"

// Default constructor
CRaster::CRaster()
  : CGriddedData() {
}

// Copy constructor
CRaster::CRaster(const CRaster &other)
  : CGriddedData(other) {
}

// Copy assignment operator
CRaster& CRaster::operator=(const CRaster &other) {
  if (this == &other) {
    return *this; // Handle self-assignment
  }

  CGriddedData::operator=(other); // Copy base class members

  return *this;
}