#ifndef BLACKBIRDINCLUDE_H
#define BLACKBIRDINCLUDE_H

#include <algorithm>
#include <cmath>
#include <cpl_conv.h>
#include <cstring>
#include <fstream>
#include <gdal_priv.h>
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
//struct hydraulic_output {
//  int nodeID                       = PLACEHOLDER;
//  int reachID                      = PLACEHOLDER;
//  int downnodeID                   = PLACEHOLDER;
//  int upnodeID1                    = PLACEHOLDER;
//  int upnodeID2                    = PLACEHOLDER;
//  std::string stationname          = PLACEHOLDER_STR;
//  double station                   = PLACEHOLDER;
//  double reach_length_DS           = PLACEHOLDER;
//  double reach_length_US1          = PLACEHOLDER;
//  double reach_length_US2          = PLACEHOLDER;
//  double flow                      = PLACEHOLDER;
//  double flow_lob                  = PLACEHOLDER;
//  double flow_main                 = PLACEHOLDER;
//  double flow_rob                  = PLACEHOLDER;
//  double min_elev                  = PLACEHOLDER;
//  double wsl                       = PLACEHOLDER;
//  double depth                     = PLACEHOLDER;
//  double hyd_depth                 = PLACEHOLDER;
//  double hyd_depth_lob             = PLACEHOLDER;
//  double hyd_depth_main            = PLACEHOLDER;
//  double hyd_depth_rob             = PLACEHOLDER;
//  double top_width                 = PLACEHOLDER;
//  double top_width_lob             = PLACEHOLDER;
//  double top_width_main            = PLACEHOLDER;
//  double top_width_rob             = PLACEHOLDER;
//  double velocity                  = PLACEHOLDER;
//  double velocity_lob              = PLACEHOLDER;
//  double velocity_main             = PLACEHOLDER;
//  double velocity_rob              = PLACEHOLDER;
//  double k_total                   = PLACEHOLDER;
//  double k_lob                     = PLACEHOLDER;
//  double k_main                    = PLACEHOLDER;
//  double k_rob                     = PLACEHOLDER;
//  double alpha                     = PLACEHOLDER;
//  double area                      = PLACEHOLDER;
//  double area_lob                  = PLACEHOLDER;
//  double area_main                 = PLACEHOLDER;
//  double area_rob                  = PLACEHOLDER;
//  double hradius                   = PLACEHOLDER;
//  double hradius_lob               = PLACEHOLDER;
//  double hradius_main              = PLACEHOLDER;
//  double hradius_rob               = PLACEHOLDER;
//  double wet_perimeter             = PLACEHOLDER;
//  double wet_perimeter_lob         = PLACEHOLDER;
//  double wet_perimeter_main        = PLACEHOLDER;
//  double wet_perimeter_rob         = PLACEHOLDER;
//  double energy_total              = PLACEHOLDER;
//  double velocity_head             = PLACEHOLDER;
//  double froude                    = PLACEHOLDER;
//  double sf                        = PLACEHOLDER;
//  double sf_avg                    = PLACEHOLDER;
//  double sbed                      = PLACEHOLDER;
//  double length_effective          = PLACEHOLDER;
//  double head_loss                 = PLACEHOLDER;
//  double manning_lob               = PLACEHOLDER;
//  double manning_main              = PLACEHOLDER;
//  double manning_rob               = PLACEHOLDER;
//  double manning_composite         = PLACEHOLDER;
//  double k_total_areaconv          = PLACEHOLDER;
//  double k_total_roughconv         = PLACEHOLDER;
//  double k_total_disconv           = PLACEHOLDER;
//  double alpha_areaconv            = PLACEHOLDER;
//  double alpha_roughconv           = PLACEHOLDER;
//  double alpha_disconv             = PLACEHOLDER;
//  double nc_equalforce             = PLACEHOLDER;
//  double nc_equalvelocity          = PLACEHOLDER;
//  double nc_wavgwp                 = PLACEHOLDER;
//  double nc_wavgarea               = PLACEHOLDER;
//  double nc_wavgconv               = PLACEHOLDER;
//  double depth_critical            = PLACEHOLDER;
//  int cp_iterations                = PLACEHOLDER;
//  double k_err                     = PLACEHOLDER;
//  double ws_err                    = PLACEHOLDER;
//  double length_energyloss         = PLACEHOLDER;
//  double length_effectiveadjusted  = PLACEHOLDER;
//  double bed_slope = PLACEHOLDER; // is this needed?
//};
struct hydraulic_output {
  int nodeID = 0;
  int reachID = 0;
  int downnodeID = 0;
  int upnodeID1 = 0;
  int upnodeID2 = 0;
  std::string stationname = "";
  double station = 0;
  double reach_length_DS = 0;
  double reach_length_US1 = 0;
  double reach_length_US2 = 0;
  double flow = 0;
  double flow_lob = 0;
  double flow_main = 0;
  double flow_rob = 0;
  double min_elev = 0;
  double wsl = 0;
  double depth = 0;
  double hyd_depth = 0;
  double hyd_depth_lob = 0;
  double hyd_depth_main = 0;
  double hyd_depth_rob = 0;
  double top_width = 0;
  double top_width_lob = 0;
  double top_width_main = 0;
  double top_width_rob = 0;
  double velocity = 0;
  double velocity_lob = 0;
  double velocity_main = 0;
  double velocity_rob = 0;
  double k_total = 0;
  double k_lob = 0;
  double k_main = 0;
  double k_rob = 0;
  double alpha = 0;
  double area = 0;
  double area_lob = 0;
  double area_main = 0;
  double area_rob = 0;
  double hradius = 0;
  double hradius_lob = 0;
  double hradius_main = 0;
  double hradius_rob = 0;
  double wet_perimeter = 0;
  double wet_perimeter_lob = 0;
  double wet_perimeter_main = 0;
  double wet_perimeter_rob = 0;
  double energy_total = 0;
  double velocity_head = 0;
  double froude = 0;
  double sf = 0;
  double sf_avg = 0;
  double sbed = 0;
  double length_effective = 0;
  double head_loss = 0;
  double manning_lob = 0;
  double manning_main = 0;
  double manning_rob = 0;
  double manning_composite = 0;
  double k_total_areaconv = 0;
  double k_total_roughconv = 0;
  double k_total_disconv = 0;
  double alpha_areaconv = 0;
  double alpha_roughconv = 0;
  double alpha_disconv = 0;
  double nc_equalforce = 0;
  double nc_equalvelocity = 0;
  double nc_wavgwp = 0;
  double nc_wavgarea = 0;
  double nc_wavgconv = 0;
  double depth_critical = 0;
  int cp_iterations = 0;
  double k_err = 0;
  double ws_err = 0;
  double length_energyloss = 0;
  double length_effectiveadjusted = 0;
  double bed_slope = 0; // is this needed?
};

//*****************************************************************
//Enumerables
//*****************************************************************
enum enum_nodetype
{
  REACH,
  XSECTION
};

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
  NONE,
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

///////////////////////////////////////////////////////////////////
/// \brief returns true if character string is double
/// \return true if character string is double
//
inline bool StringIsDouble(const char *s1) {
  char *p;
  strtod(s1, &p);
  return !(*p);
}

inline std::valarray<double> ConvCalc(std::valarray<double> n,
                                      std::valarray<double> A,
                                      std::valarray<double> Rh) {
  std::valarray<double> res = (1. / n) * A * std::pow(Rh, 2. / 3.);
  res[res < 0] = 0;
  return res;
}
inline double ConvCalc(double n, double A, double Rh) {
  double res = (1. / n) * A * std::pow(Rh, 2. / 3.);
  res = res < 0 ? 0 : res;
  return res;
}

///////////////////////////////////////////////////////////////////
/// \brief returns a vector of all values in the f column of a vector of hydraulic outputs.
/// Example call with std::vector<hydraulic_output *> v as the vector: res = hyd_out_collect(&hydraulic_output::wsl, v)
/// \return a vector of all values in the f column of a vector of hydraulic outputs
//
template<typename T>
inline std::vector<T> hyd_out_collect(T hydraulic_output::* f, std::vector<hydraulic_output *> const& v) {
  std::vector<T> output;
  for (auto const &elem : v) {
    output.push_back(elem->*f);
  }
  return output;
}

///////////////////////////////////////////////////////////////////
/// \brief returns extrapolated value for the f column of hydraulic_output based on the last 2 rows of depthdf and new wsl value
/// \return extrapolated value for the f column of hydraulic_output based on the last 2 rows of depthdf and new wsl value
//
inline double extrapolate(double new_wsl, double hydraulic_output::* f, std::vector<hydraulic_output*> const& v) {
  double x2 = v[v.size() - 1]->wsl;
  double x1 = v[v.size() - 2]->wsl;
  double y2 = v[v.size() - 1]->*f;
  double y1 = v[v.size() - 2]->*f;
  if (x2 == PLACEHOLDER || x1 == PLACEHOLDER || y2 == PLACEHOLDER || y1 == PLACEHOLDER) {
    return PLACEHOLDER;
  }
  double slope = (y2 - y1) / (x2 - x1);
  return y2 + slope * (new_wsl - x2);
}

///////////////////////////////////////////////////////////////////
/// \brief returns interpolated value for the f column of hydraulic_output based on the closest 2 rows of depthdf and new wsl value
/// \return interpolated value for the f column of hydraulic_output based on the closest 2 rows of depthdf and new wsl value
//
inline double interpolate(double new_wsl, double hydraulic_output::* f, std::vector<hydraulic_output*> const& v) {
  ExitGracefullyIf(v.size() == 0,
                   "BlackbirdInclude.h: interpolate: provided v is of size 0",
                   exitcode::RUNTIME_ERR);
  int i = 0;
  while (i < v.size() && v[i]->wsl < new_wsl) {
    i++;
  }
  if (i == v.size() || i == 0) {
    ExitGracefully("BlackbirdInclude.h: interpolate: new_wsl found to be "
                   "outside of depthdf bounds",
                   exitcode::RUNTIME_ERR);
  }
  double x2 = v[i]->wsl;
  double x1 = v[i - 1]->wsl;
  double y2 = v[i]->*f;
  double y1 = v[i - 1]->*f;
  if (x2 == PLACEHOLDER || x1 == PLACEHOLDER || y2 == PLACEHOLDER || y1 == PLACEHOLDER) {
    return PLACEHOLDER;
  }
  double slope = (y2 - y1) / (x2 - x1);
  return y2 + slope * (new_wsl - x2);
}

///////////////////////////////////////////////////////////////////
/// \brief helper functions for printing enumerables
/// \return string corresponding to enumerable option
//
inline std::string toString(enum_nodetype type) {
  switch (type) {
  case REACH: return "REACH";
  case XSECTION: return "XSECTION";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_mt_method method) {
  switch (method) {
  case HAND_MANNING: return "HAND_MANNING";
  case STEADYFLOW: return "STEADYFLOW";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_rt_method method) {
  switch (method) {
  case SUBCRITICAL: return "SUBCRITICAL";
  case SUPERCRITICAL: return "SUPERCRITICAL";
  case MIXED: return "MIXED";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_fs_method method) {
  switch (method) {
  case AVERAGE_CONVEYANCE: return "AVERAGE_CONVEYANCE";
  case AVERAGE_FRICTION: return "AVERAGE_FRICTION";
  case GEOMETRIC_FRICTION: return "GEOMETRIC_FRICTION";
  case HARMONIC_FRICTION: return "HARMONIC_FRICTION";
  case REACH_FRICTION: return "REACH_FRICTION";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_mc_method method) {
  switch (method) {
  case EQUAL_FORCE: return "EQUAL_FORCE";
  case WEIGHTED_AVERAGE_AREA: return "WEIGHTED_AVERAGE_AREA";
  case WEIGHTED_AVERAGE_WETPERIMETER: return "WEIGHTED_AVERAGE_WETPERIMETER";
  case WEIGHTED_AVERAGE_CONVEYANCE: return "WEIGHTED_AVERAGE_CONVEYANCE";
  case EQUAL_VELOCITY: return "EQUAL_VELOCITY";
  case BLENDED_NC: return "BLENDED_NC";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_xsc_method method) {
  switch (method) {
  case OVERBANK_CONVEYANCE: return "OVERBANK_CONVEYANCE";
  case DEFAULT_CONVEYANCE: return "DEFAULT_CONVEYANCE";
  case COORDINATE_CONVEYANCE: return "COORDINATE_CONVEYANCE";
  case DISCRETIZED_CONVEYANCE_XS: return "DISCRETIZED_CONVEYANCE_XS";
  case AREAWEIGHTED_CONVEYANCE_ONECALC_XS:
    return "AREAWEIGHTED_CONVEYANCE_ONECALC_XS";
  case AREAWEIGHTED_CONVEYANCE: return "AREAWEIGHTED_CONVEYANCE";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_rc_method method) {
  switch (method) {
  case DISCRETIZED_CONVEYANCE_R: return "DISCRETIZED_CONVEYANCE_R";
  case AREAWEIGHTED_CONVEYANCE_ONECALC_R:
    return "AREAWEIGHTED_CONVEYANCE_ONECALC_R";
  case ROUGHZONE_CONVEYANCE: return "ROUGHZONE_CONVEYANCE";
  case BLENDED_CONVEYANCE: return "BLENDED_CONVEYANCE";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_ri_method method) {
  switch (method) {
  case EFFECTIVE_LENGTH: return "EFFECTIVE_LENGTH";
  case REACH_LENGTH: return "REACH_LENGTH";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_ppi_method method) {
  switch (method) {
  case NONE: return "NONE";
  case CATCHMENT_HAND: return "CATCHMENT_HAND";
  case CATCHMENT_DHAND: return "CATCHMENT_DHAND";
  case INTERP_HAND: return "INTERP_HAND";
  case INTERP_DHAND: return "INTERP_DHAND";
  case INTERP_DHAND_WSLCORR: return "INTERP_DHAND_WSLCORR";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_bc_type type) {
  switch (type) {
  case NORMAL_DEPTH: return "NORMAL_DEPTH";
  case SET_WSL: return "SET_WSL";
  case SET_DEPTH: return "SET_DEPTH";
  default: return "UNKNOWN";
  }
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