#include "BlackbirdInclude.h"
#include "Raster.h"

// Default constructor
CRaster::CRaster()
  : name(PLACEHOLDER_STR),
  data(nullptr),
  xsize(PLACEHOLDER),
  ysize(PLACEHOLDER),
  proj(nullptr),
  na_val(PLACEHOLDER),
  datatype(GDT_Unknown) {
  // Default constructor implementation
  for (int i = 0; i < std::size(geotrans); i++) {
    geotrans[i] = PLACEHOLDER;
  }
}

// Copy constructor
CRaster::CRaster(const CRaster &other)
  : name(other.name),
  data(static_cast<double *>(CPLMalloc(sizeof(double) * other.xsize * other.ysize))),
  xsize(other.xsize),
  ysize(other.ysize),
  geotrans(),
  na_val(other.na_val),
  datatype(other.datatype) {
  for (int i = 0; i < std::size(geotrans); i++) {
    geotrans[i] = other.geotrans[i];
  }
  std::copy(other.data, other.data + other.xsize * other.ysize, data);
  if (other.proj != nullptr) {
    size_t len = strlen(other.proj) + 1;
    proj = new char[len];
    memcpy(proj, other.proj, len);
  } else {
    proj = nullptr;
  }
}

// Copy assignment operator
CRaster &CRaster::operator=(const CRaster &other) {
  if (this == &other)
    return *this; // Handle self-assignment

  if (data) {
    CPLFree(data);
  }

  name = other.name;
  xsize = other.xsize;
  ysize = other.ysize;
  proj = other.proj;
  na_val = other.na_val;
  datatype = other.datatype;

  for (int i = 0; i < std::size(geotrans); i++) {
    geotrans[i] = other.geotrans[i];
  }

  data = static_cast<double *>(CPLMalloc(sizeof(double) * xsize * ysize));
  std::copy(other.data, other.data + xsize * ysize, data);

  if (other.proj != nullptr) {
    size_t len = strlen(other.proj) + 1;
    proj = new char[len];
    memcpy(proj, other.proj, len);
  } else {
    proj = nullptr;
  }

  return *this;
}

// Destructor
CRaster::~CRaster()
{
  if (data) {
    CPLFree(data);
    data = nullptr;
  }
  if (proj) {
    delete[] proj;
  }
}