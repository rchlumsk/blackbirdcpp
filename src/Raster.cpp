#include "BlackbirdInclude.h"
#include "Model.h"
#include "ParseLib.h"

//////////////////////////////////////////////////////////////////
/// \brief Reads Raster files generated from GenerateRasterFilepaths()
/// \return True if operation is successful
//
bool CModel::ReadRasterFiles()
{
  GDALAllRegister();
  std::string tmp_filename{""};

  // bb_hand.tif
  tmp_filename = bbopt->raster_folder + "/bb_hand.tif";
  GDALDataset *dataset = static_cast<GDALDataset *>(GDALOpen(tmp_filename.c_str(), GA_ReadOnly));
  ExitGracefullyIf(dataset == nullptr, "Raster.cpp: ReadRasterFiles: couldn't open bb_hand.tif", exitcode::FILE_OPEN_ERR);
  raster_xsize = dataset->GetRasterXSize();
  raster_ysize = dataset->GetRasterYSize();
  hand = static_cast<double *>(CPLMalloc(sizeof(double) * raster_xsize * raster_ysize));
  GDALRasterBand *band = dataset->GetRasterBand(1);
  band->RasterIO(GF_Read, 0, 0, raster_xsize, raster_ysize, hand, raster_xsize, raster_ysize, GDT_Float64, 0, 0);
  GDALClose(dataset);


  return true;
}