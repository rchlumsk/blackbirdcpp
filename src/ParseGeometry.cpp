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
/// \param *&pModel [in/out] Reference to model object
/// \param *&pOptions [in] Global model options information
/// \return True if operation is successful
//
bool ParseGeometryFile(CModel*& pModel, COptions*const& pOptions)
{
  CStreamnode*     pSN(NULL);             //temp pointers
  hydraulic_output *pHO(NULL);
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
    else if (!strcmp(s[0], ":PreprocessedHydraulicTables")) { code = -2; } //treat as comment - unnecessary marker?
    else if (!strcmp(s[0], ":PreprocHydTable")) { code = 2; }
    else if (!strcmp(s[0], ":CrossSections")) { code = -2; } //treat as comment - unnecessary marker?
    else if (!strcmp(s[0], ":StreamnodeCrossSection")) { code = 3; }

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

      filename = CorrectForRelativePath(filename, pOptions->bbg_filename);

      INPUT2.open(filename.c_str());
      if (INPUT2.fail()) {
        std::string warn = "ParseGeometryFile: :RedirectToFile: Cannot find file " + filename;
        ExitGracefully(warn.c_str(), BAD_DATA);
      }
      else {
        if (pMainParser != NULL) {
          ExitGracefully("ParseGeometryFile::nested :RedirectToFile commands (in already redirected files) are not allowed.", BAD_DATA);
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
                error = "ParseGeometry File: nodetype \"" + std::string(s[1]) + "\" in row " + std::to_string(row) + " of :Streamnodes must be a REACH or XSECTION";
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
    { /*:PreprocHydTable*/
      if (pOptions->noisy_run) { std::cout << "Preprocessed hydraulic table..." << std::endl; }
      ExitGracefullyIf(pModel->bbsn->size() == 0, "ParseGeometry File: :Streamnodes must come before :PreprocHydTable", exitcode::BAD_DATA);
      bool done = false;
      int row = 0;
      if (Len < 2) { pp->ImproperFormat(s); }
      else {
        std::string error;
        if (StringIsLong(s[1])) {
          pSN = NULL;
          pSN = pModel->get_streamnode_by_id(std::stoi(s[1]));
          if (pSN == NULL) {
            error = "ParseGeometry File: nodeID \"" + std::string(s[1]) + "\" after :PreprocHydTable does not exist in streamnodes object";
            ExitGracefully(error.c_str(), BAD_DATA_WARN);
          } else if (pSN->nodetype != enum_nodetype::REACH) {
            error = "ParseGeometry File: nodeID \"" + std::string(s[1]) + "\" is not of nodetype REACH and cannot have a :PreprocHydTable block";
            ExitGracefully(error.c_str(), BAD_DATA_WARN);
          }
        } else {
          error = "ParseGeometry File: nodeID \"" + std::string(s[1]) + "\" after :PreprocHydTable must be unique integer or long integer";
          ExitGracefully(error.c_str(), BAD_DATA_WARN);
        }
        while ((!done) && (!end_of_file))
        {
          end_of_file = pp->Tokenize(s, Len);
          if (IsComment(s[0], Len)) {}//comment line
          else if (!strcmp(s[0], ":Attributes")) {}//ignored by Blackbird - needed for GUIs
          else if (!strcmp(s[0], ":EndPreprocHydTable")) { done = true; }
          else
          {
            row++;
            if (Len < 75) { pp->ImproperFormat(s); }
            pHO = NULL;
            pHO = new hydraulic_output();
            ExitGracefullyIf(pHO == NULL, "ParsePreprocesssedTables", OUT_OF_MEMORY);

            if (strcmp(s[0], "NA")) {
              if (StringIsLong(s[0])) {
                pHO->nodeID = std::stoi(s[0]);
              }
              else {
                error = "ParseGeometry File: nodeID \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[1], "NA")) {
              if (StringIsLong(s[1])) {
                pHO->reachID = std::stoi(s[1]);
              }
              else {
                error = "ParseGeometry File: reachID \"" + std::string(s[1]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[2], "NA")) {
              if (StringIsLong(s[2])) {
                pHO->downnodeID = std::stoi(s[2]);
              }
              else {
                error = "ParseGeometry File: downnodeID \"" + std::string(s[2]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[3], "NA")) {
              if (StringIsLong(s[3])) {
                pHO->upnodeID1 = std::stoi(s[3]);
              }
              else {
                error = "ParseGeometry File: upnodeID1 \"" + std::string(s[3]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[4], "NA")) {
              if (StringIsLong(s[4])) {
                pHO->upnodeID2 = std::stoi(s[4]);
              }
              else {
                error = "ParseGeometry File: upnodeID2 \"" + std::string(s[4]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a unique integer or long integer";
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
                error = "ParseGeometry File: station \"" + std::string(s[6]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[7], "NA")) {
              if (StringIsDouble(s[7])) {
                pHO->reach_length_DS = std::stod(s[7]);
              }
              else {
                error = "ParseGeometry File: reach_length_DS \"" + std::string(s[7]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[8], "NA")) {
              if (StringIsDouble(s[8])) {
                pHO->reach_length_US1 = std::stod(s[8]);
              }
              else {
                error = "ParseGeometry File: reach_length_US1 \"" + std::string(s[8]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[9], "NA")) {
              if (StringIsDouble(s[9])) {
                pHO->reach_length_US2 = std::stod(s[9]);
              }
              else {
                error = "ParseGeometry File: reach_length_US2 \"" + std::string(s[9]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[10], "NA")) {
              if (StringIsDouble(s[10])) {
                pHO->flow = std::stod(s[10]);
              }
              else {
                error = "ParseGeometry File: flow \"" + std::string(s[10]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[11], "NA")) {
              if (StringIsDouble(s[11])) {
                pHO->flow_lob = std::stod(s[11]);
              }
              else {
                error = "ParseGeometry File: flow_lob \"" + std::string(s[11]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[12], "NA")) {
              if (StringIsDouble(s[12])) {
                pHO->flow_main = std::stod(s[12]);
              }
              else {
                error = "ParseGeometry File: flow_main \"" + std::string(s[12]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[13], "NA")) {
              if (StringIsDouble(s[13])) {
                pHO->flow_rob = std::stod(s[13]);
              }
              else {
                error = "ParseGeometry File: flow_rob \"" + std::string(s[13]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[14], "NA")) {
              if (StringIsDouble(s[14])) {
                pHO->min_elev = std::stod(s[14]);
              }
              else {
                error = "ParseGeometry File: min_elev \"" + std::string(s[14]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[15], "NA")) {
              if (StringIsDouble(s[15])) {
                pHO->wsl = std::stod(s[15]);
              }
              else {
                error = "ParseGeometry File: wsl \"" + std::string(s[15]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[16], "NA")) {
              if (StringIsDouble(s[16])) {
                pHO->depth = std::stod(s[16]);
              }
              else {
                error = "ParseGeometry File: depth \"" + std::string(s[16]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[17], "NA")) {
              if (StringIsDouble(s[17])) {
                pHO->hyd_depth = std::stod(s[17]);
              }
              else {
                error = "ParseGeometry File: hyd_depth \"" + std::string(s[17]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[18], "NA")) {
              if (StringIsDouble(s[18])) {
                pHO->hyd_depth_lob = std::stod(s[18]);
              }
              else {
                error = "ParseGeometry File: hyd_depth_lob \"" + std::string(s[18]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[19], "NA")) {
              if (StringIsDouble(s[19])) {
                pHO->hyd_depth_main = std::stod(s[19]);
              }
              else {
                error = "ParseGeometry File: hyd_depth_main \"" + std::string(s[19]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[20], "NA")) {
              if (StringIsDouble(s[20])) {
                pHO->hyd_depth_rob = std::stod(s[20]);
              }
              else {
                error = "ParseGeometry File: hyd_depth_rob \"" + std::string(s[20]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[21], "NA")) {
              if (StringIsDouble(s[21])) {
                pHO->top_width = std::stod(s[21]);
              }
              else {
                error = "ParseGeometry File: top_width \"" + std::string(s[21]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[22], "NA")) {
              if (StringIsDouble(s[22])) {
                pHO->top_width_lob = std::stod(s[22]);
              }
              else {
                error = "ParseGeometry File: top_width_lob \"" + std::string(s[22]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[23], "NA")) {
              if (StringIsDouble(s[23])) {
                pHO->top_width_main = std::stod(s[23]);
              }
              else {
                error = "ParseGeometry File: top_width_main \"" + std::string(s[23]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[24], "NA")) {
              if (StringIsDouble(s[24])) {
                pHO->top_width_rob = std::stod(s[24]);
              }
              else {
                error = "ParseGeometry File: top_width_rob \"" + std::string(s[24]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[25], "NA")) {
              if (StringIsDouble(s[25])) {
                pHO->velocity = std::stod(s[25]);
              }
              else {
                error = "ParseGeometry File: velocity \"" + std::string(s[25]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[26], "NA")) {
              if (StringIsDouble(s[26])) {
                pHO->velocity_lob = std::stod(s[26]);
              }
              else {
                error = "ParseGeometry File: velocity_lob \"" + std::string(s[26]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[27], "NA")) {
              if (StringIsDouble(s[27])) {
                pHO->velocity_main = std::stod(s[27]);
              }
              else {
                error = "ParseGeometry File: velocity_main \"" + std::string(s[27]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[28], "NA")) {
              if (StringIsDouble(s[28])) {
                pHO->velocity_rob = std::stod(s[28]);
              }
              else {
                error = "ParseGeometry File: velocity_rob \"" + std::string(s[28]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[29], "NA")) {
              if (StringIsDouble(s[29])) {
                pHO->k_total = std::stod(s[29]);
              }
              else {
                error = "ParseGeometry File: k_total \"" + std::string(s[29]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[30], "NA")) {
              if (StringIsDouble(s[30])) {
                pHO->k_lob = std::stod(s[30]);
              }
              else {
                error = "ParseGeometry File: k_lob \"" + std::string(s[30]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[31], "NA")) {
              if (StringIsDouble(s[31])) {
                pHO->k_main = std::stod(s[31]);
              }
              else {
                error = "ParseGeometry File: k_main \"" + std::string(s[31]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[32], "NA")) {
              if (StringIsDouble(s[32])) {
                pHO->k_rob = std::stod(s[32]);
              }
              else {
                error = "ParseGeometry File: k_rob \"" + std::string(s[32]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[33], "NA")) {
              if (StringIsDouble(s[33])) {
                pHO->alpha = std::stod(s[33]);
              }
              else {
                error = "ParseGeometry File: alpha \"" + std::string(s[33]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[34], "NA")) {
              if (StringIsDouble(s[34])) {
                pHO->area = std::stod(s[34]);
              }
              else {
                error = "ParseGeometry File: area \"" + std::string(s[34]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[35], "NA")) {
              if (StringIsDouble(s[35])) {
                pHO->area_lob = std::stod(s[35]);
              }
              else {
                error = "ParseGeometry File: area_lob \"" + std::string(s[35]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[36], "NA")) {
              if (StringIsDouble(s[36])) {
                pHO->area_main = std::stod(s[36]);
              }
              else {
                error = "ParseGeometry File: area_main \"" + std::string(s[36]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[37], "NA")) {
              if (StringIsDouble(s[37])) {
                pHO->area_rob = std::stod(s[37]);
              }
              else {
                error = "ParseGeometry File: area_rob \"" + std::string(s[37]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[38], "NA")) {
              if (StringIsDouble(s[38])) {
                pHO->hradius = std::stod(s[38]);
              }
              else {
                error = "ParseGeometry File: hradius \"" + std::string(s[38]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[39], "NA")) {
              if (StringIsDouble(s[39])) {
                pHO->hradius_lob = std::stod(s[39]);
              }
              else {
                error = "ParseGeometry File: hradius_lob \"" + std::string(s[39]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[40], "NA")) {
              if (StringIsDouble(s[40])) {
                pHO->hradius_main = std::stod(s[40]);
              }
              else {
                error = "ParseGeometry File: hradius_main \"" + std::string(s[40]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[41], "NA")) {
              if (StringIsDouble(s[41])) {
                pHO->hradius_rob = std::stod(s[41]);
              }
              else {
                error = "ParseGeometry File: hradius_rob \"" + std::string(s[41]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[42], "NA")) {
              if (StringIsDouble(s[42])) {
                pHO->wet_perimeter = std::stod(s[42]);
              }
              else {
                error = "ParseGeometry File: wet_perimeter \"" + std::string(s[42]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[43], "NA")) {
              if (StringIsDouble(s[43])) {
                pHO->wet_perimeter_lob = std::stod(s[43]);
              }
              else {
                error = "ParseGeometry File: wet_perimeter_lob \"" + std::string(s[43]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[44], "NA")) {
              if (StringIsDouble(s[44])) {
                pHO->wet_perimeter_main = std::stod(s[44]);
              }
              else {
                error = "ParseGeometry File: wet_perimeter_main \"" + std::string(s[44]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[45], "NA")) {
              if (StringIsDouble(s[45])) {
                pHO->wet_perimeter_rob = std::stod(s[45]);
              }
              else {
                error = "ParseGeometry File: wet_perimeter_rob \"" + std::string(s[45]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[46], "NA")) {
              if (StringIsDouble(s[46])) {
                pHO->energy_total = std::stod(s[46]);
              }
              else {
                error = "ParseGeometry File: energy_total \"" + std::string(s[46]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[47], "NA")) {
              if (StringIsDouble(s[47])) {
                pHO->velocity_head = std::stod(s[47]);
              }
              else {
                error = "ParseGeometry File: velocity_head \"" + std::string(s[47]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[48], "NA")) {
              if (StringIsDouble(s[48])) {
                pHO->froude = std::stod(s[48]);
              }
              else {
                error = "ParseGeometry File: froude \"" + std::string(s[48]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[49], "NA")) {
              if (StringIsDouble(s[49])) {
                pHO->sf = std::stod(s[49]);
              }
              else {
                error = "ParseGeometry File: sf \"" + std::string(s[49]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[50], "NA")) {
              if (StringIsDouble(s[50])) {
                pHO->sf_avg = std::stod(s[50]);
              }
              else {
                error = "ParseGeometry File: sf_avg \"" + std::string(s[50]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[51], "NA")) {
              if (StringIsDouble(s[51])) {
                pHO->sbed = std::stod(s[51]);
              }
              else {
                error = "ParseGeometry File: sbed \"" + std::string(s[51]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[52], "NA")) {
              if (StringIsDouble(s[52])) {
                pHO->length_effective = std::stod(s[52]);
              }
              else {
                error = "ParseGeometry File: length_effective \"" + std::string(s[52]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[53], "NA")) {
              if (StringIsDouble(s[53])) {
                pHO->head_loss = std::stod(s[53]);
              }
              else {
                error = "ParseGeometry File: head_loss \"" + std::string(s[53]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[54], "NA")) {
              if (StringIsDouble(s[54])) {
                pHO->manning_lob = std::stod(s[54]);
              }
              else {
                error = "ParseGeometry File: manning_lob \"" + std::string(s[54]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[55], "NA")) {
              if (StringIsDouble(s[55])) {
                pHO->manning_main = std::stod(s[55]);
              }
              else {
                error = "ParseGeometry File: manning_main \"" + std::string(s[55]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[56], "NA")) {
              if (StringIsDouble(s[56])) {
                pHO->manning_rob = std::stod(s[56]);
              }
              else {
                error = "ParseGeometry File: manning_rob \"" + std::string(s[56]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[57], "NA")) {
              if (StringIsDouble(s[57])) {
                pHO->manning_composite = std::stod(s[57]);
              }
              else {
                error = "ParseGeometry File: manning_composite \"" + std::string(s[57]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[58], "NA")) {
              if (StringIsDouble(s[58])) {
                pHO->k_total_areaconv = std::stod(s[58]);
              }
              else {
                error = "ParseGeometry File: k_total_areaconv \"" + std::string(s[58]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[59], "NA")) {
              if (StringIsDouble(s[59])) {
                pHO->k_total_roughconv = std::stod(s[59]);
              }
              else {
                error = "ParseGeometry File: k_total_roughconv \"" + std::string(s[59]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[60], "NA")) {
              if (StringIsDouble(s[60])) {
                pHO->k_total_disconv = std::stod(s[60]);
              }
              else {
                error = "ParseGeometry File: k_total_disconv \"" + std::string(s[60]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[61], "NA")) {
              if (StringIsDouble(s[61])) {
                pHO->alpha_areaconv = std::stod(s[61]);
              }
              else {
                error = "ParseGeometry File: alpha_areaconv \"" + std::string(s[61]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[62], "NA")) {
              if (StringIsDouble(s[62])) {
                pHO->alpha_roughconv = std::stod(s[62]);
              }
              else {
                error = "ParseGeometry File: alpha_roughconv \"" + std::string(s[62]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[63], "NA")) {
              if (StringIsDouble(s[63])) {
                pHO->alpha_disconv = std::stod(s[63]);
              }
              else {
                error = "ParseGeometry File: alpha_disconv \"" + std::string(s[63]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[64], "NA")) {
              if (StringIsDouble(s[64])) {
                pHO->nc_equalforce = std::stod(s[64]);
              }
              else {
                error = "ParseGeometry File: nc_equalforce \"" + std::string(s[64]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[65], "NA")) {
              if (StringIsDouble(s[65])) {
                pHO->nc_equalvelocity = std::stod(s[65]);
              }
              else {
                error = "ParseGeometry File: nc_equalvelocity \"" + std::string(s[65]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[66], "NA")) {
              if (StringIsDouble(s[66])) {
                pHO->nc_wavgwp = std::stod(s[66]);
              }
              else {
                error = "ParseGeometry File: nc_wavgwp \"" + std::string(s[66]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[67], "NA")) {
              if (StringIsDouble(s[67])) {
                pHO->nc_wavgarea = std::stod(s[67]);
              }
              else {
                error = "ParseGeometry File: nc_wavgarea \"" + std::string(s[67]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[68], "NA")) {
              if (StringIsDouble(s[68])) {
                pHO->nc_wavgconv = std::stod(s[68]);
              }
              else {
                error = "ParseGeometry File: nc_wavgconv \"" + std::string(s[68]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[69], "NA")) {
              if (StringIsDouble(s[69])) {
                pHO->depth_critical = std::stod(s[69]);
              }
              else {
                error = "ParseGeometry File: depth_critical \"" + std::string(s[69]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[70], "NA")) {
              if (StringIsLong(s[70])) {
                pHO->cp_iterations = std::stoi(s[70]);
              }
              else {
                error = "ParseGeometry File: cp_iterations \"" + std::string(s[70]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be an integer or long integer";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[71], "NA")) {
              if (StringIsDouble(s[71])) {
                pHO->k_err = std::stod(s[71]);
              }
              else {
                error = "ParseGeometry File: k_err \"" + std::string(s[71]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[72], "NA")) {
              if (StringIsDouble(s[72])) {
                pHO->ws_err = std::stod(s[72]);
              }
              else {
                error = "ParseGeometry File: ws_err \"" + std::string(s[72]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[73], "NA")) {
              if (StringIsDouble(s[73])) {
                pHO->length_energyloss = std::stod(s[73]);
              }
              else {
                error = "ParseGeometry File: length_energyloss \"" + std::string(s[73]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            if (strcmp(s[74], "NA")) {
              if (StringIsDouble(s[74])) {
                pHO->length_effectiveadjusted = std::stod(s[74]);
              }
              else {
                error = "ParseGeometry File: length_effectiveadjusted \"" + std::string(s[74]) + "\" in row " + std::to_string(row) + " of :PreprocHydTable must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            pSN->add_depthdf_row(pHO);
          }
        }
      }
      break;
    }
    case(3):  //----------------------------------------------
    { /*:StreamnodeCrossSection*/
      if (pOptions->noisy_run) { std::cout << "Cross section data..." << std::endl; }
      ExitGracefullyIf(pModel->bbsn->size() == 0, "ParseGeometry File: :Streamnodes must come before :StreamnodeCrossSection", exitcode::BAD_DATA);
      bool done = false;
      int row = 0;
      std::vector<double> xx_vec;
      std::vector<double> zz_vec;
      std::vector<double> nn_vec;
      if (Len < 2) { pp->ImproperFormat(s); }
      else {
        std::string error;
        if (StringIsLong(s[1])) {
          pSN = NULL;
          pSN = pModel->get_streamnode_by_id(std::stoi(s[1]));
          if (pSN == NULL) {
            error = "ParseGeometry File: nodeID \"" + std::string(s[1]) + "\" after :StreamnodeCrossSection does not exist in streamnodes object";
            ExitGracefully(error.c_str(), BAD_DATA_WARN);
          } else if (pSN->nodetype != enum_nodetype::XSECTION) {
            error = "ParseGeometry File: nodeID \"" + std::string(s[1]) + "\" is not of nodetype XSECTION and cannot have a :StreamnodeCrossSection block";
            ExitGracefully(error.c_str(), BAD_DATA_WARN);
          }
        } else {
          error = "ParseGeometry File: nodeID \"" + std::string(s[1]) + "\" after :StreamnodeCrossSection must be unique integer or long integer";
          ExitGracefully(error.c_str(), BAD_DATA_WARN);
        }
        while ((!done) && (!end_of_file))
        {
          end_of_file = pp->Tokenize(s, Len);
          if (IsComment(s[0], Len)) {}//comment line
          else if (!strcmp(s[0], ":Attributes")) {}//ignored by Blackbird - needed for GUIs
          else if (!strcmp(s[0], ":EndStreamnodeCrossSection")) { done = true; }
          else
          {
            row++;
            if (Len < 2 || (row == 0 && Len < 3) ) { pp->ImproperFormat(s); }

            if (strcmp(s[0], "NA")) {
              if (StringIsDouble(s[0])) {
                xx_vec.push_back(fast_s_to_d(s[0]));
              }
              else {
                error = "ParseGeometry File: xx value \"" + std::string(s[0]) + "\" in row " + std::to_string(row) + " of :StreamnodeCrossSection must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            else {
              xx_vec.push_back(PLACEHOLDER);
            }
            if (strcmp(s[1], "NA")) {
              if (StringIsDouble(s[1])) {
                zz_vec.push_back(fast_s_to_d(s[1]));
              }
              else {
                error = "ParseGeometry File: zz value \"" + std::string(s[1]) + "\" in row " + std::to_string(row) + " of :StreamnodeCrossSection must be a double";
                ExitGracefully(error.c_str(), BAD_DATA_WARN);
              }
            }
            else {
              zz_vec.push_back(PLACEHOLDER);
            }
            if (Len > 2) {
              if (strcmp(s[2], "NA")) {
                if (StringIsDouble(s[2])) {
                  nn_vec.push_back(fast_s_to_d(s[2]));
                } else {
                  error = "ParseGeometry File: nn value \"" + std::string(s[2]) + "\" in row " + std::to_string(row) + " of :StreamnodeCrossSection must be a double";
                  ExitGracefully(error.c_str(), BAD_DATA_WARN);
                }
              }
            }
            else {
              nn_vec.push_back(nn_vec.back());
            }
          }
        }
        CXSection *xs_pSN = (CXSection *)pSN;
        xs_pSN->xx = std::valarray<double>(xx_vec.data(), xx_vec.size());
        xs_pSN->zz = std::valarray<double>(zz_vec.data(), zz_vec.size());
        xs_pSN->manning = std::valarray<double>(nn_vec.data(), nn_vec.size());
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
