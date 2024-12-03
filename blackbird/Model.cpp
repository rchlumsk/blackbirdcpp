#include "Model.h"

// Default constructor
CModel::CModel()
  : bbsn(), bbbc(), bbopt() {
  // Initialize member variables as needed
}

// Function to compute hydraulic profile
hydraulic_output CModel::hyd_compute_profile() {
  hydraulic_output result;
  return result;
}

// Function to postprocess flood results
bool CModel::postprocess_floodresults() {
  return true;
}

// Function to add streamnode to model
bool CModel::add_streamnode(CStreamnode*& pSN) {
  bbsn->push_back(pSN);
  streamnode_map[pSN->nodeID] = bbsn->size() - 1;
  return true;
}

// Returns streamnode with id 'sid'
CStreamnode* CModel::get_streamnode_by_id(int sid) {
  return streamnode_map.find(sid) != streamnode_map.end() ? bbsn->at(streamnode_map[sid]) : NULL;
}