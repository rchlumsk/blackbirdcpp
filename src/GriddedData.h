#ifndef GRIDDEDDATA_H
#define GRIDDEDDATA_H

#include "BlackbirdInclude.h"

class CGriddedData {
public:
  // Member variables
  std::string name;                                           // name of layer
  std::string fp_name;                                        // flow profile name of layer
  double* data;                                               // raw data values
  int xsize;                                                  // x dimension of gridded values
  int ysize;                                                  // y dimension of gridded values
  double na_val;                                              // data value representing NA; _FillValue for NetCDF
  enum_grid_type gridtype;									  // type of data (depth, wsl, velocity)
  std::string filepath;										  // export file name when written

  double min_val;											  // min value computed in raster statistics
  double max_val;											  // max value computed in raster statistics
  double mean_val;											  // mean value computed in raster statistics
  double stddev_val;										  // standard deviation value computed in raster statistics
  bool stats_valid;											  // boolean indicated whether the statistics are valid

  // Constructors and Destructor
  CGriddedData();
  CGriddedData(const CGriddedData& other);
  ~CGriddedData();

  // Copy assignment operator
  CGriddedData &operator=(const CGriddedData &other);

  // Polymorphic clone
  virtual std::unique_ptr<CGriddedData> clone() const = 0;

  // Member functions
  void transpose_data();                                      // transposes the data variable to match with expected formatting
  void compute_statistics();								  // recompute raster statistics

  // I/O Functions
  virtual void WriteToFile(std::string filepath) = 0;         // defined in StandardOutput.cpp
  void WriteToPng(std::string filepath);                      // defined in StandardOutput.cpp
  virtual void pretty_print() const;                          // defined in StandardOutput.cpp
};

#endif
