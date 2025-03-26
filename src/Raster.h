#ifndef RASTER_H
#define RASTER_H

#include "GriddedData.h"

class CRaster : public CGriddedData {
public:

  // Constructors and Destructor
  CRaster();
  CRaster(const CRaster& other);

  // Copy assignment operator
  CRaster& operator=(const CRaster& other);

  // Member functions
  // I/O Functions
  //void WriteToFile(std::string filepath); // defined in StandardOutput.cpp
  //void pretty_print() const;              // defined in StandardOutput.cpp
};

#endif