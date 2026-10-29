#include "BlackbirdInclude.h"
#include "GriddedData.h"

// Default constructor
CGriddedData::CGriddedData()
  : name(PLACEHOLDER_STR),
  fp_name(PLACEHOLDER_STR),
  data(nullptr),
  xsize(PLACEHOLDER),
  ysize(PLACEHOLDER),
  na_val(PLACEHOLDER){
}

// Copy constructor
CGriddedData::CGriddedData(const CGriddedData &other)
  : name(other.name),
  fp_name(other.fp_name),
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
  fp_name = other.fp_name;
  xsize = other.xsize;
  ysize = other.ysize;
  na_val = other.na_val;

  data = static_cast<double *>(CPLMalloc(sizeof(double) * xsize * ysize));
  std::copy(other.data, other.data + xsize * ysize, data);

  return *this;
}


//////////////////////////////////////////////////////////////////
/// \brief Transposes the x and y dimension of the gridded data
//
void CGriddedData::transpose_data() {
  double *transposed_data = static_cast<double *>(CPLMalloc(sizeof(double) * xsize * ysize));

  for (size_t i = 0; i < ysize; ++i) {
    for (size_t j = 0; j < xsize; ++j) {
      transposed_data[j * ysize + i] = data[i * xsize + j];
    }
  }

  CPLFree(data);
  data = transposed_data;
}

//////////////////////////////////////////////////////////////////
/// \brief Computes raster statistics
//
void CGriddedData::compute_statistics()
{
    min_val = DBL_MAX;
    max_val = -DBL_MAX;

    double sum = 0.0;
    size_t count = 0;

    for (int i = 0; i < xsize * ysize; ++i)
    {
        double v = data[i];

        if (!std::isfinite(v))
            continue;

        min_val = std::min(min_val, v);
        max_val = std::max(max_val, v);

        sum += v;
        ++count;
    }

    if (count == 0)
    {
        min_val = max_val = mean_val =
            std::numeric_limits<double>::quiet_NaN();

        stddev_val =
            std::numeric_limits<double>::quiet_NaN();

        stats_valid = false;
        return;
    }

    mean_val = sum / count;

    double ss = 0.0;

    for (int i = 0; i < xsize * ysize; ++i)
    {
        double v = data[i];

        if (!std::isfinite(v))
            continue;

        double d = v - mean_val;
        ss += d * d;
    }

    stddev_val = std::sqrt(ss / count);

    stats_valid = true;
}

// Destructor
CGriddedData::~CGriddedData() {
  if (data) {
    CPLFree(data);
    data = nullptr;
  }
}
