#ifndef NETCDFCLASS_H
#define NETCDFCLASS_H

#include "GriddedData.h"

class CNetCDF : public CGriddedData {
public:

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
  //void WriteToFile(std::string filepath); // defined in StandardOutput.cpp
  //void pretty_print() const;              // defined in StandardOutput.cpp
};

#endif
