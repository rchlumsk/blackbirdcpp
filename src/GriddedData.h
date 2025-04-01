#ifndef GRIDDEDDATA_H
#define GRIDDEDDATA_H

#include "BlackbirdInclude.h"

class CGriddedData {
public:
  // Member variables
  std::string name;       // name of layer
  double* data;           // raw data values
  int xsize;              // x dimension of gridded values
  int ysize;              // y dimension of gridded values
  double na_val;          // data value representing NA; _FillValue for NetCDF

  // Constructors and Destructor
  CGriddedData();
  CGriddedData(const CGriddedData& other);
  ~CGriddedData();

  // Copy assignment operator
  CGriddedData &operator=(const CGriddedData &other);

  // Polymorphic clone
  virtual std::unique_ptr<CGriddedData> clone() const = 0;

  // Member functions
  // I/O Functions
  virtual void WriteToFile(std::string filepath) = 0; // defined in StandardOutput.cpp
  virtual void pretty_print() const;              // defined in StandardOutput.cpp
};

#endif