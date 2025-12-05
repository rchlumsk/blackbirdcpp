#include "BlackbirdInclude.h"
#include "Raster.h"

// Default constructor
CRaster::CRaster()
  : CGriddedData(),
  proj(nullptr),
  datatype(GDT_Unknown) {
  std::fill(std::begin(geotrans), std::end(geotrans), PLACEHOLDER);
}

// Copy constructor
CRaster::CRaster(const CRaster &other)
  : CGriddedData(other),
  datatype(other.datatype) {

  std::copy(std::begin(other.geotrans), std::end(other.geotrans), std::begin(geotrans));
  if (other.proj != nullptr) {
    size_t len = strlen(other.proj) + 1;
    proj = new char[len];
    memcpy(proj, other.proj, len);
  } else {
    proj = nullptr;
  }
}

// Copy assignment operator
CRaster& CRaster::operator=(const CRaster &other) {
  if (this == &other) {
    return *this; // Handle self-assignment
  }

  CGriddedData::operator=(other); // Copy base class members
  datatype = other.datatype;
  std::copy(std::begin(other.geotrans), std::end(other.geotrans), std::begin(geotrans));

  delete[] proj;
  if (other.proj != nullptr) {
    size_t len = strlen(other.proj) + 1;
    proj = new char[len];
    memcpy(proj, other.proj, len);
  } else {
    proj = nullptr;
  }

  return *this;
}

CRaster::~CRaster() {
  if (proj) {
    delete[] proj;
  }
}
