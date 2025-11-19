#ifndef BLACKBIRD_MAIN
#define BLACKBIRD_MAIN

#include "BlackbirdInclude.h"
#include "Model.h"

//Defined in ParseInput.cpp
bool ParseInputFiles(CModel*& pModel, COptions*& Options); // Parses all input files

//Local functions defined below main() in BlackbirdMain.cpp
void ProcessExecutableArguments(int argc, char* argv[], COptions*& Options); // Parses input parameters
void CheckForErrorWarnings(bool quiet, CModel* pModel); // Checks if errors have been written to Blackbird_errors.txt, if so, exits gracefully

#endif
