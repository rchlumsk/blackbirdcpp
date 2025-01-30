#ifndef MODEL_H
#define MODEL_H

#include "BlackbirdInclude.h"
#include "Streamnode.h"
#include "BoundaryConditions.h"
#include "Options.h"
#include "Raster.h"

class CModel {
public:
  // Member variables
  std::vector<CStreamnode*> *bbsn;     // A vector of Streamnode objects
  CBoundaryConditions *bbbc;           // A single bb_boundarycondition object
  COptions *bbopt;                     // A single bb_options object
  std::vector<double> hand_depth_seq;  // sequence of depths for hand
  std::vector<double> dhand_depth_seq; // sequence of depths for dhand
  CRaster c_from_s;                    // raster object for catchments from streamnodes
  CRaster hand;                        // raster object for hand
  CRaster handid;                      // raster object for hand pourpoints
  std::vector<CRaster> dhand;          // raster object for dhand
  std::vector<CRaster> dhandid;        // raster object for dhand pourpoints

  // Outputs
  std::vector<hydraulic_output *> *hyd_result;  // hydraulic outputs here reference 
  std::vector<CRaster> out_rasters;             // output depth raster objects to be written to file

  // Constructor and destructor
  CModel();
  ~CModel();

  // Functions
  void hyd_compute_profile();                                     // computes hydraulic profile for all streamnodes
  void calc_output_flows();                                       // calculates flows of all streamnodes based on headwater nodes steady flows and source sinks

  void add_streamnode(CStreamnode*& pSN);                         // adds streamnode to bbsn and maps
  CStreamnode* get_streamnode_by_id(int sid);                     // returns streamnode using id map
  CStreamnode *get_streamnode_by_stationname(std::string name);   // returns streamnode using stationname map
  int get_index_by_id(int id);                                    // returns streamnode index usind id map
  int get_hyd_res_index(int flow_ind, int sid);                   // returns hyd_result index

  // I/O Functions defined in StandardOutput.cpp
  void WriteOutputFileHeaders(COptions*const& pOptions);
  void WriteMajorOutput(std::string solfile, bool final) const;
  void WriteRasterOutput();                                       // writes raster output to files
  void WriteFullModel() const;                                    // writes full model data to testoutput
  void hyd_result_pretty_print() const;                           // writes hyd_result to testoutput
  void hyd_result_pretty_print_csv() const;                       // writes hyd_result to csv file

  // Raster Functions defined in Raster.cpp
  void ReadRasterFiles();                                         // reads necessary raster files
  void ReadRasterFile(std::string filename, CRaster &raster_obj); //reads specified raster file
  void postprocess_floodresults();                                // postprocesses flood results based on bbopt method

protected:
  // Private variables
  std::unordered_map<int, int> streamnode_map;            // maps streamnode id to index
  std::unordered_map<std::string, int> stationname_map;   // maps stationname to index
  int flow;

  // Private functions
  void compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res); // helper function uses in hyd_compute_profile
};

#endif