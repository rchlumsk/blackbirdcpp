#include "BlackbirdInclude.h"
#include "Raster.h"

// Default constructor
CRaster::CRaster()
  : data(nullptr),
  xsize(PLACEHOLDER),
  ysize(PLACEHOLDER),
  proj(PLACEHOLDER_STR.c_str()),
  geotrans(),
  na_val(PLACEHOLDER),
  datatype(GDT_Float64) {
  // Default constructor implementation
  for (int i = 0; i < std::size(geotrans); i++) {
    geotrans[i] = PLACEHOLDER;
  }
}

// Copy constructor
CRaster::CRaster(const CRaster &other)
  : data(static_cast<double *>(CPLMalloc(sizeof(double) * other.xsize * other.ysize))),
  xsize(other.xsize),
  ysize(other.ysize),
  proj(other.proj),
  geotrans(),
  na_val(other.na_val),
  datatype(other.datatype) {
  for (int i = 0; i < std::size(geotrans); i++) {
    geotrans[i] = other.geotrans[i];
  }
  std::copy(other.data, other.data + other.xsize * other.ysize, data);
}

// Destructor
CRaster::~CRaster()
{
  if (data) {
    CPLFree(data);
    data = nullptr;
  }
}