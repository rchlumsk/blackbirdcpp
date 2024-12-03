#include "BlackbirdInclude.h"
#include "Model.h"
#include "ParseLib.h"
#include "Streamnode.h"

//////////////////////////////////////////////////////////////////
/// \brief Parses Boundary Conditions file
/// \details model.bbb: input file that defines boundary conditions \n
///
/// \param *&pModel [out] Reference to model object
/// \param *&pOptions [out] Global model options information
/// \return True if operation is successful
//
bool ParseBoundaryConditionsFile(CModel*& pModel, const COptions*& pOptions)
{
  CStreamnode*       pSN(NULL);             //temp pointers
  hydraulic_output*  pHO(NULL);
  bool               ended = false;
  bool               in_ifmode_statement = false;

  std::ifstream      bbb;
  bbb.open(pOptions->bbb_filename.c_str());
  if (bbb.fail()) {
    std::cout << "ERROR opening file: " << pOptions->bbb_filename << std::endl; return false;
  }

  int   Len, line(0), code;
  char* s[MAXINPUTITEMS];
  CParser* pp = new CParser(bbb, pOptions->bbb_filename, line);

  std::ifstream INPUT2;           //For Secondary input
  CParser* pMainParser = NULL; //for storage of main parser while reading secondary files

  if (pOptions->noisy_run) {
    std::cout << "======================================================" << std::endl;
    std::cout << "Parsing BBB Input File " << pOptions->bbb_filename << "..." << std::endl;
    std::cout << "======================================================" << std::endl;
  }

  //--Sift through file-----------------------------------------------
  bool end_of_file = pp->Tokenize(s, Len);
  while (!end_of_file)
  {
    if (ended) { break; }
    if (pOptions->noisy_run) { std::cout << "reading line " << pp->GetLineNumber() << ": "; }

    /*assign code for switch statement
      ------------------------------------------------------------------
      <0           : ignored/special
      0   thru 100 : All other
      ------------------------------------------------------------------
    */

    code = 0;
    //---------------------SPECIAL -----------------------------
    if (Len == 0) { code = -1; }//blank line
    else if (IsComment(s[0], Len)) { code = -2; }//comment
    else if (!strcmp(s[0], ":End")) { code = -4; }//stop reading
    else if (!strcmp(s[0], ":IfModeEquals")) { code = -5; }
    else if (in_ifmode_statement) { code = -6; }
    else if (!strcmp(s[0], ":EndIfModeEquals")) { code = -2; }//treat as comment - unused mode
    else if (!strcmp(s[0], ":RedirectToFile")) { code = -3; }//redirect to secondary file
    //--------------------MODEL OPTIONS ------------------------
    else if (!strcmp(s[0], ":BoundaryConditions")) { code = 1; }
    else if (!strcmp(s[0], ":SteadyFlows")) { code = 2; }
    else if (!strcmp(s[0], ":StreamnodeSourcesSinks")) { code = 3; }

    switch (code)
    {
    case(-1):  //----------------------------------------------
    {/*Blank Line*/
      if (pOptions->noisy_run) { std::cout << "" << std::endl; }break;
    }
    case(-2):  //----------------------------------------------
    {/*Comment*/
      if (pOptions->noisy_run) { std::cout << "*" << std::endl; } break;
    }
    case(-3):  //----------------------------------------------
    {/*:RedirectToFile*/
      //std::string filename = "";
      //for (int i = 1;i < Len;i++) { filename += s[i]; if (i < Len - 1) { filename += ' '; } }
      //if (pOptions->noisy_run) { std::cout << "Redirect to file: " << filename << std::endl; }

      //filename = CorrectForRelativePath(filename, Options.rvt_filename);

      //INPUT2.open(filename.c_str());
      //if (INPUT2.fail()) {
      //  string warn = ":RedirectToFile: Cannot find file " + filename;
      //  ExitGracefully(warn.c_str(), BAD_DATA);
      //}
      //else {
      //  if (pMainParser != NULL) {
      //    ExitGracefully("ParseHRUPropsFile::nested :RedirectToFile commands (in already redirected files) are not allowed.", BAD_DATA);
      //  }
      //  pMainParser = pp;   //save pointer to primary parser
      //  pp = new CParser(INPUT2, filename, line);//open new parser
      //}
      //break;
    }
    case(-4):  //----------------------------------------------
    {/*:End*/
      if (pOptions->noisy_run) { std::cout << "EOF" << std::endl; } ended = true; break;
    }
    case(1):  //----------------------------------------------
    { /*:BoundaryConditions*/
      if (pOptions->noisy_run) { std::cout << "Boundary conditions table..." << std::endl; }
      bool done = false;
      int row = 0;
      if (Len < 2) { pp->ImproperFormat(s); }
      else {
        pSN = new CStreamnode();
        std::string error;
        if (StringIsLong(s[0])) {
          pSN->nodeID = std::stoi(s[0]);
        }
        else {
          error = "ParsePreprocessedTables File: nodeID \"" + std::string(s[0]) + "\" after :PreprocHydTable must be unique integer or long integer";
          ExitGracefully(error.c_str(), BAD_DATA_WARN);
        }
        while ((!done) && (!end_of_file))
        {
          row++;
          end_of_file = pp->Tokenize(s, Len);
          if (IsComment(s[0], Len)) {}//comment line
          else if (!strcmp(s[0], ":Attributes")) {}//ignored by Blackbird - needed for GUIs
          else if (!strcmp(s[0], ":EndPreprocHydTable")) { done = true; }
          else
          {
            if (Len < 75) { pp->ImproperFormat(s); }
            pHO = NULL;
            pHO = new hydraulic_output();
            ExitGracefullyIf(pHO == NULL, "ParsePreprocesssedTables", OUT_OF_MEMORY);

            if (strcmp(s[0], "NA")) {
              if (StringIsLong(s[0])) {
                pHO->nodeID = std::stoi(s[0]);
              }
              else {
                error = "ParsePreprocessedTables File: nodeID \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[1], "NA")) {
              if (StringIsLong(s[1])) {
                pHO->reachID = std::stoi(s[1]);
              }
              else {
                error = "ParsePreprocessedTables File: reachID \"" + std::string(s[1]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[2], "NA")) {
              if (StringIsLong(s[2])) {
                pHO->downnodeID = std::stoi(s[2]);
              }
              else {
                error = "ParsePreprocessedTables File: downnodeID \"" + std::string(s[2]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[3], "NA")) {
              if (StringIsLong(s[3])) {
                pHO->upnodeID1 = std::stoi(s[3]);
              }
              else {
                error = "ParsePreprocessedTables File: upnodeID1 \"" + std::string(s[3]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[4], "NA")) {
              if (StringIsLong(s[4])) {
                pHO->upnodeID2 = std::stoi(s[4]);
              }
              else {
                error = "ParsePreprocessedTables File: upnodeID2 \"" + std::string(s[4]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[5], "NA")) {
              pHO->stationname = std::string(s[5]);
            }
            if (strcmp(s[6], "NA")) {
              if (StringIsDouble(s[6])) {
                pHO->station = std::stod(s[6]);
              }
              else {
                error = "ParsePreprocessedTables File: station \"" + std::string(s[6]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            pSN->depthdf->push_back(pHO);
          }
        }
        pModel->bbsn->push_back(pSN);
      }
      break;
    }
    default://------------------------------------------------
    {
      char firstChar = *(s[0]);
      switch (firstChar)
      {
      case ':':
      {
        if (!strcmp(s[0], ":FileType")) { if (pOptions->noisy_run) { std::cout << "Filetype" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":Application")) { if (pOptions->noisy_run) { std::cout << "Application" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":Version")) { if (pOptions->noisy_run) { std::cout << "Version" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":WrittenBy")) { if (pOptions->noisy_run) { std::cout << "WrittenBy" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":CreationDate")) { if (pOptions->noisy_run) { std::cout << "CreationDate" << std::endl; } }//do nothing
        else if (!strcmp(s[0], ":SourceFile")) { if (pOptions->noisy_run) { std::cout << "SourceFile" << std::endl; } }//do nothing
        else if (pOptions->noisy_run)
        {
          std::string warn = "IGNORING unrecognized command: " + std::string(s[0]) + " in .rvh file";
          WriteWarning(warn, pOptions->noisy_run);
        }
      }
      break;
      default:
      {
        std::string errString = "Unrecognized command in .bbb file:\n   " + std::string(s[0]);
        ExitGracefully(errString.c_str(), BAD_DATA);//STRICT
      }
      break;
      }
    }
    }//end switch(code)

    end_of_file = pp->Tokenize(s, Len);

    //return after file redirect, if in secondary file
    if ((end_of_file) && (pMainParser != NULL))
    {
      INPUT2.clear();
      INPUT2.close();
      delete pp;
      pp = pMainParser;
      pMainParser = NULL;
      end_of_file = pp->Tokenize(s, Len);
    }
  } //end while !end_of_file
  bbb.close();

  delete pp;
  pp = NULL;
  return true;
}
