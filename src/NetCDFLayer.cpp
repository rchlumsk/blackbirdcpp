#include "BlackbirdInclude.h"
#include "NetCDFLayer.h"

// Default constructor
CNetCDFLayer::CNetCDFLayer()
  : CGriddedData(),
  x_coords(),
  y_coords(),
  epsg(PLACEHOLDER_STR),
  datatype(NC_NAT) {
}

// Copy constructor
CNetCDFLayer::CNetCDFLayer(const CNetCDFLayer &other)
  : CGriddedData(other),
  x_coords(other.x_coords),
  y_coords(other.y_coords),
  epsg(other.epsg),
  datatype(other.datatype){
}

// Copy assignment operator
CNetCDFLayer& CNetCDFLayer::operator=(const CNetCDFLayer &other) {
  if (this == &other) {
    return *this; // Handle self-assignment
  }

  CGriddedData::operator=(other); // Copy base class members

  x_coords = other.x_coords;
  y_coords = other.y_coords;
  epsg = other.epsg;
  datatype = other.datatype;

  return *this;
}