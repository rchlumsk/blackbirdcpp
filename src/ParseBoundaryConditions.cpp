#include "BlackbirdInclude.h"
#include "Model.h"
#include "ParseLib.h"
#include "Streamnode.h"

void  ImproperFormatWarning(std::string command, CParser* p, bool noisy);

//////////////////////////////////////////////////////////////////
/// \brief Parses Boundary Conditions file
/// \details model.bbb: input file that defines boundary conditions \n
///
/// \param *&pModel [in/out] Reference to model object
/// \param *&pOptions [in] Global model options information
/// \return True if operation is successful
//
bool ParseBoundaryConditionsFile(CModel*& pModel, COptions*const& pOptions)
{
  CStreamnode*               pSN(NULL);             //temp pointers
  CBoundaryCondition*       pBC(NULL);
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
      0   thru 100 : boundary conditions
      100 thru 200 : All other
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
    else if (!strcmp(s[0], ":EndBoundaryConditions")) { code = 2; }
    else if (!strcmp(s[0], ":nodeID")) { code = 3; }
    else if (!strcmp(s[0], ":BCType")) { code = 7; }
    else if (!strcmp(s[0], ":BCValue")) { code = 8; }
    else if (!strcmp(s[0], ":InitialWSL")) { code = 9; }
    else if (!strcmp(s[0], ":SteadyFlows")) { code = 100; }
    else if (!strcmp(s[0], ":StreamnodeSourcesSinks")) { code = 101; }

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
      std::string filename = "";
      for (int i = 1;i < Len;i++) { filename += s[i]; if (i < Len - 1) { filename += ' '; } }
      if (pOptions->noisy_run) { std::cout << "Redirect to file: " << filename << std::endl; }

      filename = CorrectForRelativePath(filename, pOptions->bbb_filename);

      INPUT2.open(filename.c_str());
      if (INPUT2.fail()) {
        std::string warn = "ParseBoundaryConditionsFile: :RedirectToFile: Cannot find file " + filename;
        ExitGracefully(warn.c_str(), BAD_DATA);
      }
      else {
        if (pMainParser != NULL) {
          ExitGracefully("ParseBoundaryConditionsFile::nested :RedirectToFile commands (in already redirected files) are not allowed.", BAD_DATA);
        }
        pMainParser = pp;   //save pointer to primary parser
        pp = new CParser(INPUT2, filename, line);//open new parser
      }
      break;
    }
    case(-4):  //----------------------------------------------
    {/*:End*/
      if (pOptions->noisy_run) { std::cout << "EOF" << std::endl; } ended = true; break;
    }
    case(1):  //----------------------------------------------
    { /*:BoundaryConditions*/
      if (pOptions->noisy_run) { std::cout << "Boundary conditions information begin" << std::endl; }
      if (Len != 1) { pp->ImproperFormat(s); }
      else {
        pBC = new CBoundaryCondition();
        pModel->bbbc->push_back(pBC);
      }
      break;
    }
    case(2):  //----------------------------------------------
    { /*:EndBoundaryConditions*/
      if (pOptions->noisy_run) { std::cout << "Boundary conditions information end" << std::endl; }
      if (Len != 1) { pp->ImproperFormat(s); }
      else {
        if (pBC == NULL)
        {
          ExitGracefully("ParseBoundaryConditions File: :EndBoundaryConditions encountered before :BoundaryConditions", BAD_DATA);
        }
        else
        {
          pBC = NULL;
        }
      }
      break;
    }
    case(3):
    {/*:nodeID [int id]*/
      if (pOptions->noisy_run) { std::cout << "nodeID" << std::endl; }
      ExitGracefullyIf(pBC == NULL,
        "ParseBoundaryConditions File: :nodeID specified outside of a :BoundaryConditions-:EndBoundaryConditions statement", BAD_DATA);
      if (Len < 2) { ImproperFormatWarning(":nodeID", pp, pOptions->noisy_run); break; }
      pBC->nodeID = std::atoi(s[1]);
      pOptions->iteration_limit_cp = std::atoi(s[1]);
      break;
    }
    case(7):
    {/*:BCType [string type]*/
      if (pOptions->noisy_run) { std::cout << "BCType" << std::endl; }
      ExitGracefullyIf(pBC == NULL,
        "ParseBoundaryConditions File: :BCType specified outside of a :BoundaryConditions-:EndBoundaryConditions statement", BAD_DATA);
      if (Len < 2) { ImproperFormatWarning(":BCType", pp, pOptions->noisy_run); break; }
      if (!strcmp(s[1], "NORMAL_DEPTH")) { pBC->bctype = enum_bc_type::NORMAL_DEPTH; }
      else if (!strcmp(s[1], "SET_WSL")) { pBC->bctype = enum_bc_type::SET_WSL; }
      else if (!strcmp(s[1], "SET_DEPTH")) { pBC->bctype = enum_bc_type::SET_DEPTH; }
      break;
    }
    case(8):
    {/*:BCValue [double value]*/
      if (pOptions->noisy_run) { std::cout << "BCValue" << std::endl; }
      ExitGracefullyIf(pBC == NULL,
        "ParseBoundaryConditions File: :BCValue specified outside of a :BoundaryConditions-:EndBoundaryConditions statement", BAD_DATA);
      if (Len < 2) { ImproperFormatWarning(":BCValue", pp, pOptions->noisy_run); break; }
      if (strcmp(s[1], "NA")) {
        if (StringIsDouble(s[1])) {
          pBC->bcvalue = std::stod(s[1]);
        }
        else {
          std::string error = "ParseBoundaryConditions File: BCValue \"" + std::string(s[1]) + "\" in :BoundaryConditions must be a double";
          ExitGracefully(error.c_str(), BAD_DATA_WARN);
        }
      }
      break;
    }
    case(9):
    {/*:InitialWSL [double wsl]*/
      if (pOptions->noisy_run) { std::cout << "InitialWSL" << std::endl; }
      ExitGracefullyIf(pBC == NULL,
        "ParseBoundaryConditions File: :InitialWSL specified outside of a :BoundaryConditions-:EndBoundaryConditions statement", BAD_DATA);
      if (Len < 2) { ImproperFormatWarning(":InitialWSL", pp, pOptions->noisy_run); break; }
      if (strcmp(s[1], "NA")) {
        if (StringIsDouble(s[1])) {
          pBC->init_WSL = std::stod(s[1]);
        }
        else {
          std::string error = "ParseBoundaryConditions File: InitialWSL \"" + std::string(s[1]) + "\" in :BoundaryConditions must be a double";
          ExitGracefully(error.c_str(), BAD_DATA_WARN);
        }
      }
      break;
    }
    case(100):  //----------------------------------------------
    { /*:SteadyFlows*/
      if (pOptions->noisy_run) { std::cout << "Steady flows table..." << std::endl; }
      bool done = false;
      int row = 0;
      if (Len != 1) { pp->ImproperFormat(s); }
      else {
        std::string error;
        while ((!done) && (!end_of_file))
        {
          end_of_file = pp->Tokenize(s, Len);
          if (IsComment(s[0], Len)) {}//comment line
          else if (!strcmp(s[0], ":Attributes")) // Attributes row. Reads in flow profile names
          {
            if (Len < 3) { pp->ImproperFormat(s); }
            for (int i = 2; i < Len; i++) {
              pModel->fp_names.push_back(s[i]);
            }
          }
          else if (!strcmp(s[0], ":EndSteadyFlows")) { done = true; }
          else
          {
            if (pModel->fp_names.empty()) {
                error = "ParseBoundaryConditions File: :Attributes must be specified at the beginning of  :SteadyFlows block";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
            }
            row++;
            if (Len < 2) { pp->ImproperFormat(s); }
            if (StringIsLong(s[0])) {
              pSN = NULL;
              pSN = pModel->get_streamnode_by_id(std::stoi(s[0]));
              if (pSN == NULL) {
                error = "ParseBoundaryConditions File: nodeID \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of  :SteadyFlows does not exist in streamnodes object";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            else {
              error = "ParseBoundaryConditions File: nodeID \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of  :SteadyFlows must be unique integer or long integer";
              ExitGracefully(error.c_str(), BAD_DATA_WARN);
            }
            for (int i = 1; i < pModel->fp_names.size() + 1; i++) {
              if (StringIsDouble(s[i])) {
                pSN->add_steadyflow(std::stod(s[i]));
              }
              else {
                error = "ParseBoundaryConditions File: flowprofile \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of  :SteadyFlows must be double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
          }
        }
      }
      break;
    }
    case(101):  //----------------------------------------------
    { /*:StreamnodeSourcesSinks*/
      if (pOptions->noisy_run) { std::cout << "Sources Sinks table..." << std::endl; }
      bool done = false;
      int row = 0;
      if (Len != 1) { pp->ImproperFormat(s); }
      else {
        std::string error;
        while ((!done) && (!end_of_file))
        {
          end_of_file = pp->Tokenize(s, Len);
          if (IsComment(s[0], Len)) {}//comment line
          else if (!strcmp(s[0], ":Attributes")) {}//ignored by Blackbird - needed for GUIs
          else if (!strcmp(s[0], ":EndStreamnodeSourcesSinks")) { done = true; }
          else
          {
            row++;
            if (Len < 2) { pp->ImproperFormat(s); }
            if (StringIsLong(s[0])) {
              pSN = NULL;
              pSN = pModel->get_streamnode_by_id(std::stoi(s[0]));
              if (pSN == NULL) {
                error = "ParseBoundaryConditions File: nodeID \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of  :StreamnodeSourcesSinks does not exist in streamnodes object";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            else {
              error = "ParseBoundaryConditions File: nodeID \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of  :StreamnodeSourcesSinks must be unique integer or long integer";
              ExitGracefully(error.c_str(), BAD_DATA_WARN);
            }
            for (int i = 1; i < Len; i += 2) {
              if (i + 1 == Len) {
                error = "ParseBoundaryConditions File: unmatched source in row " + std::to_string(row) + " of  :StreamnodeSourcesSinks needs a sink";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
              if (!StringIsDouble(s[i])) {
                error = "ParseBoundaryConditions File: source \"" + std::string(s[i]) + "\" in row " + std::to_string(row) + " of  :StreamnodeSourcesSinks must be double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
              else if (!StringIsDouble(s[i + 1])) {
                error = "ParseBoundaryConditions File: sink \"" + std::string(s[i + 1]) + "\" in row " + std::to_string(row) + " of  :StreamnodeSourcesSinks must be double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
              else {
                int index = (i - 1) / 2;
                pSN->add_sourcesink(index, std::stod(s[i]), std::stod(s[i + 1]));
              }
            }
          }
        }
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
          std::string warn = "IGNORING unrecognized command: " + std::string(s[0]) + " in .bbb file";
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
