#ifndef GRIDDEDDATA_H
#define GRIDDEDDATA_H

class CGriddedData {
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
  CGriddedData();
  CGriddedData(const CGriddedData& other);
  ~CGriddedData();

  // Copy assignment operator
  CGriddedData &operator=(const CGriddedData &other);

  // Member functions
  // I/O Functions
  virtual void WriteToFile(std::string filepath); // defined in StandardOutput.cpp
  virtual void pretty_print() const;              // defined in StandardOutput.cpp
};

#endif