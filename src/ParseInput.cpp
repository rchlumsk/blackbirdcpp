#include "Model.h"
#include "BlackbirdInclude.h"
#include "ParseLib.h"

bool ParseMainInputFile(CModel*& pModel, COptions*& pOptions);
//bool ParsePreprocessedTablesFile(CModel*& pModel, COptions*const& pOptions);
bool ParseBoundaryConditionsFile(CModel*& pModel, COptions*const& pOptions);
bool ParseGeometryFile(CModel*& pModel, COptions*const& pOptions);
void ImproperFormatWarning(std::string command, CParser* p, bool noisy);

//////////////////////////////////////////////////////////////////
/// \brief This method is the primary Blackbird input routine that parses input files, called by main
///
/// \details This method provides an interface by which the main method can parse input files\n
///   Files used include:\n
///   - \b [modelname].bbi: input file that determines general model settings
///   - \b [modelname].bbg: model geometry
///   - \b [modelname].bbb: boundary conditions
///
/// \param *&pModel [in/out] The input model object
/// \param *&pOptions [in/out] Global model options object
/// \return Boolean variable indicating success of parsing
//
bool ParseInputFiles(CModel*& pModel,
                     COptions*& pOptions)
{
  bool   runname_overridden(false);
  // Main input file (.bbi)
  //--------------------------------------------------------------------------------
  if (pOptions->run_name != "") { runname_overridden = true; }
  if (!ParseMainInputFile(pModel, pOptions)) {
    if (pOptions->bbi_filename.compare("nomodel.bbi") == 0) {
      ExitGracefully("A model input file name must be supplied as an argument to the Blackbird executable.", BAD_DATA);return false;
    }
    ExitGracefully("Cannot find or read .bbi file", BAD_DATA);return false;
  }

  // Geometry file (.bbg)
  //--------------------------------------------------------------------------------
  if (!ParseGeometryFile(pModel, pOptions)) {
    ExitGracefully("Cannot find or read .bbg file", BAD_DATA);return false;
  }

  // Boundary Conditions file (.bbb)
  //--------------------------------------------------------------------------------
  if (!ParseBoundaryConditionsFile(pModel, pOptions)) {
    ExitGracefully("Cannot find or read .bbb file", BAD_DATA);return false;
  }

  if (!pOptions->silent_run) {
    std::cout << "...model input successfully parsed" << std::endl;
    std::cout << std::endl;
  }

  return true;
}

///////////////////////////////////////////////////////////////////
/// \brief This local method (called by ParseInputFiles) reads an input .bbi file and generates
/// a new model with all options and processes created.
///
/// \param *filename [in] The fully-qualified file name of .bbi input file
/// \param *&pModel [in/out] Input object that determines general model settings
/// \param *&pOptions [in/out] Global model options information
/// \return Boolean   value indicating success of parsing
//
bool ParseMainInputFile(CModel*& pModel,
                        COptions*& pOptions)
{
  std::ifstream          INPUT;
  std::ifstream          INPUT2;           //For Secondary input
  CParser* pMainParser = NULL; //for storage of main parser while reading secondary files

  int               code;            //Parsing vars
  bool              ended(false);
  int               Len, line(0);
  char* s[MAXINPUTITEMS];

  //temp vars for handling depth sequences
  double dhand_max_depth(PLACEHOLDER);
  double dhand_depth_step(PLACEHOLDER);

  if (pOptions->noisy_run) {
    std::cout << "======================================================" << std::endl;
    std::cout << "Parsing Input File " << pOptions->bbi_filename << "..." << std::endl;
    std::cout << "======================================================" << std::endl;
  }

  INPUT.open(pOptions->bbi_filename.c_str());
  if (INPUT.fail()) { std::cout << "Cannot find file " << pOptions->bbi_filename << std::endl; return false; }

  CParser* p = new CParser(INPUT, pOptions->bbi_filename, line);

  //===============================================================================================
  // Sift through file, processing each command
  //===============================================================================================
  bool end_of_file = p->Tokenize(s, Len);
  while (!end_of_file)
  {
    if (ended) { break; }
    if (pOptions->noisy_run) { std::cout << "reading line " << p->GetLineNumber() << ": "; }

    /*assign code for switch statement
      ------------------------------------------------------------------
      <0           : ignored/special
      0   thru 100 : General Model Setup
      100 thru 200 : Calibration
      200 thru 300 : XSection Specific
      300 thru 400 : Reach Specific
      400 thru 500 : Postprocessing
      ------------------------------------------------------------------
    */

    code = 0;
    //---------------------SPECIAL -----------------------------
    if (Len == 0) { code = -1; }
    else if (!strcmp(s[0], "*")) { code = -2; }//comment
    else if (!strcmp(s[0], "%")) { code = -2; }//comment
    else if (!strcmp(s[0], "#")) { code = -2; }//comment
    else if (s[0][0] == '#') { code = -2; }//comment
    else if (!strcmp(s[0], ":End")) { code = -3; }//premature end of file
    //else if (!strcmp(s[0], ":RedirectToFile")) { code = -4; }//redirect to secondary file

    //-------------------- GENERAL MODEL SETUP OPTIONS ------------------------
    else if (!strcmp(s[0], ":ModelType")) { code = 2; }
    else if (!strcmp(s[0], ":RegimeType")) { code = 3; }
    else if (!strcmp(s[0], ":Tolerance")) { code = 4; }
    else if (!strcmp(s[0], ":IterationLimit")) { code = 5; }
    else if (!strcmp(s[0], ":WSLSplit")) { code = 6; }
    else if (!strcmp(s[0], ":ToleranceNormalDepth")) { code = 7; }
    else if (!strcmp(s[0], ":IterationLimitNormalDepth")) { code = 8; }
    else if (!strcmp(s[0], ":WSLSplitNormalDepth")) { code = 9; }
    else if (!strcmp(s[0], ":MaxRHRatio")) { code = 10; }
    else if (!strcmp(s[0], ":MinRHRatio")) { code = 11; }
    else if (!strcmp(s[0], ":ExtrapolateDepthTable")) { code = 12; }
    else if (!strcmp(s[0], ":NumExtrapolationPoints")) { code = 13; }
    else if (!strcmp(s[0], ":FrictionSlopeMethod")) { code = 15; }
    else if (!strcmp(s[0], ":EnforceDeltaLeff")) { code = 16; }
    else if (!strcmp(s[0], ":ReachLengthDelta")) { code = 17; }
    else if (!strcmp(s[0], ":ManningCompositeMethod")) { code = 18; }
    else if (!strcmp(s[0], ":SilentRun")) { code = 19; }
    else if (!strcmp(s[0], ":DHANDDepthSeq")) { code = 23; }
    else if (!strcmp(s[0], ":DHANDMaxDepth")) { code = 24; }
    else if (!strcmp(s[0], ":DHANDDepthStep")) { code = 25; }
    else if (!strcmp(s[0], ":WriteNetcdfFormat")) { code = 26; }
    else if (!strcmp(s[0], ":InputNCFile")) { code = 27; }
    else if (!strcmp(s[0], ":WritePngFormat")) { code = 28; }

    //-------------------- CALIBRATION PARAMETER ------------------------
    else if (!strcmp(s[0], ":RoughnessMultiplier")) { code = 100; }

    //-------------------- XSECTION SPECIFIC OPTIONS ------------------------
    else if (!strcmp(s[0], ":XSectionDX")) { code = 200; }
    else if (!strcmp(s[0], ":XSectionConveyanceMethod")) { code = 201; }
    else if (!strcmp(s[0], ":ManningEnforceValues")) { code = 202; }

    //-------------------- REACH SPECIFIC OPTIONS ------------------------
    else if (!strcmp(s[0], ":ReachConveyanceMethod")) { code = 300; }
    else if (!strcmp(s[0], ":ReachIntegrationMethod")) { code = 301; }

    //-------------------- POSTPROCESSING OPTIONS ------------------------
    else if (!strcmp(s[0], ":PostprocessingInterpolationMethod")) { code = 400; }
    else if (!strcmp(s[0], ":DHandMethod")) { code = 401; }
    else if (!strcmp(s[0], ":GISPath")) { code = 402; }

    switch (code)
    {
    case(-1):  //----------------------------------------------
    {/*Blank Line*/
      if (pOptions->noisy_run) { std::cout << "" << std::endl; }break;
    }
    case(-2):  //----------------------------------------------
    {/*Comment # */
      if (pOptions->noisy_run) { std::cout << "*" << std::endl; } break;
    }
    case(-3):  //----------------------------------------------
    {/*:End*/
      if (pOptions->noisy_run) { std::cout << "EOF" << std::endl; } ended = true; break;
    }
    case(-4):  //----------------------------------------------
    {/*:RedirectToFile*/
      std::string filename = "";
      for (int i = 1;i < Len;i++) { filename += s[i]; if (i < Len - 1) { filename += ' '; } }
      if (pOptions->noisy_run) { std::cout << "Redirect to file: " << filename << std::endl; }

      filename = CorrectForRelativePath(filename, pOptions->bbi_filename);

      INPUT2.open(filename.c_str());
      if (INPUT2.fail()) {
        std::string warn = "ParseMainInputFile: :RedirectToFile: Cannot find file " + filename;
        ExitGracefully(warn.c_str(), BAD_DATA);
      }
      else {
        if (pMainParser != NULL) {
            ExitGracefully("ParseMainInputFile::nested :RedirectToFile commands (in already redirected files) are not allowed.", BAD_DATA);
        }
        pMainParser = p;    //save pointer to primary parser
        p = new CParser(INPUT2, filename, line);//open new parser
      }
      break;
    }
    case(2):
    {/*:ModelType [string type]*/
      if (pOptions->noisy_run) { std::cout << "ModelType" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":ModelType", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "HAND_MANNING")) { pOptions->modeltype = enum_mt_method::HAND_MANNING; }
      else if (!strcmp(s[1], "STEADYFLOW")) { pOptions->modeltype = enum_mt_method::STEADYFLOW; }
      break;
    }
    case(3):
    {/*:RegimeType [string type]*/
      if (pOptions->noisy_run) { std::cout << "RegimeType" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":RegimeType", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "SUBCRITICAL")) { pOptions->regimetype = enum_rt_method::SUBCRITICAL; }
      else if (!strcmp(s[1], "SUPERCRITICAL")) { pOptions->regimetype = enum_rt_method::SUPERCRITICAL; }
      else if (!strcmp(s[1], "MIXED")) { pOptions->regimetype = enum_rt_method::MIXED; }
      break;
    }
    case(4):
    {/*:Tolerance [double tolerance]*/
      if (pOptions->noisy_run) { std::cout << "Tolerance" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":Tolerance", p, pOptions->noisy_run); break; }
      pOptions->tolerance_cp = std::atof(s[1]);
      break;
    }
    case(5):
    {/*:IterationLimit [int limit]*/
      if (pOptions->noisy_run) { std::cout << "IterationLimit" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":IterationLimit", p, pOptions->noisy_run); break; }
      pOptions->iteration_limit_cp = std::atoi(s[1]);
      break;
    }
    case(6):
    {/*:WSLSplit [double WSL_split]*/
      if (pOptions->noisy_run) { std::cout << "WSLSplit" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":WSLSplit", p, pOptions->noisy_run); break; }
      pOptions->next_WSL_split_cp = std::atof(s[1]);
      break;
    }
    case(7):
    {/*:ToleranceNormalDepth [double tolerance]*/
      if (pOptions->noisy_run) { std::cout << "ToleranceNormalDepth" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":ToleranceNormalDepth", p, pOptions->noisy_run); break; }
      pOptions->tolerance_nd = std::atof(s[1]);
      break;
    }
    case(8):
    {/*:IterationLimitNormalDepth [int limit]*/
      if (pOptions->noisy_run) { std::cout << "IterationLimitNormalDepth" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":IterationLimitNormalDepth", p, pOptions->noisy_run); break; }
      pOptions->iteration_limit_nd = std::atoi(s[1]);
      break;
    }
    case(9):
    {/*:WSLSplitNormalDepth [double WSL_split]*/
      if (pOptions->noisy_run) { std::cout << "WSLSplitNormalDepth" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":WSLSplitNormalDepth", p, pOptions->noisy_run); break; }
      pOptions->next_WSL_split_nd = std::atof(s[1]);
      break;
    }
    case(10):
    {/*:MaxRHRatio [double ratio]*/
      if (pOptions->noisy_run) { std::cout << "MaxRHRatio" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":MaxRHRatio", p, pOptions->noisy_run); break; }
      pOptions->max_RHSQ_ratio = std::atof(s[1]);
      break;
    }
    case(11):
    {/*:MinRHRatio [double ratio]*/
      if (pOptions->noisy_run) { std::cout << "MinRHRatio" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":MinRHRatio", p, pOptions->noisy_run); break; }
      pOptions->min_RHSQ_ratio = std::atof(s[1]);
      break;
    }
    case(12):
    {/*:ExtrapolateDepthTable [string method]*/
      if (pOptions->noisy_run) { std::cout << "ExtrapolateDepthTable" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":ExtrapolateDepthTable", p, pOptions->noisy_run); break; }
      std::istringstream(s[1]) >> pOptions->extrapolate_depth_table;
      break;
    }
    case(13):
    {/*:NumExtrapolationPoints [int num]*/
      if (pOptions->noisy_run) { std::cout << "NumExtrapolationPoints" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":NumExtrapolationPoints", p, pOptions->noisy_run); break; }
      pOptions->num_extrapolation_points = std::atoi(s[1]);
      break;
    }
    case(15):
    {/*:FrictionSlopeMethod [string method]*/
      if (pOptions->noisy_run) { std::cout << "FrictionSlopeMethod" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":FrictionSlopeMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "AVERAGE_CONVEYANCE")) { pOptions->friction_slope_method = enum_fs_method::AVERAGE_CONVEYANCE; }
      else if (!strcmp(s[1], "AVERAGE_FRICTION")) { pOptions->friction_slope_method = enum_fs_method::AVERAGE_FRICTION; }
      else if (!strcmp(s[1], "GEOMETRIC_FRICTION")) { pOptions->friction_slope_method = enum_fs_method::GEOMETRIC_FRICTION; }
      else if (!strcmp(s[1], "HARMONIC_FRICTION")) { pOptions->friction_slope_method = enum_fs_method::HARMONIC_FRICTION; }
      else if (!strcmp(s[1], "REACH_FRICTION")) { pOptions->friction_slope_method = enum_fs_method::REACH_FRICTION; }
      break;
    }
    case(16):
    {/*:EnforceDeltaLeff [bool enforce]*/
      if (pOptions->noisy_run) { std::cout << "EnforceDeltaLeff" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":EnforceDeltaLeff", p, pOptions->noisy_run); break; }
      std::istringstream(s[1]) >> pOptions->enforce_delta_Leff;
      break;
    }
    case(17):
    {/*:ReachLengthDelta [double length]*/
      if (pOptions->noisy_run) { std::cout << "ReachLengthDelta" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":ReachLengthDelta", p, pOptions->noisy_run); break; }
      pOptions->delta_reachlength = std::atof(s[1]);
      break;
    }
    case(18):
    {/*:ManningCompositeMethod [string method]*/
      if (pOptions->noisy_run) { std::cout << "ManningCompositeMethod" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":ManningCompositeMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "EQUAL_FORCE")) { pOptions->manning_composite_method = enum_mc_method::EQUAL_FORCE; }
      else if (!strcmp(s[1], "WEIGHTED_AVERAGE_AREA")) { pOptions->manning_composite_method = enum_mc_method::WEIGHTED_AVERAGE_AREA; }
      else if (!strcmp(s[1], "WEIGHTED_AVERAGE_WETPERIMETER")) { pOptions->manning_composite_method = enum_mc_method::WEIGHTED_AVERAGE_WETPERIMETER; }
      else if (!strcmp(s[1], "WEIGHTED_AVERAGE_CONVEYANCE")) { pOptions->manning_composite_method = enum_mc_method::WEIGHTED_AVERAGE_CONVEYANCE; }
      else if (!strcmp(s[1], "EQUAL_VELOCITY")) { pOptions->manning_composite_method = enum_mc_method::EQUAL_VELOCITY; }
      else if (!strcmp(s[1], "BLENDED_NC")) { pOptions->manning_composite_method = enum_mc_method::BLENDED_NC; }
      break;
    }
    case(19):
    {/*:SilentRun [bool silent]*/
      if (pOptions->noisy_run) { std::cout << "SilentRun" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":SilentRun", p, pOptions->noisy_run); break; }
      std::istringstream(s[1]) >> pOptions->silent_run;
      break;
    }
    case(23):
    {/*:DHANDDepthSeq [std::vector<double> sequence]*/
      if (pOptions->noisy_run) { std::cout << "DHANDDepthSeq" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":DHANDDepthSeq", p, pOptions->noisy_run); break; }
      if (dhand_max_depth != PLACEHOLDER && dhand_depth_step != PLACEHOLDER) {
        WriteWarning(":DHANDDepthSeq dhand_depth_seq has already been defined. ignoring command", pOptions->noisy_run);
      } else {
        for (int i = 1; i < Len; i++) {
          pModel->dhand_depth_seq.push_back(std::atof(s[i]));
        }
      }
      break;
    }
    case(24):
    {/*:DHANDMaxDepth [double max_depth]*/
      if (pOptions->noisy_run) { std::cout << "DHANDMaxDepth" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":DHANDMaxDepth", p, pOptions->noisy_run); break; }
      if (pModel->dhand_depth_seq.size() > 0) {
        WriteWarning(":DHANDMaxDepth dhand_depth_seq has already been defined. ignoring command", pOptions->noisy_run);
      } else if (dhand_max_depth != PLACEHOLDER) {
        WriteWarning(":DHANDMaxDepth dhand_max_depth has already been defined. ignoring command", pOptions->noisy_run);
      } else {
        dhand_max_depth = std::atof(s[1]);
      }
      break;
    }
    case(25):
    {/*:DHANDDepthStep [double depth_step]*/
      if (pOptions->noisy_run) { std::cout << "DHANDDepthStep" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":DHANDDepthStep", p, pOptions->noisy_run); break; }
      if (pModel->dhand_depth_seq.size() > 0) {
        WriteWarning(":DHANDDepthStep dhand_depth_seq has already been defined. ignoring command", pOptions->noisy_run);
      } else if (dhand_depth_step != PLACEHOLDER) {
        WriteWarning(":DHANDDepthStep dhand_depth_step has already been defined. ignoring command", pOptions->noisy_run);
      } else {
        dhand_depth_step = std::atof(s[1]);
      }
      break;
    }
    case(26):
    {/*:WriteNetcdfFormat*/
      if (pOptions->noisy_run) { std::cout << "WriteNetcdfFormat" << std::endl; }
      pOptions->out_format = enum_gridded_format::NETCDF;
      break;
    }
    case(27):
    {/*:InputNCFile [string filename]*/
      if (pOptions->noisy_run) { std::cout << "InputNCFile" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":InputNCFile", p, pOptions->noisy_run); break; }
      pOptions->in_nc_name = std::string(s[1]);
      break;
    }
    case(28):
    {/*:WritePngFormat*/
      if (pOptions->noisy_run) { std::cout << "WritePngFormat" << std::endl; }
      pOptions->out_format = enum_gridded_format::PNG;
      break;
    }
    case(100):
    {/*:RoughnessMultiplier [double mult]*/
      if (pOptions->noisy_run) { std::cout << "RoughnessMultiplier" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":RoughnessMultiplier", p, pOptions->noisy_run); break; }
      pOptions->roughness_multiplier = std::atof(s[1]);
      break;
    }
    case(200):
    {/*:XSectionDX [double dx]*/
      if (pOptions->noisy_run) { std::cout << "XSectionDX" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":XSectionDX", p, pOptions->noisy_run); break; }
      pOptions->dx = std::atof(s[1]);
      break;
    }
    case(201):
    {/*:XSectionConveyanceMethod [string method]*/
      if (pOptions->noisy_run) { std::cout << "XSectionConveyanceMethod" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":XSectionConveyanceMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "OVERBANK_CONVEYANCE")) { pOptions->xsection_conveyance_method = enum_xsc_method::OVERBANK_CONVEYANCE; }
      else if (!strcmp(s[1], "DEFAULT_CONVEYANCE")) { pOptions->xsection_conveyance_method = enum_xsc_method::DEFAULT_CONVEYANCE; }
      else if (!strcmp(s[1], "COORDINATE_CONVEYANCE")) { pOptions->xsection_conveyance_method = enum_xsc_method::COORDINATE_CONVEYANCE; }
      else if (!strcmp(s[1], "DISCRETIZED_CONVEYANCE_XS")) { pOptions->xsection_conveyance_method = enum_xsc_method::DISCRETIZED_CONVEYANCE_XS; }
      else if (!strcmp(s[1], "AREAWEIGHTED_CONVEYANCE_ONECALC_XS")) { pOptions->xsection_conveyance_method = enum_xsc_method::AREAWEIGHTED_CONVEYANCE_ONECALC_XS; }
      else if (!strcmp(s[1], "AREAWEIGHTED_CONVEYANCE")) { pOptions->xsection_conveyance_method = AREAWEIGHTED_CONVEYANCE; }
      break;
    }
    case(202):
    {/*:MannningEnforceValues [bool enforce]*/
      if (pOptions->noisy_run) { std::cout << "MannningEnforceValues" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":MannningEnforceValues", p, pOptions->noisy_run); break; }
      std::istringstream(s[1]) >> pOptions->manning_enforce_values;
      break;
    }
    case(300):
    {/*:ReachConveyanceMethod [string method]*/
      if (pOptions->noisy_run) { std::cout << "ReachConveyanceMethod" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":ReachConveyanceMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "DISCRETIZED_CONVEYANCE_R")) { pOptions->reach_conveyance_method = enum_rc_method::DISCRETIZED_CONVEYANCE_R; }
      else if (!strcmp(s[1], "AREAWEIGHTED_CONVEYANCE_ONECALC_R")) { pOptions->reach_conveyance_method = enum_rc_method::AREAWEIGHTED_CONVEYANCE_ONECALC_R; }
      else if (!strcmp(s[1], "ROUGHZONE_CONVEYANCE")) { pOptions->reach_conveyance_method = enum_rc_method::ROUGHZONE_CONVEYANCE; }
      else if (!strcmp(s[1], "BLENDED_CONVEYANCE")) { pOptions->reach_conveyance_method = enum_rc_method::BLENDED_CONVEYANCE; }
      break;
    }
    case(301):
    {/*:ReachIntegrationMethod [string method]*/
      if (pOptions->noisy_run) { std::cout << "ReachIntegrationMethod" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":ReachIntegrationMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "EFFECTIVE_LENGTH")) { pOptions->reach_integration_method = enum_ri_method::EFFECTIVE_LENGTH; }
      else if (!strcmp(s[1], "REACH_LENGTH")) { pOptions->reach_integration_method = enum_ri_method::REACH_LENGTH; }
      break;
    }
    case(400):
    {/*:PostprocessingInterpolationMethod [string method]*/
      if (pOptions->noisy_run) { std::cout << "PostprocessingInterpolationMethod" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":PostprocessingInterpolationMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "NONE")) { pOptions->interpolation_postproc_method = enum_ppi_method::NONE; }
      else if (!strcmp(s[1], "CATCHMENT_HAND")) { pOptions->interpolation_postproc_method = enum_ppi_method::CATCHMENT_HAND; }
      else if (!strcmp(s[1], "CATCHMENT_DHAND")) { pOptions->interpolation_postproc_method = enum_ppi_method::CATCHMENT_DHAND; }
      else if (!strcmp(s[1], "INTERP_HAND")) { pOptions->interpolation_postproc_method = enum_ppi_method::INTERP_HAND; }
      else if (!strcmp(s[1], "INTERP_DHAND")) { pOptions->interpolation_postproc_method = enum_ppi_method::INTERP_DHAND; }
      else if (!strcmp(s[1], "INTERP_DHAND_WSLCORR")) { pOptions->interpolation_postproc_method = enum_ppi_method::INTERP_DHAND_WSLCORR; }
      break;
    }
    case(401):
    {/*:DHandMethod [string method]*/
      if (pOptions->noisy_run) { std::cout << "DHandMethod" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":DHandMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "INTERPOLATE")) { pOptions->dhand_method = enum_dh_method::INTERPOLATE; }
      else if (!strcmp(s[1], "FLOOR")) { pOptions->dhand_method = enum_dh_method::FLOOR; }
      break;
    }
    case(402):
    {/*:GISPath [string path_to_folder]*/
      if (pOptions->noisy_run) { std::cout << "GISPath" << std::endl; }
      if (Len < 2) { ImproperFormatWarning(":GISPath", p, pOptions->noisy_run); break; }
      pOptions->gis_path = s[1];
      break;
    }
    default://----------------------------------------------
    {
      char firstChar = *(s[0]);
      if (firstChar == ':')
      {
        if (!strcmp(s[0], ":FileType")) { if (pOptions->noisy_run) { std::cout << "Filetype" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":Application")) { if (pOptions->noisy_run) { std::cout << "Application" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":Version")) { if (pOptions->noisy_run) { std::cout << "Version" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":WrittenBy")) { if (pOptions->noisy_run) { std::cout << "WrittenBy" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":CreationDate")) { if (pOptions->noisy_run) { std::cout << "CreationDate" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":SourceFile")) { if (pOptions->noisy_run) { std::cout << "SourceFile" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":Name")) { if (pOptions->noisy_run) { std::cout << "Name" << std::endl; } }//do nothing
        else
        {
          std::string warn = "IGNORING unrecognized command: " + std::string(s[0]) + " in .bbi file";
          WriteWarning(warn, pOptions->noisy_run);
        }
    }
      else
      {
        std::string errString = "Unrecognized command in .bbi file:\n   " + std::string(s[0]);
        ExitGracefully(errString.c_str(), BAD_DATA_WARN);
      }
      break;
    }
    }//switch

    end_of_file = p->Tokenize(s, Len);

    //return after file redirect, if in secondary file
    if ((end_of_file) && (pMainParser != NULL))
    {
      INPUT2.clear();
      INPUT2.close();
      delete p;
      p = pMainParser;
      pMainParser = NULL;
      end_of_file = p->Tokenize(s, Len);
    }
  } //end while (!end_of_file)
  INPUT.close();

  // Clean up dhand_depth_seq logic
  if (dhand_max_depth != PLACEHOLDER || dhand_depth_step != PLACEHOLDER) {
    if (dhand_max_depth == PLACEHOLDER) {
      WriteWarning("ParseInput.cpp: dhand_depth_step provided without "
                   "dhand_max_depth. dhand_depth_seq cannot be determined",
                   pOptions->noisy_run);
    } else if (dhand_depth_step == PLACEHOLDER || dhand_depth_step == 0) {
      WriteWarning("ParseInput.cpp: dhand_max_depth provided without "
                   "dhand_depth_step. dhand_depth_seq cannot be determined",
                   pOptions->noisy_run);
    } else {
      for (double i = 0; i < dhand_max_depth; i += dhand_depth_step) {
        pModel->dhand_depth_seq.push_back(i);
      }
    }
  }
  
  //===============================================================================================
  //Check input quality
  //===============================================================================================
  // tbd



  delete p; p = NULL;
  return true;
}

///////////////////////////////////////////////////////////////////
/// \brief writes warning to Blackbird errors file and screen for improper formatting
///
/// \param command [in] string command name (e.g., ":CanopyDrip")
/// \param *p [in]  parser object
/// \param noisy [in] boolean: if true, warning written to screen
//
void ImproperFormatWarning(std::string command, CParser* p, bool noisy)
{
  std::string warn;
  warn = command + " command: improper line length at line " + std::to_string(p->GetLineNumber());
  WriteWarning(warn, noisy);
}