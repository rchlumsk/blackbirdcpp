#include "BlackbirdInclude.h"
#include "Model.h"

//////////////////////////////////////////////////////////////////
/// \brief Reads Raster files
/// \return True if operation is successful
//
void CModel::ReadRasterFiles()
{
  GDALAllRegister();

  GDALDataset *first_dataset = static_cast<GDALDataset *>(GDALOpen((bbopt->raster_folder + "/bb_catchments_fromstreamnodes.tif").c_str(), GA_ReadOnly));
  ExitGracefullyIf(first_dataset == nullptr, "Raster.cpp: ReadRasterFiles: couldn't open bb_catchments_fromstreamnodes.tif", exitcode::FILE_OPEN_ERR);
  raster_xsize = first_dataset->GetRasterXSize();
  raster_ysize = first_dataset->GetRasterYSize();
  raster_proj = first_dataset->GetProjectionRef();
  first_dataset->GetGeoTransform(raster_geotrans);
  GDALClose(first_dataset);

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
      dhand.push_back(nullptr);
      ReadRasterFile(bbopt->raster_folder + "/bb_dhand_depth_" + stream.str() + "m.tif", dhand.back());
      if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
          bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR ||
          bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
        dhandid.push_back(nullptr);
        ReadRasterFile(bbopt->raster_folder + "/bb_dhand_pourpoint_id_depth_" + stream.str() + "m.tif", dhand.back());
      }
    }
  }
  if (!bbopt->silent_run) {
    std::cout << "...raster data successfully read" << std::endl;
    std::cout << std::endl;
  }
}

void CModel::ReadRasterFile(std::string filename, double *&buf)
{
  GDALDataset *dataset = static_cast<GDALDataset *>(GDALOpen(filename.c_str(), GA_ReadOnly));
  ExitGracefullyIf(dataset == nullptr, ("Raster.cpp: ReadRasterFile: couldn't open " + filename).c_str(), exitcode::FILE_OPEN_ERR);
  buf = static_cast<double *>(CPLMalloc(sizeof(double) * raster_xsize * raster_ysize));
  GDALRasterBand *band = dataset->GetRasterBand(1);
  band->RasterIO(GF_Read, 0, 0, raster_xsize, raster_ysize, buf, raster_xsize, raster_ysize, GDT_Float64, 0, 0);
  GDALClose(dataset);
}

void CModel::postprocess_floodresults()
{
  if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
    // do stuff 99
  }
  ExitGracefullyIf(!c_from_s,
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
    if (bbopt->interpolation_postproc_method == enum_ppi_method::CATCHMENT_HAND) {
      double *result_buffer = static_cast<double *>(CPLMalloc(sizeof(double) * raster_xsize * raster_ysize));
      std::fill(result_buffer, result_buffer + (raster_xsize * raster_ysize), 0.0);
      for (int j = 0; j < raster_xsize * raster_ysize; j++) { //need to deal with nas?
        result_buffer[j] = (*hyd_result)[get_hyd_res_index(i, c_from_s[j])]->depth - hand[j];
      }
      out_rasters.push_back(result_buffer);
    } else {
      std::cout << "not yet available" << std::endl;
    }
  }
  if (!bbopt->silent_run) {
    std::cout << "finished post processing flood results" << std::endl;
  }
}