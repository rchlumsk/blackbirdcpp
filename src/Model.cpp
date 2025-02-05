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

// Copy constructor
CModel::CModel(const CModel &other)
    : hand_depth_seq(other.hand_depth_seq),
      dhand_depth_seq(other.dhand_depth_seq), c_from_s(other.c_from_s),
      hand(other.hand), handid(other.handid), dhand(other.dhand),
      dhandid(other.dhandid), out_rasters(other.out_rasters),
      streamnode_map(other.streamnode_map),
      stationname_map(other.stationname_map), flow(other.flow) {
  if (other.bbsn) {
    bbsn = new std::vector<CStreamnode *>();
    for (const auto &node : *other.bbsn) {
      bbsn->push_back(new CStreamnode(*node));
    }
  } else {
    bbsn = nullptr;
  }

  bbbc = other.bbbc ? new CBoundaryConditions(*other.bbbc) : nullptr;
  bbopt = other.bbopt ? new COptions(*other.bbopt) : nullptr;

  if (other.hyd_result) {
    hyd_result = new std::vector<hydraulic_output *>();
    for (const auto &res : *other.hyd_result) {
      hyd_result->push_back(
          new hydraulic_output(*res));
    }
  } else {
    hyd_result = nullptr;
  }
}

// Copy assignment operator
CModel &CModel::operator=(const CModel &other) {
  if (this == &other) {
    return *this; // Handle self-assignment
  }

  if (bbsn) {
    for (auto ptr : *bbsn) {
      delete ptr;
    }
    delete bbsn;
    bbsn = nullptr;
  }
  delete bbbc;
  bbbc = nullptr;
  delete bbopt;
  bbopt = nullptr;
  if (hyd_result) {
    for (auto ptr : *hyd_result) {
      delete ptr;
    }
    delete hyd_result;
    hyd_result = nullptr;
  }

  hand_depth_seq = other.hand_depth_seq;
  dhand_depth_seq = other.dhand_depth_seq;
  c_from_s = other.c_from_s;
  hand = other.hand;
  handid = other.handid;
  dhand = other.dhand;
  dhandid = other.dhandid;
  out_rasters = other.out_rasters;
  streamnode_map = other.streamnode_map;
  stationname_map = other.stationname_map;
  flow = other.flow;

  if (other.bbsn) {
    bbsn = new std::vector<CStreamnode *>();
    for (const auto &node : *other.bbsn) {
      bbsn->push_back(new CStreamnode(*node));
    }
  } else {
    bbsn = nullptr;
  }

  bbbc = other.bbbc ? new CBoundaryConditions(*other.bbbc) : nullptr;
  bbopt = other.bbopt ? new COptions(*other.bbopt) : nullptr;

  if (other.hyd_result) {
    hyd_result = new std::vector<hydraulic_output *>();
    for (const auto &res : *other.hyd_result) {
      hyd_result->push_back(
          new hydraulic_output(*res));
    }
  } else {
    hyd_result = nullptr;
  }

  return *this;
}

//////////////////////////////////////////////////////////////////
/// \brief Computes the hydraulic profile for the model
//
void CModel::hyd_compute_profile() {
  bbopt->froude_threshold = 0.94; // hardcoded value

  ExitGracefullyIf(bbopt->regimetype != enum_rt_method::SUBCRITICAL,
                   "Model.cpp: hyd_compute_profile(): only subcritical mode "
                   "with hardcoded options currently available",
                   BAD_DATA);

  ExitGracefullyIf(bbopt->dx <= 0,
                   "Model.cpp: hyd_compute_profile(): dx must be greater than 0",
                   BAD_DATA);

  // Get streamnode associated with the boundary condition
  CStreamnode *start_streamnode = get_streamnode_by_stationname(bbbc->stationname);

  ExitGracefullyIf(!start_streamnode,
                   "Model.cpp: hyd_compute_profile(): boundary condition "
                   "station name not represented in streamnodes",
                   BAD_DATA);

  // Initialize container for hydraulic result
  hyd_result = new std::vector<hydraulic_output *>(
      start_streamnode->output_flows.size() * bbsn->size());

  // Loop through each flow
  // Compute the corresponding hydraulic profile starting at the boundary condition streamnode
  for (int f = 0; f < start_streamnode->output_flows.size(); f++) {
    flow = f;
    compute_streamnode(start_streamnode, start_streamnode, hyd_result);
  }
  if (!bbopt->silent_cp) {
    std::cout << "Successfully completed all hydraulic calculations :-)" << std::endl;
  }
  flow = PLACEHOLDER;
  return;
}

//////////////////////////////////////////////////////////////////
/// \brief Calculate output flows of all streamnodes
//
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

//////////////////////////////////////////////////////////////////
/// \brief Add streamnode to model and map
//
void CModel::add_streamnode(CStreamnode*& pSN) {
  bbsn->push_back(pSN);
  streamnode_map[pSN->nodeID] = bbsn->size() - 1;
  stationname_map[pSN->stationname] = bbsn->size() - 1;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns streamnode with id 'sid'
/// \return streamnode with id 'sid'
//
CStreamnode* CModel::get_streamnode_by_id(int sid) {
  //std::cout << "bbsn size: " << bbsn->size() << std::endl;
  //std::cout << "sid: " << sid << std::endl;
  return streamnode_map.find(sid) != streamnode_map.end() ? bbsn->at(streamnode_map[sid]) : NULL;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns streamnode with stationname 'name'
/// \return streamnode with stationname 'name'
//
CStreamnode *CModel::get_streamnode_by_stationname(std::string name) {
  // std::cout << "bbsn size: " << bbsn->size() << std::endl;
  // std::cout << "stationname: " << name << std::endl;
  return stationname_map.find(name) != stationname_map.end()
             ? bbsn->at(stationname_map[name])
             : NULL;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns index of streamnode with id 'sid'
/// \return index of streamnode with id 'sid'
//
int CModel::get_index_by_id(int sid) {
  // std::cout << "bbsn size: " << bbsn->size() << std::endl;
  // std::cout << "sid: " << sid << std::endl;
  return streamnode_map.find(sid) != streamnode_map.end()
             ? streamnode_map[sid]
             : NULL;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns index in hyd_result of streamnode with id 'sid' and a flow index of flow_ind
/// \return index in hyd_result of streamnode with id 'sid' and a flow index of flow_ind
//
int CModel::get_hyd_res_index(int flow_ind, int sid) {
  return flow_ind * bbsn->size() + get_index_by_id(sid);
}

//////////////////////////////////////////////////////////////////
/// \brief Recursively computes hyd profile for the tree with downstream most streamnode "sn"
/// \note if sn and down_sn are identical, it is assumed they are the boundary condition streamnode
/// \param sn [in] streamnode for which to compute hydraulic profile
/// \param down_sn [in] streamnode one node downstream of sn
/// \param res [in/out] object to add output to when done computing
//
void CModel::compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res) {
  if (!bbopt->silent_cp) {
    std::cout << "Computing profile for streamnode with node id " << sn->nodeID << std::endl;
  }

  // Initialize values
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

  if (sn->nodeID == down_sn->nodeID) { // if so, assumes this is the boundary condition streamnode
    if (bbopt->modeltype == enum_mt_method::HAND_MANNING) {
      sn->mm->wsl = sn->compute_normal_depth(sn->mm->flow, sn->mm->bed_slope, -99, bbopt);
    } else { // bbopt->modeltype != enum_mt_method::HAND_MANNING
      // estimate first streamnode from supplied boundary conditions
      switch (bbbc->bctype)
      {
      case (enum_bc_type::NORMAL_DEPTH): {
        if (bbbc->bcvalue <= 0 || bbbc->bcvalue > 1) {
          WriteWarning("Model.cpp: compute_streamnode: bcvalue may not be a "
                       "reasonable slope, please check!",
                       bbopt->noisy_run);
        }
        sn->mm->wsl = sn->compute_normal_depth(sn->mm->flow, bbbc->bcvalue, bbbc->init_WSL, bbopt);
        sn->mm->sf = bbbc->bcvalue; // override sf from preproc
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
  } else { // not boundary condition node
    if (bbopt->modeltype == enum_mt_method::HAND_MANNING) {
      sn->mm->wsl = sn->compute_normal_depth(sn->mm->flow, sn->mm->bed_slope, -99, bbopt);
      sn->compute_profile(sn->mm->flow, sn->mm->wsl, bbopt);
    } else { // bbopt->modeltype != enum_mt_method::HAND_MANNING
      sn->mm->wsl = down_sn->mm->depth + sn->mm->min_elev; // best guess at upstream WSL, same depth applied to bottom bed elevation
      sn->mm->depth = down_sn->mm->depth;

      double err_lag1 = PLACEHOLDER, err_lag2 = PLACEHOLDER,
             prevWSL_lag1 = PLACEHOLDER, prevWSL_lag2 = PLACEHOLDER,
             min_err_wsl = PLACEHOLDER, min_err = PLACEHOLDER,
             actual_err = PLACEHOLDER, min_fr = PLACEHOLDER;
      bool found_supercritical = false; // reset before every i iteration, use in iteration as needed
      for (int i = 0; i < bbopt->iteration_limit_cp; i++) {
        prevWSL_lag2 = prevWSL_lag1;
        prevWSL_lag1 = sn->mm->wsl;
        sn->compute_profile_next(sn->mm->flow, sn->mm->wsl, down_sn->mm, bbopt);
        double max_depth_change = 0.5 * sn->mm->depth;
        double comp_wsl = down_sn->mm->wsl + down_sn->mm->velocity_head + sn->mm->head_loss - sn->mm->velocity_head;

        // checks/modifications against min_elev
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

            // setting to min error result
            sn->compute_profile_next(sn->mm->flow, min_err_wsl, down_sn->mm, bbopt);
            sn->mm->ws_err = actual_err;
            sn->mm->k_err = sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);

            if (min_err < 0.03 && sn->mm->froude <= bbopt->froude_threshold) {
              // keeping the min err result
              if (!bbopt->silent_cp) {
                std::cout << "setting to min error result on streamnode " << sn->nodeID << std::endl;
              }
            } else {
              // optimization placeholder
              double depth_critical = std::max(0.5, down_sn->mm->depth);

              if (true) { // if optimization worked
                sn->mm->depth_critical = depth_critical;
                sn->compute_profile_next(sn->mm->flow, sn->mm->min_elev + sn->mm->depth_critical, down_sn->mm, bbopt);
                sn->mm->ws_err = PLACEHOLDER;
                sn->mm->k_err = sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);
                if (!bbopt->silent_cp) {
                  std::cout << "setting to critical depth result on streamnode " << sn->nodeID << std::endl;
                }
              } else {
                if (!bbopt->silent_cp) {
                  std::cout << "Failed to get critical depth at streamnode "
                            << sn->nodeID << ", keeping lowest err result"
                            << std::endl;
                }
                break;
              }
            }
            break;
          }
          if (i == 0) {
            // setting based on second trial rules
            double proposed_wsl = sn->mm->wsl + 0.7 * err_lag1;
            if (std::abs(proposed_wsl - sn->mm->wsl) > max_depth_change) {
              sn->mm->wsl = sn->mm->wsl + std::copysign(max_depth_change, proposed_wsl - sn->mm->wsl);
            } else {
              sn->mm->wsl = proposed_wsl;
            }
          } else {
            if (std::abs(err_diff) < 0.03 || i + 1 > bbopt->iteration_limit_cp / 2 || i >= 20) {
              // for small error differences, secant method can fail
              // take average of prevWSL_lag1 and prevWSL_lag2
              double proposed_wsl = (comp_wsl + prevWSL_lag1) / 2;
              // change by half of max_depth_change in direction of comp_WSL if massive swing
              if (std::abs(comp_wsl - prevWSL_lag1) > max_depth_change) {
                proposed_wsl = prevWSL_lag1 + std::copysign(0.5*max_depth_change, comp_wsl - prevWSL_lag1);
              }
              sn->mm->wsl = proposed_wsl;
            } else {
              // secant method
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
            if (!bbopt->silent_cp) {
              std::cout << "need to check crit depth" << std::endl;
            }
            // need to compute critical depth at streamnode
            // optimization placeholder
            double depth_critical = std::max(0.5, down_sn->mm->depth);

            if (true) { // if optimization worked
              sn->mm->depth_critical = depth_critical;
              if (sn->mm->depth < sn->mm->depth_critical) {
                if (found_supercritical) {
                  sn->compute_profile_next(sn->mm->flow, sn->mm->min_elev + sn->mm->depth_critical, down_sn->mm, bbopt);
                  if (!bbopt->silent_cp) {
                    std::cout << "setting to supercritical" << std::endl;
                  }
                  break;
                } else {
                  if (!bbopt->silent_cp) {
                    std::cout << "first time with supercritical, resetting" << std::endl;
                  }
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
  // check that results are within the Hseq bounds
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

  // assign output to its designated location
  (*res)[flow * bbsn->size() + ind] = new hydraulic_output(*(sn->mm));

  // basic checks for instabilities
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
  
  // recursively call compute_streamnode for upstream streamnodes
  CStreamnode *temp_sn = get_streamnode_by_id(sn->upnodeID1);
  if (temp_sn) {
    compute_streamnode(temp_sn, sn, res);
  }
  temp_sn = get_streamnode_by_id(sn->upnodeID2);
  if (temp_sn) {
    compute_streamnode(temp_sn, sn, res);
  }
}


//////////////////////////////////////////////////////////////////
/// \brief Reads Raster files
//
void CModel::ReadRasterFiles() {
  // mandatory setup
  GDALAllRegister();

  ReadRasterFile(bbopt->raster_folder + "/bb_catchments_fromstreamnodes.tif",
                 c_from_s);
  c_from_s.name = "Catchments from Streamnodes";
  if (!bbopt->use_dhand) {
    ReadRasterFile(bbopt->raster_folder + "/bb_hand.tif", hand);
    hand.name = "HAND";
    if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
        bbopt->interpolation_postproc_method ==
            enum_ppi_method::INTERP_DHAND_WSLCORR ||
        bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
      ReadRasterFile(bbopt->raster_folder + "/bb_hand_pourpoint_id.tif", handid);
      handid.name = "HAND ID";
    }
  } else { // use dhand
    for (auto d : dhand_depth_seq) {
      std::stringstream stream;
      stream << std::fixed << std::setprecision(4) << d;
      dhand.push_back(CRaster());
      ReadRasterFile(bbopt->raster_folder + "/bb_dhand_depth_" + stream.str() +
                         "m.tif",
                     dhand.back());
      dhand.back().name = "DHAND " + stream.str();
      if (bbopt->interpolation_postproc_method ==
              enum_ppi_method::INTERP_DHAND ||
          bbopt->interpolation_postproc_method ==
              enum_ppi_method::INTERP_DHAND_WSLCORR ||
          bbopt->interpolation_postproc_method ==
              enum_ppi_method::INTERP_HAND) {
        dhandid.push_back(CRaster());
        ReadRasterFile(bbopt->raster_folder + "/bb_dhand_pourpoint_id_depth_" +
                           stream.str() + "m.tif",
                       dhand.back());
        dhandid.back().name = "DHAND ID " + stream.str();
      }
    }
  }
  if (!bbopt->silent_run) {
    std::cout << "...raster data successfully read" << std::endl;
    std::cout << std::endl;
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Reads specified Raster file
//
void CModel::ReadRasterFile(std::string filename, CRaster &raster_obj) {
  GDALDataset *dataset =
      static_cast<GDALDataset *>(GDALOpen(filename.c_str(), GA_ReadOnly));
  raster_obj.xsize = dataset->GetRasterXSize();
  raster_obj.ysize = dataset->GetRasterYSize();

  if (dataset->GetProjectionRef() != nullptr) {
    size_t len = strlen(dataset->GetProjectionRef()) + 1;
    raster_obj.proj = new char[len];
    memcpy(raster_obj.proj, dataset->GetProjectionRef(), len);
  } else {
    raster_obj.proj = nullptr;
  }

  dataset->GetGeoTransform(raster_obj.geotrans);
  ExitGracefullyIf(
      dataset == nullptr,
      ("Raster.cpp: ReadRasterFile: couldn't open " + filename).c_str(),
      exitcode::FILE_OPEN_ERR);
  raster_obj.data = static_cast<double *>(
      CPLMalloc(sizeof(double) * raster_obj.xsize * raster_obj.ysize));
  GDALRasterBand *band = dataset->GetRasterBand(1);
  raster_obj.datatype = band->GetRasterDataType();
  raster_obj.na_val = band->GetNoDataValue();
  band->RasterIO(GF_Read, 0, 0, raster_obj.xsize, raster_obj.ysize,
                 raster_obj.data, raster_obj.xsize, raster_obj.ysize,
                 GDT_Float64, 0, 0);
  GDALClose(dataset);
}

//////////////////////////////////////////////////////////////////
/// \brief Postprocesses flood results based on method defined in bbopt settings
//
void CModel::postprocess_floodresults() {
  if (bbopt->interpolation_postproc_method == enum_ppi_method::NONE) {
    return;
  }
  if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
    // do stuff 99
  }
  ExitGracefullyIf(!c_from_s.data,
                   "Raster.cpp: postprocess_floodresults: catchments from "
                   "streamnodes missing",
                   exitcode::RUNTIME_ERR);
  ExitGracefullyIf(
      !hyd_result,
      "Raster.cpp: postprocess_floodresults: hydraulic output missing",
      exitcode::RUNTIME_ERR);
  // subset streamnodes checks?
  
  // loop for each flow_profiles
  for (int i = 0; i < bbsn->front()->output_flows.size(); i++) {
    if (!bbopt->silent_run) {
      std::cout << "post processing flood results for flow " +
                       std::to_string(i + 1)
                << std::endl;
    }
    if (bbopt->interpolation_postproc_method ==
        enum_ppi_method::CATCHMENT_HAND) { // maybe make more checks on validity
                                           // of data
      CRaster result = hand;
      result.name = "Result " + std::to_string(i + 1);
      std::fill(result.data, result.data + (result.xsize * result.ysize), 0.0);
      for (int j = 0; j < result.xsize * result.ysize; j++) {
        double curr_depth = (*hyd_result)[get_hyd_res_index(i, c_from_s.data[j])]->depth;
        if (c_from_s.data[j] != c_from_s.na_val && hand.data[j] != hand.na_val && c_from_s.data[j] && curr_depth >= hand.data[j]) {
          result.data[j] = curr_depth - hand.data[j];
        } else {
          result.data[j] = result.na_val;
        }
      }
      out_rasters.push_back(result);
    } else {
      std::cout << "not yet available" << std::endl;
    }
  }
  if (!bbopt->silent_run) {
    std::cout << "finished post processing flood results" << std::endl;
  }
}

// Destructor
CModel::~CModel() {
  if (bbsn) {
    for (auto ptr : *bbsn) {
      delete ptr;
    }
    delete bbsn;
    bbsn = nullptr;
  }
  delete bbbc;
  bbbc = nullptr;
  delete bbopt;
  bbopt = nullptr;
  if (hyd_result) {
    for (auto ptr : *hyd_result) {
      delete ptr;
    }
    delete hyd_result;
    hyd_result = nullptr;
  }
}