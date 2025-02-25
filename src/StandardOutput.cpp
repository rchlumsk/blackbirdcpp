#include "Model.h"

#if defined(_WIN32)
#include <direct.h>
#elif defined(__linux__)
#include <sys/stat.h>
#elif defined(__unix__)
#include <sys/stat.h>
#elif defined(__APPLE__)
#include <sys/stat.h>
#endif

//////////////////////////////////////////////////////////////////
/// \brief Returns filebase prepended with output directory & prefix
/// \param filebase [in] base filename, with extension, no directory information
//
std::string CModel::FilenamePrepare(std::string filebase) const
{
  std::string fn;
  if (bbopt->run_name == PLACEHOLDER_STR || bbopt->run_name == "") {
    fn = bbopt->main_output_dir + filebase;
  } else {
    fn = bbopt->main_output_dir + bbopt->run_name + "_" + filebase;
  }
  return fn;
}

//////////////////////////////////////////////////////////////////
/// \brief returns directory path given filename
///
/// \param fname [in] filename, e.g., C:\temp\thisfile.txt returns c:\temp
//
std::string GetDirectoryName(const std::string &fname)
{
  size_t pos = fname.find_last_of("\\/");
  if (std::string::npos == pos) { return ""; }
  else                          { return fname.substr(0, pos); }
}

//////////////////////////////////////////////////////////////////
/// \brief returns directory path given filename and relative path
///
/// \param filename [in] filename, e.g., C:/temp/thisfile.txt returns c:/temp
/// \param relfile [in] filename of reference file
/// e.g.,
///       absolute path of reference file is adopted
///       if filename = something.txt         and relfile= c:/temp/myfile.rvi,
///       returns c:/temp/something.txt
///
///       relative path of reference file is adopted
///       if filename = something.txt         and relfile= ../dir/myfile.rvi,
///       returns ../dir/something.txt
///
///       if path of reference file is same as file, then nothing changes
///       if filename = ../temp/something.txt and relfile= ../temp/myfile.rvi,
///       returns ../temp/something.txt
///
///       if absolute paths of file is given, nothing changes
///       if filename = c:/temp/something.txt and relfile= ../temp/myfile.rvi,
///       returns c:/temp/something.txt
//
std::string CorrectForRelativePath(const std::string filename, const std::string relfile)
{
  std::string filedir = GetDirectoryName(relfile); // if a relative path name, e.g., "/path/model.rvt", only returns e.g., "/path"

  if (StringToUppercase(filename).find(StringToUppercase(filedir)) == std::string::npos) // checks to see if absolute dir already included in redirect filename
  {
    std::string firstchar = filename.substr(0, 1); // if '/' --> absolute path on UNIX systems
    std::string secondchar = filename.substr(1, 1); // if ':' --> absolute path on WINDOWS system

    if ((firstchar.compare("/") != 0) && (secondchar.compare(":") != 0)) {
      // std::cout << "This is not an absolute filename!  --> " << filename << std::endl;
      //+"//"
      // std::cout << "StandardOutput: corrected filename: " << filedir + "//" + filename << std::endl;
      return filedir + "//" + filename;
    }
  }
  // std::cout << "StandardOutput: corrected filename: " << filename << std::endl;
  return filename;
}

//////////////////////////////////////////////////////////////////
/// \brief Write output file headers
/// \details Called prior to simulation (but after initialization) from CModel::Initialize()
/// \param *&pOptions [in] Global model options information
//
void CModel::WriteOutputFileHeaders(COptions*const& pOptions)
{
  if (pOptions->noisy_run) { std::cout << "  Writing Output File Headers..." << std::endl; }

  // write some output
}

//////////////////////////////////////////////////////////////////
/// \brief Replaces the WriteOutputFileHeaders function by not requiring the Options structure as an argument
/// \param &tt [in] Local (model) time *at the end of* the pertinent time step
/// \param solfile [in] Name of the solution file to be written
/// \param final [in] Whether this is the final solution file to be written
//
void CModel::WriteMajorOutput(std::string solfile, bool final) const
{
  int i, k;
  std::string tmpFilename;
  COptions* pOptions = this->bbopt;  // just to make the code more readable

  //if (Options->output_format == OUTPUT_NONE) { return; } //:SuppressOutput is on

  // write some output
}

void CModel::WriteRasterOutput()
{
  if (bbopt->interpolation_postproc_method == enum_ppi_method::NONE) {
    return;
  }
  for (int i = 0; i < out_rasters.size(); i++) {
    std::string filepath = FilenamePrepare("bb_results_depth_" + std::to_string(i + 1) + ".tif");
    out_rasters[i].WriteToFile(filepath);
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Writes raster data to geotiff raster file
//
void CRaster::WriteToFile(std::string filepath)
{
  GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("GTiff");
  ExitGracefullyIf(
      driver == nullptr,
      "StandardOutput.cpp: CRaster::WriteToFile: Failed to get GTiff driver.",
      exitcode::RUNTIME_ERR);
  ExitGracefullyIf(xsize == PLACEHOLDER || ysize == PLACEHOLDER ||
                       proj == PLACEHOLDER_STR.c_str() || data == nullptr ||
                       na_val == PLACEHOLDER ||
                       std::find(std::begin(geotrans), std::end(geotrans),
                                 PLACEHOLDER) != std::end(geotrans),
                   "StandardOutput.cpp: CRaster::WriteToFile: Raster "
                   "information not complete",
                   exitcode::RUNTIME_ERR);
  GDALDataset *output_dataset = driver->Create(filepath.c_str(), xsize, ysize, 1, datatype, nullptr);
  ExitGracefullyIf(output_dataset == nullptr,
                   "StandardOutput.cpp: CRaster::WriteToFile: Failed to create "
                   "output raster file.",
                   exitcode::RUNTIME_ERR);
  GDALRasterBand *output_band = output_dataset->GetRasterBand(1);
  output_band->RasterIO(GF_Write, 0, 0, xsize, ysize, data, xsize, ysize,
                        GDT_Float64, 0, 0);
  output_dataset->SetProjection(proj);
  output_dataset->SetGeoTransform(geotrans);
  output_band->SetNoDataValue(na_val);

  GDALClose(output_dataset);
}

//////////////////////////////////////////////////////////////////
/// \brief Writes model data to test output
//
void CModel::WriteFullModel() const
{
  if (bbopt->noisy_run) { std::cout << "  Writing Test Output File full model..." << std::endl; }
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open(FilenamePrepare("Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "===================== Full Model =====================" << std::endl;
  TESTOUTPUT << "\n================== Model ==================" << std::endl;
  TESTOUTPUT << std::left << std::setw(35) << "DHand Depth Sequence:";
  for (auto d : dhand_depth_seq) {
    TESTOUTPUT << d << "  ";
  }
  TESTOUTPUT << std::endl;
  TESTOUTPUT << "===========================================\n" << std::endl;
  TESTOUTPUT.close();
  if (c_from_s.name != PLACEHOLDER_STR) {
    c_from_s.pretty_print();
  }
  if (spp.name != PLACEHOLDER_STR) {
    spp.pretty_print();
  }
  if (hand.name != PLACEHOLDER_STR) {
    hand.pretty_print();
  }
  if (handid.name != PLACEHOLDER_STR) {
    handid.pretty_print();
  }
  for (auto r : dhand) {
    r.pretty_print();
  }
  for (auto r : dhandid) {
    r.pretty_print();
  }
  for (auto r : out_rasters) {
    r.pretty_print();
  }
  this->bbopt->pretty_print();
  this->bbbc->pretty_print();
  for (auto sn : *bbsn) {
    if (sn->nodetype == enum_nodetype::REACH) {
      ((CReach *)sn)->pretty_print();
    } else { // XSECTION
      ((CXSection *)sn)->pretty_print();
    }
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints CRaster class data to testoutput (except data)
//
void CRaster::pretty_print() const {
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "\n================== Raster =================" << std::endl;
  TESTOUTPUT << std::left << std::setw(35) << "Name:" << name << std::endl;
  TESTOUTPUT << std::setw(35) << "X Dimension:" << xsize << std::endl;
  TESTOUTPUT << std::setw(35) << "Y Dimension:" << ysize << std::endl;
  TESTOUTPUT << std::setw(35) << "Projection:" << proj << std::endl;
  TESTOUTPUT << std::setw(35) << "Geo Transform:" << geotrans[0] << ", "
             << geotrans[1] << ", " << geotrans[2] << ", " << geotrans[3]
             << ", " << geotrans[4] << ", " << geotrans[5] << std::endl;
  TESTOUTPUT << std::setw(35) << "NA Value:" << na_val << std::endl;
  TESTOUTPUT << std::setw(35) << "GDAL Datatype:" << datatype << std::endl;
  TESTOUTPUT << "===========================================\n" << std::endl;
  TESTOUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints CVector class data to testoutput (except features)
//
void CVector::pretty_print() const {
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "\n================== Vector =================" << std::endl;
  TESTOUTPUT << std::left << std::setw(35) << "Name:" << name << std::endl;
  TESTOUTPUT << std::setw(35) << "Geometry Type:" << OGRGeometryTypeToName(geom_type) << std::endl;

  char *wkt = nullptr;
  if (spat_ref) {
    spat_ref->exportToPrettyWkt(&wkt, FALSE);
    TESTOUTPUT << std::setw(35) << "Spatial Reference:\n" << wkt << std::endl;
    CPLFree(wkt);
  } else {
    TESTOUTPUT << std::setw(35) << "Spatial Reference:" << "NULL" << std::endl;
  }

  TESTOUTPUT << std::setw(35) << "Number of Fields:" << field_defs.size() << std::endl;
  TESTOUTPUT << std::setw(35) << "Field Names:";
  for (const auto &field : field_defs) {
    TESTOUTPUT << field->GetNameRef() << " (" << OGRFieldDefn::GetFieldTypeName(field->GetType()) << "), ";
  }
  TESTOUTPUT << std::endl;
  TESTOUTPUT << std::setw(35) << "Number of Features:" << features.size() << std::endl;
  TESTOUTPUT << "===========================================\n" << std::endl;
  TESTOUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints COptions class data to testoutput
//
void COptions::pretty_print() const
{
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "\n================= Options =================" << std::endl;
  TESTOUTPUT << std::left << std::setw(35) << "Version:" << version << std::endl;
  TESTOUTPUT << std::setw(35) << "Run Name:" << run_name << std::endl;
  TESTOUTPUT << std::setw(35) << "BBI Filename:" << bbi_filename << std::endl;
  TESTOUTPUT << std::setw(35) << "BBG Filename:" << bbg_filename << std::endl;
  TESTOUTPUT << std::setw(35) << "BBB Filename:" << bbb_filename << std::endl;
  TESTOUTPUT << std::setw(35) << "Main Output Directory:" << main_output_dir << std::endl;
  TESTOUTPUT << std::setw(35) << "Working Directory:" << working_dir << std::endl;
  TESTOUTPUT << std::setw(35) << "GIS Path:" << gis_path << std::endl;
  TESTOUTPUT << std::setw(35) << "Model Type:" << toString(modeltype) << std::endl;
  TESTOUTPUT << std::setw(35) << "Regime Type:" << toString(regimetype) << std::endl;
  TESTOUTPUT << std::setw(35) << "DX:" << dx << std::endl;
  TESTOUTPUT << std::setw(35) << "Extrapolate Depth Table:" << (extrapolate_depth_table ? "True" : "False") << std::endl;
  TESTOUTPUT << std::setw(35) << "Num Extrapolation Points:" << num_extrapolation_points << std::endl;
  TESTOUTPUT << std::setw(35) << "Friction Slope Method:" << toString(friction_slope_method) << std::endl;
  TESTOUTPUT << std::setw(35) << "X-Section Conveyance Method:" << toString(xsection_conveyance_method) << std::endl;
  TESTOUTPUT << std::setw(35) << "Reach Conveyance Method:" << toString(reach_conveyance_method) << std::endl;
  TESTOUTPUT << std::setw(35) << "Enforce Delta Leff:" << (enforce_delta_Leff ? "True" : "False") << std::endl;
  TESTOUTPUT << std::setw(35) << "Delta Reach Length:" << delta_reachlength << std::endl;
  TESTOUTPUT << std::setw(35) << "Tolerance CP:" << tolerance_cp << std::endl;
  TESTOUTPUT << std::setw(35) << "Iteration Limit CP:" << iteration_limit_cp << std::endl;
  TESTOUTPUT << std::setw(35) << "Next WSL Split CP:" << next_WSL_split_cp << std::endl;
  TESTOUTPUT << std::setw(35) << "Tolerance ND:" << tolerance_nd << std::endl;
  TESTOUTPUT << std::setw(35) << "Iteration Limit ND:" << iteration_limit_nd << std::endl;
  TESTOUTPUT << std::setw(35) << "Next WSL Split ND:" << next_WSL_split_nd << std::endl;
  TESTOUTPUT << std::setw(35) << "Max RHSQ Ratio:" << max_RHSQ_ratio << std::endl;
  TESTOUTPUT << std::setw(35) << "Min RHSQ Ratio:" << min_RHSQ_ratio << std::endl;
  TESTOUTPUT << std::setw(35) << "Manning Composite Method:" << toString(manning_composite_method) << std::endl;
  TESTOUTPUT << std::setw(35) << "Manning Enforce Values:" << (manning_enforce_values ? "True" : "False") << std::endl;
  TESTOUTPUT << std::setw(35) << "Reach Integration Method:" << toString(reach_integration_method) << std::endl;
  TESTOUTPUT << std::setw(35) << "Interpolation Postproc Method:" << toString(interpolation_postproc_method) << std::endl;
  TESTOUTPUT << std::setw(35) << "DHand Postproc Method:" << toString(dhand_method) << std::endl;
  TESTOUTPUT << std::setw(35) << "Postproc Elev Corr Threshold:" << postproc_elev_corr_threshold << std::endl;
  TESTOUTPUT << std::setw(35) << "Roughness Multiplier:" << roughness_multiplier << std::endl;
  TESTOUTPUT << std::setw(35) << "Blended Conveyance Weights:" << blended_conveyance_weights << std::endl;
  TESTOUTPUT << std::setw(35) << "Blended NC Weights:" << blended_nc_weights << std::endl;
  TESTOUTPUT << std::setw(35) << "Froude Threshold:" << froude_threshold << std::endl;
  TESTOUTPUT << std::setw(35) << "Silent Run:" << (silent_run ? "True" : "False") << std::endl;
  TESTOUTPUT << std::setw(35) << "Noisy Run:" << (noisy_run ? "True" : "False") << std::endl;
  TESTOUTPUT << "===========================================\n" << std::endl;
  TESTOUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints CBoundaryConditions class data to testoutput
//
void CBoundaryConditions::pretty_print() const
{
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "\n=========== Boundary Conditions ===========" << std::endl;
  TESTOUTPUT << std::left << std::setw(20) << "Node ID:" << nodeID << std::endl;
  TESTOUTPUT << std::setw(20) << "Boundary Type:" << toString(bctype) << std::endl;
  TESTOUTPUT << std::setw(20) << "Boundary Value:" << bcvalue << std::endl;
  TESTOUTPUT << std::setw(20) << "Initial WSL:" << init_WSL << std::endl;
  TESTOUTPUT << "===========================================\n" << std::endl;
  TESTOUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints CReach class data to testoutput
//
void CReach::pretty_print() const
{
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "\n=================== Reach ===================" << std::endl;
  TESTOUTPUT.close();
  this->CStreamnode::pretty_print();
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "=============================================\n" << std::endl;
  TESTOUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints CXSection class data to testoutput
//
void CXSection::pretty_print() const
{
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "\n================= XSection ==================" << std::endl;
  TESTOUTPUT.close();
  this->CStreamnode::pretty_print();
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << std::left << std::setw(35) << "xx Sequence:";
  for (auto d : xx) {
    TESTOUTPUT << d << "  ";
  }
  TESTOUTPUT << std::endl;
  TESTOUTPUT << std::left << std::setw(35) << "zz Sequence:";
  for (auto d : zz) {
    TESTOUTPUT << d << "  ";
  }
  TESTOUTPUT << std::endl;
  TESTOUTPUT << std::left << std::setw(35) << "manning Sequence:";
  for (auto d : manning) {
    TESTOUTPUT << d << "  ";
  }
  TESTOUTPUT << std::endl;
  TESTOUTPUT << std::left << std::setw(25) << "Node ID:" << nodeID << std::endl;
  TESTOUTPUT << std::setw(25) << "Manning LOB:" << manning_LOB << std::endl;
  TESTOUTPUT << std::setw(25) << "Manning Main:" << manning_main << std::endl;
  TESTOUTPUT << std::setw(25) << "Manning ROB:" << manning_ROB << std::endl;
  TESTOUTPUT << std::setw(25) << "LBS xx:" << lbs_xx << std::endl;
  TESTOUTPUT << std::setw(25) << "RBS xx:" << rbs_xx << std::endl;
  TESTOUTPUT << std::setw(25) << "DS Length LOB:" << ds_length_LOB << std::endl;
  TESTOUTPUT << std::setw(25) << "DS Length Main" << ds_length_main << std::endl;
  TESTOUTPUT << std::setw(25) << "DS Length ROB:" << ds_length_ROB << std::endl;
  TESTOUTPUT << "=============================================\n" << std::endl;
  TESTOUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints CStreamnode class data to testoutput
//
void CStreamnode::pretty_print() const
{
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << std::left << std::setw(25) << "Node ID:" << nodeID << std::endl;
  TESTOUTPUT << std::setw(25) << "Node Type:" << toString(nodetype) << std::endl;
  TESTOUTPUT << std::setw(25) << "Downstream Node ID:" << downnodeID << std::endl;
  TESTOUTPUT << std::setw(25) << "Upstream Node ID 1:" << upnodeID1 << std::endl;
  TESTOUTPUT << std::setw(25) << "Upstream Node ID 2:" << upnodeID2 << std::endl;
  TESTOUTPUT << std::setw(25) << "Station Name:" << stationname << std::endl;
  TESTOUTPUT << std::setw(25) << "Station Number:" << station << std::endl;
  TESTOUTPUT << std::setw(25) << "Reach ID:" << reachID << std::endl;
  TESTOUTPUT << std::setw(25) << "Downstream Reach Length:" << ds_reach_length << std::endl;
  TESTOUTPUT << std::setw(25) << "Upstream Reach Length 1:" << us_reach_length1 << std::endl;
  TESTOUTPUT << std::setw(25) << "Upstream Reach Length 2:" << us_reach_length2 << std::endl;
  TESTOUTPUT << std::setw(25) << "Contraction Coeff:" << contraction_coeff << std::endl;
  TESTOUTPUT << std::setw(25) << "Expansion Coeff:" << expansion_coeff << std::endl;
  TESTOUTPUT << std::setw(25) << "Min Elevation:" << min_elev << std::endl;
  TESTOUTPUT << std::setw(25) << "Bed Slope:" << bed_slope << std::endl;
  TESTOUTPUT << std::setw(25) << "Output Depth:" << output_depth << std::endl;

  TESTOUTPUT << std::setw(25) << "Upstream Flows:" << std::endl;
  for (size_t i = 0; i < upstream_flows.size(); ++i) {
    TESTOUTPUT << std::setw(25) << "  Flow " + std::to_string(i + 1) + ":" << upstream_flows[i] << std::endl;
  }

  TESTOUTPUT << std::setw(25) << "Flow Sources:" << std::endl;
  for (size_t i = 0; i < flow_sources.size(); ++i) {
    TESTOUTPUT << std::setw(25) << "  Source " + std::to_string(i + 1) + ":" << flow_sources[i] << std::endl;
  }

  TESTOUTPUT << std::setw(25) << "Flow Sinks:" << std::endl;
  for (size_t i = 0; i < flow_sinks.size(); ++i) {
    TESTOUTPUT << std::setw(25) << "  Sink " + std::to_string(i + 1) + ":" << flow_sinks[i] << std::endl;
  }

  TESTOUTPUT << std::setw(25) << "Output Flows:" << std::endl;
  for (size_t i = 0; i < output_flows.size(); ++i) {
    TESTOUTPUT << std::setw(25) << "  Flow " + std::to_string(i + 1) + ":" << output_flows[i] << std::endl;
  }

  if (depthdf) {
    TESTOUTPUT << std::setw(20) << "============ Depthdf ============" << std::endl;
    // Print headers for the hydraulic_output table
    TESTOUTPUT << std::setw(10) << "nodeId"
      << std::setw(10) << "reachId"
      << std::setw(15) << "downNodeId"
      << std::setw(15) << "upNodeId1"
      << std::setw(15) << "upNodeId2"
      << std::setw(15) << "stationName"
      << std::setw(15) << "station"
      << std::setw(15) << "reachLengthDs"
      << std::setw(15) << "reachLengthUs1"
      << std::setw(15) << "reachLengthUs2"
      << std::setw(10) << "flow"
      << std::setw(10) << "flowLob"
      << std::setw(10) << "flowMain"
      << std::setw(10) << "flowRob"
      << std::setw(15) << "minElev"
      << std::setw(15) << "wsl"
      << std::setw(10) << "depth"
      << std::setw(15) << "hydDepth"
      << std::setw(15) << "hydDepthLob"
      << std::setw(15) << "hydDepthMain"
      << std::setw(15) << "hydDepthRob"
      << std::setw(15) << "topWidth"
      << std::setw(15) << "topWidthLob"
      << std::setw(15) << "topWidthMain"
      << std::setw(15) << "topWidthRob"
      << std::setw(10) << "velocity"
      << std::setw(15) << "velocityLob"
      << std::setw(15) << "velocityMain"
      << std::setw(15) << "velocityRob"
      << std::setw(10) << "kTotal"
      << std::setw(10) << "kLob"
      << std::setw(10) << "kMain"
      << std::setw(10) << "kRob"
      << std::setw(10) << "alpha"
      << std::setw(10) << "area"
      << std::setw(10) << "areaLob"
      << std::setw(10) << "areaMain"
      << std::setw(10) << "areaRob"
      << std::setw(15) << "radius"
      << std::setw(15) << "radiusLob"
      << std::setw(15) << "radiusMain"
      << std::setw(15) << "radiusRob"
      << std::setw(15) << "wetPerimeter"
      << std::setw(20) << "wetPerimeterLob"
      << std::setw(20) << "wetPerimeterMain"
      << std::setw(20) << "wetPerimeterRob"
      << std::setw(15) << "energyTotal"
      << std::setw(15) << "velocityHead"
      << std::setw(10) << "froude"
      << std::setw(10) << "sf"
      << std::setw(15) << "sfAvg"
      << std::setw(10) << "sbed"
      << std::setw(15) << "lengthEffective"
      << std::setw(15) << "headLoss"
      << std::setw(15) << "manningLob"
      << std::setw(15) << "manningMain"
      << std::setw(15) << "manningRob"
      << std::setw(20) << "manningComposite"
      << std::setw(20) << "kTotalAreaConv"
      << std::setw(20) << "kTotalRoughConv"
      << std::setw(20) << "kTotalDisconv"
      << std::setw(20) << "alphaAreaConv"
      << std::setw(20) << "alphaRoughConv"
      << std::setw(20) << "alphaDisconv"
      << std::setw(20) << "ncEqualForce"
      << std::setw(20) << "ncEqualVelocity"
      << std::setw(15) << "ncWavgwp"
      << std::setw(15) << "ncWavgArea"
      << std::setw(15) << "ncWavgConv"
      << std::setw(20) << "criticalDepth"
      << std::setw(20) << "cpIterations"
      << std::setw(10) << "kErr"
      << std::setw(10) << "wsErr"
      << std::setw(20) << "lengthEnergyloss"
      << std::setw(25) << "lengthEffectiveAdjusted"
      << std::setw(25) << "peakHoursRequired"
      << std::endl;

    // Iterate over all hydraulic_output objects in depthdf and print them
    for (const auto& ho : *depthdf) {
      TESTOUTPUT << std::setw(10) << ho->nodeID
        << std::setw(10) << ho->reachID
        << std::setw(15) << ho->downnodeID
        << std::setw(15) << ho->upnodeID1
        << std::setw(15) << ho->upnodeID2
        << std::setw(15) << ho->stationname
        << std::setw(15) << ho->station
        << std::setw(15) << ho->reach_length_DS
        << std::setw(15) << ho->reach_length_US1
        << std::setw(15) << ho->reach_length_US2
        << std::setw(10) << ho->flow
        << std::setw(10) << ho->flow_lob
        << std::setw(10) << ho->flow_main
        << std::setw(10) << ho->flow_rob
        << std::setw(15) << ho->min_elev
        << std::setw(15) << ho->wsl
        << std::setw(10) << ho->depth
        << std::setw(15) << ho->hyd_depth
        << std::setw(15) << ho->hyd_depth_lob
        << std::setw(15) << ho->hyd_depth_main
        << std::setw(15) << ho->hyd_depth_rob
        << std::setw(15) << ho->top_width
        << std::setw(15) << ho->top_width_lob
        << std::setw(15) << ho->top_width_main
        << std::setw(15) << ho->top_width_rob
        << std::setw(10) << ho->velocity
        << std::setw(15) << ho->velocity_lob
        << std::setw(15) << ho->velocity_main
        << std::setw(15) << ho->velocity_rob
        << std::setw(10) << ho->k_total
        << std::setw(10) << ho->k_lob
        << std::setw(10) << ho->k_main
        << std::setw(10) << ho->k_rob
        << std::setw(10) << ho->alpha
        << std::setw(10) << ho->area
        << std::setw(10) << ho->area_lob
        << std::setw(10) << ho->area_main
        << std::setw(10) << ho->area_rob
        << std::setw(15) << ho->hradius
        << std::setw(15) << ho->hradius_lob
        << std::setw(15) << ho->hradius_main
        << std::setw(15) << ho->hradius_rob
        << std::setw(15) << ho->wet_perimeter
        << std::setw(20) << ho->wet_perimeter_lob
        << std::setw(20) << ho->wet_perimeter_main
        << std::setw(20) << ho->wet_perimeter_rob
        << std::setw(15) << ho->energy_total
        << std::setw(15) << ho->velocity_head
        << std::setw(10) << ho->froude
        << std::setw(10) << ho->sf
        << std::setw(15) << ho->sf_avg
        << std::setw(10) << ho->sbed
        << std::setw(15) << ho->length_effective
        << std::setw(15) << ho->head_loss
        << std::setw(15) << ho->manning_lob
        << std::setw(15) << ho->manning_main
        << std::setw(15) << ho->manning_rob
        << std::setw(20) << ho->manning_composite
        << std::setw(20) << ho->k_total_areaconv
        << std::setw(20) << ho->k_total_roughconv
        << std::setw(20) << ho->k_total_disconv
        << std::setw(20) << ho->alpha_areaconv
        << std::setw(20) << ho->alpha_roughconv
        << std::setw(20) << ho->alpha_disconv
        << std::setw(20) << ho->nc_equalforce
        << std::setw(20) << ho->nc_equalvelocity
        << std::setw(15) << ho->nc_wavgwp
        << std::setw(15) << ho->nc_wavgarea
        << std::setw(15) << ho->nc_wavgconv
        << std::setw(20) << ho->depth_critical
        << std::setw(20) << ho->cp_iterations
        << std::setw(10) << ho->k_err
        << std::setw(10) << ho->ws_err
        << std::setw(20) << ho->length_energyloss
        << std::setw(25) << ho->length_effectiveadjusted
        << std::setw(25) << ho->peak_hrs_required
        << std::endl;
    }
    TESTOUTPUT << "=================================" << std::endl;
  }
  TESTOUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints hydraulic_output data to testoutput
//
void CModel::hyd_result_pretty_print() const
{
  if (bbopt->noisy_run) {
    std::cout << "  Writing Test Output File hyd_result..." << std::endl;
  }
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((g_output_directory + "Blackbird_testoutput.txt").c_str(), std::ios::app);
  TESTOUTPUT << "===================== Hydraulic Output =====================" << std::endl;
  if (this->hyd_result) {
    // Print headers for the hydraulic_output table
    TESTOUTPUT << std::setw(10) << "nodeId"
      << std::setw(10) << "reachId"
      << std::setw(15) << "downNodeId"
      << std::setw(15) << "upNodeId1"
      << std::setw(15) << "upNodeId2"
      << std::setw(15) << "stationName"
      << std::setw(15) << "station"
      << std::setw(15) << "reachLengthDs"
      << std::setw(15) << "reachLengthUs1"
      << std::setw(15) << "reachLengthUs2"
      << std::setw(10) << "flow"
      << std::setw(10) << "flowLob"
      << std::setw(10) << "flowMain"
      << std::setw(10) << "flowRob"
      << std::setw(15) << "minElev"
      << std::setw(15) << "wsl"
      << std::setw(10) << "depth"
      << std::setw(15) << "hydDepth"
      << std::setw(15) << "hydDepthLob"
      << std::setw(15) << "hydDepthMain"
      << std::setw(15) << "hydDepthRob"
      << std::setw(15) << "topWidth"
      << std::setw(15) << "topWidthLob"
      << std::setw(15) << "topWidthMain"
      << std::setw(15) << "topWidthRob"
      << std::setw(10) << "velocity"
      << std::setw(15) << "velocityLob"
      << std::setw(15) << "velocityMain"
      << std::setw(15) << "velocityRob"
      << std::setw(10) << "kTotal"
      << std::setw(10) << "kLob"
      << std::setw(10) << "kMain"
      << std::setw(10) << "kRob"
      << std::setw(10) << "alpha"
      << std::setw(10) << "area"
      << std::setw(10) << "areaLob"
      << std::setw(10) << "areaMain"
      << std::setw(10) << "areaRob"
      << std::setw(15) << "radius"
      << std::setw(15) << "radiusLob"
      << std::setw(15) << "radiusMain"
      << std::setw(15) << "radiusRob"
      << std::setw(15) << "wetPerimeter"
      << std::setw(20) << "wetPerimeterLob"
      << std::setw(20) << "wetPerimeterMain"
      << std::setw(20) << "wetPerimeterRob"
      << std::setw(15) << "energyTotal"
      << std::setw(15) << "velocityHead"
      << std::setw(10) << "froude"
      << std::setw(10) << "sf"
      << std::setw(15) << "sfAvg"
      << std::setw(10) << "sbed"
      << std::setw(15) << "lengthEffective"
      << std::setw(15) << "headLoss"
      << std::setw(15) << "manningLob"
      << std::setw(15) << "manningMain"
      << std::setw(15) << "manningRob"
      << std::setw(20) << "manningComposite"
      << std::setw(20) << "kTotalAreaConv"
      << std::setw(20) << "kTotalRoughConv"
      << std::setw(20) << "kTotalDisconv"
      << std::setw(20) << "alphaAreaConv"
      << std::setw(20) << "alphaRoughConv"
      << std::setw(20) << "alphaDisconv"
      << std::setw(20) << "ncEqualForce"
      << std::setw(20) << "ncEqualVelocity"
      << std::setw(15) << "ncWavgwp"
      << std::setw(15) << "ncWavgArea"
      << std::setw(15) << "ncWavgConv"
      << std::setw(20) << "criticalDepth"
      << std::setw(20) << "cpIterations"
      << std::setw(10) << "kErr"
      << std::setw(10) << "wsErr"
      << std::setw(20) << "lengthEnergyloss"
      << std::setw(25) << "lengthEffectiveAdjusted"
      << std::setw(25) << "peakHoursRequired"
      << std::endl;

    // Iterate over all hydraulic_output objects in hyd_result and print them
    for (const auto &ho : *(this->hyd_result)) {
      TESTOUTPUT << std::setw(10) << ho->nodeID
        << std::setw(10) << ho->reachID
        << std::setw(15) << ho->downnodeID
        << std::setw(15) << ho->upnodeID1
        << std::setw(15) << ho->upnodeID2
        << std::setw(15) << ho->stationname
        << std::setw(15) << ho->station
        << std::setw(15) << ho->reach_length_DS
        << std::setw(15) << ho->reach_length_US1
        << std::setw(15) << ho->reach_length_US2
        << std::setw(10) << ho->flow
        << std::setw(10) << ho->flow_lob
        << std::setw(10) << ho->flow_main
        << std::setw(10) << ho->flow_rob
        << std::setw(15) << ho->min_elev
        << std::setw(15) << ho->wsl
        << std::setw(10) << ho->depth
        << std::setw(15) << ho->hyd_depth
        << std::setw(15) << ho->hyd_depth_lob
        << std::setw(15) << ho->hyd_depth_main
        << std::setw(15) << ho->hyd_depth_rob
        << std::setw(15) << ho->top_width
        << std::setw(15) << ho->top_width_lob
        << std::setw(15) << ho->top_width_main
        << std::setw(15) << ho->top_width_rob
        << std::setw(10) << ho->velocity
        << std::setw(15) << ho->velocity_lob
        << std::setw(15) << ho->velocity_main
        << std::setw(15) << ho->velocity_rob
        << std::setw(10) << ho->k_total
        << std::setw(10) << ho->k_lob
        << std::setw(10) << ho->k_main
        << std::setw(10) << ho->k_rob
        << std::setw(10) << ho->alpha
        << std::setw(10) << ho->area
        << std::setw(10) << ho->area_lob
        << std::setw(10) << ho->area_main
        << std::setw(10) << ho->area_rob
        << std::setw(15) << ho->hradius
        << std::setw(15) << ho->hradius_lob
        << std::setw(15) << ho->hradius_main
        << std::setw(15) << ho->hradius_rob
        << std::setw(15) << ho->wet_perimeter
        << std::setw(20) << ho->wet_perimeter_lob
        << std::setw(20) << ho->wet_perimeter_main
        << std::setw(20) << ho->wet_perimeter_rob
        << std::setw(15) << ho->energy_total
        << std::setw(15) << ho->velocity_head
        << std::setw(10) << ho->froude
        << std::setw(10) << ho->sf
        << std::setw(15) << ho->sf_avg
        << std::setw(10) << ho->sbed
        << std::setw(15) << ho->length_effective
        << std::setw(15) << ho->head_loss
        << std::setw(15) << ho->manning_lob
        << std::setw(15) << ho->manning_main
        << std::setw(15) << ho->manning_rob
        << std::setw(20) << ho->manning_composite
        << std::setw(20) << ho->k_total_areaconv
        << std::setw(20) << ho->k_total_roughconv
        << std::setw(20) << ho->k_total_disconv
        << std::setw(20) << ho->alpha_areaconv
        << std::setw(20) << ho->alpha_roughconv
        << std::setw(20) << ho->alpha_disconv
        << std::setw(20) << ho->nc_equalforce
        << std::setw(20) << ho->nc_equalvelocity
        << std::setw(15) << ho->nc_wavgwp
        << std::setw(15) << ho->nc_wavgarea
        << std::setw(15) << ho->nc_wavgconv
        << std::setw(20) << ho->depth_critical
        << std::setw(20) << ho->cp_iterations
        << std::setw(10) << ho->k_err
        << std::setw(10) << ho->ws_err
        << std::setw(20) << ho->length_energyloss
        << std::setw(25) << ho->length_effectiveadjusted
        << std::setw(25) << ho->peak_hrs_required
        << std::endl;
    }
    TESTOUTPUT << "=================================" << std::endl;
  }
  TESTOUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Cleanly prints hydraulic_output data to testoutput as csv
//
void CModel::hyd_result_pretty_print_csv() const
{
  std::string tmpFilename = FilenamePrepare("HydraulicOutput.csv");
  std::ofstream HYD_OUTPUT;
  HYD_OUTPUT.open(tmpFilename.c_str());
  if (HYD_OUTPUT.fail()) {
    ExitGracefully(
        ("CModel::hyd_result_pretty_print_csv: Unable to open output file " +
         tmpFilename + " for writing.")
            .c_str(),
        FILE_OPEN_ERR);
  }

  HYD_OUTPUT << "nodeId" << "," << "reachId" << "," << "downNodeId" << ","
             << "upNodeId1" << "," << "upNodeId2" << "," << "stationName" << ","
             << "station" << "," << "reachLengthDs" << "," << "reachLengthUs1"
             << "," << "reachLengthUs2" << "," << "flow" << "," << "flowLob"
             << "," << "flowMain" << "," << "flowRob" << "," << "minElev" << ","
             << "wsl" << "," << "depth" << "," << "hydDepth" << ","
             << "hydDepthLob" << "," << "hydDepthMain" << "," << "hydDepthRob"
             << "," << "topWidth" << "," << "topWidthLob" << ","
             << "topWidthMain" << "," << "topWidthRob" << "," << "velocity"
             << "," << "velocityLob" << "," << "velocityMain" << ","
             << "velocityRob" << "," << "kTotal" << "," << "kLob" << ","
             << "kMain" << "," << "kRob" << "," << "alpha" << "," << "area"
             << "," << "areaLob" << "," << "areaMain" << "," << "areaRob" << ","
             << "radius" << "," << "radiusLob" << "," << "radiusMain" << ","
             << "radiusRob" << "," << "wetPerimeter" << "," << "wetPerimeterLob"
             << "," << "wetPerimeterMain" << "," << "wetPerimeterRob" << ","
             << "energyTotal" << "," << "velocityHead" << "," << "froude" << ","
             << "sf" << "," << "sfAvg" << "," << "sbed" << ","
             << "lengthEffective" << "," << "headLoss" << "," << "manningLob"
             << "," << "manningMain" << "," << "manningRob" << ","
             << "manningComposite" << "," << "kTotalAreaConv" << ","
             << "kTotalRoughConv" << "," << "kTotalDisconv" << ","
             << "alphaAreaConv" << "," << "alphaRoughConv" << ","
             << "alphaDisconv" << "," << "ncEqualForce" << ","
             << "ncEqualVelocity" << "," << "ncWavgwp" << "," << "ncWavgArea"
             << "," << "ncWavgConv" << "," << "criticalDepth" << ","
             << "cpIterations" << "," << "kErr" << "," << "wsErr" << ","
             << "lengthEnergyloss" << "," << "lengthEffectiveAdjusted" << "peakHoursRequired"
             << std::endl;
  // Iterate over all hydraulic_output objects in depthdf and print them
  for (const auto &ho : *(this->hyd_result)) {
    HYD_OUTPUT << ho->nodeID << "," << ho->reachID << "," << ho->downnodeID
               << "," << ho->upnodeID1 << "," << ho->upnodeID2 << ","
               << ho->stationname << "," << ho->station << ","
               << ho->reach_length_DS << "," << ho->reach_length_US1 << ","
               << ho->reach_length_US2 << "," << ho->flow << "," << ho->flow_lob
               << "," << ho->flow_main << "," << ho->flow_rob << ","
               << ho->min_elev << "," << ho->wsl << "," << ho->depth << ","
               << ho->hyd_depth << "," << ho->hyd_depth_lob << ","
               << ho->hyd_depth_main << "," << ho->hyd_depth_rob << ","
               << ho->top_width << "," << ho->top_width_lob << ","
               << ho->top_width_main << "," << ho->top_width_rob << ","
               << ho->velocity << "," << ho->velocity_lob << ","
               << ho->velocity_main << "," << ho->velocity_rob << ","
               << ho->k_total << "," << ho->k_lob << "," << ho->k_main << ","
               << ho->k_rob << "," << ho->alpha << "," << ho->area << ","
               << ho->area_lob << "," << ho->area_main << "," << ho->area_rob
               << "," << ho->hradius << "," << ho->hradius_lob << ","
               << ho->hradius_main << "," << ho->hradius_rob << ","
               << ho->wet_perimeter << "," << ho->wet_perimeter_lob << ","
               << ho->wet_perimeter_main << "," << ho->wet_perimeter_rob << ","
               << ho->energy_total << "," << ho->velocity_head << ","
               << ho->froude << "," << ho->sf << "," << ho->sf_avg << ","
               << ho->sbed << "," << ho->length_effective << ","
               << ho->head_loss << "," << ho->manning_lob << ","
               << ho->manning_main << "," << ho->manning_rob << ","
               << ho->manning_composite << "," << ho->k_total_areaconv << ","
               << ho->k_total_roughconv << "," << ho->k_total_disconv << ","
               << ho->alpha_areaconv << "," << ho->alpha_roughconv << ","
               << ho->alpha_disconv << "," << ho->nc_equalforce << ","
               << ho->nc_equalvelocity << "," << ho->nc_wavgwp << ","
               << ho->nc_wavgarea << "," << ho->nc_wavgconv << ","
               << ho->depth_critical << "," << ho->cp_iterations << ","
               << ho->k_err << "," << ho->ws_err << "," << ho->length_energyloss
               << "," << ho->length_effectiveadjusted << ","
               << ho->peak_hrs_required << std::endl;
  }
  HYD_OUTPUT.close();
}