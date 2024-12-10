#ifndef BLACKBIRD_MAIN
#define BLACKBIRD_MAIN

#include "BlackbirdInclude.h"
#include "Model.h"

//Defined in ParseInput.cpp
bool ParseInputFiles(CModel*& pModel, COptions*& Options);

//Local functions defined below main() in BlackbirdMain.cpp
void ProcessExecutableArguments(int argc, char* argv[], COptions*& Options);
void CheckForErrorWarnings(bool quiet, CModel* pModel);
bool CheckForStopfile(const int step, CModel* pModel);

#endif
