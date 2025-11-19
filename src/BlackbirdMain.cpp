// blackbird.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "BlackbirdInclude.h"
#include "BlackbirdMain.h"
#include "Model.h"
#include "GracefulEnd.h"
#include "ParseLib.h"

// Global variables - declared as extern in BlackbirdInclude.h--------
std::string g_output_directory = "";
bool   g_suppress_warnings = false;

static std::string BlackbirdBuildDate(__DATE__);

int main(int argc, char* argv[])
{
  clock_t     t0, t1, t2;          //computational time markers
  // Initialize model
  pModel = new CModel();
  COptions*& pOptions = pModel->bbopt;

  pOptions->version = __BLACKBIRD_VERSION__;

  // Parse input arguments and set output directory
  ProcessExecutableArguments(argc, argv, pOptions);
  pOptions->PrepareOutputdirectory();

  if (!pOptions->silent_run) {
    int year = std::stoi(BlackbirdBuildDate.substr(BlackbirdBuildDate.length() - 4, 4).c_str());
    std::cout << "=================================================================" << std::endl;
    std::cout << "                            BLACKBIRD                            " << std::endl;
    std::cout << " a robust hydraulic modelling framework supporting flood mapping " << std::endl;
    std::cout << "       Copyright 2025-" << year << ", the Blackbird Development Team " << std::endl;
    std::cout << "                          Version " << pOptions->version << std::endl;
    std::cout << "                      BuildDate " << BlackbirdBuildDate << std::endl;
    std::cout << "=================================================================" << std::endl;
  }
  
  // Setup errors file for warnings to be logged
  std::ofstream WARNINGS;
  WARNINGS.open((pOptions->main_output_dir + "Blackbird_errors.txt").c_str());
  if (WARNINGS.fail()) {
    ExitGracefully("Main::Unable to open Blackbird_errors.txt. Bad output directory specified?", BLACKBIRD_OPEN_ERR);
  }
  WARNINGS.close();

  t0 = clock();
  
  // Read input files, create model, set model options
  if (!ParseInputFiles(pModel, pOptions)) {
    ExitGracefully("Main::Unable to read input file(s)", BAD_DATA);
  }

  // Initialize GDAL
  GDALAllRegister();

  // Read input gridded data if applicable
  if (pOptions->interpolation_postproc_method != enum_ppi_method::NONE) {
    if (!pOptions->silent_run) {
      std::cout << "======================================================" << std::endl;
      std::cout << "Reading Gridded Data..." << std::endl;
    }
    pModel->ReadGISFiles();
  }

  CheckForErrorWarnings(true, pModel);

  if (!pOptions->silent_run) {
    std::cout << "======================================================" << std::endl;
    std::cout << "Initializing Model..." << std::endl;
  }
  // Calculate output flows for all streamnodes
  pModel->calc_output_flows();

  CheckForErrorWarnings(false, pModel);

  if (!pOptions->silent_run) {
    std::cout << std::endl << "======================================================" << std::endl;
    std::cout << "Simulation Start..." << std::endl;
  }

  t1 = clock();
  
  // Compute hydraulic profile for all streamnodes
  pModel->hyd_compute_profile();
  // Post-process flood results with method specified by input parameter
  pModel->postprocess_floodresults();

  t2 = clock();

  //Finished Solving----------------------------------------------------
  // Initialize test output file for writing to
  //std::ofstream TESTOUTPUT;
  //TESTOUTPUT.open(pModel->FilenamePrepare("Blackbird_testoutput.txt").c_str());
  //if (TESTOUTPUT.fail()) {
  //  ExitGracefully("Main::Unable to open Blackbird_testoutput.txt. Bad output directory specified?", BLACKBIRD_OPEN_ERR);
  //}
  //TESTOUTPUT.close();
  //pModel->WriteFullModel(); // writes full model to test output
  //pModel->hyd_result_pretty_print(); // writes hydraulic result to test output
  pModel->hyd_result_pretty_print_csv(); // writes hydraulic result to csv
  pModel->WriteGriddedOutput(); // if applicable, writes raster output to raster files
  pModel->write_catchments_from_streamnodes_json(); // if applicable, updates catchments from streamnodes json with calculated flows, depths, and wsls

  if (!pOptions->silent_run)
  {
    std::cout << "======================================================" << std::endl;
    std::cout << "...Blackbird Simulation Complete: " << pOptions->run_name << std::endl;
    std::cout << "        Parsing & initialization: " << float(t1 - t0) / CLOCKS_PER_SEC << " seconds elapsed . " << std::endl;
    std::cout << "                      Simulation: " << float(t2 - t1) / CLOCKS_PER_SEC << " seconds elapsed . " << std::endl;
    std::cout << "                  Writing output: " << float(clock() - t2) / CLOCKS_PER_SEC << " seconds elapsed . " << std::endl;
    if (pOptions->main_output_dir != "") {
      std::cout << "  Output written to " << pOptions->main_output_dir << std::endl;
    }
    std::cout << "======================================================" << std::endl;
  }

  ExitGracefully("Successful Simulation", SIMULATION_DONE);
  return 0;
}

//////////////////////////////////////////////////////////////////
/// \param argc [in] number of arguments to executable
/// \param argv[] [in] executable arguments; blackbird.exe [filebase] [-g bbg_file] [-b bbb_file] [-o output_dir]
/// \param pOptions [in] Global model options
/// \details initializes input files and output directory
/// \details filebase has no extension, all others require .rv* extension
//
void ProcessExecutableArguments(int argc, char* argv[], COptions*& pOptions)
{
  std::string word, argument;
  int mode = 0;
  argument = "";
  //initialization for parameters:
  pOptions->run_name = "";
  pOptions->bbi_filename = "";
  pOptions->bbg_filename = "";
  pOptions->bbb_filename = "";
  pOptions->main_output_dir = "";
  pOptions->silent_run = false;
  pOptions->noisy_run = false;

  //Parse argument list and assign to pOptions
  for (int i = 1; i <= argc; i++)
  {
    if (i != argc) {
      word = argv[i];
    }
    if ((word == "-g") || (word == "-b") || (word == "-o") || (word == "-s") ||
        (word == "-n") || (word == "-r") || (word == "-f") || (i == argc))
    {
      if (mode == 0) {
        pOptions->bbi_filename = argument + ".bbi";
        pOptions->bbg_filename = argument + ".bbg";
        pOptions->bbb_filename = argument + ".bbb";
        argument = "";
        mode = 10;
      }
      else if (mode == 1) { pOptions->bbg_filename = argument; argument = ""; }
      else if (mode == 2) { pOptions->bbb_filename = argument; argument = ""; }
      else if (mode == 3) { pOptions->main_output_dir = argument; argument = ""; }
      else if (mode == 4) { pOptions->run_name = argument; argument = ""; }
      else if (mode == 5) {
        if (argument == "r") { pOptions->out_format = enum_gridded_format::RASTER; argument = ""; }
        else if (argument == "n") { pOptions->out_format = enum_gridded_format::NETCDF; argument = ""; }
        else if (argument == "p") { pOptions->out_format = enum_gridded_format::PNG; argument = ""; }
        else { ExitGracefully(("BlackbirdMain: \"-f " + argument + "\" unsupported output format").c_str(), exitcode::BAD_DATA); }
      }

      if (word == "-g") { mode = 1; }
      else if (word == "-b") { mode = 2; }
      else if (word == "-o") { mode = 3; }
      else if (word == "-s") { pOptions->silent_run = true; mode = 10; }
      else if (word == "-n") { pOptions->noisy_run = true;  mode = 10; }
      else if (word == "-r") { mode = 4; }
      else if (word == "-f") { mode = 5; }
    }
    else {
      if (argument == "") { argument += word; }
      else { argument += " " + word; }
    }
  }
  if (argc == 1) {//no arguments
    pOptions->bbi_filename = "nomodel.bbi";
    pOptions->bbg_filename = "nomodel.bbg";
    pOptions->bbb_filename = "nomodel.bbb";
  }

  // make sure that output dir has trailing '/' if not empty
  if ((pOptions->main_output_dir.compare("") != 0) && (pOptions->main_output_dir.back() != '/')) { pOptions->main_output_dir += "/"; }

  char cCurrentPath[FILENAME_MAX];
  if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
    ExitGracefully("BlackbirdMain: unable to retrieve current directory.", RUNTIME_ERR);
  }
  pOptions->working_dir = cCurrentPath;
}


/////////////////////////////////////////////////////////////////
/// \brief Checks if errors have been written to Blackbird_errors.txt, if so, exits gracefully
/// \note called prior to simulation initialization, after parsing everything
///
//
void CheckForErrorWarnings(bool quiet, CModel* pModel)
{
  int      Len;
  char* s[MAXINPUTITEMS];
  bool     errors_found(false);
  bool     warnings_found(false);
  COptions*& pOptions = pModel->bbopt;

  std::ifstream WARNINGS;
  WARNINGS.open((pOptions->main_output_dir + "Blackbird_errors.txt").c_str());
  if (WARNINGS.fail()) { WARNINGS.close();return; }

  CParser* p = new CParser(WARNINGS, pOptions->main_output_dir + "Blackbird_errors.txt", 0);

  while (!(p->Tokenize(s, Len)))
  {
    if (Len > 0) {
      if (!strcmp(s[0], "ERROR")) { errors_found = true; }
      if (!strcmp(s[0], "WARNING")) { warnings_found = true; }
    }
  }
  delete p;
  p = nullptr;

  WARNINGS.close();
  if ((warnings_found) && (!quiet)) {
    std::cout << "*******************************************************" << std::endl << std::endl;
    std::cout << "WARNING: Warnings have been issued while parsing data. " << std::endl;
    std::cout << "         See Blackbird_errors.txt for details          " << std::endl << std::endl;
    std::cout << "*******************************************************" << std::endl << std::endl;
  }

  if (errors_found) {
    ExitGracefully("Errors found in input data. See Blackbird_errors.txt for details", BAD_DATA);
  }
}
