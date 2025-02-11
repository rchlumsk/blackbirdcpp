#ifndef RASTER_H
#define RASTER_H

class CRaster {
public:
  // Member variables
  std::string name;       // name of raster
  double* data;           // raster values
  int xsize;              // x dimension of raster values
  int ysize;              // y dimension of raster values
  char* proj;             // raster projection
  double geotrans[6];     // raster geo transform
  double na_val;          // raster value representing NA
  GDALDataType datatype;  // datatype of raster values

  // Constructors and Destructor
  CRaster();
  CRaster(const CRaster& other);
  ~CRaster();

  // Copy assignment operator
  CRaster &operator=(const CRaster &other);

  // Member functions
  // I/O Functions
  void WriteToFile(std::string filepath); // defined in StandardOutput.cpp
  void pretty_print() const;              // defined in StandardOutput.cpp
};

#endif