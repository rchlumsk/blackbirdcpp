#include "Model.h"
#include "XSection.h"
#include <unordered_set>

// Default constructor
CModel::CModel()
  : bbsn(new std::vector<CStreamnode*>),
  bbbc(new std::vector<CBoundaryCondition*>),
  bbopt(new COptions),
  dhand_depth_seq(),
  c_from_s(nullptr),
  hand(nullptr),
  handid(nullptr),
  dhand(),
  dhandid(),
  hyd_result(nullptr),
  out_gridded(),
  streamnode_map(),
  flow(PLACEHOLDER),
  peak_hrs_min(PLACEHOLDER),
  peak_hrs_max(PLACEHOLDER),
  spp_depths(),
  dhand_vals(),
  dhandid_vals(),
  flow_mult(1),
  conditional_model(false),
  structure_model(false),
  snconntbl(new std::vector<streamnodeconn*>) {
  // Default constructor implementation
}

// Copy constructor
CModel::CModel(const CModel &other)
    : streamnode_map(other.streamnode_map), flow(other.flow),
      peak_hrs_min(other.peak_hrs_min), peak_hrs_max(other.peak_hrs_max),
      spp_depths(other.spp_depths), dhand_vals(other.dhand_vals),
      dhandid_vals(other.dhandid_vals), flow_mult(other.flow_mult),
      snconntbl(other.snconntbl) {
  if (other.c_from_s) {
    c_from_s = other.c_from_s->clone();
  }
  if (other.hand) {
    hand = other.hand->clone();
  }
  if (other.handid) {
    handid = other.handid->clone();
  }
  for (const auto &layer : other.dhand) {
    dhand.push_back(layer->clone());
  }
  for (const auto &layer : other.dhandid) {
    dhandid.push_back(layer->clone());
  }
  for (const auto &layer : other.out_gridded) {
    out_gridded.push_back(layer->clone());
  }

  if (other.bbsn) {
    bbsn = new std::vector<CStreamnode *>();
    for (const auto &node : *other.bbsn) {
      bbsn->push_back(new CStreamnode(*node));
    }
  } else {
    bbsn = nullptr;
  }

  if (other.bbbc) {
    bbbc = new std::vector<CBoundaryCondition *>();
    for (const auto &node : *other.bbbc) {
      bbbc->push_back(new CBoundaryCondition(*node));
    }
  } else {
    bbbc = nullptr;
  }

  bbopt = other.bbopt ? new COptions(*other.bbopt) : nullptr;

  if (other.hyd_result) {
    hyd_result = new std::vector<hydraulic_output *>();
    for (const auto &res : *other.hyd_result) {
      hyd_result->push_back(new hydraulic_output(*res));
    }
  } else {
    hyd_result = nullptr;
  }

  if (other.snconntbl) {
    snconntbl = new std::vector<streamnodeconn *>();
    for (const auto &res : *other.snconntbl) {
      snconntbl->push_back(new streamnodeconn(*res));
    }
  } else {
    snconntbl = nullptr;
  }
}

// Copy assignment operator
CModel &CModel::operator=(const CModel &other) {
  if (this == &other) {
    return *this; // Handle self-assignment
  }

  if (other.c_from_s) {
    c_from_s = other.c_from_s->clone();
  }
  if (other.hand) {
    hand = other.hand->clone();
  }
  if (other.handid) {
    handid = other.handid->clone();
  }
  for (const auto &layer : other.dhand) {
    dhand.push_back(layer->clone());
  }
  for (const auto &layer : other.dhandid) {
    dhandid.push_back(layer->clone());
  }
  for (const auto &layer : other.out_gridded) {
    out_gridded.push_back(layer->clone());
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
  if (snconntbl) {
    for (auto ptr : *snconntbl) {
      delete ptr;
    }
    delete snconntbl;
    snconntbl = nullptr;
  }

  dhand_depth_seq = other.dhand_depth_seq;
  streamnode_map = other.streamnode_map;
  flow = other.flow;
  peak_hrs_min = other.peak_hrs_min;
  peak_hrs_max = other.peak_hrs_max;
  spp_depths = other.spp_depths;
  dhand_vals = other.dhand_vals;
  dhandid_vals = other.dhandid_vals;
  flow_mult = other.flow_mult;
  snconntbl = other.snconntbl;

  if (other.bbsn) {
    bbsn = new std::vector<CStreamnode *>();
    for (const auto &node : *other.bbsn) {
      bbsn->push_back(new CStreamnode(*node));
    }
  } else {
    bbsn = nullptr;
  }

  if (other.bbbc) {
    bbbc = new std::vector<CBoundaryCondition *>();
    for (const auto &node : *other.bbbc) {
      bbbc->push_back(new CBoundaryCondition(*node));
    }
  } else {
    bbbc = nullptr;
  }

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

  if (other.snconntbl) {
    snconntbl = new std::vector<streamnodeconn *>();
    for (const auto &res : *other.snconntbl) {
      snconntbl->push_back(
          new streamnodeconn(*res));
    }
  } else {
    snconntbl = nullptr;
  }

  return *this;
}

//////////////////////////////////////////////////////////////////
/// \brief Computes the Raven-style profile and generates the channel_properties_blackbird.rvp file
// output file will look like:
/* :ChannelRatingCurves[name]
     :Bedslope [slope]
     :StageRelations
        [depth, area, top_width, discharge, perimeter] x (number of rating curve points)
     :EndStageRelations
   :EndChannelRatingCurves
*/
//
void CModel::create_raven_profiles() {

    // xxx add functionality to write out the rvh file for :Subbasin and :HRUs table as well

    // add a default profile that is used for headwaters instead (one time for all headwaters)

    int total_nodes = bbsn->size();
    CStreamnode *start_streamnode = get_streamnode_by_id((*bbbc)[0]->nodeID);
    int nrowdepthdf = start_streamnode->depthdf->size();

    std::string tmpFilename = FilenamePrepare("channel_properties_blackbird.rvp");
    std::ofstream RVNPROFILE_OUTPUT(tmpFilename.c_str());

    if (RVNPROFILE_OUTPUT.fail()) {
        ExitGracefully(
            ("CModel::channel_properties_blackbird.rvp: Unable to open output file " +
             tmpFilename + " for writing.").c_str(),
            FILE_OPEN_ERR);

    }

    RVNPROFILE_OUTPUT 
        << "## Channel Properties written by Blackbird for use in Raven model\n"
        << "## Profiles determined from pre-processed reach data and flows calculated using a normal depth assumption\n\n";

    for (int i = 0; i < total_nodes; i++) {
    CStreamnode *&temp_sn = (*bbsn)[i];


   

      if (bbopt->skip_headwater && temp_sn->upnodeID1 == -1) {
        continue;    
      } else {
        RVNPROFILE_OUTPUT
        << ":ChannelRatingCurves profile_" << std::setw(5) << std::setfill('0') << temp_sn->nodeID << "\n"; // write in %05d format

        if (temp_sn->bed_slope < 0.01) {
          RVNPROFILE_OUTPUT
            << "  :Bedslope " <<  std::fixed << std::setprecision(4) << 0.01 << "\n"; // write in %.4f format
        } else {
          RVNPROFILE_OUTPUT
            << "  :Bedslope " <<  std::fixed << std::setprecision(4) << temp_sn->bed_slope << "\n"; // write in %.4f format
        }

        RVNPROFILE_OUTPUT
        // << "  :Bedslope " <<  std::fixed << std::setprecision(4) << std::max(temp_sn->bed_slope,0.01) << "\n" // write in %.4f format
        << "  :StageRelations" 
        << "  # [depth, area, top_width, discharge, perimeter]" 
        << "\n";

        for (int j = 0; j < nrowdepthdf; j++) {
            hydraulic_output *row = (*temp_sn->depthdf)[j];
            // Q = (1/n)*A*R^(2/3)*S^(1/2) rearranged to solve for flow using manning composite n, bed slope, and depth df outputs to get rating curve points for each depth df row
            double tempflow = (1/row->manning_composite)*std::pow(row->hradius,2/3)*std::sqrt(temp_sn->bed_slope)*row->area; 
            RVNPROFILE_OUTPUT << "    " 
                                << std::fixed << std::setprecision(2)
                                << row->depth << ", " 
                                << std::fixed << std::setprecision(3)
                                << row->area << ", "
                                << row->top_width << ", " << tempflow << ", "
                                << row->wet_perimeter << "\n"; // write in %.3f format
        }
        RVNPROFILE_OUTPUT << "  :EndStageRelations" << "\n";
        RVNPROFILE_OUTPUT << ":EndChannelRatingCurves" << "\n\n";
      }
    }
    RVNPROFILE_OUTPUT.close();
}

//////////////////////////////////////////////////////////////////
/// \brief Computes the hydraulic profile for the model
//
void CModel::hyd_compute_profile() {
  ExitGracefullyIf(bbopt->regimetype != enum_rt_method::SUBCRITICAL,
                   "Model.cpp: hyd_compute_profile(): only subcritical mode "
                   "with hardcoded options currently available",
                   BAD_DATA);

  ExitGracefullyIf(bbopt->dx <= 0,
                   "Model.cpp: hyd_compute_profile(): dx must be greater than 0",
                   BAD_DATA);

  // special case for SET_ALL_DEPTHS boundary condition, in which the depths are specified and solver is not used
  if ((*bbbc)[0]->bctype == enum_bc_type::SET_ALL_DEPTHS)
  {
    CBoundaryCondition *bc = (*bbbc)[0];

    const size_t nProfiles = bc->specified_depths.size();
    const size_t nNodes    = bc->nodeIDs.size();

    ExitGracefullyIf(
        nProfiles == 0,
        "Model.cpp: hyd_compute_profile(): SET_ALL_DEPTHS contains no profiles",
        BAD_DATA);

    ExitGracefullyIf(
        nNodes == 0,
        "Model.cpp: hyd_compute_profile(): SET_ALL_DEPTHS contains no nodes",
        BAD_DATA);

    hyd_result = new std::vector<hydraulic_output *>(
        nProfiles * bbsn->size(), nullptr);

    for (size_t f = 0; f < nProfiles; ++f)
    {
        ExitGracefullyIf(
            bc->specified_depths[f].size() != nNodes,
            "Model.cpp: hyd_compute_profile(): SET_ALL_DEPTHS profile size "
            "does not match node count",
            BAD_DATA);

        flow = static_cast<int>(f);

        for (size_t i = 0; i < nNodes; ++i)
        {
            CStreamnode *sn =
                get_streamnode_by_id(bc->nodeIDs[i]);

            ExitGracefullyIf(
                sn == nullptr,
                "Model.cpp: hyd_compute_profile(): SET_ALL_DEPTHS nodeID "
                "not found in stream network",
                BAD_DATA);

            //-----------------------------------------------------
            // Populate mm the same way compute_streamnode does
            //-----------------------------------------------------

            sn->mm->nodeID         = sn->nodeID;
            sn->mm->reachID        = sn->reachID;
            sn->mm->downnodeID     = sn->downnodeID;
            sn->mm->upnodeID1      = sn->upnodeID1;
            sn->mm->upnodeID2      = sn->upnodeID2;
            sn->mm->stationname    = sn->stationname;
            sn->mm->station        = sn->station;
            sn->mm->min_elev       = sn->min_elev; // xxx to update
            sn->mm->reach_length_DS  = sn->ds_reach_length;
            sn->mm->reach_length_US1 = sn->us_reach_length1;
            sn->mm->reach_length_US2 = sn->us_reach_length2;
            sn->mm->bed_slope      = sn->bed_slope;
            sn->mm->flow           = sn->output_flows[f];

            //-----------------------------------------------------
            // Apply specified depth
            //-----------------------------------------------------

            sn->mm->depth = bc->specified_depths[f][i];

            ExitGracefullyIf(
                sn->mm->depth < 0.0,
                "Model.cpp: hyd_compute_profile(): "
                "SET_ALL_DEPTHS depth cannot be negative",
                BAD_DATA);

            // update WSL based on difference in streamnode and specified min elev
            // xxx to do
            sn->mm->wsl =
                sn->mm->min_elev +
                sn->mm->depth;

            //-----------------------------------------------------
            // Compute all hydraulic properties
            //-----------------------------------------------------

            sn->compute_profile(
                sn->mm->flow,
                sn->mm->wsl,
                bbopt);

            // compute peak hours required and update min and max accordingly
            sn->compute_peak_hours();
            //sn->mm->peak_hrs_required = (sn->mm->area * sn->mm->length_effectiveadjusted / sn->mm->flow) / 3600;
            if (peak_hrs_min == PLACEHOLDER || sn->mm->peak_hrs_required < peak_hrs_min) {
            peak_hrs_min = sn->mm->peak_hrs_required;
            }
            if (peak_hrs_max == PLACEHOLDER || sn->mm->peak_hrs_required > peak_hrs_max) {
            peak_hrs_max = sn->mm->peak_hrs_required;
            }

            //-----------------------------------------------------
            // Store result
            //-----------------------------------------------------

            int ind = get_index_by_id(sn->nodeID);

            ExitGracefullyIf(
                ind < 0,
                "Model.cpp: hyd_compute_profile(): "
                "invalid streamnode index",
                BAD_DATA);

            hydraulic_output *pHO =
                new hydraulic_output(*(sn->mm));

            (*hyd_result)[f * bbsn->size() + ind] = pHO;
        }
        WriteAdvisory("The range of required peak flood times ranges from " +
                      std::to_string(peak_hrs_min) + " to " +
                      std::to_string(peak_hrs_max) + " hours for flow profile " +
                      std::to_string(flow+1) + " (" + fp_names[flow] +
                      ") within the computed streamnodes",
                  bbopt->noisy_run);
        peak_hrs_min = PLACEHOLDER;
        peak_hrs_max = PLACEHOLDER;
    }

    if (!bbopt->silent_run) {
        std::cout
            << "Hydraulic properties assigned from "
                "SET_ALL_DEPTHS boundary condition."
            << std::endl;
    }

    flow = PLACEHOLDER;
    return;
  }
  
  // Get streamnode associated with the boundary condition
  ExitGracefullyIf(
      bbbc->empty(),
      "Model.cpp: hyd_compute_profile(): no boundary conditions provided",
      BAD_DATA);
  CStreamnode *start_streamnode = get_streamnode_by_id((*bbbc)[0]->nodeID);

  ExitGracefullyIf(!start_streamnode,
                   "Model.cpp: hyd_compute_profile(): boundary condition "
                   "streamnode id not represented in streamnodes",
                   BAD_DATA);

  // Initialize container for hydraulic result
  hyd_result = new std::vector<hydraulic_output *>(
      start_streamnode->output_flows.size() * bbsn->size(), nullptr);

  // Loop through each flow
  // Compute the corresponding hydraulic profile starting at the boundary condition streamnode
  for (int f = 0; f < start_streamnode->output_flows.size(); f++) {
    flow = f;
    for (auto bc : *bbbc) {
      start_streamnode = get_streamnode_by_id(bc->nodeID);

      CStreamnode **junction_sns = new CStreamnode * [bbsn->size()](); // [MAX_TREE_DEPTH], taking all sn as upper bound
      CStreamnode *up_sn; // next upstreamnode streamnode
      CStreamnode *curr_sn=start_streamnode; //starting streamnode (most downstream)

      // compute current streamnode
      compute_streamnode(curr_sn, curr_sn, hyd_result, bc);

      // loop to compute other streamnodes moving upstream throgh branches
      int i = 0; // index for junction_sns
      while (curr_sn) {
        up_sn = get_streamnode_by_id(curr_sn->upnodeID1);
        if (up_sn)
        {
          // check for branching, save the current streamnode (curr_sn) if it is a junction
          CStreamnode *branch_sn = get_streamnode_by_id(curr_sn->upnodeID2);
          if (branch_sn) {
            junction_sns[i] = curr_sn;
            i++;
          }

          compute_streamnode(up_sn, curr_sn, hyd_result, bc);
          curr_sn = up_sn;
        }  else // hit most upstream of branch, move back down to most recent junction
        {
          if (i > 0) {
            if (i >= bbsn->size()) {
              // error: too many junctions
              ExitGracefully(
                  "Model.cpp: hyd_compute_profile(): too many junctions in stream network",
                             BAD_DATA);
              break;
            }

            i--;
            curr_sn = junction_sns[i];            
            up_sn = get_streamnode_by_id(curr_sn->upnodeID2);
            compute_streamnode(up_sn, curr_sn, hyd_result, bc);
            curr_sn = up_sn;
          } else {
            curr_sn = nullptr; // ran out of branches; all done
          }
        }
      }
    }
    WriteAdvisory("The range of required peak flood times ranges from " +
                      std::to_string(peak_hrs_min) + " to " +
                      std::to_string(peak_hrs_max) + " hours for flow profile " +
                      std::to_string(flow+1) + " (" + fp_names[flow] +
                      ") within the computed streamnodes",
                  bbopt->noisy_run);
    peak_hrs_min = PLACEHOLDER;
    peak_hrs_max = PLACEHOLDER;
  }
  if (!bbopt->silent_run) {
    std::cout << "Successfully completed all hydraulic calculations :-)" << std::endl;
  }
  flow = PLACEHOLDER;
  return;
}

//////////////////////////////////////////////////////////////////
/// \brief Calculate output flows of all streamnodes
//
void CModel::calc_output_flows() {
    int total_nodes = bbsn->size();           // number of nodes in bbsn
    int num_fp = PLACEHOLDER;                 // number of flow profiles

    if (bbopt->explicit_flows == false) {
        // update flows based on accumulated flows in each reach
        std::set<int> finished_nodes;             // sorted list of nodes where output_flows has been calculated
        std::unordered_map<int, int> id_to_ind;   // maps nodeID to index in bbsn

        for (int iter = 0; finished_nodes.size() < total_nodes && iter < 1000; iter++) {

            for (int i = 0; i < total_nodes; i++) {
                CStreamnode*& temp_sn = (*bbsn)[i]; // reference to current streamnode pointer to simplify code

                // If first while loop iteration, add node to id_to_ind
                if (iter == 0) {
                    id_to_ind[temp_sn->nodeID] = i;
                }

                // If node has been calculated, skip
                if (finished_nodes.find(temp_sn->nodeID) != finished_nodes.end()) {
                    continue;
                }
                // If either upnodes have not been calculated already, skip
                if ((temp_sn->upnodeID1 != -1 && (finished_nodes.find(temp_sn->upnodeID1) == finished_nodes.end()))
                    || (temp_sn->upnodeID2 != -1 && (finished_nodes.find(temp_sn->upnodeID2) == finished_nodes.end()))) {
                    continue;
                }

                // If headwater that has not been initialized in ParseBoundaryConditions, initialize output_flows to zero 
                if (temp_sn->upnodeID1 == -1 && temp_sn->upnodeID2 == -1 && temp_sn->output_flows.size() == 0) {
                    // num_fp is initialized from the first headwater node. If we hit an unintialized headwater node before 
                    // num_fp is initialized, try to find another headwater node and use that.
                    if (num_fp == PLACEHOLDER) {
                        // Try to find another initialized headwater node
                        for (int k = 0; k < total_nodes; k++) {
                            CStreamnode* other = (*bbsn)[k];
                            bool is_headwater =
                                other->upnodeID1 == -1 &&
                                other->upnodeID2 == -1 &&
                                other->output_flows.size() > 0;
                            if (is_headwater) {
                                num_fp = other->output_flows.size();
                                break;
                            }
                        }

                        // If still not found, default to 1 flow profile
                        if (num_fp == PLACEHOLDER) {
                            num_fp = 1;
                          WriteWarning("Number of flow profiles has not been assigned yet. Defaulting to 1 flow profile for headwater node " +
                                           std::to_string(temp_sn->nodeID),
                                       bbopt->noisy_run);
                        }
                    }
                    // Allocate flow profiles
                    temp_sn->allocate_flowprofiles(num_fp);
                    // Set the flow to zero
                    temp_sn->output_flows[0] = 0.0;
                    // Mark this as a headwater flow
                    temp_sn->upstream_flows[0] = HEADWATER;
                }

                // If headwater, add to finished nodes and continue
                if (temp_sn->upstream_flows.size() > 0 && temp_sn->upstream_flows[0] == HEADWATER) {
                    // finished_nodes.insert(temp_sn->nodeID);
                    // If num_fp has yet to be assigned, assign it (will execute on a headwater node in first iteration)
                    if (num_fp == PLACEHOLDER) {
                        num_fp = temp_sn->output_flows.size();
                    }

                    temp_sn->calc_output_flows_headwaternode(bbopt);
                    finished_nodes.insert(temp_sn->nodeID);
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
    else {
        // flows are explicitly defined
        // error check the inputs to ensure that each node has a defined flow
        // xxx to do
    }

    // Apply global flow multiplier if applicable
    if (flow_mult != 1) {
        for (int i = 0; i < total_nodes; i++) {
            CStreamnode*& temp_sn = (*bbsn)[i];
            for (int j = 0; j < num_fp; j++) {
                temp_sn->output_flows[j] *= flow_mult;
            }
        }
    }

    // print statements for sum of flows and flow_sources
    if (bbopt->debug_run && bbopt->enable_spill_flows) {
      write_total_flows();
    }
}

// brief calculate flows on the second iteration
// note: flow multiplier not used here as it would have already been included in first pass, propograted here
void CModel::calc_output_flows_secondpass() {

    int total_nodes = bbsn->size();
    int num_fp = bbsn->at(0)->output_flows.size();

    // update flows based on accumulated flows in each reach
    std::set<int> finished_nodes;             // sorted list of nodes where output_flows has been calculated
    std::unordered_map<int, int> id_to_ind;   // maps nodeID to index in bbsn

    // xxx iter should be increased for larger models?

    for (int iter = 0; finished_nodes.size() < total_nodes && iter < 1000; iter++) {

        for (int i = 0; i < total_nodes; i++) {
            CStreamnode*& temp_sn = (*bbsn)[i]; // reference to current streamnode pointer to simplify code

            // If first while loop iteration, add node to id_to_ind
            if (iter == 0) {
                id_to_ind[temp_sn->nodeID] = i;
            }

            // If node has been calculated, skip
            if (finished_nodes.find(temp_sn->nodeID) != finished_nodes.end()) {
                continue;
            }
            // If either upnodes have not been calculated already, skip
            if ((temp_sn->upnodeID1 != -1 && (finished_nodes.find(temp_sn->upnodeID1) == finished_nodes.end()))
                || (temp_sn->upnodeID2 != -1 && (finished_nodes.find(temp_sn->upnodeID2) == finished_nodes.end()))) {
                continue;
            }

            // If headwater, set flows to current flow + sources + sinks
            if (temp_sn->upnodeID1 == -1 && temp_sn->upnodeID2 == -1) {
                temp_sn->calc_output_flows_headwaternode(bbopt);
                finished_nodes.insert(temp_sn->nodeID);
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

    if (bbopt->debug_run && bbopt->enable_spill_flows) {
        write_total_flows();
    }
}

//////////////////////////////////////////////////////////////////
/// \brief Bound the transfer rate in spill flows based on inputs
/// \param cumqtransfer [in] cumulative flow transfer calculated based on spill height difference and kspillflows for a given streamnode connection (can be positive or negative based on direction of flow transfer)
/// \param Q1 [in] flow at node 1 (donor if cumqtransfer is positive, receiver if cumqtransfer is negative)
/// \param Q2 [in] flow at node 2 (receiver if cumqtransfer is positive, donor if cumqtransfer is negative)
/// \param minQ1 [in] minimum flow to maintain at node 1 (donor if cumqtransfer is positive, receiver if cumqtransfer is negative) after transfer based on coeff_min_flow
/// \param minQ2 [in] minimum flow to maintain at node 2 (receiver if cumqtransfer is positive, donor if cumqtransfer is negative) after transfer based on coeff_min_flow
/// \param maxChange1 [in] maximum change allowed in flow at node 1 (donor if cumqtransfer is positive, receiver if cumqtransfer is negative) based on coeff_qi_rate to avoid large changes in flow each iteration
/// \param maxChange2 [in] maximum change allowed in flow at node 2 (receiver if cumqtransfer is positive, donor if cumqtransfer is negative) based on coeff_qi_rate to avoid large changes in flow each iteration
//
double clamp_transfer(double cumqtransfer,
    double Q1, double Q2,
    double minQ1, double minQ2,
    double maxChange1, double maxChange2)
{
    // Direction of transfer
    double sign = (cumqtransfer >= 0.0 ? 1.0 : -1.0);
    double amount = std::abs(cumqtransfer);

    // Donor limits
    double donor_limit;
    if (sign > 0) {
        // Flow from node1 → node2
        donor_limit = Q1 - minQ1;
        donor_limit = std::min(donor_limit, maxChange1);
    }
    else {
        // Flow from node2 → node1
        donor_limit = Q2 - minQ2;
        donor_limit = std::min(donor_limit, maxChange2);
    }

    // Receiver limits
    double receiver_limit = (sign > 0 ? maxChange2 : maxChange1);

    // Final allowed transfer
    double allowed = std::min({ amount, donor_limit, receiver_limit });

    return sign * allowed;
}

//////////////////////////////////////////////////////////////////
/// \brief Update sources and sinks based on spill flow calculations
//
double CModel::update_spill_flows() {
    double max_deltaQ = 0.0;                  // max exchange in flow between streamnodes
    double max_spillheightdiff = 0.0;             // max difference in spill height between streamnodes (used to compute flow transfer rate)
    double cumqtransfer = 0.0;

    // In the :StreamnodeConnectionsTable, the transfer column indicates the connection of the nodes within the network
    // transfer ==  1 -> nodeID is upstream of adjacent_nodeID
    // transfer == -1 -> nodeID is downstream of adjacent_nodeID
    // transfer ==  0 -> nodeID is not upstream or downstream of adjacent_nodeID (likely on a parallel branch)
    // In the Blackbird compiled code, only transfers between parallel nodes (i.e., transfer==0) is allowed unless a conditioanl streamnode is configured

    // In this function, cumqtransfer assumes water is moving from temp_sn1 -> temp_sn2
    // cumqtransfer > 0 -> water is moving from temp_sn1 -> temp_sn2 (i.e., nodeID -> adjnodeID)
    // cumqtransfer < 0 -> water is moving from temp_sn2 -> temp_sn1 (i.e., adjnodeID -> nodeID)

    // note that if no conditional node is in place, then water is only allowed to move if transfer == 0 (i.e., parallel nodes) and will not move if transfer == 1 or -1 (i.e., upstream/downstream nodes)
    // if conditional node, then water can move from US->DS (i.e., short circuiting flow in the network), as this preserves the steady state flow balance
    // however, water is not allowed to move DS->US under any steady state circumstances, as this would violate the flow mass balance

    double coeff_qi_rate = bbopt->spillflowsmaxqrate; // computes max flow rate change based on coeff_qi_rate*Q for each iteration [0..1]
    // e.g., coeff_qi_rate = 0.9, max flow rate change is 90% of the current flow rate

    double coeff_min_flow = bbopt->spillflowsminq; // minimum flow to maintain in streamnodes from original, computed as coeff_min_flow*Q [0..1]
    // e.g., coeff_min_flow = 0.1, flow will not reduce below 10% of original flow from previous solution

    for (int i = 0; i < snconntbl->size(); i++) {
        // get streamnode references
        streamnodeconn* row = (*snconntbl)[i];
        int ind1 = get_index_by_id(row->nodeID);
        int ind2 = get_index_by_id(row->adjnodeID);
        CStreamnode*& temp_sn1 = (*bbsn)[ind1];
        CStreamnode*& temp_sn2 = (*bbsn)[ind2];
        bool setqtransfer = false;

        // conditons to end qtransfer calc or not
        if (i == snconntbl->size() - 1) {
            setqtransfer = true; // ended due to end of file
        }
        else {
            // end if the next row has a change in the pairing of cid & adjid OR if the reachID changes
            streamnodeconn* nextrow = (*snconntbl)[i + 1];
            int ind3 = get_index_by_id(nextrow->adjnodeID);
            if (ind3 != ind2 || nextrow->reachID != row->reachID) {
                setqtransfer = true;
            }
        }

        // get depths at each node from hydraulic results
        hydraulic_output* depthrow1 = (*hyd_result)[ind1];
        hydraulic_output* depthrow2 = (*hyd_result)[ind2];

        // compute difference in transfer elevation (absolute value)
        double spillheightdiff = (std::max(depthrow1->depth - row->minhand1, 0.0) + row->minelev1) -
                                 (std::max(depthrow2->depth - row->minhand2, 0.0) + row->minelev2);

        if (std::abs(spillheightdiff) > 0) {
            // update max spillheightdiff
            if (std::abs(spillheightdiff) > max_spillheightdiff) {
                max_spillheightdiff = std::abs(spillheightdiff);
            }
        }

        // limit spillheightdiff based on the difference in cell WSE vs elevation
        // limit height diff and flow rate transfer where the elevation alone > elev+depth on other side
        if      (spillheightdiff > 0 && row->minelev1 > std::max(depthrow2->depth - row->minhand2, 0.0) + row->minelev2) {
            // limit to just the depth of water on the upstream side
            spillheightdiff = std::max(depthrow1->depth - row->minhand1, 0.0);       
        }
        else if (spillheightdiff < 0 && row->minelev2 > std::max(depthrow1->depth - row->minhand1, 0.0) + row->minelev1) {
            // limit to just the depth of water on the upstream side
            spillheightdiff = std::max(depthrow2->depth - row->minhand2, 0.0)*(-1.0);       
        }

        double qtransfer = spillheightdiff * bbopt->kspillflows;
        cumqtransfer = cumqtransfer + qtransfer;

      if (setqtransfer) {

        // limit qtransfer based on receiving body's flow rate, avoid a 50% increase in flows
        /* if (cumqtransfer > 0) {
          if (cumqtransfer > depthrow2->flow*coeff_qi_rate) {
            cumqtransfer = depthrow2->flow * coeff_qi_rate;
          }
        } else {
          if (abs(cumqtransfer) > depthrow1->flow * coeff_qi_rate) {
            cumqtransfer = depthrow1->flow * (-coeff_qi_rate);
          }
        }
        */

        // clamp the transfer if neither node is conditional
        if (!(temp_sn1->is_conditional || temp_sn2->is_conditional)) {
            // clamp the transfer rate with bounds
            double Q1 = temp_sn1->output_flows[0];
            double Q2 = temp_sn2->output_flows[0];

            double minQ1 = depthrow1->flow * coeff_min_flow;
            double minQ2 = depthrow2->flow * coeff_min_flow;

            double maxChange1 = Q1 * coeff_qi_rate;
            double maxChange2 = Q2 * coeff_qi_rate;


            cumqtransfer = clamp_transfer(cumqtransfer,
                                    Q1, Q2,
                                    minQ1, minQ2,
                                    maxChange1, maxChange2);
        }

        if (row->transfer == 1 || row->transfer==-1) {

            // update to set the water transfer rate to zero if either is true:
            // - water would move from DS->US (not allowed even if it's a conditional node)
            // - water would move from US->DS but the receiving node is not conditional

            if (cumqtransfer > 0 && row->transfer == -1 ||       // water moving DS->US
                cumqtransfer < 0 && row->transfer == 1 ||        // water moving DS->US
                cumqtransfer > 0 && !temp_sn2->is_conditional || // water moving US->DS but receiving node is not conditional
                cumqtransfer < 0 && !temp_sn1->is_conditional) { // water moving US->DS but receiving node is not conditional
                cumqtransfer = 0.0; // set to zero to prevent transfer flow along connected streamnodes
                if (bbopt->debug_run) {
                std::cout << "setting cumqtransfer to zero for nodes "
                << std::to_string(temp_sn1->nodeID) << " -> "
                << std::to_string(temp_sn2->nodeID) << std::endl;
                }
            }
        }

        // update flow sources
        temp_sn1->flow_sources[0] += cumqtransfer*(-1);
        temp_sn2->flow_sources[0] += cumqtransfer;
        
        // recalculate all flows based on updated sources and sinks to ensure 
        /// they are reflected in the iterative flow changes along reaches
        //calc_output_flows();

        if (bbopt->noisy_run || bbopt->debug_run) {
          
          std::cout << "-- cumqtransfer = " + std::to_string(cumqtransfer) 
                    << " for nodes " << std::to_string(row->nodeID) << " -> " << std::to_string(row->adjnodeID) 
                    /*
                    << ", cid node original flow = "
                    << std::to_string(depthrow1->flow) << ", updated flow = "
                    << std::to_string(temp_sn1->output_flows[0] + temp_sn1->flow_sources[0])
                    << ", adjid node original flow = "
                    << std::to_string(depthrow2->flow) << ", updated flow = "
                    << std::to_string(temp_sn2->output_flows[0] + temp_sn2->flow_sources[0]) */
                    << ", total flow adjustment = " << std::abs(temp_sn1->flow_sources[0])
                    << std::endl;
        }

        // reset cumulative flow transfer calculation for next streamnode
        cumqtransfer = 0.0; 
      }
  }

  return max_spillheightdiff; // return max spill height diff to evaluate against threshold
}

//////////////////////////////////////////////////////////////////
/// \brief Add streamnode to model and map
/// \param pSN [in] pointer reference to CStreamnode being added
//
void CModel::add_streamnode(CStreamnode*& pSN) {
  // Check for duplicate nodeID
  if (streamnode_map.find(pSN->nodeID) != streamnode_map.end()) {
    ExitGracefully(
          ("Error: Streamnode with nodeID " + std::to_string(pSN->nodeID) +
                       " already exists in streamnode_map.").c_str(),
          BAD_DATA);
  }

  bbsn->push_back(pSN);
  streamnode_map[pSN->nodeID] = bbsn->size() - 1;
}

//////////////////////////////////////////////////////////////////
/// \brief Add node ID to conditional node ID integer vector
/// \param id [in] integer to add to vector
//
void CModel::add_condsnid(int id, int mapsto)
{
    pairedcondids p(id, mapsto);
    // avoid duplicates
    auto it = std::find_if(
        condsnIDs.begin(),
        condsnIDs.end(),
        [&](const pairedcondids& x) {
            return x.id1 == id && x.id2 == mapsto;
        }
    );

    if (it == condsnIDs.end()) {
        condsnIDs.push_back(p);
    }
    conditional_model = true;
}

//////////////////////////////////////////////////////////////////
/// \brief Checks whether a node is conditional based on ID value
/// \param id [in] integer to check
//
bool CModel::is_conditional_node(int id) const
{
    for (const auto& p : condsnIDs) {
        if (p.id2 == id) return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns the mapto paired ID from a conditional node ID
/// \param id [in] integer to check
//
int CModel::get_conditional_condid(int id) const
{
    for (const auto& p : condsnIDs) {
        if (p.id2 == id) return p.id1;
    }
    return -1;
}

//////////////////////////////////////////////////////////////////
/// \brief Updates hydraulic property tables for streamnodes with hydraulic
/// structures (bridges, culverts, etc.)
//
void CModel::update_hydraulic_structures()
{
    for (auto& pSN : *(this->bbsn))
    {
        if (!pSN->is_structure)
            continue;

        hydraulic_structure& hs = pSN->hydstructure;
        auto& df = *(pSN->depthdf);   // dereference pointer

        size_t N = df.size();

        double n_bed = 0.035;
        hydraulic_output* firstrow = df[0];
        n_bed = firstrow->manning_composite;
        // double n_structure = hs.structure_manning_roughness;

        // check optional length argument - set to upstream reach length if not provided
        if (hs.length < 0) {
          hs.length = pSN->us_reach_length1;          
        }

        if (N == 0)
        {
            std::string error =
                "HydraulicStructure Error: Streamnode " +
                std::to_string(pSN->nodeID) +
                " is marked as a hydraulic structure, but its depthdf "
                "table is empty or missing. Cannot update hydraulic properties.";

            ExitGracefully(error.c_str(), BAD_DATA_WARN);
        }

        // Extract baseline columns
        std::vector<double> depth(N);
        std::vector<double> area_base(N);
        std::vector<double> perim_base(N);
        std::vector<double> topw_base(N);
        std::vector<double> K_base(N);
        std::vector<double> hR_base(N);
        std::vector<double> mann_base(N);
        std::vector<double> leff_base(N);
        // std::vector<double> leffadj_base(N);

        for (size_t i = 0; i < N; ++i)
        {
            hydraulic_output* row = df[i];

            depth[i]        = row->depth;
            area_base[i]    = row->area;
            perim_base[i]   = row->wet_perimeter;
            topw_base[i]    = row->top_width;
            K_base[i]       = row->k_total;
            hR_base[i]      = row->hradius;
            mann_base[i]    = row->manning_composite;
            leff_base[i]    = row->length_effective;
            // leffadj_base[i]      = row->length_effectiveadjusted;
        }

        // New geometry arrays
        std::vector<double> new_area(N,0.0);
        std::vector<double> new_perim(N,0.0);
        std::vector<double> new_topw(N,0.0);
        std::vector<double> new_K(N,0.0);
        std::vector<double> new_hr(N,0.0);
        std::vector<double> new_mann(N,0.0);
        std::vector<double> theta(N,0.0);
        std::vector<double> new_leff(N,0.0);

        int idx_top_opening = -1;
        int idx_deck = -1;

        // Structure to do list ===============
        // xxx consider setting the properties to be the max of the properties here and the original?
        // xxx update manning roughness handling, dont need gravel, take the baseline nc and use that, blend with structure sides
        // xxx check the top openng and deck indices


        // Identify key depths
        if (hs.type == enum_struc_type::BRIDGE)
        {
            double H = hs.height;
            double deck = hs.deck_thickness;

            auto it = std::lower_bound(depth.begin(), depth.end(), H);

            if (it != depth.end())
                idx_top_opening = (int)std::distance(depth.begin(), it);

            it = std::lower_bound(depth.begin(), depth.end(), H + deck);

            if (it != depth.end())
                idx_deck = (int)std::distance(depth.begin(), it);
        }
        else if (hs.type == enum_struc_type::CULVERT_CIRCULAR)
        {
            double R = hs.diameter / 2.0;
            double bottom = hs.bottom_height;
            double deck = hs.deck_thickness;

            for (size_t i = 0; i < N; ++i)
            {
                if (depth[i] >= bottom + 2*R) idx_top_opening = (int)i;
                if (depth[i] >= deck) idx_deck = (int)i;
            }
        }

        // Compute modified geometry
        for (size_t i = 0; i < N; ++i)
        {
            double d = depth[i];

            if (hs.type == enum_struc_type::BRIDGE)
            {
                double W = hs.width;
                double H = hs.height;
                double deck = hs.deck_thickness;
                double n_channel = mann_base[i];
                double n_channel_top = (idx_top_opening >= 0 ? mann_base[idx_top_opening] : n_channel);
                

                if (d < H)
                {
                    // set lengths to static structure length
                    new_leff[i] = hs.length;

                    new_topw[i] = std::min(W, topw_base[i]);
                    new_area[i] = std::min(d * W, area_base[i]);
                    double P_structure = 2*d;
                    double P_natural = std::min(perim_base[i], W);
                    // if existing perim less than d*W, indicates bathymetry is not rectangular and water may not be in contact yet
                    // if perim_base[i] < d*W,
                    /* if (perim_base[i] < 2 * d + W) { 
                      P_structure = 2*d;
                      P_natural = std::min(perim_base[i], W);
                    }*/
                    new_perim[i] = P_structure + P_natural; //  std::min(W + 2*d, perim_base[i]);
                    new_hr[i] = (new_perim[i] > 0.0) ? (new_area[i] / new_perim[i]) : 0.0;

                    // new_topw[i] = W; // std::min(W, topw_base[i]);
                    // new_area[i] = d*W; // std::min(d * W, area_base[i]);
                    // new_perim[i] = W + 2*d; // std::min(W + 2*d, perim_base[i]);
                    // new_perim[i] = perim_base[i] + 2*d; // std::min(W + 2*d, perim_base[i]);

                    // breakdown for different methods (equal force, equal velocity, inverse_mannings, etc.)
                    new_mann[i] = std::pow(
                        ( P_structure*std::pow(hs.structure_manning_roughness,1.5) +
                          P_natural*std::pow(n_channel,1.5) ) / new_perim[i],
                        2.0/3.0);

                    if (!(new_mann[i] >= 0.0 && std::isfinite(new_mann[i]))) {
                      new_mann[i] = mann_base[i];
                    }

                    // calc conveyance based flow through structure
                    new_K[i] = std::max(0.0, new_area[i] * std::pow(new_hr[i], 2.0/3.0) / new_mann[i]); 
                }
                else if (d < H + deck)
                {
                    // set lengths to static structure length
                    new_leff[i] = hs.length;

                    // new_topw[i] = W; // std::min(W, topw_base[i]);
                    // new_area[i] = H*W; //std::min(H * W, area_base[i]);

                    // new_topw[i] = std::min(W, topw_base[i]);
                    new_topw[i] = std::max(W, topw_base[i]);

                    new_area[i] = std::min(H * W, area_base[i]);
                    // new_perim[i] = 2*W + 2*H; //  std::min(2*W + 2*H, perim_base[i]);

                    double P_structure = 2*H+W;
                    double P_natural = std::min(perim_base[i], W);
                    // if existing perim less than W+2*H, indicates bathymetry is not rectangular and water may not be in contact yet
                    // if perim_base[i] < d*W,
                    /* if (perim_base[i] < W + 2 * H) { 
                      P_structure = 2*H;
                      P_natural = std::min(perim_base[i], 2*W);
                    }*/
                    new_perim[i] = P_structure + P_natural; //  std::min(W + 2*d, perim_base[i]);
                    //new_perim[i] = perim_base[i] + W + 2*H; //  std::min(2*W + 2*H, perim_base[i]);
                    new_hr[i] = (new_perim[i] > 0.0) ? (new_area[i] / new_perim[i]) : 0.0;

                    new_mann[i] = std::pow(
                        ( P_structure*std::pow(hs.structure_manning_roughness,1.5) +
                          P_natural*std::pow(n_channel,1.5) ) / new_perim[i],
                        2.0/3.0);

                    if (!(new_mann[i] >= 0.0 && std::isfinite(new_mann[i]))) {
                      new_mann[i] = mann_base[i];
                    }

                    // calc conveyance based on roughness from selected method
                    new_K[i] = std::max(0.0, new_area[i] * std::pow(new_hr[i], 2.0/3.0) / new_mann[i]);
                    // (new_mann[i] == 0 ? 0 : (new_area[i] * std::pow(new_hr[i], 2.0/3.0)) / new_mann[i]);

                }
                else // depth exceeds the deck, overtopping is occuring
                {
                   ExitGracefullyIf(i < 1, "Insufficient points in the preprocess hyd table",RUNTIME_ERR);

                    // compute new area based on old properties ----
                    double A_base_diff = area_base[i] - area_base[i-1]; // incremental difference in area from base properties at the deck

                    // check differences in depth curve and depth threshold at H+deck, account for that proportional to depth as estimate
                    // only relevant within one step of the idx_top_opening
                    if (depth[i - 1] < H+deck) {
                      A_base_diff =
                          A_base_diff * ((depth[i] - (H+deck)) / (depth[i] - depth[i-1]));
                    }

                    // correct area for any discrepancy in leff_base[i] / hs.length
                    A_base_diff = A_base_diff * leff_base[i] / hs.length;
                    
                    new_area[i] = A_base_diff + new_area[i-1];      // previous area plus incremental difference
                    double A_structure = new_area[idx_top_opening]; // area through the structure, used for weighing length
                    double A_natural = new_area[i] - A_structure;   // overtopping flow area, used for weighing length
                    // ----

                    // estimate the new effective length (leff=Hs.length for everything less than H+deck)
                    // will be close to the structure length if most area is through the structure, and approach the leff_base for larger depths
                    new_leff[i] = (leff_base[i]*A_natural + hs.length*A_structure) / new_area[i];

                    // compute the new wetted perimeter
                    // xxx need to fix how this is computed - should just take the perimeter at idx_top_opening combined with mannn then, and blend with next
                    // double P_structure = (2 * W + 2 * H) + topw_base[idx_top_opening];
                    // double P_base_diff = perim_base[i] - perim_base[i - 1];


                    double P_structure = new_perim[idx_top_opening];
                    double P_natural = perim_base[i] - perim_base[i - 1];


                    // check differences in depth curve and depth threshold at H+deck, account for that proportional to depth as estimate
                    // only relevant within one step of the idx_top_opening
                    if (depth[i - 1] < H+deck) {
                      P_natural =
                          P_natural * ((depth[i] - (H+deck)) / (depth[i] - depth[i-1]));
                    }

                    // correct area for any discrepancy in leff_base[i] / hs.length
                    P_natural = P_natural * leff_base[i] / hs.length;
                    new_perim[i] = P_natural + P_structure;

                    // calculate new top width
                    new_topw[i] = topw_base[i] * leff_base[i] / hs.length;
                    new_hr[i] = (new_perim[i] > 0.0) ? (new_area[i] / new_perim[i]) : 0.0;

                    new_mann[i] = std::pow(
                        (
                            P_structure*std::pow(new_mann[idx_top_opening],1.5) + // roughness computed at top opening
                            P_natural * std::pow(mann_base[i],1.5) // baseline roughness and natural new wetted perimeter
                        ) / new_perim[i],
                        2.0/3.0);

                    if (!(new_mann[i] >= 0.0 && std::isfinite(new_mann[i]))) {
                      new_mann[i] = mann_base[i];
                    }

                    // calc conveyance based on roughness from selected method
                    new_K[i] = std::max(0.0, new_area[i] * std::pow(new_hr[i], 2.0/3.0) / new_mann[i]); 
                }

                // new_hR[i] = (new_perim[i] == 0 ? 0 : new_area[i] / new_perim[i]);
                // new_K[i] = (new_mann[i] == 0 ? 0 :
                            // (new_area[i] * std::pow(new_hR[i], 2.0/3.0)) / new_mann[i]);
            }
            else if (hs.type == enum_struc_type::CULVERT_CIRCULAR)

                // xxx note: all circular culvert properties need to be updated 
            {

                double R = hs.diameter / 2.0;
                double bottom = hs.bottom_height;
                double deck = hs.deck_thickness;
                int B = hs.barrels;

                double n_channel = mann_base[i];
                double n_channel_top = (idx_top_opening >= 0 ? mann_base[idx_top_opening] : n_channel);
                // double n_conc = hs.structure_manning_roughness; // xxx update to remove this 

                if (d <= bottom)
                {
                    theta[i] = 0;
                    new_area[i] = 0;
                    new_perim[i] = 0;
                    new_topw[i] = 0;
                    new_mann[i] = 0;
                    //new_hR[i] = 0;
                    new_K[i] = 0;
                }
                else if (d < bottom + 2*R)
                {
                    theta[i] = std::acos(1 - (d - bottom)/R);
                    new_area[i] = B * R*R * (theta[i] - std::sin(theta[i]))/2.0;
                    new_perim[i] = B * theta[i] * R;
                    new_mann[i] = hs.structure_manning_roughness;
                    new_topw[i] = B * 2*R * std::sin(theta[i]);
                }
                else if (d == bottom + 2*R)
                {
                    theta[i] = std::acos(1 - (d - bottom)/R);
                    new_area[i] = B * M_PI * R*R;
                    new_perim[i] = B * 2*M_PI * R;
                    new_mann[i] = hs.structure_manning_roughness;
                    new_topw[i] = 0;
                }
                else if (d < deck)
                {
                    theta[i] = 0;
                    new_area[i] = B * M_PI * R*R;
                    new_perim[i] = B * 2*M_PI * R;
                    new_mann[i] = hs.structure_manning_roughness;
                    new_topw[i] = 0;
                }
                else
                {

                    // compute weighted average of new effective length once overtopped
                    double length_ww = 0.0;
                    if (d > deck + hs.diameter) {
                      length_ww = 1.0;
                    } else {
                      length_ww = (d - deck) / hs.diameter; // close to zero if just over threshold, close to 1 if quite deep
                    }

                    double length_ratio = leff_base[i] / (hs.length*(1-length_ww) + leff_base[i]*length_ww); // ratio to use in adjusting properties, original leff / proposed leff
                    
                    // set lengths to static structure length
                    new_leff[i] = hs.length*(1-length_ww) + leff_base[i]*length_ww; // make this a weighted average instead

                    theta[i] = 0;

                    double A_base = area_base[i];
                    double A_deck = (idx_deck >= 0 ? area_base[idx_deck] : 0.0);
                    new_area[i] = (A_base - A_deck)*length_ratio + B * M_PI * R*R;

                    new_topw[i] = topw_base[i]*length_ratio;

                    double P_base = perim_base[i];
                    double P_deck = (idx_deck >= 0 ? perim_base[idx_deck] : 0.0);
                    double TW_deck = (idx_deck >= 0 ? topw_base[idx_deck] : 0.0);

                    new_perim[i] = (P_base - P_deck + TW_deck)*length_ratio + B * 2*M_PI * R ;

                    double n_base = mann_base[i];

                    new_mann[i] = std::pow(
                        (
                            (B * 2*M_PI * R)*std::pow(hs.structure_manning_roughness,1.5) +
                            (P_base - P_deck)*std::pow(n_base,1.5) +
                            TW_deck*std::pow(n_bed,1.5)
                        ) / new_perim[i],
                        2.0/3.0);
                }

                // new_hR[i] = (new_perim[i] == 0 ? 0 : new_area[i] / new_perim[i]);
                // new_K[i] = (new_mann[i] == 0 ? 0 :
                      //       (new_area[i] * std::pow(new_hR[i], 2.0/3.0)) / new_mann[i]);
            }
        }

        // Write updated geometry back into hydraulic_output rows
        for (size_t i = 0; i < N; ++i)
        {
            hydraulic_output* row = df[i];

            row->top_width         = new_topw[i];
            row->area              = new_area[i];
            row->wet_perimeter     = new_perim[i];
            row->hradius           = new_hr[i]; 
            row->manning_composite = new_mann[i];
            // row->k_total           = (new_mann[i] == 0 ? 0 : (new_area[i] * std::pow(row->hradius, 2.0/3.0)) / new_mann[i]);
            row->k_total           = new_K[i];
            row->length_effective  = new_leff[i];
            row->length_effectiveadjusted = new_leff[i];
            row->hyd_depth =  (row->top_width == 0 ? 0 : row->area / row->top_width); // updated from other info

            // xxx will want to update roughness handling

            // Only culverts use theta
            // (Add a theta field to hydraulic_output if needed)
            // row->theta = theta[i];
        }
    }
}

//////////////////////////////////////////////////////////////////
/// \brief Returns streamnode with id 'sid'
/// \param sid [in] id of streamnode to get index of
/// \return streamnode with id 'sid'
//
CStreamnode* CModel::get_streamnode_by_id(int sid) const {
  return streamnode_map.find(sid) != streamnode_map.end() ? bbsn->at(streamnode_map.find(sid)->second) : nullptr;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns index of streamnode with id 'sid'
/// \param sid [in] id of streamnode to get index of
/// \return index of streamnode with id 'sid'
//
int CModel::get_index_by_id(int sid) {
  return streamnode_map.find(sid) != streamnode_map.end()
             ? streamnode_map[sid]
             : PLACEHOLDER;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns index in hyd_result of streamnode with id 'sid' and a flow index of flow_ind
/// \param flow_ind [in] index of the flow currently being considered
/// \param sid [in] id of streamnode to get index of
/// \return index in hyd_result of streamnode with id 'sid' and a flow index of flow_ind
//
int CModel::get_hyd_res_index(int flow_ind, int sid) {
  return flow_ind * bbsn->size() + get_index_by_id(sid);
}

//////////////////////////////////////////////////////////////////
/// \brief Add row to snconntbl
///
/// \param row [in] row to be added
//
void CModel::add_snconntbl_row(streamnodeconn *&row) {
  snconntbl->push_back(row);
}

//////////////////////////////////////////////////////////////////
/// \brief Reads GIS files required for model
//
void CModel::ReadGISFiles() {
  if (bbopt->gis_path == PLACEHOLDER_STR) {
    ExitGracefully(
        "Model.cpp: ReadGISFiles: :GISPath must be provided in .bbi file",
        exitcode::BAD_DATA);
  }

  // Read vector file for snapped pourpoints if using POURPOINT_SHP method
  if (bbopt->interpmethod == enum_interp_method::POURPOINT_SHP &&
      (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND)) {
    ReadVectorFile(bbopt->gis_path + "/bb_snapped_pourpoints_hand.shp", spp);
    spp.name = "Snapped Pourpoints";  // xxx may be able to remove from usage soon
  }

  // Read gridded data either from netcdf or raster
  if (!bbopt->silent_run) {
    std::cout << "Searching for NetCDF file" << std::endl;
  }
  if (std::filesystem::exists(bbopt->gis_path + "/" + bbopt->in_nc_name)) {
    if (!bbopt->silent_run) {
      std::cout << "Reading from NetCDF" << std::endl;
    }
    bbopt->in_format = enum_gridded_format::NETCDF;
    ReadNetCDFFile(bbopt->gis_path + "/" + bbopt->in_nc_name);
  } else {
    if (!bbopt->silent_run) {
      std::cout << "NetCDF file not found at " + bbopt->gis_path + "/" + bbopt->in_nc_name << std::endl;
      std::cout << "Reading from Rasters" << std::endl;
    }

    c_from_s = std::make_unique<CRaster>();

    bbopt->in_format = enum_gridded_format::RASTER;
    ReadRasterFile(bbopt->gis_path + "/bb_catchments_fromstreamnodes.tif", dynamic_cast<CRaster *>(c_from_s.get()));
    c_from_s->name = "Catchments from Streamnodes";

    if (bbopt->extrachecks) {

      // -------------------------------------------------------------
      // Collect unique IDs from catchment_from_streamnodes raster
      // -------------------------------------------------------------
      std::unordered_set<int> catchment_ids;

      for (int j = 0; j < c_from_s->xsize * c_from_s->ysize; ++j) {
        double v = c_from_s->data[j];
        if (!std::isnan(v) && v != c_from_s->na_val) {
          catchment_ids.insert(static_cast<int>(v));
        }
      }

      // -------------------------------------------------------------
      // Collect actual streamnode IDs from the model
      // -------------------------------------------------------------
      std::unordered_set<int> streamnode_ids;

      for (const auto &sn : *bbsn) { // adjust if your container differs
        streamnode_ids.insert(sn->nodeID);
      }

      // -------------------------------------------------------------
      // Warn if any catchment IDs do not correspond to a real streamnode
      // -------------------------------------------------------------
      for (int id : catchment_ids) {
        if (streamnode_ids.find(id) == streamnode_ids.end()) {
          std::string msg = "Catchment raster contains streamnode ID " +
                            std::to_string(id) +
                            " which does not exist in the model streamnodes.";
          WriteWarning(msg.c_str(), bbopt->noisy_run);
        }
      }
    }

    if (bbopt->interpolation_postproc_method == enum_ppi_method::CATCHMENT_HAND ||
        bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) { // no dhand
      hand = std::make_unique<CRaster>();
      handid = std::make_unique<CRaster>();
      ReadRasterFile(bbopt->gis_path + "/bb_hand.tif",  dynamic_cast<CRaster *>(hand.get()));
      hand->name = "HAND";
      if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND && 
          bbopt->interpmethod == enum_interp_method::POURPOINT_SHP) {
        ReadRasterFile(bbopt->gis_path + "/bb_hand_pourpoint_id.tif",  dynamic_cast<CRaster *>(handid.get()));
        handid->name = "HAND ID";
      }
      if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND && 
          bbopt->interpmethod == enum_interp_method::INTERP_RASTER) {
        interpdepth = std::make_unique<CRaster>();
        ReadRasterFile(bbopt->gis_path + "/bb_interp.tif",  dynamic_cast<CRaster *>(interpdepth.get()));
        interpdepth->name = "INTERP DEPTH";
      }
      if (conditional_model) {
        condhand = std::make_unique<CRaster>();
        ReadRasterFile(bbopt->gis_path + "/bb_hand_conditional.tif",  dynamic_cast<CRaster *>(condhand.get()));
        condhand->name = "CONDHAND";
      }
    } else { // use dhand
      for (auto d : dhand_depth_seq) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(4) << d;
        dhand.push_back(std::make_unique<CRaster>());
        ReadRasterFile(bbopt->gis_path + "/bb_dhand_depth_" + stream.str() + "m.tif", dynamic_cast<CRaster *>(dhand.back().get()));
        dhand.back()->name = "DHAND " + stream.str();
        if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
            bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR) {
          dhandid.push_back(std::make_unique<CRaster>());
          ReadRasterFile(bbopt->gis_path + "/bb_dhand_pourpoint_id_depth_" + stream.str() + "m.tif", dynamic_cast<CRaster *>(dhandid.back().get()));
          dhandid.back()->name = "DHAND ID " + stream.str();
        }
      }
    }
  }
  if (!bbopt->silent_run) {
    std::cout << "...gridded data successfully read" << std::endl;
    std::cout << std::endl;
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Reads specified NetCDF file
/// \param filename [in] full path to netcdf file to read from
//
void CModel::ReadNetCDFFile(std::string filename) {
  int ncid;
  if (nc_open(filename.c_str(), NC_NOWRITE, &ncid) != NC_NOERR) {
    ExitGracefully(
        ("Model.cpp: ReadNetCDFFile: Failed to open NetCDF file: " +
         filename)
            .c_str(),
        exitcode::BAD_DATA);
  }

  // Read dimensions
  size_t x_len, y_len, depth_len;
  int x_dimid, y_dimid, depth_dimid;

  if (nc_inq_dimid(ncid, "easting", &x_dimid) == NC_NOERR) {
    nc_inq_dimlen(ncid, x_dimid, &x_len);
  } else {
    ExitGracefully("Model.cpp: ReadNetCDFFile: NetCDF file missing 'easting' dimension.", exitcode::BAD_DATA);
  }
  if (nc_inq_dimid(ncid, "northing", &y_dimid) == NC_NOERR) {
    nc_inq_dimlen(ncid, y_dimid, &y_len);
  } else {
    ExitGracefully("Model.cpp: ReadNetCDFFile: NetCDF file missing 'northing' dimension.", exitcode::BAD_DATA);
  }

  bool has_depth = (nc_inq_dimid(ncid, "depth", &depth_dimid) == NC_NOERR);
  if (nc_inq_dimid(ncid, "depth", &depth_dimid) == NC_NOERR) {
    nc_inq_dimlen(ncid, depth_dimid, &depth_len);
  }

  // Read coordinate variables
  std::vector<double> x_coords(x_len);
  std::vector<double> y_coords(y_len);

  int x_varid, y_varid;
  if (nc_inq_varid(ncid, "easting", &x_varid) == NC_NOERR) {
    nc_get_var_double(ncid, x_varid, x_coords.data());
  } else {
    ExitGracefully("Model.cpp: ReadNetCDFFile: NetCDF file missing 'easting' coordinate variable.", exitcode::BAD_DATA);
  }
  if (nc_inq_varid(ncid, "northing", &y_varid) == NC_NOERR) {
    nc_get_var_double(ncid, y_varid, y_coords.data());
  } else {
    ExitGracefully("Model.cpp: ReadNetCDFFile: NetCDF file missing 'northing' coordinate variable.", exitcode::BAD_DATA);
  }

  // Read the epsg
  std::size_t len;
  std::string epsg;
  if (nc_inq_attlen(ncid, NC_GLOBAL, "EPSG", &len) == NC_NOERR) {
    std::vector<char> buffer(len + 1, '\0');

    if (nc_get_att_text(ncid, NC_GLOBAL, "EPSG", buffer.data()) == NC_NOERR) {
      std::istringstream iss(buffer.data());
      std::string token;
      while (iss >> token) {
        epsg = token;
      }
    } else {
      ExitGracefully("Model.cpp: ReadNetCDFFile: failed to read global attribute \"EPSG\"", exitcode::RUNTIME_ERR);
    }
  } else {
    ExitGracefully("Model.cpp: ReadNetCDFFile: global attribute \"EPSG\" not found", exitcode::BAD_DATA);
  }

  // Read layers
  c_from_s = std::make_unique<CNetCDFLayer>();

  ReadNetCDFLayer(dynamic_cast<CNetCDFLayer *>(c_from_s.get()), ncid, "catchments_streamnodes", x_len, y_len, x_coords, y_coords, epsg);
  c_from_s->name = "Catchments from Streamnodes";
  if (bbopt->interpolation_postproc_method == enum_ppi_method::CATCHMENT_HAND ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) { // no dhand
    hand = std::make_unique<CNetCDFLayer>();
    handid = std::make_unique<CNetCDFLayer>();
    ReadNetCDFLayer(dynamic_cast<CNetCDFLayer *>(hand.get()), ncid, "hand", x_len, y_len, x_coords, y_coords, epsg);
    hand->name = "HAND";
    if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
      ReadNetCDFLayer(dynamic_cast<CNetCDFLayer *>(handid.get()), ncid, "handid", x_len, y_len, x_coords, y_coords, epsg);
      handid->name = "HAND ID";
    }
  } else { // use dhand
    for (int i = 0; i < dhand_depth_seq.size(); i++) {
      std::stringstream stream;
      stream << std::fixed << std::setprecision(4) << dhand_depth_seq[i];
      dhand.push_back(std::make_unique<CNetCDFLayer>());
      ReadNetCDFLayer(dynamic_cast<CNetCDFLayer *>(dhand.back().get()), ncid, "dhand", x_len, y_len, x_coords, y_coords, epsg, i);
      dhand.back()->name = "DHAND " + stream.str();
      if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
          bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR) {
        dhandid.push_back(std::make_unique<CNetCDFLayer>());
        ReadNetCDFLayer(dynamic_cast<CNetCDFLayer *>(dhandid.back().get()), ncid, "dhandid", x_len, y_len, x_coords, y_coords, epsg, i);
        dhandid.back()->name = "DHAND ID " + stream.str();
      }
    }
  }

  nc_close(ncid);
}

//////////////////////////////////////////////////////////////////
/// \brief Reads specified NetCDF layer
/// \param netcdf_obj [in] pointer to CNetCDFLayer object to write to
/// \param ncid [in] ncid of file, for netcdf API
/// \param var_name [in] name of variable to read
/// \param xsize [in] size of x dimension of data
/// \param ysize [in] size of y dimension of data
/// \param x_coords [in] x dimension data
/// \param y_coords [in] y dimension data
/// \param epsg [in] EPSG projection code of data
/// \param depth_index [in] the index of dhand depth to be read, if applicable. DEFAULT VALUE = -1
//
void CModel::ReadNetCDFLayer(CNetCDFLayer *netcdf_obj, int ncid,
                             const std::string &var_name, int xsize, int ysize,
                             std::vector<double> x_coords,
                             std::vector<double> y_coords, std::string epsg,
                             int depth_index) {
  int varid;
  if (nc_inq_varid(ncid, var_name.c_str(), &varid) != NC_NOERR) {
    ExitGracefully(("Model.cpp: ReadNetCDFLayer: NetCDF file missing '" + var_name + "' variable.").c_str(),
                   exitcode::BAD_DATA);
  }

  netcdf_obj->xsize = xsize;
  netcdf_obj->ysize = ysize;
  netcdf_obj->x_coords = x_coords;
  netcdf_obj->y_coords = y_coords;
  netcdf_obj->epsg = epsg;

  // Read the datatype(e.g., nc_double, nc_float)
  if (nc_inq_vartype(ncid, varid, &netcdf_obj->datatype) != NC_NOERR) {
    ExitGracefully(("Model.cpp: ReadNetCDFFile: failed to read datatype of variable" + var_name).c_str(), exitcode::RUNTIME_ERR);
  }

  // Read data
  netcdf_obj->data = static_cast<double *>(
      CPLMalloc(sizeof(double) * netcdf_obj->xsize * netcdf_obj->ysize));
  if (netcdf_obj->datatype == NC_DOUBLE) { // reading double
    if (depth_index < 0) { // reading 2d
      if (nc_get_var_double(ncid, varid, netcdf_obj->data) != NC_NOERR) {
        ExitGracefully("Model.cpp: ReadNetCDFFile: failed to read 2d data", exitcode::RUNTIME_ERR);
      }
    } else { // reading slice of 3d
      size_t start[3] = {depth_index, 0, 0};
      size_t count[3] = {1, netcdf_obj->ysize, netcdf_obj->xsize};
      if (nc_get_vara_double(ncid, varid, start, count, netcdf_obj->data) != NC_NOERR) {
        ExitGracefully("Model.cpp: ReadNetCDFFile: failed to read slice of 3d data", exitcode::RUNTIME_ERR);
      }
    }
  } else if (netcdf_obj->datatype == NC_FLOAT) { // reading float and casting to double
    float *temp = static_cast<float *>(
        CPLMalloc(sizeof(float) * netcdf_obj->xsize * netcdf_obj->ysize));
    if (depth_index < 0) { // reading 2d
      if (nc_get_var_float(ncid, varid, temp) != NC_NOERR) {
        ExitGracefully("Model.cpp: ReadNetCDFFile: failed to read 2d data", exitcode::RUNTIME_ERR);
      }
    } else { // reading slice of 3d
      size_t start[3] = {depth_index, 0, 0};
      size_t count[3] = {1, netcdf_obj->ysize, netcdf_obj->xsize};
      if (nc_get_vara_float(ncid, varid, start, count, temp) != NC_NOERR) {
        ExitGracefully("Model.cpp: ReadNetCDFFile: failed to read slice of 3d data", exitcode::RUNTIME_ERR);
      }
    }
    for (size_t i = 0; i < netcdf_obj->xsize * netcdf_obj->ysize; i++) {
      netcdf_obj->data[i] = static_cast<double>(temp[i]);
    }
    CPLFree(temp);
  } else {
    ExitGracefully("Model.cpp: ReadNetCDFLayer: unsupported datatype", exitcode::BAD_DATA);
  }

  // Read _FillValue
  float fill_value;
  nc_type var_type;
  if (nc_inq_att(ncid, varid, "_FillValue", &var_type, nullptr) == NC_NOERR) {
    nc_get_att_float(ncid, varid, "_FillValue", &fill_value);
    netcdf_obj->na_val = static_cast<double>(fill_value);
  } else {
    netcdf_obj->na_val = std::numeric_limits<double>::quiet_NaN();
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Reads specified Raster file
/// \param filename [in] full path to raster file to read from
/// \param raster_obj [in] pointer to CRaster object to write to
//
void CModel::ReadRasterFile(std::string filename, CRaster *raster_obj) {
  CPLPushErrorHandler(SilentErrorHandler);
  GDALDataset *dataset =
      static_cast<GDALDataset *>(GDALOpen(filename.c_str(), GA_ReadOnly));
  if (dataset == nullptr) {
    dataset = static_cast<GDALDataset *>(GDALOpen((filename + "f").c_str(), GA_ReadOnly));
  }
  CPLPopErrorHandler();
  ExitGracefullyIf(
      dataset == nullptr,
      ("Model.cpp: ReadRasterFile: couldn't open " + filename + " or " + filename + "f").c_str(),
      exitcode::FILE_OPEN_ERR);

  raster_obj->xsize = dataset->GetRasterXSize();
  raster_obj->ysize = dataset->GetRasterYSize();
  if (dataset->GetProjectionRef() != nullptr) {
    size_t len = strlen(dataset->GetProjectionRef()) + 1;
    raster_obj->proj = new char[len];
    memcpy(raster_obj->proj, dataset->GetProjectionRef(), len);
  } else {
    raster_obj->proj = nullptr;
  }
  dataset->GetGeoTransform(raster_obj->geotrans);

  raster_obj->data = static_cast<double *>(
      CPLMalloc(sizeof(double) * raster_obj->xsize * raster_obj->ysize));
  GDALRasterBand *band = dataset->GetRasterBand(1);
  raster_obj->datatype = band->GetRasterDataType();
  raster_obj->na_val = band->GetNoDataValue();
  band->RasterIO(GF_Read, 0, 0, raster_obj->xsize, raster_obj->ysize,
                 raster_obj->data, raster_obj->xsize, raster_obj->ysize,
                 GDT_Float64, 0, 0);
  GDALClose(dataset);
}

//////////////////////////////////////////////////////////////////
/// \brief Reads specified Vector file
/// \param filename [in] full path to vector file to read from
/// \param raster_obj [in] pointer to CVector object to write to
//
void CModel::ReadVectorFile(std::string filename, CVector &vector_obj) {
  CPLPushErrorHandler(SilentErrorHandler);
  GDALDataset *dataset = static_cast<GDALDataset *>(
      GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
  CPLPopErrorHandler();
  ExitGracefullyIf(
      dataset == nullptr,
      ("Model.cpp: ReadVectorFile: couldn't open " + filename).c_str(),
      exitcode::FILE_OPEN_ERR);

  OGRLayer *layer = dataset->GetLayer(0);
  if (layer == nullptr) {
    GDALClose(dataset);
    ExitGracefully(
        ("Model.cpp: ReadVectorFile: couldn't get layer in " + filename) .c_str(),
        exitcode::BAD_DATA);
  }
  
  vector_obj.spat_ref = layer->GetSpatialRef() ? layer->GetSpatialRef()->Clone() : nullptr;
  vector_obj.geom_type = layer->GetGeomType();

  OGRFeatureDefn *feature_defn = layer->GetLayerDefn();
  for (int i = 0; i < feature_defn->GetFieldCount(); i++) {
    vector_obj.field_defs.push_back(new OGRFieldDefn(feature_defn->GetFieldDefn(i)));
    vector_obj.add_to_field_def_map(vector_obj.field_defs.back()->GetNameRef(), i);
  }

  layer->ResetReading();
  OGRFeature *feature;
  while ((feature = layer->GetNextFeature()) != nullptr) {
    OGRFeature *feature_copy = OGRFeature::CreateFeature(feature_defn);
    feature_copy->SetGeometry(feature->GetGeometryRef());
    feature_copy->SetFrom(feature);
    vector_obj.features.push_back(feature_copy);
    OGRFeature::DestroyFeature(feature);
  }

  GDALClose(dataset);
}

//////////////////////////////////////////////////////////////////
/// \brief Postprocesses flood results based on method defined in bbopt settings
//
void CModel::postprocess_floodresults() {
  if (bbopt->interpolation_postproc_method == enum_ppi_method::NONE) {
    return;
  }
  ExitGracefullyIf(!c_from_s || !c_from_s->data,
                   "Raster.cpp: postprocess_floodresults: catchments from "
                   "streamnodes missing",
                   exitcode::RUNTIME_ERR);
  ExitGracefullyIf(
      !hyd_result,
      "Raster.cpp: postprocess_floodresults: hydraulic output missing",
      exitcode::RUNTIME_ERR);
  
  // loop for each flow_profiles
  for (int flow_ind = 0; flow_ind < bbsn->front()->output_flows.size(); flow_ind++) {
    if (!bbopt->silent_run) {
      std::cout << "post processing flood results for flow " + std::to_string(flow_ind + 1) << std::endl;
    }

    if (bbopt->interpmethod == enum_interp_method::POURPOINT_SHP) {
    
            switch (bbopt->interpolation_postproc_method)
            {
            case (enum_ppi_method::CATCHMENT_HAND):
            {
              generate_out_gridded(flow_ind, false, false);
              break;
            }
            case (enum_ppi_method::INTERP_HAND):
            {
              generate_spp_depths(flow_ind);
              generate_out_gridded(flow_ind, true, false);
              break;
            }
            case (enum_ppi_method::CATCHMENT_DHAND):
            {
              generate_dhand_vals(flow_ind, false);
              generate_out_gridded(flow_ind, false, true);
              break;
            }
            case (enum_ppi_method::INTERP_DHAND):
            {
              generate_spp_depths(flow_ind);
              generate_dhand_vals(flow_ind, true);
              generate_out_gridded(flow_ind, true, true);
              break;
            }
    
            //case (enum_ppi_method::INTERP_DHAND_WSLCORR):
            //{
            //  break;
            //}
            default:
            {
              ExitGracefully(
                  ("Model.cpp: postprocess_floodresults: postprocessing method " +
                   toString(bbopt->interpolation_postproc_method) +
                   " is not yet available")
                      .c_str(),
                  exitcode::BAD_DATA);
              break;
            }
        } //end switch
    
    } else {
      // enum_ppi_method postprocmethod = ;
      // use updated method that just relies on the INTERP_RASTER, simplifies quite a bit
      generate_out_gridded2(flow_ind, bbopt->interpolation_postproc_method);
    }

    spp_depths.clear(); // if applicable, clear spp_depths for next flow profile
    dhand_vals.clear(); // if applicable, clear dhand_vals for next flow profile
  }
  if (!bbopt->silent_run) {
    std::cout << "finished post processing flood results" << std::endl;
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Recursively computes hyd profile for the tree with downstream most streamnode "sn"
/// \note if sn and down_sn are identical, it is assumed they are the boundary condition streamnode
/// \param sn [in] streamnode for which to compute hydraulic profile
/// \param down_sn [in] streamnode one node downstream of sn
/// \param res [in/out] object to add output to when done computing
/// \param bc [in] boundary condition for current set of streamnodes
//
void CModel::compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res, CBoundaryCondition *&bc) {
  if (!bbopt->silent_run) {
    std::cout << "Computing profile for streamnode with node id " << std::to_string(sn->nodeID) << std::endl;
  }

  static constexpr double FLOW_TOL = 1e-6;

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

  bool using_exhaustive = false; // by default, not using exhaustive solution until triggered
    
  if (sn->nodeID == bc->nodeID) { // boundary condition streamnode
    if (bbopt->modeltype == enum_mt_method::HAND_MANNING) {
      sn->mm->wsl = sn->compute_normal_depth(sn->mm->flow, sn->mm->bed_slope, -99, bbopt);
    } else { // bbopt->modeltype != enum_mt_method::HAND_MANNING
      // estimate first streamnode from supplied boundary conditions
      switch (bc->bctype)
      {
      case (enum_bc_type::NORMAL_DEPTH): {
        if (bc->bcvalue <= 0 || bc->bcvalue > 1) {
          WriteWarning("Model.cpp: compute_streamnode: bcvalue may not be a "
                       "reasonable slope, please check!",
                       bbopt->noisy_run);
        }
        sn->mm->wsl = sn->compute_normal_depth(sn->mm->flow, bc->bcvalue, bc->init_WSL, bbopt);
        sn->mm->sf = bc->bcvalue; // override sf from preproc
        break;
      }
      case (enum_bc_type::SET_WSL): {
        sn->mm->wsl = bc->bcvalue;
        ExitGracefullyIf(sn->mm->wsl < sn->mm->min_elev,
                         "Model.cpp: compute_streamnode: SET_WSL used as "
                         "boundary condition but value provided is less than "
                         "minimum elevation, revise boundary condition!",
                         BAD_DATA);
        break;
      }
      case (enum_bc_type::SET_DEPTH): {
        sn->mm->wsl = bc->bcvalue + sn->mm->min_elev;
        ExitGracefullyIf(bc->bcvalue < 0,
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
      //sn->mm->wsl = down_sn->mm->depth + sn->mm->min_elev; // best guess at upstream WSL, same depth applied to bottom bed elevation
      sn->mm->depth = down_sn->mm->depth;
      sn->mm->wsl = sn->mm->depth + sn->mm->min_elev;

      if (sn->mm->flow <= FLOW_TOL) {
        sn->mm->flow = 0.0;
        sn->mm->depth = 0.0;
        sn->mm->wsl = sn->mm->min_elev;

        sn->compute_profile(sn->mm->flow, sn->mm->wsl, bbopt);

        // Zero all hydraulic properties
        sn->mm->velocity = 0.0;
        sn->mm->velocity_head = 0.0;
        sn->mm->pressure_head = 0.0;
        sn->mm->energy_total = sn->mm->wsl;
        sn->mm->froude = 0.0;
        sn->mm->sf = 0.0;

        sn->mm->k_total = 0.0;
        sn->mm->alpha = 0.0;
        sn->mm->area = 0.0;
        sn->mm->hradius = 0.0;
        sn->mm->wet_perimeter = 0.0;
        sn->mm->manning_composite = 0.0;
        sn->mm->length_effective = 0.0;
        sn->mm->length_effectiveadjusted = 0.0;
        sn->mm->hyd_depth = 0.0;
        sn->mm->top_width = 0.0;

        sn->mm->k_total_areaconv = 0.0;
        sn->mm->k_total_disconv = 0.0;
        sn->mm->k_total_roughconv = 0.0;

        sn->mm->alpha_areaconv = 0.0;
        sn->mm->alpha_disconv = 0.0;
        sn->mm->alpha_roughconv = 0.0;

        sn->mm->nc_equalforce = 0.0;
        sn->mm->nc_equalvelocity = 0.0;
        sn->mm->nc_wavgwp = 0.0;
        sn->mm->nc_wavgarea = 0.0;
        sn->mm->nc_wavgconv = 0.0;
        sn->mm->nc_invmanning = 0.0;

        // Peak hours required = 0 for zero flow
        sn->mm->peak_hrs_required = 0.0;

        // Assign output safely
        (*res)[flow * bbsn->size() + ind] = new hydraulic_output(*(sn->mm));
        sn->output_depths[flow] = sn->mm->depth;
        sn->output_wsls[flow] = sn->mm->wsl;

        // No profile computation needed
        return;
      } else {
          if (bbopt->solvermethod == enum_sm_method::BRENT) {

              /* if (bbopt->noisy_run) {
                std::cout << "Using Brent method to solve for depth at streamnode "
                        << std::to_string(sn->nodeID) << std::endl;
              }*/

              // compute critical depth
              // // xxx to resolve - cannot seem to get the derivative version of critical_wsl_brent working in any way. Reverting to exhaustive search for now.
              // double wsl_critical = solve_critical_wsl_brent(sn, down_sn);
              // double wsl_critical = solve_critical_wsl_brent_analytical(sn, down_sn);
              // double wsl_critical = solve_critical_wsl_brent(sn, down_sn);   
              double wsl_critical = solve_critical_wsl_exhaustive(sn, down_sn);                         

              // convert to depth
              double depth_critical = wsl_critical - sn->mm->min_elev;
              sn->mm->depth_critical = depth_critical;

              // checks for critical depth ~= 0
              if (depth_critical < 0.01) {
                WriteWarning("Estimated critical depth at streamnode " + std::to_string(sn->nodeID) + " is " + std::to_string(depth_critical) + 
                    ", which is very close or at zero. This may indicate instability in results or errors in the inputs.",
                     bbopt->noisy_run);
              }

              // calculate wsl_estimated using Brent method
              double wsl_estimated = solve_wsl_standard_step_brent(
                    sn,          // upstream node
                    down_sn,     // downstream node
                    wsl_critical // critical depth used to bound the solution space for the Brent method
                );

              // set depth in sn based on wsl_estimated from the Brent method
              sn->mm->wsl = wsl_estimated;
              sn->mm->depth_critical = wsl_critical - sn->mm->min_elev;
              sn->mm->depth          = sn->mm->wsl - sn->mm->min_elev;
              sn->compute_profile_next(sn->mm->flow, sn->mm->wsl, down_sn->mm, bbopt);
              double wsl_error = sn->get_wsl_error(sn->mm->wsl, down_sn->mm, bbopt);
              sn->mm->ws_err = wsl_error;
              sn->mm->k_err = sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);

              // perform checks on the results

              // check if wsl_estimated is at the upper bound
              if (sn->mm->depth >= sn->depthdf->back()->depth*0.99) {
              WriteWarning(
                          "Estimated depth at streamnode " + std::to_string(sn->nodeID) + " is " + std::to_string(sn->mm->depth) + "m, which is at the available upper bound. The preprocesed table max depth may need to be increased.",
                          bbopt->noisy_run);
              }
              // check the wsl_error from wsl determined by Brent, and report if it exceeds threshold
              /* if (wsl_error > bbopt->tolerance_cp) {
                WriteAdvisory(
                          "WSL Error from brent method at streamnode " + std::to_string(sn->nodeID) + " is " + std::to_string(wsl_error) + "m, which is greater than tolerance.",
                          bbopt->noisy_run);
              }
              */
              // check froude number
              if (sn->mm->froude > bbopt->froude_threshold) {
              WriteAdvisory(
                          "Froude number at streamnode " + std::to_string(sn->nodeID) + " is " + std::to_string(sn->mm->froude) + ", which indicates that the regime here may be supercritical.",
                          bbopt->noisy_run);
              }
      
          } else if (bbopt->solvermethod == enum_sm_method::SECANT) {

              double err_lag1 = PLACEHOLDER, err_lag2 = PLACEHOLDER,
                 prevWSL_lag1 = PLACEHOLDER, prevWSL_lag2 = PLACEHOLDER,
                 min_err_wsl = PLACEHOLDER, min_err = PLACEHOLDER,
                 actual_err = PLACEHOLDER, min_fr = PLACEHOLDER,
                 proposed_wsl = PLACEHOLDER;
          bool found_supercritical = false; // reset before every i iteration, use in iteration as needed

          for (int i = 0; i < bbopt->iteration_limit_cp; i++) {
            prevWSL_lag2 = prevWSL_lag1;
            prevWSL_lag1 = sn->mm->wsl;
            sn->compute_profile_next(sn->mm->flow, sn->mm->wsl, down_sn->mm, bbopt);

            double max_depth_change = std::max(0.5 * sn->mm->depth, 0.5);
            // xxx adjsut by adding pressure head?
            double comp_wsl = down_sn->mm->wsl + down_sn->mm->velocity_head +
                              sn->mm->head_loss - sn->mm->velocity_head;

            // checks/modifications against min_elev
            if (comp_wsl <= sn->mm->min_elev) {
              comp_wsl =
                  sn->mm->flow == 0
                      ? sn->mm->min_elev
                      : std::max(comp_wsl,
                                 sn->mm->min_elev + 0.05 +
                                     0.05 *
                                         (1 - (i + 1) / bbopt->iteration_limit_cp));
            }

            err_lag2 = err_lag1;
            err_lag1 = comp_wsl - prevWSL_lag1;
            double err_diff =
                err_lag2 == PLACEHOLDER ? PLACEHOLDER : err_lag2 - err_lag1;
            double assum_diff = prevWSL_lag2 == PLACEHOLDER
                                    ? PLACEHOLDER
                                    : prevWSL_lag2 - prevWSL_lag1;

            if (min_err == PLACEHOLDER || std::abs(err_lag1) < min_err) {
              min_err = std::abs(err_lag1);
              actual_err = err_lag1;
              min_err_wsl = prevWSL_lag1;
              min_fr = sn->mm->froude;
            }

            sn->mm->ws_err = err_lag1;
            sn->mm->k_err = sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);
            sn->mm->cp_iterations = i + 1;

            if (using_exhaustive) {
              err_lag1 =
                  0.0; // set to zero since using exhaustive solution, exit loop
            }

            if (std::abs(err_lag1) > bbopt->tolerance_cp) {
              if (i + 1 == bbopt->iteration_limit_cp) {
                WriteWarning(
                    "compute_profile(): iteration limit reached on streamnode" +
                        sn->nodeID,
                    bbopt->noisy_run);

                // setting to min error result
                sn->compute_profile_next(sn->mm->flow, min_err_wsl, down_sn->mm,
                                         bbopt);
                sn->mm->ws_err = actual_err;
                sn->mm->k_err =
                    sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);

                if (min_err < 0.1 && sn->mm->froude <= bbopt->froude_threshold) {
                  // keeping the min err result
                  if (!bbopt->silent_run) {
                    std::cout << "setting to min error result on streamnode "
                              << std::to_string(sn->nodeID) << std::endl;
                  }
                } else {
                  if (!bbopt->silent_run) {
                    std::cout << "need to check crit depth" << std::endl;
                  }
                  // optimization

                  double initguess = sn->mm->min_elev + down_sn->mm->depth_critical;
                  if (down_sn->mm->depth_critical <= 0) {
                    // if downstream critical depth is not available or <= 0, use downstream depth as initial guess instead
                    initguess = sn->mm->min_elev + down_sn->mm->depth;
                  }

                  double wsl_critical = brent_minimize(
                      sn->mm->min_elev+0.001,
                      sn->mm->min_elev + sn->depthdf->back()->depth, 
                      initguess, // initial guess based on downstream depth_critical
                      [&](double x) {
                        auto temp_sn = std::make_unique<CStreamnode>(*sn);
                        return temp_sn->get_total_energy(x, down_sn->mm, bbopt);
                      });

                  if (wsl_critical >= sn->mm->min_elev &&
                      wsl_critical <=
                          sn->depthdf->back()->depth) { // if optimization worked
                    double depth_critical = wsl_critical - sn->mm->min_elev;
                    sn->mm->depth_critical = depth_critical;
                    sn->compute_profile_next(
                        sn->mm->flow, sn->mm->min_elev + sn->mm->depth_critical,
                        down_sn->mm, bbopt);
                    sn->mm->ws_err = PLACEHOLDER;
                    sn->mm->k_err =
                        sn->mm->flow - sn->mm->k_total * std::sqrt(sn->mm->sf);
                    if (!bbopt->silent_run) {
                      std::cout << "setting to critical depth result on streamnode "
                                << std::to_string(sn->nodeID) << std::endl;
                    }
                  } else {
                    if (!bbopt->silent_run) {
                      std::cout << "Failed to get critical depth at streamnode "
                                << std::to_string(sn->nodeID)
                                << ", keeping lowest err result" << std::endl;
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
                  sn->mm->wsl =
                      sn->mm->wsl +
                      std::copysign(max_depth_change, proposed_wsl - sn->mm->wsl);
                } else {
                  sn->mm->wsl = proposed_wsl;
                }
              } else {
                if (std::abs(err_diff) < 0.03 ||
                    i + 1 > bbopt->iteration_limit_cp / 2. || i >= 5) {
                  // if enable_exhaustive, then use solveddepth (or critdepth if
                  // solvedfroude < froude_threshold)
                  if (bbopt->enable_exhaustive) {
                    if (!bbopt->silent_run) {
                      std::cout << "using exhaustive solution at node "
                                << std::to_string(sn->nodeID) << std::endl;
                    }

                    using_exhaustive = true;
                    ExhaustiveWSLResult res = solve_wsl_exhaustive(sn, down_sn);

                    // store results
                    sn->mm->wsl            = res.wsl_estimated;
                    sn->mm->depth_critical = res.wsl_critical - sn->mm->min_elev;
                    sn->mm->depth          = sn->mm->wsl - sn->mm->min_elev;
                    sn->compute_profile_next(sn->mm->flow, sn->mm->wsl, down_sn->mm, bbopt);

                  } else {
                    // for small error differences, secant method can fail
                    // take average of prevWSL_lag1 and prevWSL_lag2
                    double proposed_wsl = (comp_wsl + prevWSL_lag1) / 2.;
                    // change by half of max_depth_change in direction of comp_WSL
                    // if massive swing
                    if (std::abs(comp_wsl - prevWSL_lag1) > max_depth_change) {
                      proposed_wsl =
                          prevWSL_lag1 + std::copysign(0.5 * max_depth_change,
                                                       comp_wsl - prevWSL_lag1);
                    }
                    sn->mm->wsl = proposed_wsl;
                  }
                } else {
                  // secant method
                  double proposed_wsl =
                      prevWSL_lag2 - err_lag2 * assum_diff / err_diff;
                  if (std::abs(proposed_wsl - sn->mm->wsl) > max_depth_change) {
                    sn->mm->wsl =
                        sn->mm->wsl +
                        std::copysign(max_depth_change, proposed_wsl - sn->mm->wsl);
                  } else {
                    sn->mm->wsl = proposed_wsl;
                  }
                }
              }
            } else {
              if (sn->mm->froude <= bbopt->froude_threshold || using_exhaustive) {
                if (!bbopt->silent_run) {
                  std::cout << "Iterated on WSL at streamnode "
                            << std::to_string(sn->nodeID)
                            << " within tolerance on iteration " << i << std::endl;
                }
                break;
              } else {
                if (!bbopt->silent_run) {
                  std::cout << "need to check crit depth" << std::endl;
                }

                double initguess = sn->mm->min_elev + down_sn->mm->depth_critical;
                  if (down_sn->mm->depth_critical <= 0) {
                    // if downstream critical depth is not available or <= 0, use downstream depth as initial guess instead
                    initguess = sn->mm->min_elev + down_sn->mm->depth;
                  }

                double wsl_critical = brent_minimize(
                    sn->mm->min_elev+0.001, 
                    sn->mm->min_elev + sn->depthdf->back()->depth,
                    initguess, // initial guess based on downstream depth_critical
                    [&](double x) {
                      auto temp_sn = std::make_unique<CStreamnode>(*sn);
                      return temp_sn->get_total_energy(x, down_sn->mm, bbopt);
                    });

                if (wsl_critical >= sn->mm->min_elev &&
                    wsl_critical <=
                        sn->depthdf->back()->depth) { // if optimization worked
                  double depth_critical = wsl_critical - sn->mm->min_elev;
                  sn->mm->depth_critical = depth_critical;
                  if (sn->mm->depth < sn->mm->depth_critical) {
                    if (found_supercritical) {
                      sn->compute_profile_next(
                          sn->mm->flow, sn->mm->min_elev + sn->mm->depth_critical,
                          down_sn->mm, bbopt);
                      if (!bbopt->silent_run) {
                        std::cout << "setting to supercritical" << std::endl;
                      }
                      break;
                    } else {
                      if (!bbopt->silent_run) {
                        std::cout << "first time with supercritical, resetting"
                                  << std::endl;
                      }
                      sn->compute_profile_next(sn->mm->flow,
                                               sn->mm->min_elev +
                                                   sn->mm->depth_critical + 1,
                                               down_sn->mm, bbopt);
                      i = 0;
                      min_err = PLACEHOLDER;
                    }
                    found_supercritical = true;
                  } else {
                    if (sn->mm->froude > 1) {
                      WriteWarning("Depth found to not be supercritical even "
                                   "though Froude is >1 at streamnode " +
                                       sn->nodeID,
                                   bbopt->noisy_run);
                    }
                    break;
                  }
                } else {
                  WriteWarning(
                      "Failed to converge in critical depth calculation for "
                      "streamnode " +
                          std::to_string(sn->nodeID) +
                          ". Leaving depth as is, which may be supercritical",
                      bbopt->noisy_run);
                  break;
                }
              }
            }
          }
        } else if (bbopt->solvermethod == enum_sm_method::EXHAUSTIVE) {
            if (!bbopt->silent_run) {
                std::cout << "using exhaustive solution at node "
                          << std::to_string(sn->nodeID) << std::endl;
            }
            using_exhaustive = true;
            ExhaustiveWSLResult res = solve_wsl_exhaustive(sn, down_sn);

            // store results
            sn->mm->wsl            = res.wsl_estimated;
            sn->mm->depth_critical = res.wsl_critical - sn->mm->min_elev;
            sn->mm->depth          = sn->mm->wsl - sn->mm->min_elev;
            sn->compute_profile_next(sn->mm->flow, sn->mm->wsl, down_sn->mm, bbopt);
        } else {
        ExitGracefully(
            "Model.cpp: compute_streamnode: error in solver method input type",
            BAD_DATA);
        }
      }

      // ensure hydraulic_output is ALWAYS written
      int out_index = flow * bbsn->size() + ind;

      if ((*res)[out_index] == nullptr) {
        (*res)[out_index] = new hydraulic_output(*(sn->mm));
      } else {
        *(*res)[out_index] = *(sn->mm);
      }

      sn->output_depths[flow] = sn->mm->depth;
      sn->output_wsls[flow] = sn->mm->wsl;
    }
  }

  // check that results are within the Hseq bounds
  // with updated Brent method, depth will be restricted to upper bound already
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

  if (sn->mm->flow <= FLOW_TOL ||
    sn->mm->area <= 0.0 ||
    sn->mm->length_effectiveadjusted <= 0.0) {

    sn->mm->peak_hrs_required = 0.0;

    (*res)[flow * bbsn->size() + ind] = new hydraulic_output(*(sn->mm));
    sn->output_depths[flow] = sn->mm->depth;
    sn->output_wsls[flow] = sn->mm->wsl;

    return;
  }

  // compute peak hours required and update min and max accordingly
  //sn->mm->peak_hrs_required = (sn->mm->area * sn->mm->length_effectiveadjusted / sn->mm->flow) / 3600;
  sn->compute_peak_hours();
  if (peak_hrs_min == PLACEHOLDER || sn->mm->peak_hrs_required < peak_hrs_min) {
    peak_hrs_min = sn->mm->peak_hrs_required;
  }
  if (peak_hrs_max == PLACEHOLDER || sn->mm->peak_hrs_required > peak_hrs_max) {
    peak_hrs_max = sn->mm->peak_hrs_required;
  }

  // assign output to its designated location
  int out_index = flow * bbsn->size() + ind;
  (*res)[out_index] = new hydraulic_output(*(sn->mm));
  sn->output_depths[flow] = sn->mm->depth;
  sn->output_wsls[flow] = sn->mm->wsl;


  //if (bbopt->debug_mode) {
  //  std::cout << "Streamnode: " << to_string(sn->mm->nodeID)
  //            << " depth: " << to_string(sn->mm->depth)
  //            << " WSL: " << to_string(sn->mm->wsl)
  //            << " min_elev: " << to_string(sn->mm->min_elev) 
  //            << " area: " << to_string(sn->mm->area)
  //            << " velocity:" << to_string(sn->mm->velocity)
  //            << " flow: " << to_string(sn->mm->flow)
  //            << " sf: " << to_string(sn->mm->sf)
  //            << " Fr: " << to_string(sn->mm->froude)
  //            << std::endl;
  //}

  // writes the model out with each iteration if in DebugMode
  /*
  if (bbopt->debug_mode) {
    std::ofstream TESTOUTPUT;
    // remove if file exists
    std::string fname = FilenamePrepare("Blackbird_testoutput.txt").c_str();
    std::cout << "Writing model snapshot to file " << fname << std::endl;
    // std::cout << "file name fname is " << fname << std::endl;
    if (std::filesystem::exists(fname)) {
      std::cout << "Deleting current model snapshot"  << std::endl;
      std::filesystem::remove(fname);
    }
    TESTOUTPUT.open(fname);
    if (TESTOUTPUT.fail()) {
      ExitGracefully("Main::Unable to open Blackbird_testoutput.txt. Bad output directory specified?", BLACKBIRD_OPEN_ERR);
    }
    TESTOUTPUT.close();
    std::cout << "Writing the WriteFullModel to output " << fname << std::endl;
    WriteFullModel(); // writes full model to test output
    hyd_result_pretty_print(); // writes hydraulic result to test output  
  }
  */

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
}

ExhaustiveWSLResult CModel::solve_wsl_exhaustive(
    const CStreamnode* sn_up,
    const CStreamnode* sn_down
)
{
    const std::size_t n = sn_up->depthdf->size();
    std::vector<double> depthsv(n);
    std::vector<double> velocities(n);
    std::vector<double> velheads(n);
    std::vector<double> energies(n);
    std::vector<double> sf(n);
    std::vector<double> sfbar(n);
    std::vector<double> losscoeff(n);
    std::vector<double> headloss(n);
    std::vector<double> deptherr(n);
    std::vector<double> fr(n);
    
    for (std::size_t i = 0; i < n; ++i) {
        hydraulic_output* row = (*sn_up->depthdf)[i];

        depthsv[i] = row->depth;
        velocities[i] = std::max(sn_up->mm->flow / row->area, 0.0);
        velheads[i] = velocities[i] * velocities[i] / (2.0 * GRAVITY) * row->alpha;
        energies[i] = sn_up->mm->min_elev + depthsv[i] + velheads[i];
        sf[i]    = std::pow(sn_up->mm->flow / row->k_total, 2.0);
        sfbar[i] = (sf[i] + sn_down->mm->sf) / 2.0;
        losscoeff[i] = sn_up->expansion_coeff;
        if (sn_down->mm->velocity_head > velheads[i]) {
            losscoeff[i] = sn_up->contraction_coeff;
        }
        // xxx integrate pressure head?
        headloss[i] =
            row->length_effective * sfbar[i] +
            losscoeff[i] * std::abs(velheads[i] - sn_down->mm->velocity_head);
        fr[i] = velocities[i] / std::sqrt(GRAVITY * depthsv[i]);
        deptherr[i] = std::abs(
            depthsv[i] + row->min_elev
            - sn_down->mm->depth
            - sn_down->mm->min_elev
            - sn_down->mm->velocity_head
            - headloss[i]
            + velheads[i]
        );
    }

    // depth with minimum depth error -> "solved" depth
    auto it = std::min_element(
        deptherr.begin(), deptherr.end(),
        [](double a, double b) {
            if (std::isnan(a)) return false;
            if (std::isnan(b)) return true;
            return a < b;
        }
    );
    std::size_t index_solved = std::distance(deptherr.begin(), it);
    double solveddepth   = depthsv[index_solved];
    double solvedfroude  = fr[index_solved];

    // depth with minimum energy -> critical depth
    auto itE = std::min_element(
        energies.begin(), energies.end(),
        [](double a, double b) {
            if (std::isnan(a)) return false;
            if (std::isnan(b)) return true;
            return a < b;
        }
    );
    std::size_t index_crit = std::distance(energies.begin(), itE);
    double critdepth = depthsv[index_crit];

    if (solvedfroude >= bbopt->froude_threshold) {
        sn_up->mm->wsl = sn_up->mm->min_elev + solveddepth;
    } else {
        sn_up->mm->wsl = sn_up->mm->min_elev + critdepth;
    }
    sn_up->mm->depth_critical = critdepth;

    double wsl_critical  = sn_up->mm->min_elev + critdepth;
    double wsl_estimated = sn_up->mm->min_elev +
                           ((solvedfroude >= bbopt->froude_threshold)
                                ? solveddepth
                                : critdepth);
    /*
    std::cout << "Exhaustive WSL solver results for streamnode " << sn_up->nodeID << ": "
              << "wsl_critical = " << wsl_critical << ", "
              << "wsl_estimated = " << wsl_estimated 
        << "depth = " << wsl_estimated-sn_up->min_elev << std::endl;
    */

    return ExhaustiveWSLResult{ wsl_critical, wsl_estimated };
}

//////////////////////////////////////////////////////////////////
/// \brief Solver for critical depth using brent method
/// \note used in the compute_streamnode for BRENT method (superseded)
/// \param sn_up [in] streamnode for which to compute hydraulic profile
/// \param sn_down [in] streamnode one node downstream of sn
/// \output wsl_critical [out] estimated water surface level at sn_up that satisfies energy balance with sn_down
//
double CModel::solve_critical_wsl_brent(
    const CStreamnode* sn_up, 
    const CStreamnode* sn_down
)
{
    const double bed = sn_up->mm->min_elev;

    // Initial guess: downstream critical depth if available
    double initguess = bed + sn_down->mm->depth_critical;
    if (sn_down->mm->depth_critical <= 0.0) {
        initguess = bed + sn_down->mm->depth;   // fallback to downstream depth
    }

    // Numerical derivative of total energy: dE/dH = 0 at critical depth
    auto dEdx = [&](double H) {
        const double h = 1e-4;
        auto s1 = std::make_unique<CStreamnode>(*sn_up);
        auto s2 = std::make_unique<CStreamnode>(*sn_up);
        double Eplus  = s1->get_total_energy(H + h, sn_down->mm, bbopt);
        double Eminus = s2->get_total_energy(H - h, sn_down->mm, bbopt);
        return (Eplus - Eminus) / (2.0 * h);
    };

    // Bounds for root search
    double lower = bed + 0.001;                         // slightly above bed
    //double upper = bed + sn_up->depthdf->back()->depth;   // max tabulated depth
    double upper = bed + std::min(sn_up->depthdf->back()->depth, 5.0); // set max as 5m

    // Solve dE/dH = 0
    double wsl_critical = brent_root2(lower, upper, dEdx, 1e-8, 100);

    return wsl_critical;
}

//////////////////////////////////////////////////////////////////
/// \brief Solver for critical depth using brent method with analytical derivative
/// \note used in the compute_streamnode for BRENT method
/// \param sn_up [in] streamnode for which to compute hydraulic profile
/// \param sn_down [in] streamnode one node downstream of sn
/// \output wsl_critical [out] estimated water surface level at sn_up that satisfies energy balance with sn_down
//
double CModel::solve_critical_wsl_brent_analytical(
    const CStreamnode* sn_up,
    const CStreamnode* sn_down
)
{
    const double bed = sn_up->mm->min_elev;
    const double Q   = sn_up->mm->flow;

    // ---------------------------------------------------------
    // Derivative of specific energy with respect to DEPTH
    // ---------------------------------------------------------
    auto dEdx = [&](double depth)
    {
        if (depth <= 0.0)
            return 1.0;   // derivative positive near bed
        double A     = sn_up->get_area(depth);
        double Tw    = sn_up->get_topwidth(depth);
        double alpha = sn_up->get_alpha(depth);
        double term = alpha * (Q * Q * Tw) / (GRAVITY * A * A * A);
        return 1.0 - term;
    };

    // ---------------------------------------------------------
    // Lower bound (depth above bed)
    // ---------------------------------------------------------
    double lower = 0.001;   // depth, not elevation
    double dE_lower = dEdx(lower);

    // ---------------------------------------------------------
    // Find smallest upper bound where sign changes
    // ---------------------------------------------------------
    const double step      = 0.05;
    double upper = lower + step;
    double dE_upper = dEdx(upper);
    const double max_upper = 6.0;   // max depth to search

    while (dE_lower * dE_upper > 0.0 && upper < max_upper)
    {
        upper += step;
        dE_upper = dEdx(upper);
    }

    // If no sign change found → monotonic specific energy curve
    if (dE_lower * dE_upper > 0.0)
    {
        return bed + 0.1;   // fallback WSL
    }

    // ---------------------------------------------------------
    // Solve using Brent on the minimal bracket (depth)
    // ---------------------------------------------------------
    double depth_critical = brent_root2(lower, upper, dEdx, 1e-8, 100);

    // Avoid zero depth
    depth_critical = std::max(depth_critical, 0.03);

    // Convert depth → water surface elevation
    double wsl_critical = bed + depth_critical;

    return wsl_critical;
}

//////////////////////////////////////////////////////////////////
/// \brief Solver for critical depth using iterative exhaustive search
/// \note used in the compute_streamnode for BRENT method
/// \param sn_up [in] streamnode for which to compute hydraulic profile
/// \param sn_down [in] streamnode one node downstream of sn
/// \output wsl_critical [out] estimated water surface level at sn_up that satisfies energy balance with sn_down
//
double CModel::solve_critical_wsl_exhaustive(
    const CStreamnode* sn_up,
    const CStreamnode* sn_down
)
{
    const double bed = sn_up->mm->min_elev;
    const double Q   = sn_up->mm->flow;

    const auto& df = *sn_up->depthdf;   // preprocessed depth table
    if (df.empty())
        return bed + 0.1;

    // ---------------------------------------------------------
    // Specific energy function E(depth)
    // ---------------------------------------------------------
    auto E = [&](double depth)
    {
        if (depth <= 0.0)
            return std::numeric_limits<double>::infinity();

        double A     = sn_up->get_area(depth);
        // double Tw    = sn_up->get_topwidth(depth);
        double alpha = sn_up->get_alpha(depth);
        double v  = Q / A;
        double ke = alpha * v * v / (2.0 * GRAVITY);
        return depth + ke;   // E = y + α v² / (2g)
    };

    // ---------------------------------------------------------
    // Step 1: Find first local minimum in depthdf
    // ---------------------------------------------------------
    double best_depth = df.front()->depth;
    double best_E     = E(best_depth);

    for (size_t i = 1; i < df.size(); ++i)
    {
        double d  = df[i]->depth;
        double Ei = E(d);

        if (Ei < best_E)
        {
            best_E     = Ei;
            best_depth = d;
        }
        else
        {
            // Energy increased → stop scanning
            break;
        }
    }

    // ---------------------------------------------------------
    // Step 2: Manual refinement around the minimum
    // ---------------------------------------------------------
    double refine_step = 0.01;   // 1 cm refinement
    double search_lo   = std::max(0.001, best_depth - 0.20);
    double search_hi   = best_depth + 0.20;

    for (double depth = search_lo; depth <= search_hi; depth += refine_step)
    {
        double Ei = E(depth);
        if (Ei < best_E)
        {
            best_E     = Ei;
            best_depth = depth;
        }
    }

    // ---------------------------------------------------------
    // Final critical depth → convert to WSL
    // ---------------------------------------------------------
    double depth_critical = std::max(best_depth, 0.03); // prevent zero critical depth
    double wsl_critical   = bed + depth_critical;

    return wsl_critical;
}

//////////////////////////////////////////////////////////////////
/// \brief Linearly interpolates to get the cross-sectional area at a given
/// water surface level H
/// \note used in the CModel::solve_critical_wsl_brent_analytical
/// \param H [in] water surface level to compute property for
/// \output area [out] linearly interpolated cross-sectional area at water surface level H
//
double CStreamnode::get_area(double depth) const
{
    const auto& df = *depthdf;   

    // Below lowest tabulated depth
    if (depth <= df.front()->depth)
        return df.front()->area;

    // Above highest tabulated depth
    if (depth >= df.back()->depth)
        return df.back()->area;

    // Linear interpolation
    for (size_t i = 0; i < df.size() - 1; ++i)
    {
        const auto& d1 = df[i];
        const auto& d2 = df[i+1];
        if (depth >= d1->depth && depth <= d2->depth)
        {
            double t = (depth - d1->depth) / (d2->depth - d1->depth);
            return d1->area + t * (d2->area - d1->area);
        }
    }
    return df.back()->area; // fallback
}

//////////////////////////////////////////////////////////////////
/// \brief Linearly interpolates to get the top width at a given
/// water surface level H
/// \note used in the CModel::solve_critical_wsl_brent_analytical
/// \param H [in] water surface level to compute property for
/// \output area [out] linearly interpolated top width at water surface level H
//
double CStreamnode::get_topwidth(double depth) const
{

    // depthdf is a POINTER → dereference it once
    const auto& df = *depthdf;   // df is now a reference to the vector

    // Below lowest tabulated depth
    if (depth <= df.front()->depth)
        return df.front()->top_width;

    // Above highest tabulated depth
    if (depth >= df.back()->depth)
        return df.back()->top_width;

    // Linear interpolation
    for (size_t i = 0; i < df.size() - 1; ++i)
    {
        const auto& d1 = df[i];
        const auto& d2 = df[i+1];

        if (depth >= d1->depth && depth <= d2->depth)
        {
            double t = (depth - d1->depth) / (d2->depth - d1->depth);
            return d1->top_width + t * (d2->top_width - d1->top_width);
        }
    }

    // Fallback (should never happen)
    return df.back()->top_width;
}

//////////////////////////////////////////////////////////////////
/// \brief Linearly interpolates to get the velocity correction
///        coefficient alpha at a given water surface level H
/// \note  Same pattern as get_area(), used in critical‑depth calcs
/// \param H [in] water surface level
/// \return alpha [out] interpolated velocity‑correction coefficient
//
double CStreamnode::get_alpha(double depth) const
{
    const auto& df = *depthdf;

    // Below lowest tabulated depth
    if (depth <= df.front()->depth)
        return df.front()->alpha;

    // Above highest tabulated depth
    if (depth >= df.back()->depth)
        return df.back()->alpha;

    // Linear interpolation
    for (size_t i = 0; i < df.size() - 1; ++i)
    {
        const auto& d1 = df[i];
        const auto& d2 = df[i+1];

        if (depth >= d1->depth && depth <= d2->depth)
        {
            double t = (depth - d1->depth) / (d2->depth - d1->depth);
            return d1->alpha + t * (d2->alpha - d1->alpha);
        }
    }

    return df.back()->alpha; // fallback
}

//////////////////////////////////////////////////////////////////
/// \brief Robust WSL solver considers monotonicity and expands domain if needed
/// \note used in the compute_streamnode for BRENT method
/// \param sn_up [in] streamnode for which to compute hydraulic profile
/// \param sn_down [in] streamnode one node downstream of sn
/// \param wsl_critical [in] critical water surface elevation at the sn_up node
/// \output wsl_estimated [out] estimated water surface level at sn_up that satisfies energy balance with sn_down
//
double CModel::solve_wsl_standard_step_brent(
    const CStreamnode* sn_up,          // upstream node (solution at this node)
    const CStreamnode* sn_down,        // downstream node (known or target)
    double wsl_critical                // critical water surface level at upstream
)
{
    // Residual wrapper: R(H) = energy_up(H) - energy_down_target
    auto residual = [&](double H) {
        auto temp_sn = std::make_unique<CStreamnode>(*sn_up);
        return temp_sn->get_wsl_residual(H, sn_down->mm, bbopt);
    };

    // Basic physical bounds
    const double bed_elev = sn_up->mm->min_elev;
    const double Hc       = wsl_critical;
    const double Htab_max = bed_elev + sn_up->depthdf->back()->depth;

    // Start with a conservative search interval
    double L = bed_elev + 1e-3;  // set to bound from 0m+tol in case it is critical
    double U = Htab_max;

    // Sample residual at a few points to infer monotonicity
    double fL = residual(L);
    double fU = residual(U);

    // If already bracketed, we’re happy
    if (fL * fU < 0.0) {
        return brent_root2(L, U, residual, 1e-8, 100);
    }

    // Try an interior point to detect trend
    double M  = 0.5 * (L + U);
    double fM = residual(M);

    // Estimate monotonicity sign: +1 ~ increasing, -1 ~ decreasing, 0 ~ unknown
    int mono_sign = 0;
    {
        // crude finite-difference style check
        double d1 = fM - fL;
        double d2 = fU - fM;

        if (d1 > 0.0 && d2 > 0.0) {
            mono_sign = +1; // increasing
        } else if (d1 < 0.0 && d2 < 0.0) {
            mono_sign = -1; // decreasing
        } else {
            mono_sign = 0;  // ambiguous; treat as unknown
        }
    }

    // If still not bracketed, expand interval guided by monotonicity
    const int    max_expand_iter = 25;
    const double base_span       = U - L;
    double       span            = base_span;

    for (int i = 0; i < max_expand_iter && fL * fU > 0.0; ++i) {

        // Expand depending on monotonicity:
        // - If increasing (mono_sign > 0): root lies where residual crosses zero,
        //   so we want to extend in the direction that might cross.
        // - If decreasing (mono_sign < 0): same idea, but opposite direction.
        // - If unknown: expand both sides symmetrically.

        if (mono_sign > 0) {
            // R increasing: if both positive, move L down; if both negative, move U up
            if (fL > 0.0 && fU > 0.0) {
                L = std::max(bed_elev + 1e-6, L - span);
                fL = residual(L);
            } else if (fL < 0.0 && fU < 0.0) {
                U = U + span;
                fU = residual(U);
            } else {
                // sign changed during expansion
                break;
            }
        } else if (mono_sign < 0) {
            // R decreasing: if both positive, move U up; if both negative, move L down
            if (fL > 0.0 && fU > 0.0) {
                U = U + span;
                fU = residual(U);
            } else if (fL < 0.0 && fU < 0.0) {
                L = std::max(bed_elev + 1e-6, L - span);
                fL = residual(L);
            } else {
                // sign changed during expansion
                break;
            }
        } else {
            // Unknown monotonicity: expand both sides
            L = std::max(bed_elev + 1e-6, L - 0.5 * span);
            U = U + 0.5 * span;
            fL = residual(L);
            fU = residual(U);
        }

        // Increase span gradually to avoid tiny steps
        span *= 1.5;
    }

    // Final check: do we have a valid bracket?
    if (fL * fU < 0.0) {
        return brent_root2(L, U, residual, 1e-8, 100);
    }

    // No bracket found: hydraulically inconsistent or outside table.
    WriteWarning("WSL residual cannot be bracketed; falling back to critical or last valid depth",
                 bbopt->noisy_run);

    // Fallback strategy:
    // 1) If residual at critical is small, accept critical.
    // 2) Otherwise, pick the point with smallest |R| in [L, M, U].
    double best_H  = Hc;
    double best_R  = std::fabs(residual(Hc));
    auto   update_best = [&](double H, double R) {
        double aR = std::fabs(R);
        if (aR < best_R) {
            best_R = aR;
            best_H = H;
        }
    };

    update_best(L, fL);
    update_best(M, fM);
    update_best(U, fU);

    if (best_H < bed_elev + 1e-6) {
      best_H = Hc;
      std::string msg =
        "WSL solver returned a depth below bed for streamnode " +
        std::to_string(sn_up->nodeID) +
        "; using critical depth instead";
      WriteWarning(msg.c_str(), bbopt->noisy_run);
    }

    return best_H;
}

//////////////////////////////////////////////////////////////////
/// \brief Finds dhand depths on either side of "depth"
/// \param depth [in] depth to consider
/// \return if "depth" is equal to some dhand depth, then    {ind, ind}
/// \return if "depth" is lower than all dhand depths, then  {PLACEHOLDER, first_ind}
/// \return if "depth" is higher than all dhand depths, then {last_ind, PLACEHOLDER}
/// \return if "depth" is between 2 dhand depths, then       {lower_ind, upper_ind}
//
std::pair<int, int> CModel::dhand_bounding_depths(double depth) {
  ExitGracefullyIf(dhand_depth_seq.empty(),
                   "Model.cpp: dhand_bounding_depths: dhand_depth_seq is empty",
                   exitcode::BAD_DATA);

  // Find first element not less than 'depth'
  auto it = std::lower_bound(dhand_depth_seq.begin(), dhand_depth_seq.end(), depth);
  int upper_ind = it - dhand_depth_seq.begin();

  if (*it == depth) { // depth is exactly the dhand depth at upper_ind
    return {upper_ind, upper_ind};
  }

  if (upper_ind == 0) { // depth smaller than smallest dhand depth
    return {PLACEHOLDER, 0};
  }
  if (upper_ind == dhand_depth_seq.size()) { // depth larger than largest dhand depth
    return {dhand_depth_seq.size() - 1, PLACEHOLDER};
  }

  int lower_ind = upper_ind - 1;

  return {lower_ind, upper_ind}; // depth is between 2 dhand depths
}

//////////////////////////////////////////////////////////////////
/// \brief Generates the interpolated depths of each spp for the "flow_ind"-th flow
/// \param flow_ind [in] index of the flow currently being considered
//
void CModel::generate_spp_depths(int flow_ind) {
  // ensure spp contains needed attributes
  ExitGracefullyIf(spp.get_index_by_fieldname("cpointid") == PLACEHOLDER,
                    "CModel.cpp: generate_spp_depths: spp does not "
                    "contain field cpointid",
                    exitcode::BAD_DATA);
  ExitGracefullyIf(spp.get_index_by_fieldname("elev") == PLACEHOLDER,
                    "CModel.cpp: generate_spp_depths: spp does not "
                    "contain field elev",
                    exitcode::BAD_DATA);
  ExitGracefullyIf(spp.get_index_by_fieldname("hpointid") == PLACEHOLDER,
                    "CModel.cpp: generate_spp_depths: spp does not "
                    "contain field hpointid",
                    exitcode::BAD_DATA);
  // initialize useful variables
  CStreamnode *pSN = nullptr;
  int sid = PLACEHOLDER; // streamnode id of catchment
  double ho_depth = PLACEHOLDER; // hydraulic output depth of catchment
  std::unordered_map<int, int> chainage_map; // for non headwater, non junction. maps hpointid to descending placement within catchment
  double seqelev_divs = PLACEHOLDER; // number of subdivisions created by spps in catchment (i.e. # of spp in catchment - 1)
  int head_ind = PLACEHOLDER; // index of first spp in catchment
  int tail_ind = PLACEHOLDER; // index of last spp in catchment
  double head_elev = PLACEHOLDER; // elevation of first spp in catchment
  double tail_elev = PLACEHOLDER; // elevation of last spp in catchment
  double L1 = PLACEHOLDER; // length for junction nodes
  double L2 = PLACEHOLDER; // length for junction nodes
  double L3 = PLACEHOLDER; // length for junction nodes

  // throw error if there are any catchments without snapped pourpoints associated with them
  std::vector<bool> catch_has_spp(bbsn->size());
  for (int j = 0; j < spp.features.size(); j++) {
    auto feat = spp.features[j];
    int t_sid = feat->GetFieldAsInteger(spp.get_index_by_fieldname("cpointid"));
    int t_ind = get_index_by_id(t_sid);
    if (t_ind != PLACEHOLDER) {
      catch_has_spp[t_ind] = true;
    } else {
      int t_hid = feat->GetFieldAsInteger(spp.get_index_by_fieldname("hpointid"));
      ExitGracefully(("Model.cpp: generate_spp_depths: spp with hpointid: " +
                      to_string(t_hid) + " is in catchment with cpointid: " +
                      to_string(t_sid) + ", which does not exist in streamnodes list")
                         .c_str(),
                     exitcode::BAD_DATA);
    }
  }
  if (std::any_of(catch_has_spp.begin(), catch_has_spp.end(), [](bool b) { return !b; })) {
    std::string empty_catchments = "";
    for (int i = 0; i < catch_has_spp.size(); i++) {
      if (!catch_has_spp[i]) {
        empty_catchments += ", " + to_string((*bbsn)[i]->nodeID);
      }
    }
    empty_catchments.erase(0, 2);
    ExitGracefully(
        ("Model.cpp: generate_spp_depths: catchments with the following "
         "catchment ids do not contain any snapped pourpoints: " +
         empty_catchments)
            .c_str(),
        exitcode::BAD_DATA);
  }

  // loop through each snapped pourpoint
  for (int j = 0; j < spp.features.size(); j++) {
    auto feat = spp.features[j];
    if (sid != feat->GetFieldAsInteger(spp.get_index_by_fieldname("cpointid"))) { // snapped pourpoint in new streamnode/catachment
      // assing variables that only change when the streamnode/catchment changes
      sid = feat->GetFieldAsInteger(spp.get_index_by_fieldname("cpointid"));
      pSN = get_streamnode_by_id(sid);
      ExitGracefullyIf(
          pSN == NULL,
          ("CModel.cpp: generate_spp_depths: spp references "
          "non-existant streamnode with id of " + std::to_string(sid)).c_str(),
          exitcode::BAD_DATA);
      ho_depth = (*hyd_result)[get_hyd_res_index(flow_ind, sid)]->depth;
      if (pSN->upnodeID1 == -1) { // headwater
        // assign/reset variables for a headwater node
        head_ind = j;
        head_elev = feat->GetFieldAsInteger(spp.get_index_by_fieldname("elev"));
        seqelev_divs = 0.0;
        int temp_sid =
            spp.features[j + seqelev_divs + 1]->GetFieldAsInteger(
                spp.get_index_by_fieldname("cpointid"));
        while (temp_sid == sid) {
          seqelev_divs++; // add a division
          // get next feature's sid
          temp_sid = spp.features[j + seqelev_divs + 1]->GetFieldAsInteger(
              spp.get_index_by_fieldname("cpointid"));
        }
        tail_ind = j + seqelev_divs;
        tail_elev = spp.features[tail_ind]->GetFieldAsInteger(
            spp.get_index_by_fieldname("elev"));

        L1 = PLACEHOLDER;
        L2 = PLACEHOLDER;
        L3 = PLACEHOLDER;

      } else if(pSN->upnodeID2 != -1) { // junction
        // assign/reset variables for a junction node
        seqelev_divs = PLACEHOLDER;
        head_ind = PLACEHOLDER;
        tail_ind = PLACEHOLDER;
        head_elev = PLACEHOLDER;
        tail_elev = PLACEHOLDER;

        L1 = pSN->us_reach_length1;
        L2 = get_streamnode_by_id(pSN->upnodeID1)->ds_reach_length;
        L3 = get_streamnode_by_id(pSN->upnodeID2)->ds_reach_length;

      } else { // neither headwater nor junction
        // assign/reset variables for a non headwater, non junction node
        chainage_map.clear();
        head_ind = j;
        head_elev = feat->GetFieldAsInteger(spp.get_index_by_fieldname("elev"));
        seqelev_divs = 0.0;
        std::vector<int> hids;
        hids.push_back(feat->GetFieldAsInteger(spp.get_index_by_fieldname("hpointid")));
        int temp_sid =
            spp.features[j + seqelev_divs + 1]->GetFieldAsInteger(
                spp.get_index_by_fieldname("cpointid"));
        while (temp_sid == sid) {
          seqelev_divs++; // add a division
          // get current feature's hid
          hids.push_back(spp.features[j + seqelev_divs]->GetFieldAsInteger(
              spp.get_index_by_fieldname("hpointid")));
          // get next feature's sid
          if (j + seqelev_divs + 1 >= spp.features.size()) {
            break;
          }
          temp_sid = spp.features[j + seqelev_divs + 1]->GetFieldAsInteger(
              spp.get_index_by_fieldname("cpointid"));
        }
        tail_ind = j + seqelev_divs;
        tail_elev = spp.features[tail_ind]->GetFieldAsInteger(
            spp.get_index_by_fieldname("elev"));

        // get descending order placement for each hid in the catchment
        std::sort(hids.begin(), hids.end(), std::greater<int>());
        for (int k = 0; k < hids.size(); k++) {
          chainage_map[hids[flow_ind]] = flow_ind;
        }

        L1 = PLACEHOLDER;
        L2 = pSN->us_reach_length1;
        L3 = PLACEHOLDER;
      }
    }

    if (pSN->upnodeID1 == -1) { // headwater node
      // headwater node, nothing to interpolate on one side of node
      double temp_elev = feat->GetFieldAsInteger(spp.get_index_by_fieldname("elev"));

      // correct depths based on elevation at nodes in the same streamnode/catchment
      ExitGracefullyIf(seqelev_divs == 0,
                        ("CModel.cpp: generate_spp_depths: only 1 "
                        "snapped pourpoint in headwater streamnode " + std::to_string(sid)).c_str(),
                        exitcode::BAD_DATA);
      double seqelev_j = head_elev +
        ((double)j - (double)head_ind) * (tail_elev - head_elev) / seqelev_divs;

      // find max change in depth (from change in elev)
      double max_change = std::abs((seqelev_j - temp_elev) / ho_depth);

      if (bbopt->postproc_elev_corr_threshold != PLACEHOLDER && bbopt->postproc_elev_corr_threshold != 0 && max_change > 0.5) {
        std::string warn = "CModel.cpp: generate_spp_depths: max change >0.5 detected at streamnode nodeID=" + std::to_string(sid);
        WriteWarning(warn, bbopt->noisy_run);
      }

      // find correction term from elevation change
      double ct = CalcCt(max_change, bbopt->postproc_elev_corr_threshold);

      // append interpolated depth to spp_depths
      spp_depths.push_back(ho_depth + (seqelev_j - temp_elev) * ct);

    } else if (pSN->upnodeID2 != -1) { // junction node
      // junction catchment with multiple sets of reaches in it

      // determine depth at junction, weighted average of depth and length
      // convention: ho_depth at downstream end, depth 2 at one upstream reach segment, depth 3 at the other
      double depth2 = (*hyd_result)[get_hyd_res_index(flow_ind, pSN->upnodeID1)]->depth;
      double depth3 = (*hyd_result)[get_hyd_res_index(flow_ind, pSN->upnodeID2)]->depth;
      double depth_junction =
          L1 + L2 + L3 == 0
              ? PLACEHOLDER
              : (ho_depth * L1 + depth2 * L2 + depth3 * L3) / (L1 + L2 + L3);

      // append interpolated depth to spp_depths
      if (depth_junction != PLACEHOLDER) {
        spp_depths.push_back(depth_junction); // for now. rob to look at R logic
        //if (feat->GetFieldAsInteger(spp.get_index_by_fieldname("reachID")) != pSN->reachID) {
        //  spp_depths.push_back(depth_junction);
        //} else {
        //  spp_depths.push_back(PLACEHOLDER);
        //}
      } else {
        spp_depths.push_back(PLACEHOLDER);
      }

    } else { // neither headwater nor junction node
      double temp_depth = PLACEHOLDER;
      double temp_elev = feat->GetFieldAsInteger(spp.get_index_by_fieldname("elev"));
      double depth3 = (*hyd_result)[get_hyd_res_index(flow_ind, pSN->upnodeID1)]->depth;

      if (seqelev_divs == 1) {
        temp_depth = ho_depth;
      } else {
        int hid = feat->GetFieldAsInteger(spp.get_index_by_fieldname("hpointid"));
        double chainage = L2 + (chainage_map[hid]) * (0 - L2) / seqelev_divs;
        temp_depth = ho_depth + (depth3 - ho_depth) / L2 * chainage;
      }
      
      // correct depths based on elevation at nodes in the same streamnode/catchment
      double seqelev_j = head_elev +
        ((double)j - (double)head_ind) * (tail_elev - head_elev) / seqelev_divs;

      // find max change in depth (from change in elev)
      double max_change = std::abs((seqelev_j - temp_elev) / temp_depth);

      if (bbopt->postproc_elev_corr_threshold != PLACEHOLDER && bbopt->postproc_elev_corr_threshold != 0 && max_change > 0.5) {
        std::string warn = "CModel.cpp: generate_spp_depths: max change >0.5 detected at streamnode nodeID=" + std::to_string(sid);
        WriteWarning(warn, bbopt->noisy_run);
      }
      
      // find correction term from elevation change
      double ct = CalcCt(max_change, bbopt->postproc_elev_corr_threshold);

      // append interpolated depth to spp_depths
      spp_depths.push_back(temp_depth + (seqelev_j - temp_elev) * ct);
    }
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Generates the hand values of each raster cell interpolated from the dhand rasters for the "flow_ind"-th flow
/// \param flow_ind [in] index of the flow currently being considered
/// \param is_interp [in] boolean indicating whether or not the post processing method is an interp method
//
void CModel::generate_dhand_vals(int flow_ind, bool is_interp) {
  // loop through each raster cell
  for (int j = 0; j < dhand[0]->xsize * dhand[0]->ysize; j++) {
    // grab the depth of the current raster cell and flow profile from the hydraulic output
    double curr_depth = (*hyd_result)[get_hyd_res_index(flow_ind, c_from_s->data[j])]->depth;

    // initialize variables and grab the corresponding dhand bounding depths
    double curr_dhand_val = PLACEHOLDER;
    int curr_dhandid_val = PLACEHOLDER;
    std::pair<int, int> bounds = dhand_bounding_depths(curr_depth);

    // assigns the dhand value of the current raster cell (and dhandid value if interp method)
    if (bounds.first == bounds.second) { // "depth" is equal to some dhand depth
      if (std::isnan(dhand[bounds.first]->data[j]) || dhand[bounds.first]->data[j] == dhand[bounds.first]->na_val) {
        curr_dhand_val = PLACEHOLDER;
      } else {
        curr_dhand_val = dhand[bounds.first]->data[j];
      }
      if (is_interp) {
        if (std::isnan(dhandid[bounds.first]->data[j]) || dhandid[bounds.first]->data[j] == dhandid[bounds.first]->na_val) {
          curr_dhandid_val = PLACEHOLDER;
        } else {
          curr_dhandid_val = dhandid[bounds.first]->data[j];
        }
      }
    } else {
      if (bbopt->dhand_method == enum_dh_method::INTERPOLATE) {
        if (bounds.first == PLACEHOLDER) { // "depth" is lower than all dhand depths
          WriteWarning(
              "Depth of " + std::to_string(curr_depth) +
                  " is lower than all provided dhand depths. Using "
                  "closest available dhand, though results should be "
                  "re-run with more dhand rasters to cover this depth",
              bbopt->noisy_run);
          if (std::isnan(dhand[bounds.second]->data[j]) || dhand[bounds.second]->data[j] == dhand[bounds.second]->na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            curr_dhand_val = dhand[bounds.second]->data[j];
          }
          if (is_interp) {
            if (std::isnan(dhandid[bounds.second]->data[j]) || dhandid[bounds.second]->data[j] == dhandid[bounds.second]->na_val) {
              curr_dhandid_val = PLACEHOLDER;
            } else {
              curr_dhandid_val = dhandid[bounds.second]->data[j];
            }
          }
        } else if (bounds.second == PLACEHOLDER) { // "depth" is higher than all dhand depths
          WriteWarning(
              "Depth of " + std::to_string(curr_depth) +
                  " is higher than all provided dhand depths. Using "
                  "closest available dhand, though results should be "
                  "re-run with more dhand rasters to cover this depth",
              bbopt->noisy_run);
          if (std::isnan(dhand[bounds.first]->data[j]) || dhand[bounds.first]->data[j] == dhand[bounds.first]->na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            curr_dhand_val = dhand[bounds.first]->data[j];
          }
          if (is_interp) {
            if (std::isnan(dhandid[bounds.first]->data[j]) || dhandid[bounds.first]->data[j] == dhandid[bounds.first]->na_val) {
              curr_dhandid_val = PLACEHOLDER;
            } else {
              curr_dhandid_val = dhandid[bounds.first]->data[j];
            }
          }
        } else { // "depth" is between 2 dhand depths
          if (std::isnan(dhand[bounds.first]->data[j]) || dhand[bounds.first]->data[j] == dhand[bounds.first]->na_val ||
              std::isnan(dhand[bounds.second]->data[j]) || dhand[bounds.second]->data[j] == dhand[bounds.second]->na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            double r1 = dhand[bounds.first]->data[j];
            double r2 = dhand[bounds.second]->data[j];
            double d1 = dhand_depth_seq[bounds.first];
            double d2 = dhand_depth_seq[bounds.second];

            curr_dhand_val = r1 * ((d1 - curr_depth) / (d1 - d2)) +
                             r2 * ((curr_depth - d2) / (d1 - d2));
            if (is_interp) {
              if (std::isnan(dhandid[bounds.first]->data[j]) || dhandid[bounds.first]->data[j] == dhandid[bounds.first]->na_val ||
                  std::isnan(dhandid[bounds.second]->data[j]) || dhandid[bounds.second]->data[j] == dhandid[bounds.second]->na_val) {
                curr_dhandid_val = PLACEHOLDER;
              } else {
                curr_dhandid_val = dhandid[bounds.first]->data[j];
              }
            }
          }
        }
      } else { // enum_dh_method::FLOOR
        if (bounds.first == PLACEHOLDER) {
          if (std::isnan(dhand[bounds.second]->data[j]) || dhand[bounds.second]->data[j] == dhand[bounds.second]->na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            curr_dhand_val = dhand[bounds.second]->data[j];
          }
          if (is_interp) {
            if (std::isnan(dhandid[bounds.second]->data[j]) || dhandid[bounds.second]->data[j] == dhandid[bounds.second]->na_val) {
              curr_dhandid_val = PLACEHOLDER;
            } else {
              curr_dhandid_val = dhandid[bounds.second]->data[j];
            }
          }
        } else {
          if (std::isnan(dhand[bounds.first]->data[j]) || dhand[bounds.first]->data[j] == dhand[bounds.first]->na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            curr_dhand_val = dhand[bounds.first]->data[j];
          }
          if (is_interp) {
            if (std::isnan(dhandid[bounds.first]->data[j]) || dhandid[bounds.first]->data[j] == dhandid[bounds.first]->na_val) {
              curr_dhandid_val = PLACEHOLDER;
            } else {
              curr_dhandid_val = dhandid[bounds.first]->data[j];
            }
          }
        }
      }
    }
    // adds dhand value of current raster cell to dhand_vals (and current dhandid to dhandid_vals if interp method)
    dhand_vals.push_back(curr_dhand_val);
    if (is_interp) {
      dhandid_vals.push_back(curr_dhandid_val);
    }
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Generates and saves a gridded data based on the post processing method for the "flow_ind"-th flow
/// \param flow_ind [in] index of the flow currently being considered
/// \param is_interp [in] boolean indicated whether post processing method is interp method
/// \param is_dhand [in] boolean indicated whether post processing method is dhand method
//
void CModel::generate_out_gridded(int flow_ind, bool is_interp, bool is_dhand) {
  initialize_out_gridded(is_dhand);
  CGriddedData *result = out_gridded.back().get();
  result->name = "result_depths_" + fp_names[flow_ind];
  result->fp_name = fp_names[flow_ind];
  std::fill(result->data, result->data + (result->xsize * result->ysize), 0.0);

  for (int j = 0; j < result->xsize * result->ysize; j++) {
    double curr_depth, curr_hand;
    // assign curr_depth based on whether post processing method is interp and/or dhand method
    if (!is_interp) {
        double value = c_from_s->data[j];

    if (!std::isnan(value) && value != c_from_s->na_val) {

        int idx = get_hyd_res_index(flow_ind, value);

        if (idx < 0 || hyd_result == nullptr) {
            curr_depth = 0.0;
        } else {

            hydraulic_output* ho = nullptr;

            // bounds check
            if (idx < hyd_result->size()) {
                ho = (*hyd_result)[idx];
            }

            // null pointer or invalid depth → zero
            if (ho == nullptr ||
                std::isnan(ho->depth) ||
                ho->depth == PLACEHOLDER ||
                ho->depth < 0.0)
            {
                curr_depth = 0.0;
            } else {
                curr_depth = ho->depth;
            }
        }

    } else {
        curr_depth = 0.0;
    }
        // compact expression form
        /* curr_depth =
          !std::isnan(c_from_s->data[j]) && c_from_s->data[j] != c_from_s->na_val
              ? (*hyd_result)[get_hyd_res_index(flow_ind, c_from_s->data[j])]->depth
              : PLACEHOLDER;
        */
    } else {
      if (!is_dhand) { // interp_hand

          // need to update to look for max index of pointid, not use spp_depths.size()
        /* if (!std::isnan(handid->data[j]) &&
                   handid->data[j] != handid->na_val &&
             (handid->data[j] - 1 >= spp_depths.size() || handid->data[j] - 1 < 0)) {
             ExitGracefully(
              ("Model.cpp: postprocess_floodresults: handid specifies a "
               "pourpoint id of " + std::to_string(handid->data[j]) +
               " which does not exist in snapped pourpoints").c_str(),
              exitcode::BAD_DATA);
         */

        // remove check for handid in spp_depths size for now, should replace with a check that it is in the spp IDs though
          // make this a warning for now
        if (!std::isnan(handid->data[j]) &&
                           handid->data[j] != handid->na_val) {
          WriteWarning(
              ("Model.cpp: postprocess_floodresults: handid specifies a "
               "pourpoint id of " + std::to_string(handid->data[j]) +
               " which does not exist in snapped pourpoints. Depths will not be computed for this hand node.").c_str(),
                   bbopt->noisy_run);  
          curr_depth = PLACEHOLDER;
        } else {
            curr_depth = !std::isnan(handid->data[j]) && handid->data[j] != handid->na_val
                                ? spp_depths[handid->data[j] - 1]
                                : PLACEHOLDER;
        }


        
      } else { // interp_dhand

          // need to update to look for max index of pointid, not use spp_depths.size()
          // commenting check out for now
        if ((dhandid_vals[j] != PLACEHOLDER &&
             (dhandid_vals[j] - 1 >= spp_depths.size() || dhandid_vals[j] - 1 < 0))) {
          ExitGracefully(
              ("Model.cpp: postprocess_floodresults: dhandid specifies a "
               "pourpoint id of " + std::to_string(dhandid_vals[j]) +
               " which does not exist in snapped pourpoints").c_str(),
              exitcode::BAD_DATA);
        }
        curr_depth = dhandid_vals[j] != PLACEHOLDER
                         ? spp_depths[dhandid_vals[j] - 1]
                         : PLACEHOLDER;
      }
    }

    // Normalize invalid depths: replace PLACEHOLDER, NaN, negatives with zero
    if (std::isnan(curr_depth) || curr_depth == PLACEHOLDER || curr_depth < 0) {
        curr_depth = 0.0;
    }

    // assign curr_hand based on whether post processing method is dhand method
    if (!is_dhand) {
      curr_hand = !std::isnan(hand->data[j]) && hand->data[j] != hand->na_val ? hand->data[j] : PLACEHOLDER;
    } else {
      curr_hand = dhand_vals[j];
    }

    if (std::isnan(curr_hand) || curr_hand == PLACEHOLDER || curr_hand < 0) {
        curr_hand = 0.0;
    }

    // calculate baseline result based on hand calc,instead of assigning directly to cell
    double final_depth =
        (curr_depth != PLACEHOLDER && curr_hand != PLACEHOLDER && curr_depth >= curr_hand)
            ? (curr_depth - curr_hand)
            : result->na_val;

    // Normalize invalid depths: replace PLACEHOLDER, NaN, negatives with zero
    if (std::isnan(final_depth) || final_depth == PLACEHOLDER || final_depth < 0) {
        final_depth = 0.0;
    }

    // assigning resulting value based on curr_depth and curr_hand
    /* if (curr_depth != PLACEHOLDER && curr_hand != PLACEHOLDER &&
             curr_depth >= curr_hand) {
      result->data[j] = curr_depth - curr_hand;
    } else {
      result->data[j] = result->na_val;
    }

    // convert all zero depths to NaN
    if (std::abs(result->data[j]) < 1e-9) {
        result->data[j] = std::numeric_limits<double>::quiet_NaN();
    }
    */
    if (conditional_model && bbopt->enable_spill_flows) {

        int sn_id = static_cast<int>(c_from_s->data[j]);

        if (!std::isnan(sn_id) && sn_id != c_from_s->na_val) {

            // check if this node is conditional
            if (is_conditional_node(sn_id)) {

                int condID = get_conditional_condid(sn_id);

                // read alternate HAND raster
                double alt_hand = condhand->data[j];
                double cond_depth = 0.0;
                double curr_cdepth = 0.0;

                if (!std::isnan(alt_hand) &&
                    alt_hand != condhand->na_val &&
                    alt_hand >= 0.0)
                {
                    int idxc = get_hyd_res_index(flow_ind, condID);

                    if (idxc < 0 || hyd_result == nullptr) {
                        cond_depth = 0.0;
                    } else {

                        hydraulic_output* hoc = nullptr;

                        if (idxc < hyd_result->size()) {
                            hoc = (*hyd_result)[idxc];
                        }

                        if (hoc == nullptr ||
                            std::isnan(hoc->depth) ||
                            hoc->depth == PLACEHOLDER ||
                            hoc->depth < 0.0)
                        {
                            curr_cdepth = 0.0;
                        } else {
                            curr_cdepth = hoc->depth;
                        }
                    }

                    // compute conditional depth based on cond hand and depth at proper node
                    cond_depth = curr_cdepth - alt_hand;

                    if (std::isnan(cond_depth) || cond_depth == PLACEHOLDER || cond_depth < 0) {
                      cond_depth = 0.0;
                    }

                    // take max of normal vs conditional depth
                    final_depth = std::max(final_depth, cond_depth);
                }
            }
        }
    }

    if (std::abs(final_depth) < 1e-9) {
      final_depth = std::numeric_limits<double>::quiet_NaN();
    }

    // assign final result to cell value
    result->data[j] = final_depth;
  }

  // Transpose data if writing to NetCDF
  if (bbopt->out_format == enum_gridded_format::NETCDF) {
    result->transpose_data();
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Generates and saves a gridded data based on the post processing method for the "flow_ind"-th flow
/// \param flow_ind [in] index of the flow currently being considered
/// \param is_interp [in] boolean indicated whether post processing method is interp method
/// \param is_dhand [in] boolean indicated whether post processing method is dhand method
//
void CModel::generate_out_gridded2(int flow_ind, enum_ppi_method postprocmethod) {
  bool is_dhand = false;
  if (postprocmethod == enum_ppi_method::INTERP_DHAND || postprocmethod == enum_ppi_method::CATCHMENT_DHAND) {
    is_dhand = true;
    ExitGracefully("DHAND methods not currently supported with the INTERP_RASTER interpolation method",
        exitcode::RUNTIME_ERR);
  }
  bool is_interp = false;
  if (postprocmethod == enum_ppi_method::INTERP_HAND || postprocmethod == enum_ppi_method::INTERP_DHAND) {
    is_interp = true;
  }

  initialize_out_gridded(is_dhand);
  CGriddedData *result = out_gridded.back().get();
  result->name = "result_depths_" + fp_names[flow_ind];
  result->fp_name = fp_names[flow_ind];
  std::fill(result->data, result->data + (result->xsize * result->ysize), 0.0);

  for (int j = 0; j < result->xsize * result->ysize; j++) {
    double curr_depth, curr_hand;
    double value = c_from_s->data[j]; // streamnode ID 
    hydraulic_output* ho = nullptr;

    // get depth for the current cell based on hydraulic results
    if (!std::isnan(value) && value != c_from_s->na_val) {
        
        int idx = get_hyd_res_index(flow_ind, value);
        if (idx >= 0 && hyd_result != nullptr && idx < hyd_result->size()) {
            ho = (*hyd_result)[idx];  
        }

        // null pointer or invalid depth → zero
        if (ho == nullptr ||
            std::isnan(ho->depth) ||
            ho->depth == PLACEHOLDER ||
            ho->depth < 0.0)
        {
            curr_depth = 0.0;
        } else {
            curr_depth = ho->depth;
        }

        if (is_interp) {
          // get other depths and calculate interpolated depth
          if (ho != nullptr && ho->upnodeID1 != -1) {
              // get that hydraulic result
              double depth1 = -99;
              double depth2 = -99;

              hydraulic_output* ho1 = (*hyd_result)[get_hyd_res_index(flow_ind, ho->upnodeID1)];
              if (!is_conditional_node(ho->upnodeID1)) {
                depth1 = ho1->depth;
              }

              if (ho->upnodeID2 != -1) {
                // get that hydraulic result too
                hydraulic_output* ho2 = (*hyd_result)[get_hyd_res_index(flow_ind, ho->upnodeID2)];
                if (!is_conditional_node(ho->upnodeID2)) {
                depth2 = ho2->depth;
                }
              }

              double interpvalue = interpdepth->data[j];
              // xxx if running interp_dhand, would need to get the right interpdepth raster here (and have multiple interpdepth generated)
              if (depth1 != -99 && !std::isnan(interpvalue) && interpvalue != interpdepth->na_val) {
                // calculate interpolated curr_depth based on junction properties
                double meandepth = depth1; // use depth from upstream node
                if (depth2 != -99) {
                  meandepth = (depth1 + depth2) / 2; // use mean depth from both upstream nodes
                }
                curr_depth = curr_depth * interpvalue + meandepth * (1-interpvalue); // weighted average interpolation of depths
              }
          }
        }

    } else {
        curr_depth = 0.0;
    }

    // Normalize invalid depths: replace PLACEHOLDER, NaN, negatives with zero
    if (std::isnan(curr_depth) || curr_depth == PLACEHOLDER || curr_depth < 0) {
        curr_depth = 0.0;
    }

    // assign curr_hand based on whether post processing method is dhand method
    if (!is_dhand) {
      curr_hand = !std::isnan(hand->data[j]) && hand->data[j] != hand->na_val ? hand->data[j] : PLACEHOLDER;
    } else {
      curr_hand = dhand_vals[j];
    }

    if (std::isnan(curr_hand) || curr_hand == PLACEHOLDER || curr_hand < 0) {
        curr_hand = 0.0;
    }

    // calculate baseline result based on hand calc,instead of assigning directly to cell
    double final_depth =
        (curr_depth != PLACEHOLDER && curr_hand != PLACEHOLDER && curr_depth >= curr_hand)
            ? (curr_depth - curr_hand)
            : result->na_val;

    // Normalize invalid depths: replace PLACEHOLDER, NaN, negatives with zero
    if (std::isnan(final_depth) || final_depth == PLACEHOLDER || final_depth < 0) {
        final_depth = 0.0;
    }

    // assigning resulting value based on curr_depth and curr_hand
    /* if (curr_depth != PLACEHOLDER && curr_hand != PLACEHOLDER &&
             curr_depth >= curr_hand) {
      result->data[j] = curr_depth - curr_hand;
    } else {
      result->data[j] = result->na_val;
    }

    // convert all zero depths to NaN
    if (std::abs(result->data[j]) < 1e-9) {
        result->data[j] = std::numeric_limits<double>::quiet_NaN();
    }
    */
    if (conditional_model && bbopt->enable_spill_flows) {

        int sn_id = static_cast<int>(c_from_s->data[j]);

        if (!std::isnan(sn_id) && sn_id != c_from_s->na_val) {

            // check if this node is conditional
            if (is_conditional_node(sn_id)) {

                int condID = get_conditional_condid(sn_id);

                // read alternate HAND raster
                double alt_hand = condhand->data[j];
                double cond_depth = 0.0;
                double curr_cdepth = 0.0;

                if (!std::isnan(alt_hand) &&
                    alt_hand != condhand->na_val &&
                    alt_hand >= 0.0)
                {
                    int idxc = get_hyd_res_index(flow_ind, condID);

                    if (idxc < 0 || hyd_result == nullptr) {
                        cond_depth = 0.0;
                    } else {

                        hydraulic_output* hoc = nullptr;

                        if (idxc < hyd_result->size()) {
                            hoc = (*hyd_result)[idxc];
                        }

                        if (hoc == nullptr ||
                            std::isnan(hoc->depth) ||
                            hoc->depth == PLACEHOLDER ||
                            hoc->depth < 0.0)
                        {
                            curr_cdepth = 0.0;
                        } else {
                            curr_cdepth = hoc->depth;
                        }
                    }

                    // compute conditional depth based on cond hand and depth at proper node
                    cond_depth = curr_cdepth - alt_hand;

                    if (std::isnan(cond_depth) || cond_depth == PLACEHOLDER || cond_depth < 0) {
                      cond_depth = 0.0;
                    }

                    // take max of normal vs conditional depth
                    final_depth = std::max(final_depth, cond_depth);
                }
            }
        }
    }

    if (std::abs(final_depth) < 1e-9) {
      final_depth = std::numeric_limits<double>::quiet_NaN();
    }

    // assign final result to cell value
    result->data[j] = final_depth;
  }

  // Transpose data if writing to NetCDF
  if (bbopt->out_format == enum_gridded_format::NETCDF) {
    result->transpose_data();
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Initializes an output gridded data instance for the flow ind-th profile
/// \param is_dhand [in] boolean indicated whether post processing method is dhand method
//
void CModel::initialize_out_gridded(bool is_dhand) {

  if (bbopt->in_format == bbopt->out_format || bbopt->out_format == enum_gridded_format::PNG) { // raster to raster OR netcdf to netcdf OR anything to png
    std::unique_ptr<CGriddedData> result;
    if (!is_dhand) {
      result = hand->clone();
    } else {
      result = dhand[0]->clone();
    }
    out_gridded.push_back(std::move(result));
  } else if (bbopt->in_format == enum_gridded_format::RASTER &&
             bbopt->out_format == enum_gridded_format::NETCDF) { // raster to netcdf
    // Initialize output
    auto res_netcdf = std::make_unique<CNetCDFLayer>();

    // Set raster to copy parameters from
    CRaster *hand_raster = nullptr;
    if (!is_dhand) {
      hand_raster = dynamic_cast<CRaster *>(hand.get());
    } else {
      hand_raster = dynamic_cast<CRaster *>(dhand[0].get());
    }

    // Assign common variables
    res_netcdf->data = static_cast<double *>(
        CPLMalloc(sizeof(double) * hand_raster->xsize * hand_raster->ysize));
    std::copy(hand_raster->data,
              hand_raster->data + hand_raster->xsize * hand_raster->ysize,
              res_netcdf->data);
    res_netcdf->xsize = hand_raster->xsize;
    res_netcdf->ysize = hand_raster->ysize;
    res_netcdf->na_val = hand_raster->na_val;

    // Convert unique variables
    res_netcdf->x_coords.resize(hand_raster->xsize);
    res_netcdf->y_coords.resize(hand_raster->ysize);
    for (int i = 0; i < hand_raster->xsize; i++) {
      res_netcdf->x_coords[i] = hand_raster->geotrans[0] + i * hand_raster->geotrans[1];
    }
    for (int j = 0; j < hand_raster->ysize; j++) {
      res_netcdf->y_coords[j] = hand_raster->geotrans[3] + j * hand_raster->geotrans[5];
    }
    res_netcdf->datatype = ConvertGDALTypeToNetCDF(hand_raster->datatype);

    // Convert projection WKT to NetCDF CF-compliant metadata
    OGRSpatialReference srs;
    if (srs.importFromWkt(hand_raster->proj) != OGRERR_NONE) {
      ExitGracefully("Model.cpp: initialize_out_gridded: Failed to parse WKT projection.", exitcode::BAD_DATA);
    }
    // Store EPSG code
    if (srs.GetAuthorityCode(nullptr) != nullptr) {
      res_netcdf->epsg = std::stoi(srs.GetAuthorityCode(nullptr));
    }

    // Save to out_gridded
    out_gridded.push_back(std::move(res_netcdf));
  } else if (bbopt->in_format == enum_gridded_format::NETCDF &&
             bbopt->out_format == enum_gridded_format::RASTER) { // netcdf to raster
    // Initialize output
    auto res_raster = std::make_unique<CRaster>();

    // Set raster to copy parameters from
    CNetCDFLayer *hand_netcdf = nullptr;
    if (!is_dhand) {
      hand_netcdf = dynamic_cast<CNetCDFLayer *>(hand.get());
    } else {
      hand_netcdf = dynamic_cast<CNetCDFLayer *>(dhand[0].get());
    }

    // Assign common variables
    res_raster->data = static_cast<double *>(
        CPLMalloc(sizeof(double) * hand_netcdf->xsize * hand_netcdf->ysize));
    std::copy(hand_netcdf->data,
              hand_netcdf->data + hand_netcdf->xsize * hand_netcdf->ysize,
              res_raster->data);
    res_raster->xsize = hand_netcdf->xsize;
    res_raster->ysize = hand_netcdf->ysize;
    res_raster->na_val = hand_netcdf->na_val;

    // Convert unique variables
    res_raster->geotrans[0] = hand_netcdf->x_coords[0];                                                              // Origin X
    res_raster->geotrans[1] = (hand_netcdf->xsize > 1) ? (hand_netcdf->x_coords[1] - hand_netcdf->x_coords[0]) : 1;  // Pixel size X
    res_raster->geotrans[2] = 0;                                                                                     // No rotation
    res_raster->geotrans[3] = hand_netcdf->y_coords[0];                                                              // Origin Y
    res_raster->geotrans[4] = 0;                                                                                     // No rotation
    res_raster->geotrans[5] =                                                                                        // Pixel size Y
        (hand_netcdf->ysize > 1)
            ? (hand_netcdf->y_coords[1] -
               hand_netcdf->y_coords[0])
            : -1;
    res_raster->datatype = ConvertNetCDFTypeToGDAL(hand_netcdf->datatype);
    
    // Create projection reference string
    OGRSpatialReference srs;
    if (srs.importFromEPSG(std::stoi(hand_netcdf->epsg)) == OGRERR_NONE) {
      char *wkt = nullptr;
      srs.exportToWkt(&wkt);
      res_raster->proj = CPLStrdup(wkt);
      CPLFree(wkt);
    } else {
      ExitGracefully("Model.cpp: initialize_out_gridded: input netcdf does not "
                     "have a specified projection system",
                     exitcode::BAD_DATA);
    }

    // Save to out_gridded
    out_gridded.push_back(std::move(res_raster));
  } else {
    ExitGracefully(("Model.cpp: generate_out_gridded: input " +
                    toString(bbopt->in_format) + " to output " +
                    toString(bbopt->out_format) +
                    " is undefined and not currently supported.")
                       .c_str(),
                   exitcode::BAD_DATA);
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Writes out the sum of flows and flow sources in the model
//  Used in debugging with the spill flow calculations
//
void CModel::write_total_flows() {
    double totalQ = 0.0;
    double minQ = 1e9;
    double maxQ = -1;
    for (int i = 0; i < bbsn->size(); ++i) {
        totalQ += (*bbsn)[i]->output_flows[0];
      if ((*bbsn)[i]->output_flows[0] < minQ) {
            minQ = (*bbsn)[i]->output_flows[0];
      }
      if ((*bbsn)[i]->output_flows[0] > maxQ) {
            maxQ = (*bbsn)[i]->output_flows[0];
      }
    }

    double sum_sources = 0.0;
    for (int i = 0; i < bbsn->size(); ++i) {
        for (double s : (*bbsn)[i]->flow_sources) {
            sum_sources += s;
        }
    }

    std::cout << "Max Flow (cms) is " << maxQ
              << ", min flow (cms) is " << minQ
              << ", total flow (cms) is " << totalQ 
              << ", sum flow sources (cms) is " << sum_sources
              << std::endl;
}

//////////////////////////////////////////////////////////////////
/// \brief Stores snapshot of headwater output_flows
//  Used in spill flow calculations to restore flows after the previous run
//    to ensure proper calculation of flows in the system
//
void CModel::snapshot_headwater_flows() {
    snapshot.clear();
    snapshot.reserve(bbsn->size());

    for (auto* sn : *bbsn) {
        // HEADWATER nodes have upstream_flows[0] == HEADWATER
        if (!sn->upstream_flows.empty() && sn->upstream_flows[0] == HEADWATER) {
            FlowSnapshot fs;
            fs.output_flows = sn->output_flows;   // store ONLY headwater output flows
            snapshot.push_back(std::move(fs));
        }
    }
}

//////////////////////////////////////////////////////////////////
/// \brief Restores headwater output_flows from snapshot
//  Used in spill flow calculations to restore flows after the previous run
//    to ensure proper calculation of flows in the system
//
void CModel::restore_headwater_flows() {
    size_t idx = 0;

    for (auto* sn : *bbsn) {
        if (!sn->upstream_flows.empty() && sn->upstream_flows[0] == HEADWATER) {
            sn->output_flows = snapshot[idx].output_flows;
            idx++;
        }
    }
}

/// brief writes the structure properties to file
void CModel::write_structure_preproc_hyd_tables(std::ostream& os) const
{
    if (bbsn == nullptr) {
    return;
    }
    for (const auto* pSN : *bbsn)
    {
        if (pSN == nullptr) {
        continue;
        }
        if (pSN->is_structure)
        {
        pSN->write_preproc_hyd_table(os);
        os << "\n";
        }
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
