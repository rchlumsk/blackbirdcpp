#ifndef MODEL_H
#define MODEL_H

#include "Streamnode.h"
#include "BoundaryConditions.h"
#include "Options.h"
#include "BlackbirdInclude.h"

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
  bool postprocess_floodresults();
  bool add_streamnode(CStreamnode*& pSN);
  CStreamnode* get_streamnode_by_id(int sid);

private:
  // Private variables
  std::unordered_map<int, int> streamnode_map;
};

#endif