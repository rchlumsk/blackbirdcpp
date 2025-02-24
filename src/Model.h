#ifndef MODEL_H
#define MODEL_H

#include "BlackbirdInclude.h"
#include "Streamnode.h"
#include "BoundaryConditions.h"
#include "Options.h"
#include "Raster.h"
#include "Vector.h"
#include "XSection.h"
#include "Reach.h"

class CModel {
public:
  // Member variables
  std::vector<CStreamnode*> *bbsn;     // A vector of Streamnode objects
  CBoundaryConditions *bbbc;           // A single bb_boundarycondition object
  COptions *bbopt;                     // A single bb_options object
  std::vector<double> dhand_depth_seq; // sequence of depths for dhand
  CRaster c_from_s;                    // raster object for catchments from streamnodes
  CVector spp;                         // vector object for snapped pourpoints
  CRaster hand;                        // raster object for hand
  CRaster handid;                      // raster object for hand pourpoints
  std::vector<CRaster> dhand;          // raster object for dhand
  std::vector<CRaster> dhandid;        // raster object for dhand pourpoints

  // Outputs
  std::vector<hydraulic_output *> *hyd_result;  // hydraulic outputs here reference 
  std::vector<CRaster> out_rasters;             // output depth raster objects to be written to file

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
  void WriteRasterOutput();                                       // writes raster output to files
  void WriteFullModel() const;                                    // writes full model data to testoutput
  void hyd_result_pretty_print() const;                           // writes hyd_result to testoutput
  void hyd_result_pretty_print_csv() const;                       // writes hyd_result to csv file

  // GIS Functions
  void ReadGISFiles();                                            // reads necessary raster files
  void ReadRasterFile(std::string filename, CRaster &raster_obj); // reads specified raster file
  void ReadVectorFile(std::string filename, CVector &vector_obj); // reads specified vector file
  void postprocess_floodresults();                                // postprocesses flood results based on bbopt method

protected:
  // Private variables
  std::unordered_map<int, int> streamnode_map;            // maps streamnode id to index
  int flow;                                               // index for flow being used in hyd_compute_profile
  double peak_hrs_min;                                    // minimum hydraulic output peak hours required for all streamnodes in hyd_compute_profile
  double peak_hrs_max;                                    // maximum hydraulic output peak hours required for all streamnodes in hyd_compute_profile

  // Private functions
  void compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res); // helper function used in hyd_compute_profile
};

#endif