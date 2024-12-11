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
  clock_t     t0, t1;          //computational time markers
  pModel = new CModel();
  COptions*& pOptions = pModel->bbopt;

  pOptions->version = __BLACKBIRD_VERSION__;

  ProcessExecutableArguments(argc, argv, pOptions);
  pOptions->PrepareOutputdirectory();

  if (!pOptions->silent_run) {
    int year = std::stoi(BlackbirdBuildDate.substr(BlackbirdBuildDate.length() - 4, 4).c_str());
    std::cout << "============================================================" << std::endl;
    std::cout << "                        BLACKBIRD                           " << std::endl;
    //std::cout << " a robust semi-distributed hydrological modelling framework " << std::endl;
    //std::cout << "    Copyright 2008-" << year << ", the Raven Development Team " << std::endl;
    std::cout << "                    Version " << pOptions->version << std::endl;
    std::cout << "                BuildDate " << BlackbirdBuildDate << std::endl;
    std::cout << "============================================================" << std::endl;
  }
  
  std::ofstream WARNINGS;
  WARNINGS.open((pOptions->main_output_dir + "Blackbird_errors.txt").c_str());
  if (WARNINGS.fail()) {
    ExitGracefully("Main::Unable to open Blackbird_errors.txt. Bad output directory specified?", BLACKBIRD_OPEN_ERR);
  }
  WARNINGS.close();

  t0 = clock();
  
  //Read input files, create model, set model options
  if (!ParseInputFiles(pModel, pOptions)) {
    ExitGracefully("Main::Unable to read input file(s)", BAD_DATA);
  }

  CheckForErrorWarnings(true, pModel);

  if (!pOptions->silent_run) {
    std::cout << "======================================================" << std::endl;
    std::cout << "Initializing Model..." << std::endl;
  }
  pModel->calc_output_flows();

  CheckForErrorWarnings(false, pModel);

  pModel->WriteOutputFileHeaders(pOptions);

  if (!pOptions->silent_run) {
    std::cout << std::endl << "======================================================" << std::endl;
    std::cout << "Simulation Start..." << std::endl;
  }

  t1 = clock();
  
  //Finished Solving----------------------------------------------------
  //pModel->WriteMajorOutput("solution", true);
  //Test output
  std::ofstream TESTOUTPUT;
  TESTOUTPUT.open((pOptions->main_output_dir + "Blackbird_testoutput.txt").c_str());
  if (TESTOUTPUT.fail()) {
    ExitGracefully("Main::Unable to open Blackbird_testoutput.txt. Bad output directory specified?", BLACKBIRD_OPEN_ERR);
  }
  TESTOUTPUT.close();
  pModel->WriteTestOutput();

  if (!pOptions->silent_run)
  {
    std::cout << "======================================================" << std::endl;
    std::cout << "...Blackbird Simulation Complete: " << pOptions->run_name << std::endl;
    std::cout << "        Parsing & initialization: " << float(t1 - t0) / CLOCKS_PER_SEC << " seconds elapsed . " << std::endl;
    std::cout << "                      Simulation: " << float(clock() - t1) / CLOCKS_PER_SEC << " seconds elapsed . " << std::endl;
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
/// \param argv[] [in] executable arguments; Raven.exe [filebase] [-p rvp_file] [-h hru_file] [-t rvt_file] [-c rvc_file] [-o output_dir]
/// \details initializes input files and output directory
/// \details filebase has no extension, all others require .rv* extension
/// \param Options [in] Global model options
//
void ProcessExecutableArguments(int argc, char* argv[], COptions*& pOptions)
{
  std::string word, argument;
  int mode = 0;
  argument = "";
  //initialization:
  pOptions->run_name = "";
  pOptions->bbi_filename = "";
  pOptions->bbg_filename = "";
  pOptions->bbp_filename = "";
  pOptions->bbb_filename = "";
  pOptions->main_output_dir = "";
  pOptions->silent_run = false;
  pOptions->noisy_run = false;

  //Parse argument list
  for (int i = 1; i <= argc; i++)
  {
    if (i != argc) {
      word = argv[i];
    }
    if ((word == "-p") || (word == "-h") || (word == "-t") || (word == "-e") || (word == "-c") || (word == "-o") ||
      (word == "-s") || (word == "-r") || (word == "-n") || (word == "-l") || (word == "-m") || (word == "-v") ||
      (word == "-we") || (word == "-tt") || (i == argc))
    {
      if (mode == 0) {
        pOptions->bbi_filename = argument + ".bbi";
        pOptions->bbg_filename = argument + ".bbg";
        pOptions->bbp_filename = argument + ".bbp";
        pOptions->bbb_filename = argument + ".bbb";
        argument = "";
        mode = 10;
      }
      else if (mode == 1) { pOptions->bbg_filename = argument; argument = ""; }
      else if (mode == 2) { pOptions->bbp_filename = argument; argument = ""; }
      else if (mode == 3) { pOptions->bbb_filename = argument; argument = ""; }
      else if (mode == 5) { pOptions->main_output_dir = argument; argument = ""; }
      else if (mode == 6) { pOptions->run_name = argument; argument = ""; }

      if (word == "-g") { mode = 1; }
      else if (word == "-p") { mode = 2; }
      else if (word == "-b") { mode = 3; }
      else if (word == "-o") { mode = 5; }
      else if (word == "-s") { pOptions->silent_run = true; mode = 10; }
      else if (word == "-n") { pOptions->noisy_run = true;  mode = 10; }
      else if (word == "-r") { mode = 6; }
    }
    else {
      if (argument == "") { argument += word; }
      else { argument += " " + word; }
    }
  }
  if (argc == 1) {//no arguments
    pOptions->bbi_filename = "nomodel.bbi";
    pOptions->bbg_filename = "nomodel.bbg";
    pOptions->bbp_filename = "nomodel.bbp";
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
/////////////////////////////////////////////////////////////////
/// \brief Checks if stopfile exists in current working directory
/// \note called during simulation to determine whether progress should be stopped
///
//
bool CheckForStopfile(const int step, CModel* pModel)
{
  if (step % 100 != 0) { return false; } //only check every 100th timestep
  std::ifstream STOP;
  STOP.open("stop");
  if (STOP.fail()) { STOP.close(); return false; }
  else //Stopfile found
  {
    STOP.close();
    pModel->WriteMajorOutput("solution", true);
    ExitGracefully("CheckForStopfile: simulation interrupted by user using stopfile", SIMULATION_DONE);
    return true;
  }
}
