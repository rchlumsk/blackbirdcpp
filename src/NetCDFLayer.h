#ifndef NETCDFCLASS_H
#define NETCDFCLASS_H

#include "GriddedData.h"

class CNetCDF : public CGriddedData {
public:
  // Member variables
  std::vector<double> x_coords;                               // netcdf x coordinates
  std::vector<double> y_coords;                               // netcdf y coordinates
  std::string epsg;                                           // netcdf epsg
  nc_type datatype;                                           // netcdf data type

  // Constructors and Destructor
  CNetCDF();
  CNetCDF(const CNetCDF &other);

  // Copy assignment operator
  CNetCDF &operator=(const CNetCDF &other);

  // Polymorphic clone
  std::unique_ptr<CGriddedData> clone() const override {
    return std::make_unique<CNetCDF>(*this);
  }

  // Member functions

  // I/O Functions
  void WriteToFile(std::string filepath) override; // defined in StandardOutput.cpp
  void pretty_print() const override;              // defined in StandardOutput.cpp
};

#endif
