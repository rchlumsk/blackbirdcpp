#ifndef MODEL_H
#define MODEL_H

#include "BlackbirdInclude.h"
#include "Streamnode.h"
#include "BoundaryConditions.h"
#include "Options.h"

class CModel {
public:
  // Member variables
  std::vector<CStreamnode*> *bbsn;  // A vector of Streamnode objects
  CBoundaryConditions *bbbc;      // A single bb_boundarycondition object
  COptions *bbopt;               // A single bb_options object

  // Constructor
  CModel();

  // Functions
  hydraulic_output hyd_compute_profile();
  void postprocess_floodresults();

  void calc_output_flows(); // calculates flows of all streamnodes based on headwater nodes steady flows and source sinks

  void add_streamnode(CStreamnode*& pSN);
  CStreamnode* get_streamnode_by_id(int sid);

  // I/O Functions defined in StandardOutput.cpp
  void WriteOutputFileHeaders(COptions*const& pOptions);
  void WriteMajorOutput(std::string solfile, bool final) const;
  void WriteTestOutput() const;

protected:
  // Private variables
  std::unordered_map<int, int> streamnode_map;
};

#endif