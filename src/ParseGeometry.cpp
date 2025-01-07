#include "BlackbirdInclude.h"
#include "Model.h"
#include "ParseLib.h"
#include "Streamnode.h"
#include "Reach.h"
#include "XSection.h"

//////////////////////////////////////////////////////////////////
/// \brief Parses Geometry file
/// \details model.bbg: input file that defines geometry \n
///
/// \param *&pModel [out] Reference to model object
/// \param *&pOptions [out] Global model options information
/// \return True if operation is successful
//
bool ParseGeometryFile(CModel*& pModel, COptions*const& pOptions)
{
  CStreamnode*     pSN(NULL);             //temp pointers
  bool             ended = false;
  bool             in_ifmode_statement = false;

  std::ifstream    bbg;
  bbg.open(pOptions->bbg_filename.c_str());
  if (bbg.fail()) {
    std::cout << "ERROR opening file: " << pOptions->bbg_filename << std::endl; return false;
  }
  
  int   Len, line(0), code;
  char* s[MAXINPUTITEMS];
  CParser* pp = new CParser(bbg, pOptions->bbg_filename, line);

  std::ifstream INPUT2;           //For Secondary input
  CParser* pMainParser = NULL; //for storage of main parser while reading secondary files

  if (pOptions->noisy_run) {
    std::cout << "======================================================" << std::endl;
    std::cout << "Parsing BBG Input File " << pOptions->bbg_filename << "..." << std::endl;
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
    else if (!strcmp(s[0], ":Streamnodes")) { code = 1; }
    else if (!strcmp(s[0], ":RasterPaths")) { code = 2; }

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
    { /*:Streamnodes*/
      if (pOptions->noisy_run) { std::cout << "Streamnodes table..." << std::endl; }
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
          else if (!strcmp(s[0], ":EndStreamnodes")) { done = true; }
          else
          {
            row++;
            if (Len < 15) { pp->ImproperFormat(s); }
            pSN = NULL;
            if (strcmp(s[1], "NA")) {
              if (!strcmp(s[1], "REACH")) {
                pSN = new CReach();
                pSN->nodetype = enum_nodetype::REACH;
              } else if (!strcmp(s[1], "XSECTION")) {
                pSN = new CXSection();
                pSN->nodetype = enum_nodetype::XSECTION;
              } else {
                error = "ParseGeometry File: nodetype \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a REACH or XSECTION";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            ExitGracefullyIf(pSN == NULL, "ParseGeometry", OUT_OF_MEMORY);

            if (strcmp(s[0], "NA")) {
              if (StringIsLong(s[0])) {
                pSN->nodeID = std::stoi(s[0]);
              }
              else {
                error = "ParseGeometry File: nodeID \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[2], "NA")) {
              if (StringIsLong(s[2])) {
                pSN->downnodeID = std::stoi(s[2]);
              }
              else {
                error = "ParseGeometry File: downnodeID \"" + std::string(s[2]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[3], "NA")) {
              if (StringIsLong(s[3])) {
                pSN->upnodeID1 = std::stoi(s[3]);
              }
              else {
                error = "ParseGeometry File: upnodeID1 \"" + std::string(s[3]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[4], "NA")) {
              if (StringIsLong(s[4])) {
                pSN->upnodeID2 = std::stoi(s[4]);
              }
              else {
                error = "ParseGeometry File: upnodeID2 \"" + std::string(s[4]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[5], "NA")) {
              pSN->stationname = std::string(s[5]);
            }
            if (strcmp(s[6], "NA")) {
              if (StringIsDouble(s[6])) {
                pSN->station = std::stod(s[6]);
              }
              else {
                error = "ParseGeometry File: station \"" + std::string(s[6]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[7], "NA")) {
              if (StringIsLong(s[7])) {
                pSN->reachID = std::stoi(s[7]);
              }
              else {
                error = "ParseGeometry File: reachID \"" + std::string(s[7]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[8], "NA")) {
              if (StringIsDouble(s[8])) {
                pSN->ds_reach_length = std::stod(s[8]);
              }
              else {
                error = "ParseGeometry File: ds_reach_length \"" + std::string(s[8]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[9], "NA")) {
              if (StringIsDouble(s[9])) {
                pSN->us_reach_length1 = std::stod(s[9]);
              }
              else {
                error = "ParseGeometry File: us_reach_length1 \"" + std::string(s[9]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[10], "NA")) {
              if (StringIsDouble(s[10])) {
                pSN->us_reach_length2 = std::stod(s[10]);
              }
              else {
                error = "ParseGeometry File: us_reach_length2 \"" + std::string(s[10]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[11], "NA")) {
              if (StringIsDouble(s[11])) {
                pSN->contraction_coeff = std::stod(s[11]);
              }
              else {
                error = "ParseGeometry File: contraction_coeff \"" + std::string(s[11]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[12], "NA")) {
              if (StringIsDouble(s[12])) {
                pSN->expansion_coeff = std::stod(s[12]);
              }
              else {
                error = "ParseGeometry File: expansion_coeff \"" + std::string(s[12]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[13], "NA")) {
              if (StringIsDouble(s[13])) {
                pSN->min_elev = std::stod(s[13]);
              }
              else {
                error = "ParseGeometry File: min_elev \"" + std::string(s[13]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[14], "NA")) {
              if (StringIsDouble(s[14])) {
                pSN->bed_slope = std::stod(s[14]);
              }
              else {
                error = "ParseGeometry File: bed_slope \"" + std::string(s[14]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            pModel->add_streamnode(pSN);
          }
        }
      }
      break;
    }
    case(2):  //----------------------------------------------
    { /*:RasterPaths*/
      if (pOptions->noisy_run) { std::cout << "RasterPaths table..." << std::endl; }
      bool done = false;
      int row = 0;
      if (Len != 1) { pp->ImproperFormat(s); }
      else {
        // store raster paths
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
          std::string warn = "IGNORING unrecognized command: " + std::string(s[0]) + " in .bbg file";
          WriteWarning(warn, pOptions->noisy_run);
        }
      }
      break;
      default:
      {
        // TEMP CONDITIONAL TO CATCH HAND FILES
        if (std::string(s[0]).find("hand") != std::string::npos) {
          break;
        }
        std::string errString = "Unrecognized command in .bbg file:\n   " + std::string(s[0]);
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
  bbg.close();

  delete pp;
  pp = NULL;
  return true;
}
