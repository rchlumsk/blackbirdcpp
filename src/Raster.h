#ifndef RASTER_H
#define RASTER_H

#include "GriddedData.h"

class CRaster : public CGriddedData {
public:
  // Member variables
  char *proj;                                                 // raster projection
  double geotrans[6];                                         // raster geo transform
  GDALDataType datatype;                                      // datatype of raster values

  // Constructors and Destructor
  CRaster();
  CRaster(const CRaster& other);
  ~CRaster();

  // Copy assignment operator
  CRaster &operator=(const CRaster &other);

  // Polymorphic clone
  std::unique_ptr<CGriddedData> clone() const override {
    return std::make_unique<CRaster>(*this);
  }

  // I/O Functions
  void WriteToFile(std::string filepath) override;            // defined in StandardOutput.cpp
  void pretty_print() const override;                         // defined in StandardOutput.cpp
};

#endif