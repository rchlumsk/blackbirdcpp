#include "BlackbirdInclude.h"
#include "Model.h"
#include "Raster.h"

// Default constructor
CRaster::CRaster()
  : data(nullptr),
  xsize(PLACEHOLDER),
  ysize(PLACEHOLDER),
  proj(PLACEHOLDER_STR.c_str()),
  geotrans(),
  na_val(PLACEHOLDER),
  datatype(GDT_Float64) {
  // Default constructor implementation
  for (int i = 0; i < std::size(geotrans); i++) {
    geotrans[i] = PLACEHOLDER;
  }
}

// Copy constructor
CRaster::CRaster(const CRaster &other)
  : data(static_cast<double *>(CPLMalloc(sizeof(double) * other.xsize * other.ysize))),
  xsize(other.xsize),
  ysize(other.ysize),
  proj(other.proj),
  geotrans(),
  na_val(other.na_val),
  datatype(other.datatype) {
  for (int i = 0; i < std::size(geotrans); i++) {
    geotrans[i] = other.geotrans[i];
  }
  std::copy(other.data, other.data + other.xsize * other.ysize, data);
}

// Destructor
CRaster::~CRaster()
{
  if (data) {
    CPLFree(data);
    data = nullptr;
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Reads Raster files
/// \return True if operation is successful
//
void CModel::ReadRasterFiles()
{
  GDALAllRegister();

  ReadRasterFile(bbopt->raster_folder + "/bb_catchments_fromstreamnodes.tif", c_from_s);
  if (!bbopt->use_dhand) {
    ReadRasterFile(bbopt->raster_folder + "/bb_hand.tif", hand);
    if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
        bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR ||
        bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
      ReadRasterFile(bbopt->raster_folder + "/bb_hand_pourpoint_id.tif", handid);
    }
  } else {
    for (auto d : dhand_depth_seq) {
      std::stringstream stream;
      stream << std::fixed << std::setprecision(4) << d;
      dhand.push_back(CRaster());
      ReadRasterFile(bbopt->raster_folder + "/bb_dhand_depth_" + stream.str() + "m.tif", dhand.back());
      if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
          bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR ||
          bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
        dhandid.push_back(CRaster());
        ReadRasterFile(bbopt->raster_folder + "/bb_dhand_pourpoint_id_depth_" + stream.str() + "m.tif", dhand.back());
      }
    }
  }
  if (!bbopt->silent_run) {
    std::cout << "...raster data successfully read" << std::endl;
    std::cout << std::endl;
  }
}

void CModel::ReadRasterFile(std::string filename, CRaster &raster_obj)
{
  GDALDataset *dataset = static_cast<GDALDataset *>(GDALOpen(filename.c_str(), GA_ReadOnly));
  raster_obj.xsize = dataset->GetRasterXSize();
  raster_obj.ysize = dataset->GetRasterYSize();
  raster_obj.proj = dataset->GetProjectionRef();
  dataset->GetGeoTransform(raster_obj.geotrans);
  ExitGracefullyIf(dataset == nullptr, ("Raster.cpp: ReadRasterFile: couldn't open " + filename).c_str(), exitcode::FILE_OPEN_ERR);
  raster_obj.data = static_cast<double *>(CPLMalloc(sizeof(double) * raster_obj.xsize * raster_obj.ysize));
  GDALRasterBand *band = dataset->GetRasterBand(1);
  raster_obj.datatype = band->GetRasterDataType();
  raster_obj.na_val = band->GetNoDataValue();
  band->RasterIO(GF_Read, 0, 0, raster_obj.xsize, raster_obj.ysize, raster_obj.data, raster_obj.xsize, raster_obj.ysize, GDT_Float64, 0, 0);
  GDALClose(dataset);
}

void CModel::postprocess_floodresults()
{
  if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
    // do stuff 99
  }
  ExitGracefullyIf(!c_from_s.data,
                   "Raster.cpp: postprocess_floodresults: catchments from "
                   "streamnodes missing",
                   exitcode::RUNTIME_ERR);
  ExitGracefullyIf(
      !hyd_result,
      "Raster.cpp: postprocess_floodresults: hydraulic output missing",
      exitcode::RUNTIME_ERR);
  for (int i = 0; i < bbsn->front()->output_flows.size(); i++) {
    if (!bbopt->silent_run) {
      std::cout << "post processing flood results for flow " + std::to_string(i + 1) << std::endl;
    }
    if (bbopt->interpolation_postproc_method == enum_ppi_method::CATCHMENT_HAND) { // maybe make more checks on validity of data
      CRaster result;
      result.xsize = hand.xsize;
      result.ysize = hand.ysize;
      result.na_val = hand.na_val;
      result.proj = hand.proj;
      for (int j = 0; j < std::size(result.geotrans); j++) {
        result.geotrans[j] = hand.geotrans[j];
      }
      result.datatype = hand.datatype;
      result.data = static_cast<double *>(CPLMalloc(sizeof(double) * result.xsize * result.ysize));
      std::fill(result.data, result.data + (result.xsize * result.ysize), 0.0);
      for (int j = 0; j < result.xsize * result.ysize; j++) {
        if (c_from_s.data[j] != c_from_s.na_val && hand.data[j] != hand.na_val) {
          result.data[j] = (*hyd_result)[get_hyd_res_index(i, c_from_s.data[j])]->depth - hand.data[j];
        } else {
          result.data[j] = result.na_val;
        }
      }
      out_rasters.push_back(result);
    } else {
      std::cout << "not yet available" << std::endl;
    }
  }
  if (!bbopt->silent_run) {
    std::cout << "finished post processing flood results" << std::endl;
  }
}