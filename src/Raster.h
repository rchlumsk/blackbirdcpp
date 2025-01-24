#ifndef RASTER_H
#define RASTER_H

class CRaster {
public:
  // Member variables
  double* data;
  int xsize;
  int ysize;
  const char* proj;
  double geotrans[6];
  double na_val;
  GDALDataType datatype;

  // s and Destructor
  CRaster();
  CRaster(const CRaster& other);
  ~CRaster();

  // Member functions
  void WriteToFile(std::string filepath); // defined in StandardOutput.cpp
};

#endif