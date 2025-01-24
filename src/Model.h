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
  std::vector<hydraulic_output *> *hyd_compute_profile();

  void calc_output_flows(); // calculates flows of all streamnodes based on headwater nodes steady flows and source sinks

  void add_streamnode(CStreamnode*& pSN);
  CStreamnode* get_streamnode_by_id(int sid);
  CStreamnode *get_streamnode_by_stationname(std::string name);
  int get_index_by_id(int id);
  int get_hyd_res_index(int flow_ind, int sid);

  // I/O Functions defined in StandardOutput.cpp
  void WriteOutputFileHeaders(COptions*const& pOptions);
  void WriteMajorOutput(std::string solfile, bool final) const;
  void WriteRasterOutput();
  void WriteTestOutput() const;
  void hyd_result_pretty_print() const;
  void hyd_result_pretty_print_csv() const;

  // Raster Functions defined in Raster.cpp
  void ReadRasterFiles();
  void ReadRasterFile(std::string filename, CRaster &raster_obj);
  void postprocess_floodresults();

protected:
  // Private variables
  std::unordered_map<int, int> streamnode_map;            // maps streamnode id to index
  std::unordered_map<std::string, int> stationname_map;   // maps stationname to index
  int flow;

  // Private functions
  void compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res);
};

#endif