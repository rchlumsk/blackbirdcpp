#ifndef BLACKBIRDINCLUDE_H
#define BLACKBIRDINCLUDE_H

#define _BBNETCDF_

#ifdef netcdf
#define _BBNETCDF_      // if Makefile is used this will be automatically be uncommented if netCDF library is available
#endif
#ifdef _BBNETCDF_
//#include <netcdf.h>
#include <netcdf>
#endif

#include <algorithm>
#include <cmath>
#include <cpl_conv.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <gdal_priv.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <memory>
#include <ogrsf_frmts.h>
#include <png.h>
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

// Program version
const std::string __BLACKBIRD_VERSION__ = "0.0.0";
//*****************************************************************
// Global Constants
//*****************************************************************
const double ALMOST_INF = 1e99;   ///< Largest possible double value to be used
const double PRETTY_SMALL = 1e-8; ///< useful small tolerance for zero tests
const double REAL_SMALL = 1e-12;  ///< Smallest possible double value to be used
const double PI = 3.1415926535898; ///< Double approximation of pi

const double HEADWATER               = -111111; // arbitrary value indicating that a node is a headwater node
const double PLACEHOLDER             = -222222; // arbitrary value indicating a placeholder value
const std::string PLACEHOLDER_STR    = "-222222";  // arbitrary value indicating a placeholder string value
const double GRAVITY                 = 9.81; // gravitational acceleration

//units conversion constants
const double SEC_PER_DAY = 86400;    ///< days to seconds
const double MIN_PER_DAY = 1440;     ///< days to minutes
const double SEC_PER_HR = 3600;      ///< hours to seconds
const double DAYS_PER_YEAR = 365.25; ///< years to days
const double HR_PER_DAY = 24;        ///< days to hours
const double DAYS_PER_MONTH[12] = {31,28,31,30,31,30,31,31,30,31,30,31};   ///< Array of doubles containing the number of days in each month

//decision constants
const double  TIME_CORRECTION         =0.0001;                                  ///< [d]      offset for time series min/max functions

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
  double bed_slope;
  double peak_hrs_required;

  // Constructor PLACEHOLDER variant
  //hydraulic_output()
  //    : nodeID(PLACEHOLDER), reachID(PLACEHOLDER), downnodeID(PLACEHOLDER),
  //      upnodeID1(PLACEHOLDER), upnodeID2(PLACEHOLDER),
  //      stationname(PLACEHOLDER_STR), station(PLACEHOLDER),
  //      reach_length_DS(PLACEHOLDER), reach_length_US1(PLACEHOLDER),
  //      reach_length_US2(PLACEHOLDER), flow(PLACEHOLDER), flow_lob(PLACEHOLDER),
  //      flow_main(PLACEHOLDER), flow_rob(PLACEHOLDER), min_elev(PLACEHOLDER),
  //      wsl(PLACEHOLDER), depth(PLACEHOLDER), hyd_depth(PLACEHOLDER),
  //      hyd_depth_lob(PLACEHOLDER), hyd_depth_main(PLACEHOLDER),
  //      hyd_depth_rob(PLACEHOLDER), top_width(PLACEHOLDER),
  //      top_width_lob(PLACEHOLDER), top_width_main(PLACEHOLDER),
  //      top_width_rob(PLACEHOLDER), velocity(PLACEHOLDER),
  //      velocity_lob(PLACEHOLDER), velocity_main(PLACEHOLDER),
  //      velocity_rob(PLACEHOLDER), k_total(PLACEHOLDER), k_lob(PLACEHOLDER),
  //      k_main(PLACEHOLDER), k_rob(PLACEHOLDER), alpha(PLACEHOLDER),
  //      area(PLACEHOLDER), area_lob(PLACEHOLDER), area_main(PLACEHOLDER),
  //      area_rob(PLACEHOLDER), hradius(PLACEHOLDER), hradius_lob(PLACEHOLDER),
  //      hradius_main(PLACEHOLDER), hradius_rob(PLACEHOLDER),
  //      wet_perimeter(PLACEHOLDER), wet_perimeter_lob(PLACEHOLDER),
  //      wet_perimeter_main(PLACEHOLDER), wet_perimeter_rob(PLACEHOLDER),
  //      energy_total(PLACEHOLDER), velocity_head(PLACEHOLDER),
  //      froude(PLACEHOLDER), sf(PLACEHOLDER), sf_avg(PLACEHOLDER),
  //      sbed(PLACEHOLDER), length_effective(PLACEHOLDER),
  //      head_loss(PLACEHOLDER), manning_lob(PLACEHOLDER),
  //      manning_main(PLACEHOLDER), manning_rob(PLACEHOLDER),
  //      manning_composite(PLACEHOLDER), k_total_areaconv(PLACEHOLDER),
  //      k_total_roughconv(PLACEHOLDER), k_total_disconv(PLACEHOLDER),
  //      alpha_areaconv(PLACEHOLDER), alpha_roughconv(PLACEHOLDER),
  //      alpha_disconv(PLACEHOLDER), nc_equalforce(PLACEHOLDER),
  //      nc_equalvelocity(PLACEHOLDER), nc_wavgwp(PLACEHOLDER),
  //      nc_wavgarea(PLACEHOLDER), nc_wavgconv(PLACEHOLDER),
  //      depth_critical(PLACEHOLDER), cp_iterations(PLACEHOLDER),
  //      k_err(PLACEHOLDER), ws_err(PLACEHOLDER), length_energyloss(PLACEHOLDER),
  //      length_effectiveadjusted(PLACEHOLDER), bed_slope(PLACEHOLDER), peak_hrs_required(PLACEHOLDER) {}

  // Constructor 0 variant
  hydraulic_output()
      : nodeID(0), reachID(0), downnodeID(0), upnodeID1(0), upnodeID2(0),
        stationname(""), station(0.0), reach_length_DS(0.0),
        reach_length_US1(0.0), reach_length_US2(0.0), flow(0.0), flow_lob(0.0),
        flow_main(0.0), flow_rob(0.0), min_elev(0.0), wsl(0.0), depth(0.0),
        hyd_depth(0.0), hyd_depth_lob(0.0), hyd_depth_main(0.0),
        hyd_depth_rob(0.0), top_width(0.0), top_width_lob(0.0),
        top_width_main(0.0), top_width_rob(0.0), velocity(0.0),
        velocity_lob(0.0), velocity_main(0.0), velocity_rob(0.0), k_total(0.0),
        k_lob(0.0), k_main(0.0), k_rob(0.0), alpha(0.0), area(0.0),
        area_lob(0.0), area_main(0.0), area_rob(0.0), hradius(0.0),
        hradius_lob(0.0), hradius_main(0.0), hradius_rob(0.0),
        wet_perimeter(0.0), wet_perimeter_lob(0.0), wet_perimeter_main(0.0),
        wet_perimeter_rob(0.0), energy_total(0.0), velocity_head(0.0),
        froude(0.0), sf(0.0), sf_avg(0.0), sbed(0.0), length_effective(0.0),
        head_loss(0.0), manning_lob(0.0), manning_main(0.0), manning_rob(0.0),
        manning_composite(0.0), k_total_areaconv(0.0), k_total_roughconv(0.0),
        k_total_disconv(0.0), alpha_areaconv(0.0), alpha_roughconv(0.0),
        alpha_disconv(0.0), nc_equalforce(0.0), nc_equalvelocity(0.0),
        nc_wavgwp(0.0), nc_wavgarea(0.0), nc_wavgconv(0.0), depth_critical(0.0),
        cp_iterations(0), k_err(0.0), ws_err(0.0), length_energyloss(0.0),
        length_effectiveadjusted(0.0), bed_slope(0.0), peak_hrs_required(0.0) {}

  // Copy Constructor
  hydraulic_output(const hydraulic_output &other) = default;
};


//*****************************************************************
//Enumerables
//*****************************************************************
// Streamnode type
enum enum_nodetype
{
  REACH,
  XSECTION
};

// Model type
enum enum_mt_method
{
  HAND_MANNING,
  STEADYFLOW
};

// Regime type
enum enum_rt_method
{
  SUBCRITICAL,
  SUPERCRITICAL,
  MIXED
};

// Friction slope method
enum enum_fs_method
{
  AVERAGE_CONVEYANCE,
  AVERAGE_FRICTION,
  GEOMETRIC_FRICTION,
  HARMONIC_FRICTION,
  REACH_FRICTION
};

// Manning composite method
enum enum_mc_method
{
  EQUAL_FORCE,
  WEIGHTED_AVERAGE_AREA,
  WEIGHTED_AVERAGE_WETPERIMETER,
  WEIGHTED_AVERAGE_CONVEYANCE,
  EQUAL_VELOCITY,
  BLENDED_NC
};

// X-section conveyance method
enum enum_xsc_method
{
  OVERBANK_CONVEYANCE,
  DEFAULT_CONVEYANCE,
  COORDINATE_CONVEYANCE,
  DISCRETIZED_CONVEYANCE_XS,
  AREAWEIGHTED_CONVEYANCE_ONECALC_XS,
  AREAWEIGHTED_CONVEYANCE
};

// Reach conveyance method
enum enum_rc_method
{
  DISCRETIZED_CONVEYANCE_R,
  AREAWEIGHTED_CONVEYANCE_ONECALC_R,
  ROUGHZONE_CONVEYANCE,
  BLENDED_CONVEYANCE
};

// Reach integraton method
enum enum_ri_method
{
  EFFECTIVE_LENGTH,
  REACH_LENGTH
};

// Post-processing interpolation method
enum enum_ppi_method
{
  NONE,
  CATCHMENT_HAND,
  CATCHMENT_DHAND,
  INTERP_HAND,
  INTERP_DHAND,
  INTERP_DHAND_WSLCORR
};

// Boundary condition type
enum enum_bc_type
{
  NORMAL_DEPTH,
  SET_WSL,
  SET_DEPTH
};

// DHand post-processing method
enum enum_dh_method
{
  INTERPOLATE,
  FLOOR
};

// DHand post-processing method
enum enum_gridded_format
{
  RASTER,
  NETCDF,
  PNG
};

//*****************************************************************
//Common Functions (inline)
//*****************************************************************

///////////////////////////////////////////////////////////////////
/// \brief Converts string parameter to integer type
/// \param *s1 [in] String to be converted to integer
/// \return Integer format of passed string
//
inline int s_to_i(const char *s1) { return (int)atof(s1); }

///////////////////////////////////////////////////////////////////
/// \brief Converts string parameter to long long integer type
/// \param *s1 [in] String to be converted to long long integer
/// \return Integer format of passed string
//
inline long long int s_to_ll(const char *s1) { return atoll(s1); }

///////////////////////////////////////////////////////////////////
/// \brief Converts string parameter to double type
/// \param *s1 [in] String to be converted to double
/// \return Double format of passed string
//
inline double s_to_d(const char *s1) { return atof(s1); }

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

///////////////////////////////////////////////////////////////////
/// \brief Conveyance calculation for valarrays
/// \return conveyance
//
inline std::valarray<double> ConvCalc(std::valarray<double> n,
                                      std::valarray<double> A,
                                      std::valarray<double> Rh) {
  std::valarray<double> res = (1. / n) * A * std::pow(Rh, 2. / 3.);
  res[res < 0] = 0;
  return res;
}

///////////////////////////////////////////////////////////////////
/// \brief Conveyance calculation for doubles
/// \return conveyance
//
inline double ConvCalc(double n, double A, double Rh) {
  double res = (1. / n) * A * std::pow(Rh, 2. / 3.);
  res = res < 0 ? 0 : res;
  return res;
}

///////////////////////////////////////////////////////////////////
/// \brief Calculation of correction term on elevation change for post-processing elevation correction
/// \return 1                   if x is 50% or less (<= tt, default 0.5)
/// \return 0 < Ct < 1          if x < tt (default 0.5)
/// \return 0                   if x >= 100% (>= 1)
//
inline double CalcCt(double x, double tt) {
  return tt == 0  || tt == PLACEHOLDER ? 0 : (1 - std::min(std::max(x - tt, 0.)/tt, 1.));
}

///////////////////////////////////////////////////////////////////
/// \brief returns a vector of all values in the f column of a vector of hydraulic outputs.
/// Example call with std::vector<hydraulic_output *> v as the vector, and collecting column wsl:
/// res = hyd_out_collect(&hydraulic_output::wsl, v)
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
/// \brief returns data type in NetCDF form given GDAL form
/// \param gdal_type [in] data type in GDAL form
/// \return returns data type in NetCDF form
//
inline nc_type ConvertGDALTypeToNetCDF(GDALDataType gdal_type) {
  switch (gdal_type) {
  case GDT_Byte: return NC_BYTE;
  case GDT_UInt16: return NC_USHORT;
  case GDT_Int16: return NC_SHORT;
  case GDT_UInt32: return NC_UINT;
  case GDT_Int32: return NC_INT;
  case GDT_Float32: return NC_FLOAT;
  case GDT_Float64: return NC_DOUBLE;
  default: return NC_NAT;
  }
}

///////////////////////////////////////////////////////////////////
/// \brief returns data type in GDAL form given NetCDF form
/// \param netcdf_type [in] data type in NetCDF form
/// \return returns data type in GDAL form
//
inline GDALDataType ConvertNetCDFTypeToGDAL(nc_type netcdf_type) {
  switch (netcdf_type) {
  case NC_BYTE: return GDT_Byte;
  case NC_CHAR: return GDT_Byte;
  case NC_SHORT: return GDT_Int16;
  case NC_USHORT: return GDT_UInt16;
  case NC_INT: return GDT_Int32;
  case NC_UINT: return GDT_UInt32;
  case NC_FLOAT: return GDT_Float32;
  case NC_DOUBLE: return GDT_Float64;
  default: return GDT_Unknown;
  }
}

///////////////////////////////////////////////////////////////////
/// \brief helper functions for printing enumerables
/// \param type [in] instance of enumerable to convert to string
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
  case AREAWEIGHTED_CONVEYANCE_ONECALC_XS: return "AREAWEIGHTED_CONVEYANCE_ONECALC_XS";
  case AREAWEIGHTED_CONVEYANCE: return "AREAWEIGHTED_CONVEYANCE";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_rc_method method) {
  switch (method) {
  case DISCRETIZED_CONVEYANCE_R: return "DISCRETIZED_CONVEYANCE_R";
  case AREAWEIGHTED_CONVEYANCE_ONECALC_R: return "AREAWEIGHTED_CONVEYANCE_ONECALC_R";
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

inline std::string toString(enum_dh_method method) {
  switch (method) {
  case INTERPOLATE: return "INTERPOLATE";
  case FLOOR: return "FLOOR";
  default: return "UNKNOWN";
  }
}

inline std::string toString(enum_gridded_format method) {
  switch (method) {
  case RASTER: return "RASTER";
  case NETCDF: return "NETCDF";
  case PNG: return "PNG";
  default: return "UNKNOWN";
  }
}

///////////////////////////////////////////////////////////////////
/// \brief converts any input to string
/// \param t [in] thing to be converted to a string
/// \return string equivalent of input t
//
template <class T> inline std::string to_string(const T &t) {
  std::stringstream ss;
  ss << t;
  return ss.str();
}

//Parsing Functions-------------------------------------------
//defined in CommonFunctions.cpp
std::string   StringToUppercase(const std::string& s);
bool          IsComment(const char* s, const int Len);
void          WriteWarning(const std::string warn, bool noisy);
void          WriteAdvisory(const std::string warn, bool noisy);
double        fast_s_to_d(const char* s);
void          SilentErrorHandler(CPLErr eErrClass, int err_no, const char *msg);

// I/O Functions-----------------------------------------------
// defined in StandardOutput.cpp
std::string GetDirectoryName(const std::string &fname);
std::string CorrectForRelativePath(const std::string filename, const std::string relfile);

#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#endif