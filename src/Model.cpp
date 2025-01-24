#include "Model.h"
#include "XSection.h"

// Default constructor
CModel::CModel()
  : bbsn(new std::vector<CStreamnode*>),
  bbbc(new CBoundaryConditions),
  bbopt(new COptions),
  hand_depth_seq(),
  dhand_depth_seq(),
  c_from_s(),
  hand(),
  handid(),
  dhand(),
  dhandid(),
  hyd_result(nullptr),
  out_rasters(),
  streamnode_map(),
  stationname_map(),
  flow(PLACEHOLDER) {
  // Default constructor implementation
}

// Function to compute hydraulic profile
std::vector<hydraulic_output *> *CModel::hyd_compute_profile() {
  bbopt->froude_threshold = 0.94;

  ExitGracefullyIf(bbopt->regimetype != enum_rt_method::SUBCRITICAL,
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

  std::vector<hydraulic_output *> *result = new std::vector<hydraulic_output *>(
      start_streamnode->output_flows.size() * bbsn->size());

  for (int f = 0; f < start_streamnode->output_flows.size(); f++) {
    flow = f;
    compute_streamnode(start_streamnode, start_streamnode, result);
  }
  std::cout << "Successfully completed all hydraulic calculations :-)" << std::endl;
  flow = PLACEHOLDER;
  hyd_result = result;
  return result;
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

// Returns index in hyd_result of streamnode with id 'sid' and a flow index of flow_ind
int CModel::get_hyd_res_index(int flow_ind, int sid) {
  return flow_ind * bbsn->size() + get_index_by_id(sid);
}

// Recursively computes hyd profile for the tree with downstream most streamnode "sn"
void CModel::compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res) {
  if (!bbopt->silent_cp) {
    std::cout << "Computing profile for streamnode with node id " << sn->nodeID << std::endl;
  }

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
    sn->mm->min_elev = sn->min_elev; // redundant?
    if (bbopt->modeltype == enum_mt_method::HAND_MANNING) {
      sn->compute_normal_depth(sn->mm->flow, sn->mm->bed_slope, -99, bbopt);
    } else {
      switch (bbbc->bctype)
      {
      case (enum_bc_type::NORMAL_DEPTH): {
        if (bbbc->bcvalue <= 0 || bbbc->bcvalue > 1) {
          WriteWarning("Model.cpp: compute_streamnode: bcvalue may not be a "
                       "reasonable slope, please check!",
                       bbopt->noisy_run);
        }
        sn->compute_normal_depth(sn->mm->flow, bbbc->bcvalue, bbbc->init_WSL, bbopt);
        sn->mm->sf = bbbc->bcvalue;
        break;
      }
      case (enum_bc_type::SET_WSL): {
        sn->mm->wsl = bbbc->bcvalue;
        ExitGracefullyIf(sn->mm->wsl < sn->mm->min_elev,
                         "Model.cpp: compute_streamnode: SET_WSL used as "
                         "boundary condition but value provided is less than "
                         "minimum elevation, revise boundary condition!",
                         BAD_DATA);
        break;
      }
      case (enum_bc_type::SET_DEPTH): {
        sn->mm->wsl = bbbc->bcvalue + sn->mm->min_elev;
        ExitGracefullyIf(bbbc->bcvalue < 0,
                         "Model.cpp: compute_streamnode: SET_DEPTH used as "
                         "boundary condition, value must be >= 0",
                         BAD_DATA);
        break;
      }
      default: {
        ExitGracefully("Model.cpp: compute_streamnode: error in boundary "
                       "condition input type",
                       BAD_DATA);
        break;
      }
      }
    }
    sn->mm->depth = sn->mm->wsl - sn->mm->min_elev;
    sn->compute_profile(sn->mm->flow, sn->mm->wsl, bbopt);
  } else {
    if (bbopt->modeltype == enum_mt_method::HAND_MANNING) {
      sn->mm->min_elev = sn->min_elev;
      sn->compute_normal_depth(sn->mm->flow, sn->mm->bed_slope, -99, bbopt);
      sn->compute_profile(sn->mm->flow, sn->mm->wsl, bbopt);
    } else {
      sn->mm->min_elev = sn->min_elev;
      sn->mm->wsl = down_sn->mm->depth + sn->mm->min_elev;
      sn->mm->depth = down_sn->mm->depth;

      double err_lag1 = PLACEHOLDER, err_lag2 = PLACEHOLDER,
             prevWSL_lag1 = PLACEHOLDER, prevWSL_lag2 = PLACEHOLDER,
             min_err_wsl = PLACEHOLDER, min_err = PLACEHOLDER,
             actual_err = PLACEHOLDER, min_fr = PLACEHOLDER;
      bool found_supercritical = false;
      for (int i = 0; i < bbopt->iteration_limit_cp; i++) {
        prevWSL_lag2 = prevWSL_lag1;
        prevWSL_lag1 = sn->mm->wsl;
        sn->compute_profile_next(sn->mm->flow, sn->mm->wsl, down_sn->mm, bbopt);
        double max_depth_change = 0.5 * sn->mm->depth;
        double comp_wsl = down_sn->mm->wsl + down_sn->mm->velocity_head + sn->mm->head_loss - sn->mm->velocity_head;

        if (comp_wsl <= sn->mm->min_elev) {
          comp_wsl = sn->mm->flow == 0
                  ? sn->mm->min_elev
                  : std::max(comp_wsl,
                             sn->mm->min_elev + 0.05 +
                                 0.05 * (1 - (i + 1) / bbopt->iteration_limit_cp));
        }

        err_lag2 = err_lag1;
        err_lag1 = comp_wsl - prevWSL_lag1;
        double err_diff = err_lag2 == PLACEHOLDER ? PLACEHOLDER : err_lag2 - err_lag1;
        double assum_diff = prevWSL_lag2 == PLACEHOLDER ? PLACEHOLDER : prevWSL_lag2 - prevWSL_lag1;

        if (min_err == PLACEHOLDER || std::abs(err_lag1) < min_err) {
          min_err = std::abs(err_lag1);
          actual_err = err_lag1;
          min_err_wsl = prevWSL_lag1;
          min_fr = sn->mm->froude;
        }

        sn->mm->ws_err = err_lag1;
        sn->mm->k_err = sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);
        sn->mm->cp_iterations = i + 1;

        if (std::abs(err_lag1) > bbopt->tolerance_cp) {
          if (i + 1 == bbopt->iteration_limit_cp) {
            WriteWarning("compute_profile(): iteration limit reached on streamnode" + sn->nodeID, bbopt->noisy_run);

            sn->compute_profile_next(sn->mm->flow, min_err_wsl, down_sn->mm, bbopt);
            sn->mm->ws_err = actual_err;
            sn->mm->k_err = sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);

            if (min_err < 0.03 && sn->mm->froude <= bbopt->froude_threshold) {
              std::cout << "setting to min error result on streamnode " << sn->nodeID << std::endl;
            } else {
              // optimization placeholder
              double depth_critical = std::max(0.5, down_sn->mm->depth);

              if (true) { // if optimization worked
                sn->mm->depth_critical = depth_critical;
                sn->compute_profile_next(sn->mm->flow, sn->mm->min_elev + sn->mm->depth_critical, down_sn->mm, bbopt);
                sn->mm->ws_err = PLACEHOLDER;
                sn->mm->k_err = sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);
                std::cout << "setting to critical depth result on streamnode " << sn->nodeID << std::endl;
              } else {
                std::cout << "Failed to get critical depth at streamnode "
                          << sn->nodeID << ", keeping lowest err result" << std::endl;
                break;
              }
            }
            break;
          }
          if (i == 0) {
            double proposed_wsl = sn->mm->wsl + 0.7 * err_lag1;
            if (std::abs(proposed_wsl - sn->mm->wsl) > max_depth_change) {
              sn->mm->wsl = sn->mm->wsl + std::copysign(max_depth_change, proposed_wsl - sn->mm->wsl);
            } else {
              sn->mm->wsl = proposed_wsl;
            }
          } else {
            if (std::abs(err_diff) < 0.03 || i + 1 > bbopt->iteration_limit_cp / 2 || i >= 20) {
              double proposed_wsl = (comp_wsl + prevWSL_lag1) / 2;
              if (std::abs(comp_wsl - prevWSL_lag1) > max_depth_change) {
                proposed_wsl = prevWSL_lag1 + std::copysign(0.5*max_depth_change, comp_wsl - prevWSL_lag1);
              }
              sn->mm->wsl = proposed_wsl;
            } else {
              double proposed_wsl = prevWSL_lag2 - err_lag2 * assum_diff / err_diff;
              if (std::abs(proposed_wsl - sn->mm->wsl) > max_depth_change) {
                sn->mm->wsl = sn->mm->wsl + std::copysign(max_depth_change, proposed_wsl - sn->mm->wsl);
              } else {
                sn->mm->wsl = proposed_wsl;
              }
            }
          }
        } else {
          if (sn->mm->froude <= bbopt->froude_threshold) {
            if (!bbopt->silent_cp) {
              std::cout << "Iterated on WSL at streamnode " << sn->nodeID
                        << " within tolerance on iteration " << i << std::endl;
            }
            break;
          } else {
            // optimization placeholder
            double depth_critical = std::max(0.5, down_sn->mm->depth);

            if (true) { // if optimization worked
              sn->mm->depth_critical = depth_critical;
              if (sn->mm->depth < sn->mm->depth_critical) {
                if (found_supercritical) {
                  sn->compute_profile_next(sn->mm->flow, sn->mm->min_elev + sn->mm->depth_critical, down_sn->mm, bbopt);
                  break;
                } else {
                  sn->compute_profile_next(sn->mm->flow, sn->mm->min_elev + sn->mm->depth_critical + 1, down_sn->mm, bbopt);
                  i = 0;
                  min_err = PLACEHOLDER;
                }
                found_supercritical = true;
              } else {
                if (sn->mm->froude > 1) {
                  WriteWarning("Depth found to not be supercritical even though Froude is >1 at streamnode " + sn->nodeID, bbopt->noisy_run);
                }
                break;
              }
            } else {
              WriteWarning(
                  "Failed to converge in critical depth calculation for streamnode " + std::to_string(sn->nodeID) + ". Leaving depth as is, which may be supercritical",
                  bbopt->noisy_run);
              break;
            }
          }
        }
      }
    }
  }
  if (sn->nodetype == enum_nodetype::REACH) {
    if (!sn->depthdf->empty() && sn->mm->depth > sn->depthdf->back()->depth) {
      WriteWarning("Model.cpp: compute_streamnode: Results for flow of " +
                       std::to_string(flow) + " at streamnode " +
                       std::to_string(sn->nodeID) + " estimated a depth " +
                       std::to_string(sn->mm->depth) +
                       " beyond those provided for pre-processing;\nResults "
                       "should be re-run with a broader set of depths",
                   bbopt->noisy_run);
    }
  } else {
    if (sn->mm->wsl > ((CXSection*)sn)->zz.max() ) {
      WriteWarning("Model.cpp: compute_streamnode: Results for flow of " +
                       std::to_string(flow) + " at streamnode " +
                       std::to_string(sn->nodeID) +
                       " estimated a water surface level " +
                       std::to_string(sn->mm->wsl) +
                       " exceeding the zz value;\nResults should be re-run "
                       "with extended cross sections",
                   bbopt->noisy_run);
    }
  }

  (*res)[flow * bbsn->size() + ind] = new hydraulic_output(*(sn->mm));

  if (sn->mm->alpha != PLACEHOLDER && sn->mm->alpha > 5) {
    WriteWarning("alpha value greater than 5 found, indicating possible instability in results. Please review.", bbopt->noisy_run);
  }
  if (sn->mm->velocity != PLACEHOLDER && sn->mm->velocity > 50) {
    WriteWarning("velocity value greater than 50 m/s found, indicating possible instability in results. Please review.", bbopt->noisy_run);
  }
  if (sn->mm->sf_avg != PLACEHOLDER && sn->mm->sf_avg > 1) {
    WriteWarning("Average friction slope value greater than 1 found, indicating possible instability in results. Please review.", bbopt->noisy_run);
  }
  if (sn->mm->cp_iterations != PLACEHOLDER && sn->mm->cp_iterations > bbopt->iteration_limit_cp) {
    WriteWarning("Iteration limit hit at streamnode " + std::to_string(sn->nodeID) + ", consider increasing bbopt->iteration_limit_cp", bbopt->noisy_run);
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

// Destructor
CModel::~CModel() {
  for (std::vector<CStreamnode *>::iterator i = bbsn->begin(); i != bbsn->end(); i++) {
    delete (*i);
    *i = nullptr;
  }
  bbsn->clear();
  bbsn->shrink_to_fit();
  delete bbsn;
  bbsn = nullptr;
  delete bbbc;
  bbbc = nullptr;
  delete bbopt;
  bbopt = nullptr;
  for (std::vector<hydraulic_output*>::iterator i = hyd_result->begin(); i != hyd_result->end(); i++) {
    delete (*i);
    *i = nullptr;
  }
  delete hyd_result;
  hyd_result = nullptr;
}