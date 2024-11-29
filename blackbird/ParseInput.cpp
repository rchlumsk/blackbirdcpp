#include "Model.h"
#include "BlackbirdInclude.h"

bool ParseMainInputFile(CModel*& pModel, COptions*& pOptions);
bool ParsePreprocessedTablesFile(CModel*& pModel, COptions*& pOptions);
bool ParseBoundaryConditionsFile(CModel*& pModel, COptions*& pOptions);
bool ParseGeometryFile(CModel*& pModel, COptions*& pOptions);
void ImproperFormatWarning(string command, CParser* p, bool noisy);
void AddProcess(CModel* pModel, CHydroProcessABC* pMover, CProcessGroup* pProcGroup);

void FromToErrorCheck(string cmd, string sFrom, string sTo, sv_type tFrom, sv_type tTo, CModel* pModel, CStateVariable* pStateVar);

evap_method    ParseEvapMethod(const string s);
potmelt_method ParsePotMeltMethod(const string s);

//////////////////////////////////////////////////////////////////
/// \brief This method is the primary Raven input routine that parses input files, called by main
///
/// \details This method provides an interface by which the main method can parse input files\n
///   Files used include:\n
///   - \b [modelname].rvi: input file that determines general model settings, nHRUs, timesteps,etc
///   - \b [modelname].rvp: default properties for LULT & soil type
///   - \b [modelname].rvh: HRU/basin property files
///   - \b [modelname].rvt: time series precip/temp input
///   - \b [modelname].rvc: initial conditions file
///   - \b [modelname].rvg: groundwater properties file
///
/// \param *&pModel [in] The input model object
/// \param &Options [in] Global model options information
/// \return Boolean variable indicating success of parsing
//
bool ParseInputFiles(CModel*& pModel,
                     COptions *& pOptions)
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

  // Preprocessed Tables file (.bbp)
  //--------------------------------------------------------------------------------
  if (!ParsePreprocessedTablesFile(pModel, pOptions)) {
    ExitGracefully("Cannot find or read .bbp file", BAD_DATA);return false;
  }

  // Boundary Conditions file (.bbb)
  //--------------------------------------------------------------------------------
  if (!ParseBoundaryConditionsFile(pModel, pOptions)) {
    ExitGracefully("Cannot find or read .bbb file", BAD_DATA);return false;
  }

  // Geometry file (.bbg)
  //--------------------------------------------------------------------------------
  if (!ParseGeometryFile(pModel, pOptions)) {
    //note: called after .rvh read so it can update class AND subbasin properties
    ExitGracefully("Cannot find or read NetCDF parameter update file", BAD_DATA); return false;
  }

  if (!Options.silent) {
    cout << "...model input successfully parsed" << endl;
    cout << endl;
  }

  return true;
}

///////////////////////////////////////////////////////////////////
/// \brief This local method (called by ParseInputFiles) reads an input .rvi file and generates
/// a new model with all options and processes created.
///
/// \param *filename [in] The fully-qualified file name of .rvi input file
/// \param *&pModel [in] Input object that determines general model settings
/// \param &Options [in] Global model options information
/// \return Boolean   value indicating success of parsing
//
bool ParseMainInputFile(CModel*& pModel,
                        COptions*& pOptions)
{
  int i;
  CHydroProcessABC* pMover;
  CmvPrecipitation* pPrecip = NULL;
  CProcessGroup* pProcGroup = NULL;
  CProcessGroup* pProcGroupOuter = NULL; //for nested processgroups
  CGroundwaterModel* pGW = NULL;
  CEnthalpyModel* pEnthalpyModel = NULL;
  bool              transprepared(false);
  bool              runname_overridden(false);
  bool              runmode_overridden(false);
  bool              rundir_overridden(false);
  int               num_ensemble_members = 1;
  unsigned int      random_seed = 0; //actually random
  std::ifstream          INPUT;
  std::ifstream          INPUT2;           //For Secondary input
  CParser* pMainParser = NULL; //for storage of main parser while reading secondary files
  bool              in_ifmode_statement = false;

  int               tmpN;
  sv_type* tmpS;
  int* tmpLev;

  int               code;            //Parsing vars
  bool              ended(false);
  int               Len, line(0);
  char* s[MAXINPUTITEMS];

  tmpS = new sv_type[MAX_STATE_VARS];
  tmpLev = new int[MAX_STATE_VARS];

  if (pOptions->noisy_run) {
    cout << "======================================================" << endl;
    cout << "Parsing Input File " << pOptions->bbi_filename << "..." << endl;
    cout << "======================================================" << endl;
  }

  INPUT.open(pOptions->bbi_filename.c_str());
  if (INPUT.fail()) { cout << "Cannot find file " << pOptions->rvi_filename << endl; return false; }

  // the 'strategy' is to go filling StateVariable objects with the information on the go and then
  // pass them to the model object at the end of the parsing process, as the model object may be
  // created after the first StateVariable attributes are read.
  CStateVariable* pStateVar = new CStateVariable();

  CParser* p = new CParser(INPUT, Options.rvi_filename, line);


  pModel = NULL;
  pMover = NULL;

  //===============================================================================================
  // Sift through file, processing each command
  //===============================================================================================
  bool end_of_file = p->Tokenize(s, Len);
  while (!end_of_file)
  {
    if (ended) { break; }
    if (pOptions->noisy_run) { cout << "reading line " << p->GetLineNumber() << ": "; }

    /*assign code for switch statement
      ------------------------------------------------------------------
      tbd
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
    else if (!strcmp(s[0], ":ModelName")) { code = 1; }
    else if (!strcmp(s[0], ":ModelType")) { code = 2; }
    else if (!strcmp(s[0], ":RegimeType")) { code = 3; }
    else if (!strcmp(s[0], ":Tolerance")) { code = 4; }
    else if (!strcmp(s[0], ":IterationLimit")) { code = 5; }
    else if (!strcmp(s[0], ":ToleranceNormalDepth")) { code = 6; }
    else if (!strcmp(s[0], ":IterationLimitNormalDepth")) { code = 7; }
    else if (!strcmp(s[0], ":WSLSplitNormalDepth")) { code = 8; }
    else if (!strcmp(s[0], ":MaxRHRatio")) { code = 9; }
    else if (!strcmp(s[0], ":MinRHRatio")) { code = 10; }
    else if (!strcmp(s[0], ":ExtrapolationMethod")) { code = 11; }
    else if (!strcmp(s[0], ":NumExtrapolationPoints")) { code = 12; }
    else if (!strcmp(s[0], ":DynamicHAND")) { code = 13; }
    else if (!strcmp(s[0], ":FrictionSlopeMethod")) { code = 14; }
    else if (!strcmp(s[0], ":EnforceDeltaLeff")) { code = 15; }
    else if (!strcmp(s[0], ":ReachLengthDelta")) { code = 16; }
    else if (!strcmp(s[0], ":ManningCompositeMethod")) { code = 17; }
    else if (!strcmp(s[0], ":SilentRun")) { code = 18; }

    //-------------------- CALIBRATION PARAMETER ------------------------
    else if (!strcmp(s[0], ":RoughnessMultiplier")) { code = 100; }

    //-------------------- XSECTION SPECIFIC OPTIONS ------------------------
    else if (!strcmp(s[0], ":XSectionDX")) { code = 200; }
    else if (!strcmp(s[0], ":UseOverbankCalcs")) { code = 201; }
    else if (!strcmp(s[0], ":XSectionConveyanceMethod")) { code = 202; }
    else if (!strcmp(s[0], ":ManningEnforceValues")) { code = 203; }

    //-------------------- REACH SPECIFIC OPTIONS ------------------------
    else if (!strcmp(s[0], ":ReachConveyanceMethod")) { code = 300; }
    else if (!strcmp(s[0], ":ReachIntegrationMethod")) { code = 301; }

    //-------------------- POSTPROCESSING OPTIONS ------------------------
    else if (!strcmp(s[0], ":PostprocessingInterpolationMethod")) { code = 400; }

    switch (code)
    {
    case(-1):  //----------------------------------------------
    {/*Blank Line*/
      if (pOptions->noisy_run) { cout << "" << endl; }break;
    }
    case(-2):  //----------------------------------------------
    {/*Comment # */
      if (pOptions->noisy_run) { cout << "*" << endl; } break;
    }
    case(-3):  //----------------------------------------------
    {/*:End*/
      if (pOptions->noisy_run) { cout << "EOF" << endl; } ended = true; break;
    }
    case(-4):  //----------------------------------------------
    {/*:RedirectToFile*/
      std::string filename = "";
      for (int i = 1;i < Len;i++) { filename += s[i]; if (i < Len - 1) { filename += ' '; } }
      if (pOptions->noisy_run) { cout << "Redirect to file: " << filename << endl; }

      filename = CorrectForRelativePath(filename, Options.rvt_filename);

      INPUT2.open(filename.c_str());
      if (INPUT2.fail()) {
        std::string warn = ":RedirectToFile: Cannot find file " + filename;
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
    case(1):
    {/*:ModelName [string name]*/
      if (pOptions->noisy_run) { cout << "ModelName" << endl; }
      if (Len < 2) { ImproperFormatWarning(":ModelName", p, pOptions->noisy_run); break; }
      pOptions->modelname = s[1];
    }
    case(2):
    {/*:ModelType [string type]*/
      if (pOptions->noisy_run) { cout << "ModelType" << endl; }
      if (Len < 2) { ImproperFormatWarning(":ModelType", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "HAND_MANNING")) { pOptions->modeltype = HAND_MANNING; }
      else if (!strcmp(s[1], "STEADYFLOW")) { pOptions->modeltype = STEADYFLOW; }
    }
    case(3):
    {/*:RegimeType [string type]*/
      if (pOptions->noisy_run) { cout << "RegimeType" << endl; }
      if (Len < 2) { ImproperFormatWarning(":RegimeType", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "SUBCRITICAL")) { pOptions->regimetype = SUBCRITICAL; }
      else if (!strcmp(s[1], "SUPERCRITICAL")) { pOptions->regimetype = SUPERCRITICAL; }
      else if (!strcmp(s[1], "MIXED")) { pOptions->regimetype = MIXED; }
    }
    case(4):
    {/*:Tolerance [double tolerance]*/
      if (pOptions->noisy_run) { cout << "Tolerance" << endl; }
      if (Len < 2) { ImproperFormatWarning(":Tolerance", p, pOptions->noisy_run); break; }
      pOptions->tolerance_cp = std::atof(s[1]);
    }
    case(5):
    {/*:IterationLimit [int limit]*/
      if (pOptions->noisy_run) { cout << "IterationLimit" << endl; }
      if (Len < 2) { ImproperFormatWarning(":IterationLimit", p, pOptions->noisy_run); break; }
      pOptions->iteration_limit_cp = std::atoi(s[1]);
    }
    case(6):
    {/*:ToleranceNormalDepth [double tolerance]*/
      if (pOptions->noisy_run) { cout << "ToleranceNormalDepth" << endl; }
      if (Len < 2) { ImproperFormatWarning(":ToleranceNormalDepth", p, pOptions->noisy_run); break; }
      pOptions->tolerance_nd = std::atof(s[1]);
    }
    case(7):
    {/*:IterationLimitNormalDepth [int limit]*/
      if (pOptions->noisy_run) { cout << "IterationLimitNormalDepth" << endl; }
      if (Len < 2) { ImproperFormatWarning(":IterationLimitNormalDepth", p, pOptions->noisy_run); break; }
      pOptions->iteration_limit_nd = std::atoi(s[1]);
    }
    case(8):
    {/*:WSLSplitNormalDepth [double WSL_split]*/
      if (pOptions->noisy_run) { cout << "WSLSplitNormalDepth" << endl; }
      if (Len < 2) { ImproperFormatWarning(":WSLSplitNormalDepth", p, pOptions->noisy_run); break; }
      pOptions->next_WSL_split_nd = std::atof(s[1]);
    }
    case(9):
    {/*:MaxRHRatio [double ratio]*/
      if (pOptions->noisy_run) { cout << "MaxRHRatio" << endl; }
      if (Len < 2) { ImproperFormatWarning(":MaxRHRatio", p, pOptions->noisy_run); break; }
      pOptions->max_RHSQ_ratio = std::atof(s[1]);
    }
    case(10):
    {/*:MinRHRatio [double ratio]*/
      if (pOptions->noisy_run) { cout << "MinRHRatio" << endl; }
      if (Len < 2) { ImproperFormatWarning(":MinRHRatio", p, pOptions->noisy_run); break; }
      pOptions->min_RHSQ_ratio = std::atof(s[1]);
    }
    case(11):
    {/*:ExtrapolationMethod [string method]*/ // ENUM??
    }
    case(12):
    {/*:NumExtrapolationPoints [int num]*/
      if (pOptions->noisy_run) { cout << "NumExtrapolationPoints" << endl; }
      if (Len < 2) { ImproperFormatWarning(":NumExtrapolationPoints", p, pOptions->noisy_run); break; }
      pOptions->num_extrapolation_points = std::atoi(s[1]);
    }
    case(13):
    {/*:DynamicHAND [bool dhand]*/
      if (pOptions->noisy_run) { cout << "DynamicHAND" << endl; }
      if (Len < 2) { ImproperFormatWarning(":DynamicHAND", p, pOptions->noisy_run); break; }
      std::istringstream(s[1]) >> pOptions->use_dhand;
    }
    case(14):
    {/*:FrictionSlopeMethod [string method]*/
      if (pOptions->noisy_run) { cout << "FrictionSlopeMethod" << endl; }
      if (Len < 2) { ImproperFormatWarning(":FrictionSlopeMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "AVERAGE_CONVEYANCE")) { pOptions->friction_slope_method = AVERAGE_CONVEYANCE; }
      else if (!strcmp(s[1], "AVERAGE_FRICTION")) { pOptions->friction_slope_method = AVERAGE_FRICTION; }
      else if (!strcmp(s[1], "GEOMETRIC_FRICTION")) { pOptions->friction_slope_method = GEOMETRIC_FRICTION; }
      else if (!strcmp(s[1], "HARMONIC_FRICTION")) { pOptions->friction_slope_method = HARMONIC_FRICTION; }
      else if (!strcmp(s[1], "REACH_FRICTION")) { pOptions->friction_slope_method = REACH_FRICTION; }
    }
    case(15):
    {/*:EnforceDeltaLeff [bool enforce]*/
      if (pOptions->noisy_run) { cout << "EnforceDeltaLeff" << endl; }
      if (Len < 2) { ImproperFormatWarning(":EnforceDeltaLeff", p, pOptions->noisy_run); break; }
      std::istringstream(s[1]) >> pOptions->enforce_delta_Leff;
    }
    case(16):
    {/*:ReachLengthDelta [double length]*/
      if (pOptions->noisy_run) { cout << "ReachLengthDelta" << endl; }
      if (Len < 2) { ImproperFormatWarning(":ReachLengthDelta", p, pOptions->noisy_run); break; }
      pOptions->delta_reachlength = std::atof(s[1]);
    }
    case(17):
    {/*:ManningCompositeMethod [string method]*/
      if (pOptions->noisy_run) { cout << "ManningCompositeMethod" << endl; }
      if (Len < 2) { ImproperFormatWarning(":ManningCompositeMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "EQUAL_FORCE")) { pOptions->manning_composite_method = EQUAL_FORCE; }
      else if (!strcmp(s[1], "WEIGHTED_AVERAGE_AREA")) { pOptions->manning_composite_method = WEIGHTED_AVERAGE_AREA; }
      else if (!strcmp(s[1], "WEIGHTED_AVERAGE_WETPERIMETER")) { pOptions->manning_composite_method = WEIGHTED_AVERAGE_WETPERIMETER; }
      else if (!strcmp(s[1], "WEIGHTED_AVERAGE_CONVEYANCE")) { pOptions->manning_composite_method = WEIGHTED_AVERAGE_CONVEYANCE; }
      else if (!strcmp(s[1], "EQUAL_VELOCITY")) { pOptions->manning_composite_method = EQUAL_VELOCITY; }
      else if (!strcmp(s[1], "BLENDED_NC")) { pOptions->manning_composite_method = BLENDED_NC; }
    }
    case(18):
    {/*:SilentRun [bool silent]*/
      if (pOptions->noisy_run) { cout << "SilentRun" << endl; }
      if (Len < 2) { ImproperFormatWarning(":SilentRun", p, pOptions->noisy_run); break; }
      std::istringstream(s[1]) >> pOptions->silent_run;
    }
    case(100):
    {/*:RoughnessMultiplier [double mult]*/
      if (pOptions->noisy_run) { cout << "RoughnessMultiplier" << endl; }
      if (Len < 2) { ImproperFormatWarning(":RoughnessMultiplier", p, pOptions->noisy_run); break; }
      pOptions->roughness_multiplier = std::atof(s[1]);
    }
    case(200):
    {/*:XSectionDX [double dx]*/
      if (pOptions->noisy_run) { cout << "XSectionDX" << endl; }
      if (Len < 2) { ImproperFormatWarning(":XSectionDX", p, pOptions->noisy_run); break; }
      pOptions->dx = std::atof(s[1]);
    }
    case(201):
    {/*:UseOverbankCalcs [double dx]*/ // ENUM?? BOOL?? 
    }
    case(202):
    {/*:XSectionConveyanceMethod [string method]*/
      if (pOptions->noisy_run) { cout << "XSectionConveyanceMethod" << endl; }
      if (Len < 2) { ImproperFormatWarning(":XSectionConveyanceMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "OVERBANK_CONVEYANCE")) { pOptions->xsection_conveyance_method = OVERBANK_CONVEYANCE; }
      else if (!strcmp(s[1], "DEFAULT_CONVEYANCE")) { pOptions->xsection_conveyance_method = DEFAULT_CONVEYANCE; }
      else if (!strcmp(s[1], "COORDINATE_CONVEYANCE")) { pOptions->xsection_conveyance_method = COORDINATE_CONVEYANCE; }
      else if (!strcmp(s[1], "DISCRETIZED_CONVEYANCE")) { pOptions->xsection_conveyance_method = DISCRETIZED_CONVEYANCE; }
      else if (!strcmp(s[1], "AREAWEIGHTED_CONVEYANCE_ONECALC")) { pOptions->xsection_conveyance_method = AREAWEIGHTED_CONVEYANCE_ONECALC; }
      else if (!strcmp(s[1], "AREAWEIGHTED_CONVEYANCE")) { pOptions->xsection_conveyance_method = AREAWEIGHTED_CONVEYANCE; }
    }
    case(203):
    {/*:MannningEnforceValues [bool enforce]*/
      if (pOptions->noisy_run) { cout << "MannningEnforceValues" << endl; }
      if (Len < 2) { ImproperFormatWarning(":MannningEnforceValues", p, pOptions->noisy_run); break; }
      std::istringstream(s[1]) >> pOptions->manning_enforce_values;
    }
    case(300):
    {/*:ReachConveyanceMethod [string method]*/
      if (pOptions->noisy_run) { cout << "ReachConveyanceMethod" << endl; }
      if (Len < 2) { ImproperFormatWarning(":ReachConveyanceMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "DISCRETIZED_CONVEYANCE")) { pOptions->reach_conveyance_method = DISCRETIZED_CONVEYANCE; }
      else if (!strcmp(s[1], "AREAWEIGHTED_CONVEYANCE_ONECALC")) { pOptions->reach_conveyance_method = AREAWEIGHTED_CONVEYANCE_ONECALC; }
      else if (!strcmp(s[1], "ROUGHZONE_CONVEYANCE")) { pOptions->reach_conveyance_method = ROUGHZONE_CONVEYANCE; }
      else if (!strcmp(s[1], "BLENDED_CONVEYANCE")) { pOptions->reach_conveyance_method = BLENDED_CONVEYANCE; }
    }
    case(301):
    {/*:ReachIntegrationMethod [string method]*/
      if (pOptions->noisy_run) { cout << "ReachIntegrationMethod" << endl; }
      if (Len < 2) { ImproperFormatWarning(":ReachIntegrationMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "EFFECTIVE_LENGTH")) { pOptions->reach_integration_method = EFFECTIVE_LENGTH; }
      else if (!strcmp(s[1], "REACH_LENGTH")) { pOptions->reach_integration_method = REACH_LENGTH; }
    }
    case(400):
    {/*:PostprocessingInterpolationMethod [string method]*/
      if (pOptions->noisy_run) { cout << "PostprocessingInterpolationMethod" << endl; }
      if (Len < 2) { ImproperFormatWarning(":PostprocessingInterpolationMethod", p, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "CATCHMENT_HAND")) { pOptions->interpolation_postproc_method = CATCHMENT_HAND; }
      else if (!strcmp(s[1], "CATCHMENT_DHAND")) { pOptions->interpolation_postproc_method = CATCHMENT_DHAND; }
      else if (!strcmp(s[1], "INTERP_HAND")) { pOptions->interpolation_postproc_method = INTERP_HAND; }
      else if (!strcmp(s[1], "INTERP_DHAND")) { pOptions->interpolation_postproc_method = INTERP_DHAND; }
      else if (!strcmp(s[1], "INTERP_DHAND_WSLCORR")) { pOptions->interpolation_postproc_method = INTERP_DHAND_WSLCORR; }
    }
    default://----------------------------------------------
    {
      char firstChar = *(s[0]);
      if (firstChar == ':')
      {
        if (!strcmp(s[0], ":FileType")) { if (pOptions->noisy_run) { cout << "Filetype" << endl; } }//do nothing
        else if (!strcmp(s[0], ":Application")) { if (pOptions->noisy_run) { cout << "Application" << endl; } }//do nothing
        else if (!strcmp(s[0], ":Version")) { if (pOptions->noisy_run) { cout << "Version" << endl; } }//do nothing
        else if (!strcmp(s[0], ":WrittenBy")) { if (pOptions->noisy_run) { cout << "WrittenBy" << endl; } }//do nothing
        else if (!strcmp(s[0], ":CreationDate")) { if (pOptions->noisy_run) { cout << "CreationDate" << endl; } }//do nothing
        else if (!strcmp(s[0], ":SourceFile")) { if (pOptions->noisy_run) { cout << "SourceFile" << endl; } }//do nothing
        else if (!strcmp(s[0], ":Name")) { if (pOptions->noisy_run) { cout << "Name" << endl; } }//do nothing
        else
        {
          std::string warn = "IGNORING unrecognized command: " + std::string(s[0]) + " in .rvi file";
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
  // LEFT OFF HERE
  //===============================================================================================
  //Check input quality
  //===============================================================================================
  ExitGracefullyIf(Options.timestep <= 0,
      "ParseMainInputFile::Must have a postitive time step", BAD_DATA);
  ExitGracefullyIf(Options.duration < 0,
      "ParseMainInputFile::Model duration less than zero. Make sure :EndDate is after :StartDate.", BAD_DATA_WARN);
  ExitGracefullyIf((pModel->GetStateVarIndex(CONVOLUTION, 0) != DOESNT_EXIST) && (pModel->GetTransportModel()->GetNumConstituents() > 0),
      "ParseMainInputFile: cannot currently perform transport with convolution processes", BAD_DATA);

  if ((Options.nNetCDFattribs > 0) && (Options.output_format != OUTPUT_NETCDF)) {
    WriteAdvisory("ParseMainInputFile: NetCDF attributes were specified but output format is not NetCDF.", pOptions->noisy_run);
  }
  for (int i = 0; i < pModel->GetNumStateVars();i++) {
    if ((pModel->GetStateVarType(i) == SOIL) && ((pModel->GetStateVarLayer(i)) > (Options.num_soillayers - 1))) {
      string warn = "A soil variable with an index (" + to_string(pModel->GetStateVarLayer(i)) + ") greater than that allowed by the limiting number of layers indicated in the :SoilModel command (" + to_string(Options.num_soillayers) + ") was included in the .rvi file";
      ExitGracefully(warn.c_str(), BAD_DATA);
    }
  }
  if (Options.SW_radiation == SW_RAD_DATA) {
    Options.SW_cloudcovercorr = SW_CLOUD_CORR_NONE;
    WriteWarning("Cloud cover corrections have been set to 'NONE', since the shortwave radiation method is SW_RAD_DATA", pOptions->noisy_run);
  } //if data provided, then cloudcover corrections not needed

  if (Options.SW_radiation == SW_RAD_NONE) {
    WriteAdvisory("The shortwave radiation calculation method is SW_RAD_NONE. This may impact some snowmelt and PET algorithms which require radiation.", pOptions->noisy_run);
  }
  if ((Options.ensemble == ENSEMBLE_ENKF) && (Options.assimilate_flow)) {
    WriteWarning("Both direct insertion and EnKF assimilation options were enabled. Direct insertion will be turned off.", pOptions->noisy_run);
    Options.assimilate_flow = false;
  }

  //===============================================================================================
  //Add Ensemble configuration to Model
  //===============================================================================================
  CEnsemble* pEnsemble = NULL;
  if (Options.ensemble == ENSEMBLE_NONE) { pEnsemble = new CEnsemble(1, Options); }
  else if (Options.ensemble == ENSEMBLE_MONTECARLO) { pEnsemble = new CMonteCarloEnsemble(num_ensemble_members, Options); }
  else if (Options.ensemble == ENSEMBLE_DDS) { pEnsemble = new CDDSEnsemble(num_ensemble_members, Options); }
  else if (Options.ensemble == ENSEMBLE_ENKF) { pEnsemble = new CEnKFEnsemble(num_ensemble_members, Options); Options.assimilate_flow = false; }

  pModel->SetEnsembleMode(pEnsemble);
  pEnsemble->SetRandomSeed(random_seed);
  //===============================================================================================

  pModel->GetTransportModel()->InitializeParams(Options);
  pModel->SetStateVarInfo(pStateVar);
  pStateVar->SetTransportModel(pModel->GetTransportModel());

  delete p; p = NULL;
  delete[] tmpS;
  delete[] tmpLev;
  return true;
}

///////////////////////////////////////////////////////////////////
/// \brief This local method checks that the passed string includes either integer of state variable of variable string
///
/// \param *s [in] String state variable name
/// \param *&pModel [in] Input model object
/// \return Integer index of state variable ins tate variable arrays, or DOESNT_EXIST (-1) if is is invalid
//
int  ParseSVTypeIndex(string s, CModel*& pModel, CStateVariable* pStateVar)
{
  int ind;
  int layer_ind(-1);
  sv_type typ = pStateVar->StringToSVType(s, layer_ind, false);
  ExitGracefullyIf(pModel == NULL, "ParseSVTypeIndex: NULL model!?", RUNTIME_ERR);

  if (typ == UNRECOGNIZED_SVTYPE)
  {
    //if IsNumeric(s){
    return DOESNT_EXIST;
  }

  ind = pModel->GetStateVarIndex(typ, layer_ind);

  if (ind != DOESNT_EXIST) {
    return ind;
  }
  else {
    //cout<<"offending state variable: "<<s<<endl;
    //ExitGracefully("ParseSVTypeIndex: storage variable used in process description does not exist",BAD_DATA);
    return DOESNT_EXIST;
  }
}
///////////////////////////////////////////////////////////////////
/// \brief writes warning to Raven errors file and screen for improper formatting
///
/// \param command [in] string command name (e.g., ":CanopyDrip")
/// \param *p [in]  parser object
/// \param noisy [in] boolean: if true, warning written to screen
//
void ImproperFormatWarning(string command, CParser* p, bool noisy)
{
  string warn;
  warn = command + " command: improper line length at line " + to_string(p->GetLineNumber());
  WriteWarning(warn, noisy);
}
///////////////////////////////////////////////////////////////////
/// \brief add process to either model or process group
//
void AddProcess(CModel* pModel, CHydroProcessABC* pMover, CProcessGroup* pProcGroup)
{
  if (pProcGroup == NULL) { pModel->AddProcess(pMover); }
  else { pProcGroup->AddProcess(pMover); }
}
///////////////////////////////////////////////////////////////////
/// \brief adds to list of NetCDF attributes in Options structure
//
void AddNetCDFAttribute(optStruct& Options, const string att, const string& val)
{
  netcdfatt* aTmp = new netcdfatt[Options.nNetCDFattribs + 1];
  for (int i = 0;i < Options.nNetCDFattribs;i++) {
    aTmp[i].attribute = Options.aNetCDFattribs[i].attribute;
    aTmp[i].value = Options.aNetCDFattribs[i].value;
  }
  aTmp[Options.nNetCDFattribs].attribute = att;
  aTmp[Options.nNetCDFattribs].value = val;
  delete[]Options.aNetCDFattribs;
  Options.aNetCDFattribs = &aTmp[0];
  Options.nNetCDFattribs++;
}

///////////////////////////////////////////////////////////////////
/// \brief returns corresponding evaporation method from string name
//
evap_method ParseEvapMethod(const string s)
{
  string tmp = StringToUppercase(s);
  if (!strcmp(tmp.c_str(), "CONSTANT")) { return PET_CONSTANT; }
  else if (!strcmp(tmp.c_str(), "FROM_FILE")) { return PET_DATA; }
  else if (!strcmp(tmp.c_str(), "FROM_MONTHLY")) { return PET_FROMMONTHLY; }
  else if (!strcmp(tmp.c_str(), "PENMAN_MONTEITH")) { return PET_PENMAN_MONTEITH; }
  else if (!strcmp(tmp.c_str(), "PENMAN_COMBINATION")) { return PET_PENMAN_COMBINATION; }
  else if (!strcmp(tmp.c_str(), "PRIESTLEY_TAYLOR")) { return PET_PRIESTLEY_TAYLOR; }
  else if (!strcmp(tmp.c_str(), "HARGREAVES")) { return PET_HARGREAVES; }
  else if (!strcmp(tmp.c_str(), "HARGREAVES_1985")) { return PET_HARGREAVES_1985; }
  else if (!strcmp(tmp.c_str(), "MONTHLY_FACTOR")) { return PET_MONTHLY_FACTOR; }
  else if (!strcmp(tmp.c_str(), "PET_CONSTANT")) { return PET_CONSTANT; }
  else if (!strcmp(tmp.c_str(), "PET_DATA")) { return PET_DATA; }
  else if (!strcmp(tmp.c_str(), "PET_NONE")) { return PET_NONE; }
  else if (!strcmp(tmp.c_str(), "PET_FROMMONTHLY")) { return PET_FROMMONTHLY; }
  else if (!strcmp(tmp.c_str(), "PET_PENMAN_MONTEITH")) { return PET_PENMAN_MONTEITH; }
  else if (!strcmp(tmp.c_str(), "PET_PENMAN_COMBINATION")) { return PET_PENMAN_COMBINATION; }
  else if (!strcmp(tmp.c_str(), "PET_HAMON")) { return PET_HAMON; }
  else if (!strcmp(tmp.c_str(), "PET_HARGREAVES")) { return PET_HARGREAVES; }
  else if (!strcmp(tmp.c_str(), "PET_HARGREAVES_1985")) { return PET_HARGREAVES_1985; }
  else if (!strcmp(tmp.c_str(), "PET_TURC_1961")) { return PET_TURC_1961; }
  else if (!strcmp(tmp.c_str(), "PET_MAKKINK_1957")) { return PET_MAKKINK_1957; }
  else if (!strcmp(tmp.c_str(), "PET_PRIESTLEY_TAYLOR")) { return PET_PRIESTLEY_TAYLOR; }
  else if (!strcmp(tmp.c_str(), "PET_MONTHLY_FACTOR")) { return PET_MONTHLY_FACTOR; }
  else if (!strcmp(tmp.c_str(), "PET_PENMAN_SIMPLE33")) { return PET_PENMAN_SIMPLE33; }
  else if (!strcmp(tmp.c_str(), "PET_PENMAN_SIMPLE39")) { return PET_PENMAN_SIMPLE39; }
  else if (!strcmp(tmp.c_str(), "PET_GRANGERGRAY")) { return PET_GRANGERGRAY; }
  else if (!strcmp(tmp.c_str(), "PET_MOHYSE")) { return PET_MOHYSE; }
  else if (!strcmp(tmp.c_str(), "PET_OUDIN")) { return PET_OUDIN; }
  else if (!strcmp(tmp.c_str(), "PET_LINACRE")) { return PET_LINACRE; }
  else if (!strcmp(tmp.c_str(), "PET_VAPDEFICIT")) { return PET_VAPDEFICIT; }
  else if (!strcmp(tmp.c_str(), "PET_BLENDED")) { return PET_BLENDED; }
  else if (!strcmp(tmp.c_str(), "PET_LINEAR_TEMP")) { return PET_LINEAR_TEMP; }
  else {
    return PET_UNKNOWN;
  }
}
///////////////////////////////////////////////////////////////////
/// \brief returns corresponding potential melt method from string name
//
potmelt_method ParsePotMeltMethod(const string s)
{
  string tmp = StringToUppercase(s);
  if (!strcmp(tmp.c_str(), "POTMELT_DEGREE_DAY")) { return POTMELT_DEGREE_DAY; }
  else if (!strcmp(tmp.c_str(), "POTMELT_EB")) { return POTMELT_EB; }
  else if (!strcmp(tmp.c_str(), "POTMELT_RESTRICTED")) { return POTMELT_RESTRICTED; }
  else if (!strcmp(tmp.c_str(), "POTMELT_DD_RAIN")) { return POTMELT_DD_RAIN; }
  else if (!strcmp(tmp.c_str(), "POTMELT_UBCWM")) { return POTMELT_UBCWM; }
  else if (!strcmp(tmp.c_str(), "POTMELT_HBV")) { return POTMELT_HBV; }
  else if (!strcmp(tmp.c_str(), "POTMELT_HBV_ROS")) { return POTMELT_HBV_ROS; }
  else if (!strcmp(tmp.c_str(), "POTMELT_DATA")) { return POTMELT_DATA; }
  else if (!strcmp(tmp.c_str(), "UBC")) { return POTMELT_UBCWM; }
  else if (!strcmp(tmp.c_str(), "HBV")) { return POTMELT_HBV; }
  else if (!strcmp(tmp.c_str(), "POTMELT_USACE")) { return POTMELT_USACE; }
  else if (!strcmp(tmp.c_str(), "POTMELT_CRHM_EBSM")) { return POTMELT_CRHM_EBSM; }
  else if (!strcmp(tmp.c_str(), "POTMELT_RILEY")) { return POTMELT_RILEY; }
  else if (!strcmp(tmp.c_str(), "POTMELT_HMETS")) { return POTMELT_HMETS; }
  else if (!strcmp(tmp.c_str(), "POTMELT_BLENDED")) { return POTMELT_BLENDED; }
  else if (!strcmp(tmp.c_str(), "POTMELT_NONE")) { return POTMELT_NONE; }
  else { return POTMELT_UNKNOWN; }
}
///////////////////////////////////////////////////////////////////
/// \brief throws warning if 'to' and 'from' state variables in process command cmd are not appropriate
//
void FromToErrorCheck(string cmd, string sFrom, string sTo, sv_type tFrom, sv_type tTo, CModel* pModel, CStateVariable* pStateVar)
{
  int lay;
  string warn;
  if ((tFrom != UNRECOGNIZED_SVTYPE) && (tFrom != USERSPEC_SVTYPE)) {
    if (pStateVar->StringToSVType(sFrom, lay, false) != tFrom) {
      warn = "ParseInputFile: " + cmd + " command only accepts a 'from' compartment type of " + pStateVar->SVTypeToString(tFrom, DOESNT_EXIST) + ". The user-specified compartment will be overridden.";
      WriteWarning(warn.c_str(), false);
    }
  }
  if ((tTo != UNRECOGNIZED_SVTYPE) && (tTo != USERSPEC_SVTYPE)) {
    if (pStateVar->StringToSVType(sTo, lay, false) != tTo) {
      warn = "ParseInputFile: " + cmd + " command only accepts a 'to' compartment type of " + pStateVar->SVTypeToString(tTo, DOESNT_EXIST) + ". The user-specified compartment will be overridden.";
      WriteWarning(warn.c_str(), false);
    }
  }
}
