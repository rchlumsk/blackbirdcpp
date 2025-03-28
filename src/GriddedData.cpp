#include "BlackbirdInclude.h"
#include "GriddedData.h"

// Default constructor
CGriddedData::CGriddedData()
  : name(PLACEHOLDER_STR),
  data(nullptr),
  xsize(PLACEHOLDER),
  ysize(PLACEHOLDER),
  na_val(PLACEHOLDER){
}

// Copy constructor
CGriddedData::CGriddedData(const CGriddedData &other)
  : name(other.name),
  data(static_cast<double *>(CPLMalloc(sizeof(double) * other.xsize * other.ysize))),
  xsize(other.xsize),
  ysize(other.ysize),
  na_val(other.na_val){
  std::copy(other.data, other.data + other.xsize * other.ysize, data);
}

// Copy assignment operator
CGriddedData &CGriddedData::operator=(const CGriddedData &other) {
  if (this == &other)
    return *this; // Handle self-assignment

  if (data) {
    CPLFree(data);
  }

  name = other.name;
  xsize = other.xsize;
  ysize = other.ysize;
  na_val = other.na_val;

  data = static_cast<double *>(CPLMalloc(sizeof(double) * xsize * ysize));
  std::copy(other.data, other.data + xsize * ysize, data);

  return *this;
}

// Destructor
CGriddedData::~CGriddedData() {
  if (data) {
    CPLFree(data);
    data = nullptr;
  }
}