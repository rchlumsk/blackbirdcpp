#ifndef MODEL_H
#define MODEL_H

#include "BlackbirdInclude.h"
#include "Streamnode.h"
#include "BoundaryConditions.h"
#include "Options.h"
#include "GriddedData.h"
#include "Raster.h"
#include "NetCDFLayer.h"
#include "Vector.h"
#include "XSection.h"
#include "Reach.h"

class CModel {
public:
  // Member variables
  std::vector<CStreamnode*> *bbsn;                      // A vector of Streamnode objects
  CBoundaryConditions *bbbc;                            // A single bb_boundarycondition object
  COptions *bbopt;                                      // A single bb_options object
  std::vector<double> dhand_depth_seq;                  // sequence of depths for dhand
  std::unique_ptr<CGriddedData> c_from_s;               // pointer to GriddedData object for catchments from streamnodes
  CVector spp;                                          // vector object for snapped pourpoints
  std::unique_ptr<CGriddedData> hand;                   // pointer to GriddedData object for hand
  std::unique_ptr<CGriddedData> handid;                 // pointer to GriddedData object for hand pourpoints
  std::vector<std::unique_ptr<CGriddedData>> dhand;     // vector of pointers to GriddedData objects for dhand
  std::vector<std::unique_ptr<CGriddedData>> dhandid;   // vector of pointers to GriddedData objects for dhand pourpoints
  std::vector<std::string> fp_names;                    // names of flowprofiles read in from .bbb

  // Outputs
  std::vector<hydraulic_output *> *hyd_result;                        // hydraulic outputs generated from hyd_compute_profile
  std::vector<std::unique_ptr<CGriddedData>> out_gridded;             // output depth GriddedData objects to be written to file

  // Constructors and destructor
  CModel();
  CModel(const CModel &other);
  ~CModel();

  // Copy assignment operator
  CModel &operator=(const CModel &other);

  // Functions
  void hyd_compute_profile();                                     // computes hydraulic profile for all streamnodes
  void calc_output_flows();                                       // calculates flows of all streamnodes based on headwater nodes steady flows and source sinks

  void add_streamnode(CStreamnode*& pSN);                         // adds streamnode to bbsn and maps
  CStreamnode* get_streamnode_by_id(int sid);                     // returns streamnode using id map
  int get_index_by_id(int id);                                    // returns streamnode index usind id map
  int get_hyd_res_index(int flow_ind, int sid);                   // returns hyd_result index

  // I/O Functions defined in StandardOutput.cpp
  std::string FilenamePrepare(std::string filebase) const;        // attaches main_output_dir folder and run_name to filebase
  void WriteOutputFileHeaders(COptions*const& pOptions);
  void WriteMajorOutput(std::string solfile, bool final) const;
  void WriteGriddedOutput();                                      // writes gridded output to files
  void WriteFullModel() const;                                    // writes full model data to testoutput
  void hyd_result_pretty_print() const;                           // writes hyd_result to testoutput
  void hyd_result_pretty_print_csv() const;                       // writes hyd_result to csv file

  // GIS Functions
  void ReadGISFiles();                                                                                      // reads necessary gis files
  void ReadNetCDFFile(std::string filename);                                                                // reads specified netcdf file
  void ReadNetCDFLayer(CNetCDFLayer *netcdf_obj, int ncid, const std::string &var_name,                          // reads specified netcdf layer
                       int xsize, int ysize, std::vector<double> x_coords,
                       std::vector<double> y_coords, std::string epsg);
  void ReadRasterFile(std::string filename, CRaster *raster_obj);                                           // reads specified raster file
  void ReadVectorFile(std::string filename, CVector &vector_obj);                                           // reads specified vector file
  void postprocess_floodresults();                                                                          // postprocesses flood results based on bbopt method

protected:
  // Private variables
  std::unordered_map<int, int> streamnode_map;            // maps streamnode id to index
  int flow;                                               // index for flow being used in hyd_compute_profile
  double peak_hrs_min;                                    // minimum hydraulic output peak hours required for all streamnodes in hyd_compute_profile
  double peak_hrs_max;                                    // maximum hydraulic output peak hours required for all streamnodes in hyd_compute_profile
  std::vector<double> spp_depths;                         // depths of each spp for a specific flow profile. used in postprocess_floodresults if bbopt->interpolation_postproc_method is an interp method
  std::vector<double> dhand_vals;                         // hand values interpolated from dhand rasters for specific flow profile. used in postprocess_floodresults if bbopt->interpolation_postproc_method is a dhand method
  std::vector<int> dhandid_vals;                          // handids corresponding to dhand_vals for specific flow profile. used in postprocess_floodresults if bbopt->interpolation_postproc_method is a dhand method and interp method

  // Private functions
  void compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res); // helper function used in hyd_compute_profile
  std::pair<int, int> dhand_bounding_depths(double depth);                                                 // finds nearest dhands to use in postprocess_floodresults
  void generate_spp_depths(int flow_ind);                                                                  // generates spp_depths for the flow_ind-th profile. used in postprocess_floodresults
  void generate_dhand_vals(int flow_ind, bool is_interp);                                                  // generates dhand_vals for the flow_ind-th profile. used in postprocess_floodresults
  void generate_out_gridded(int flow_ind, bool is_interp, bool is_dhand);                                  // generates an output gridded for the flow_ind-th profile. used in postprocess_floodresults
  void initialize_out_gridded(bool is_dhand);                                                              // initializes an output gridded data instance for the flow ind-th profile. used in generate_out_gridded
};

#endif