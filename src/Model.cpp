#include "Model.h"

// Default constructor
CModel::CModel()
  : bbsn(new std::vector<CStreamnode*>),
  bbbc(new CBoundaryConditions),
  bbopt(new COptions),
  streamnode_map() {
  // Default constructor implementation
}

// Function to compute hydraulic profile
hydraulic_output CModel::hyd_compute_profile() {
  hydraulic_output result;
  return result;
}

// Function to postprocess flood results
void CModel::postprocess_floodresults() {
}

// Function to calculate output flows of all streamnodes
void CModel::calc_output_flows() {
  std::set<int> finished_nodes;             // sorted list of nodes where output_flows has been calculated
  std::unordered_map<int, int> id_to_ind;   // maps nodeID to index in bbsn
  int total_nodes = bbsn->size();           // number of nodes in bbsn
  int num_fp = PLACEHOLDER;                 // number of flow profiles
  for (int iter = 0; finished_nodes.size() < total_nodes && iter < 1000; iter++) {
    for (int i = 0; i < total_nodes; i++) {
      CStreamnode*& temp_sn = (*bbsn)[i]; // reference to current streamnode pointer to simplify code

      // If first while loop iteration, add node to id_to_ind
      if (iter == 1) {
        id_to_ind[temp_sn->nodeID] = i;
      }

      // If node has been calculated, skip. If num_fp has yet to be assigned, assign it (will execute on a headwater node in first iteration)
      if (finished_nodes.find(temp_sn->nodeID) != finished_nodes.end()) {
        if (num_fp == PLACEHOLDER) {
          ExitGracefullyIf(temp_sn->upstream_flows[0] != HEADWATER,
            "Model.cpp: ERROR initial finished node not HEADWATER", RUNTIME_ERR);
          num_fp = temp_sn->output_flows.size();
        }
        continue;
      }
      // If either upnodes have not been calculated already, skip
      if ( ( temp_sn->upnodeID1 != -1 && (finished_nodes.find(temp_sn->upnodeID1) == finished_nodes.end()) ) 
        || ( temp_sn->upnodeID2 != -1 && (finished_nodes.find(temp_sn->upnodeID2) == finished_nodes.end()) )) {
        continue;
      }

      // Calculate total upstream flow to send into calc_output_flows function
      std::vector<double> upflows = (*bbsn)[id_to_ind[temp_sn->upnodeID1]]->output_flows;
      if (temp_sn->upnodeID2 != -1) {
        for (int j = 0; j < num_fp; j++) {
          upflows[j] += (*bbsn)[id_to_ind[temp_sn->upnodeID2]]->output_flows[j];
        }
      }
      temp_sn->calc_output_flows(upflows);

      // add node to finished_nodes
      finished_nodes.insert(temp_sn->nodeID);
    }
  }
}

// Function to add streamnode to model and map
void CModel::add_streamnode(CStreamnode*& pSN) {
  bbsn->push_back(pSN);
  streamnode_map[pSN->nodeID] = bbsn->size() - 1;
}

// Returns streamnode with id 'sid'
CStreamnode* CModel::get_streamnode_by_id(int sid) {
  //std::cout << "bbsn size: " << bbsn->size() << std::endl;
  //std::cout << "sid: " << sid << std::endl;
  return streamnode_map.find(sid) != streamnode_map.end() ? bbsn->at(streamnode_map[sid]) : NULL;
}