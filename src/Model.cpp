#include "Model.h"

// Default constructor
CModel::CModel()
  : bbsn(new std::vector<CStreamnode*>),
  bbbc(new CBoundaryConditions),
  bbopt(new COptions),
  streamnode_map(),
  stationname_map(),
  flow(PLACEHOLDER) {
  // Default constructor implementation
}

// Function to compute hydraulic profile
hydraulic_output CModel::hyd_compute_profile() {
  double froude_threshold = 0.94;

  ExitGracefullyIf(std::to_string(bbopt->regimetype) != "subcritical",
                   "Model.cpp: hyd_compute_profile(): only subcritical mode "
                   "with hardcoded options currently available",
                   BAD_DATA);

  ExitGracefullyIf(bbopt->dx <= 0,
                   "Model.cpp: hyd_compute_profile(): dx must be greater than 0",
                   BAD_DATA);

  CStreamnode *start_streamnode = get_streamnode_by_stationname(bbbc->stationname);

  ExitGracefullyIf(!start_streamnode,
                   "Model.cpp: hyd_compute_profile(): boundary condition "
                   "station name not represented in streamnodes",
                   BAD_DATA);

  // update preproc tables?

  std::vector<hydraulic_output *> *result = new std::vector<hydraulic_output *>(
      start_streamnode->output_flows.size() * bbsn->size());

  for (int f = 0; f < start_streamnode->output_flows.size(); f++) {
    //std::vector<hydraulic_output*> *temp_result = new std::vector<hydraulic_output*>(bbsn->size());
    flow = f;
    //int num_computed = 0;
    //while (num_computed != bbsn->size()) {
    //  for (int s = 0; s < bbsn->size(); s++) {
    //    if ((*bbsn)[s]->output_depth[f] == PLACEHOLDER && (*bbsn)[s]->down)
    //    if (bbopt->modeltype == enum_mt_method::HAND_MANNING) {
    //
    //    }
    //  }
    //  
    //}
    // 
    // change below ifs to use enums
    if (bbopt->modeltype != enum_mt_method::HAND_MANNING) {
      switch (bbbc->bctype)
      {
      case (enum_bc_type::NORMAL_DEPTH): {
        if (bbbc->bcvalue <= 0 || bbbc->bcvalue > 1) {
          WriteWarning("Model.cpp: compute_streamnode: bcvalue may not be a "
                       "reasonable slope, please check!",
                       bbopt->noisy_run);
        }
      }
      case (enum_bc_type::SET_WSL): {
        // review, finish, and revise these
      }
      case (enum_bc_type::SET_DEPTH): {
        // review, finish, and revise these
      }
      default: {
        ExitGracefully("Model.cpp: compute_streamnode: error in boundary "
                       "condition input type",
                       BAD_DATA);
      }
      }
    }
    compute_streamnode(start_streamnode, start_streamnode, result);
  }
  flow = PLACEHOLDER;
  return *(*result)[0];
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
  stationname_map[pSN->stationname] = bbsn->size() - 1;
}

// Returns streamnode with id 'sid'
CStreamnode* CModel::get_streamnode_by_id(int sid) {
  //std::cout << "bbsn size: " << bbsn->size() << std::endl;
  //std::cout << "sid: " << sid << std::endl;
  return streamnode_map.find(sid) != streamnode_map.end() ? bbsn->at(streamnode_map[sid]) : NULL;
}

// Returns streamnode with stationname 'name'
CStreamnode *CModel::get_streamnode_by_stationname(std::string name) {
  // std::cout << "bbsn size: " << bbsn->size() << std::endl;
  // std::cout << "stationname: " << name << std::endl;
  return stationname_map.find(name) != stationname_map.end()
             ? bbsn->at(stationname_map[name])
             : NULL;
}

// Returns index of streamnode with id 'sid'
int CModel::get_index_by_id(int sid) {
  // std::cout << "bbsn size: " << bbsn->size() << std::endl;
  // std::cout << "sid: " << sid << std::endl;
  return streamnode_map.find(sid) != streamnode_map.end()
             ? streamnode_map[sid]
             : NULL;
}

// Recursively computes hyd profile for the tree with downstream most streamnode "sn"
void CModel::compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res) {
  int ind = get_index_by_id(sn->nodeID);
  sn->mm->nodeID = sn->nodeID;
  sn->mm->reachID = sn->reachID;
  sn->mm->downnodeID = sn->downnodeID;
  sn->mm->upnodeID1 = sn->upnodeID1;
  sn->mm->upnodeID2 = sn->upnodeID2;
  sn->mm->stationname = sn->stationname;
  sn->mm->station = sn->station;
  sn->mm->min_elev = sn->min_elev;
  sn->mm->reach_length_DS = sn->ds_reach_length;
  sn->mm->reach_length_US1 = sn->us_reach_length1;
  sn->mm->reach_length_US2 = sn->us_reach_length2;
  sn->mm->bed_slope = sn->bed_slope;
  sn->mm->flow = sn->output_flows[flow];

  if (sn->nodeID == down_sn->nodeID) {
    sn->mm->min_elev = sn->min_elev;
    if (bbopt->modeltype == enum_mt_method::HAND_MANNING) {
      sn->mm->wsl = (sn->compute_normal_depth()).wsl; // function needs definition. consider return type
    } else {
      switch (bbbc->bctype)
      {
      case (enum_bc_type::NORMAL_DEPTH): {
        sn->mm->wsl = (sn->compute_normal_depth()).wsl; // function needs definition. consider return type
        sn->mm->sf = bbbc->bcvalue;
      }
      case (enum_bc_type::SET_WSL): {
        sn->mm->wsl = bbbc->bcvalue;
        ExitGracefullyIf(sn->mm->wsl < sn->mm->min_elev,
                         "Model.cpp: compute_streamnode: SET_WSL used as "
                         "boundary condition but value provided is less than "
                         "minimum elevation, revise boundary condition!",
                         BAD_DATA);
      }
      case (enum_bc_type::SET_DEPTH): {
        sn->mm->wsl = bbbc->bcvalue + sn->mm->min_elev;
        ExitGracefullyIf(bbbc->bcvalue < 0,
                         "Model.cpp: compute_streamnode: SET_DEPTH used as "
                         "boundary condition, value must be >= 0",
                         BAD_DATA);
      }
      default: {
        ExitGracefully("Model.cpp: compute_streamnode: error in boundary "
                       "condition input type",
                       BAD_DATA);
      }
      }
    }
    sn->mm->depth = sn->mm->wsl - sn->mm->min_elev;
    //compute profile, etc.
  } else {
    if (bbopt->modeltype == enum_mt_method::HAND_MANNING) {
      sn->compute_normal_depth();
      sn->compute_profile();
    } else {
      sn->min_elev = sn->min_elev;
      for (int i = 0; i < sn->depthdf->size(); i++) {
        (*sn->depthdf)[i]->min_elev = sn->min_elev; // redundant?
        (*sn->depthdf)[i]->wsl =
            (*down_sn->depthdf)[i]->wsl + (*sn->depthdf)[i]->min_elev;
      }
    }
    // do stuff
  }

  CStreamnode *temp_sn = get_streamnode_by_id(sn->upnodeID1);
  if (temp_sn) {
    compute_streamnode(temp_sn, sn, res);
  }
  temp_sn = get_streamnode_by_id(sn->upnodeID2);
  if (temp_sn) {
    compute_streamnode(temp_sn, sn, res);
  }
}