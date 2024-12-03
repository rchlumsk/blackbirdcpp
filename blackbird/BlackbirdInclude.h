#ifndef BLACKBIRDINCLUDE_H
#define BLACKBIRDINCLUDE_H

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <strstream>
#include <sstream>
#include <unordered_map>
#include <vector>

//*****************************************************************
// Global Variables (necessary, but minimized, evils)
//*****************************************************************
extern std::string g_output_directory; ///< Had to be here to avoid passing Options structure around willy-nilly
extern bool   g_suppress_warnings;///< Had to be here to avoid passing Options object around willy-nilly

//*****************************************************************
//Exit Strategies
//*****************************************************************

///////////////////////////////////////////////////////////////////
/// \brief Series of codes containing possible reasons for exiting
//
enum exitcode
{
  BAD_DATA,       ///< For bad input provided by user (requires immediate exit from program)
  BAD_DATA_WARN,  ///< For bad input provided by user (requires shutdown prior to simulation)
  RUNTIME_ERR,    ///< For runtime error (bad programming)
  FILE_OPEN_ERR,  ///< For bad file open (requires immediate exit)
  BLACKBIRD_OPEN_ERR, ///< for bad BlackbirdErrors.txt file open
  STUB,           ///< Stub function
  OUT_OF_MEMORY,  ///< When out of memory
  SIMULATION_DONE ///< Upon completion of the simulation
};

void FinalizeGracefully(const char* statement, exitcode code);  //defined in BlackbirdMain.cpp
void ExitGracefully(const char* statement, exitcode code);      //defined in BlackbirdMain.cpp

/////////////////////////////////////////////////////////////////
/// \brief In-line function that calls ExitGracefully function in the case of condition
///
/// \param condition [in] Boolean indicating if program should exit gracefully
/// \param statement [in] String to print to user upon exit
/// \param code [in] Code to determine why the system is exiting
//
inline void ExitGracefullyIf(bool condition, const char* statement, exitcode code)
{
  if (condition) { ExitGracefully(statement, code); }
}

//*****************************************************************
//Structures
//*****************************************************************
struct hydraulic_output {
  // Attributes
  int nodeID;
  int reachID;
  int downnodeID;
  int upnodeID1;
  int upnodeID2;
  std::string stationname;
  double station;
  double reach_length_DS;
  double reach_length_US1;
  double reach_length_US2;
  double flow;
  double flow_lob;
  double flow_main;
  double flow_rob;
  double min_elev;
  double wsl;
  double depth;
  double hyd_depth;
  double hyd_depth_lob;
  double hyd_depth_main;
  double hyd_depth_rob;
  double top_width;
  double top_width_lob;
  double top_width_main;
  double top_width_rob;
  double velocity;
  double velocity_lob;
  double velocity_main;
  double velocity_rob;
  double k_total;
  double k_lob;
  double k_main;
  double k_rob;
  double alpha;
  double area;
  double area_lob;
  double area_main;
  double area_rob;
  double hradius;
  double hradius_lob;
  double hradius_main;
  double hradius_rob;
  double wet_perimeter;
  double wet_perimeter_lob;
  double wet_perimeter_main;
  double wet_perimeter_rob;
  double energy_total;
  double velocity_head;
  double froude;
  double sf;
  double sf_avg;
  double sbed;
  double length_effective;
  double head_loss;
  double manning_lob;
  double manning_main;
  double manning_rob;
  double manning_composite;
  double k_total_areaconv;
  double k_total_roughconv;
  double k_total_disconv;
  double alpha_areaconv;
  double alpha_roughconv;
  double alpha_disconv;
  double nc_equalforce;
  double nc_equalvelocity;
  double nc_wavgwp;
  double nc_wavgarea;
  double nc_wavgconv;
  double depth_critical;
  int cp_iterations;
  double k_err;
  double ws_err;
  double length_energyloss;
  double length_effectiveadjusted;
};

//*****************************************************************
//Enumerables
//*****************************************************************
enum enum_mt_method
{
  HAND_MANNING,
  STEADYFLOW
};

enum enum_rt_method
{
  SUBCRITICAL,
  SUPERCRITICAL,
  MIXED
};

enum enum_fs_method
{
  AVERAGE_CONVEYANCE,
  AVERAGE_FRICTION,
  GEOMETRIC_FRICTION,
  HARMONIC_FRICTION,
  REACH_FRICTION
};

enum enum_mc_method
{
  EQUAL_FORCE,
  WEIGHTED_AVERAGE_AREA,
  WEIGHTED_AVERAGE_WETPERIMETER,
  WEIGHTED_AVERAGE_CONVEYANCE,
  EQUAL_VELOCITY,
  BLENDED_NC
};

enum enum_xsc_method
{
  OVERBANK_CONVEYANCE,
  DEFAULT_CONVEYANCE,
  COORDINATE_CONVEYANCE,
  DISCRETIZED_CONVEYANCE_XS,
  AREAWEIGHTED_CONVEYANCE_ONECALC_XS,
  AREAWEIGHTED_CONVEYANCE
};

enum enum_rc_method
{
  DISCRETIZED_CONVEYANCE_R,
  AREAWEIGHTED_CONVEYANCE_ONECALC_R,
  ROUGHZONE_CONVEYANCE,
  BLENDED_CONVEYANCE
};

enum enum_ri_method
{
  EFFECTIVE_LENGTH,
  REACH_LENGTH
};

enum enum_ppi_method
{
  CATCHMENT_HAND,
  CATCHMENT_DHAND,
  INTERP_HAND,
  INTERP_DHAND,
  INTERP_DHAND_WSLCORR
};

//*****************************************************************
//Common Functions (inline)
//*****************************************************************

///////////////////////////////////////////////////////////////////
/// \brief returns true if character string is long integer
/// \return true if character string is long integer
//
inline bool StringIsLong(const char* s1)
{
  char* p;
  strtol(s1, &p, 10);
  return !(*p);
}

///////////////////////////////////////////////////////////////////
/// \brief returns true if character string is long integer
/// \return true if character string is long integer
//
inline bool StringIsDouble(const char* s1)
{
  char* p;
  strtod(s1, &p);
  return !(*p);
}

//Parsing Functions-------------------------------------------
//defined in CommonFunctions.cpp
std::string   StringToUppercase(const std::string& s);
bool          IsComment(const char* s, const int Len);
void          WriteWarning(const std::string warn, bool noisy);
void          WriteAdvisory(const std::string warn, bool noisy);
double        fast_s_to_d(const char* s);

#endif