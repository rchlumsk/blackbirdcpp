#ifndef BLACKBIRDINCLUDE_H
#define BLACKBIRDINCLUDE_H

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <set>
#include <stdlib.h>
#include <string>
#include <strstream>
#include <sstream>
#include <unordered_map>
#include <valarray>
#include <vector>

//*****************************************************************
// Global Variables (necessary, but minimized, evils)
//*****************************************************************
extern std::string g_output_directory; ///< Had to be here to avoid passing Options structure around willy-nilly
extern bool   g_suppress_warnings;///< Had to be here to avoid passing Options object around willy-nilly

// Model version
const std::string __BLACKBIRD_VERSION__ = "0.0.0";
//*****************************************************************
// Global Constants
//*****************************************************************
const double HEADWATER               = -111111; // arbitrary value indicating that a node is a headwater node
const double PLACEHOLDER             = -222222; // arbitrary value indicating a placeholder value
const std::string PLACEHOLDER_STR    = "-222222";  // arbitrary value indicating a placeholder string value
const double GRAVITY                 = 9.81; // gravitational acceleration

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
  int nodeID                       = PLACEHOLDER;
  int reachID                      = PLACEHOLDER;
  int downnodeID                   = PLACEHOLDER;
  int upnodeID1                    = PLACEHOLDER;
  int upnodeID2                    = PLACEHOLDER;
  std::string stationname          = PLACEHOLDER_STR;
  double station                   = PLACEHOLDER;
  double reach_length_DS           = PLACEHOLDER;
  double reach_length_US1          = PLACEHOLDER;
  double reach_length_US2          = PLACEHOLDER;
  double flow                      = PLACEHOLDER;
  double flow_lob                  = PLACEHOLDER;
  double flow_main                 = PLACEHOLDER;
  double flow_rob                  = PLACEHOLDER;
  double min_elev                  = PLACEHOLDER;
  double wsl                       = PLACEHOLDER;
  double depth                     = PLACEHOLDER;
  double hyd_depth                 = PLACEHOLDER;
  double hyd_depth_lob             = PLACEHOLDER;
  double hyd_depth_main            = PLACEHOLDER;
  double hyd_depth_rob             = PLACEHOLDER;
  double top_width                 = PLACEHOLDER;
  double top_width_lob             = PLACEHOLDER;
  double top_width_main            = PLACEHOLDER;
  double top_width_rob             = PLACEHOLDER;
  double velocity                  = PLACEHOLDER;
  double velocity_lob              = PLACEHOLDER;
  double velocity_main             = PLACEHOLDER;
  double velocity_rob              = PLACEHOLDER;
  double k_total                   = PLACEHOLDER;
  double k_lob                     = PLACEHOLDER;
  double k_main                    = PLACEHOLDER;
  double k_rob                     = PLACEHOLDER;
  double alpha                     = PLACEHOLDER;
  double area                      = PLACEHOLDER;
  double area_lob                  = PLACEHOLDER;
  double area_main                 = PLACEHOLDER;
  double area_rob                  = PLACEHOLDER;
  double hradius                   = PLACEHOLDER;
  double hradius_lob               = PLACEHOLDER;
  double hradius_main              = PLACEHOLDER;
  double hradius_rob               = PLACEHOLDER;
  double wet_perimeter             = PLACEHOLDER;
  double wet_perimeter_lob         = PLACEHOLDER;
  double wet_perimeter_main        = PLACEHOLDER;
  double wet_perimeter_rob         = PLACEHOLDER;
  double energy_total              = PLACEHOLDER;
  double velocity_head             = PLACEHOLDER;
  double froude                    = PLACEHOLDER;
  double sf                        = PLACEHOLDER;
  double sf_avg                    = PLACEHOLDER;
  double sbed                      = PLACEHOLDER;
  double length_effective          = PLACEHOLDER;
  double head_loss                 = PLACEHOLDER;
  double manning_lob               = PLACEHOLDER;
  double manning_main              = PLACEHOLDER;
  double manning_rob               = PLACEHOLDER;
  double manning_composite         = PLACEHOLDER;
  double k_total_areaconv          = PLACEHOLDER;
  double k_total_roughconv         = PLACEHOLDER;
  double k_total_disconv           = PLACEHOLDER;
  double alpha_areaconv            = PLACEHOLDER;
  double alpha_roughconv           = PLACEHOLDER;
  double alpha_disconv             = PLACEHOLDER;
  double nc_equalforce             = PLACEHOLDER;
  double nc_equalvelocity          = PLACEHOLDER;
  double nc_wavgwp                 = PLACEHOLDER;
  double nc_wavgarea               = PLACEHOLDER;
  double nc_wavgconv               = PLACEHOLDER;
  double depth_critical            = PLACEHOLDER;
  int cp_iterations                = PLACEHOLDER;
  double k_err                     = PLACEHOLDER;
  double ws_err                    = PLACEHOLDER;
  double length_energyloss         = PLACEHOLDER;
  double length_effectiveadjusted  = PLACEHOLDER;
  double bed_slope = PLACEHOLDER; // is this needed?
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

enum enum_bc_type
{
  NORMAL_DEPTH,
  SET_WSL,
  SET_DEPTH
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

inline std::valarray<double> ConvCalc(std::valarray<double> n,
                                      std::valarray<double> A,
                                      std::valarray<double> Rh) {
  std::valarray<double> res = (1. / n) * A * std::pow(Rh, 2. / 3.);
  res[res < 0] = 0;
  return res;
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

#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#endif