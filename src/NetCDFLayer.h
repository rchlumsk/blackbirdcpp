#ifndef NETCDFCLASS_H
#define NETCDFCLASS_H

#include "GriddedData.h"

class CNetCDFLayer : public CGriddedData {
public:
  // Member variables
  std::vector<double> x_coords;                               // netcdf x coordinates
  std::vector<double> y_coords;                               // netcdf y coordinates
  std::string epsg;                                           // netcdf epsg
  nc_type datatype;                                           // netcdf data type

  // Constructors and Destructor
  CNetCDFLayer();
  CNetCDFLayer(const CNetCDFLayer &other);

  // Copy assignment operator
  CNetCDFLayer &operator=(const CNetCDFLayer &other);

  // Polymorphic clone
  std::unique_ptr<CGriddedData> clone() const override {
    return std::make_unique<CNetCDFLayer>(*this);
  }

  // I/O Functions  
  void WriteToFile(std::string filepath) override;            // defined in StandardOutput.cpp
  void pretty_print() const override;                         // defined in StandardOutput.cpp
};

#endif
