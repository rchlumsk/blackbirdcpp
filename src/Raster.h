#ifndef RASTER_H
#define RASTER_H

class CRaster {
public:
  // Member variables
  double* data;           // raster values
  int xsize;              // x dimension of raster values
  int ysize;              // y dimension of raster values
  const char* proj;       // raster projection
  double geotrans[6];     // raster geo transform
  double na_val;          // raster value representing NA
  GDALDataType datatype;  // datatype of raster values

  // Constructors and Destructor
  CRaster();
  CRaster(const CRaster& other);
  ~CRaster();

  // Member functions
  void WriteToFile(std::string filepath); // defined in StandardOutput.cpp
};

#endif