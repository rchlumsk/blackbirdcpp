#include "Model.h"
#include "XSection.h"

// Default constructor
CModel::CModel()
  : bbsn(new std::vector<CStreamnode*>),
  bbbc(new CBoundaryConditions),
  bbopt(new COptions),
  dhand_depth_seq(),
  c_from_s(),
  hand(),
  handid(),
  dhand(),
  dhandid(),
  hyd_result(nullptr),
  out_rasters(),
  streamnode_map(),
  flow(PLACEHOLDER),
  peak_hrs_min(PLACEHOLDER),
  peak_hrs_max(PLACEHOLDER),
  spp_depths(),
  dhand_vals(),
  dhandid_vals() {
  // Default constructor implementation
}

// Copy constructor
CModel::CModel(const CModel &other)
    : c_from_s(other.c_from_s), hand(other.hand), handid(other.handid),
      dhand(other.dhand), dhandid(other.dhandid),
      out_rasters(other.out_rasters), streamnode_map(other.streamnode_map),
      flow(other.flow), peak_hrs_min(other.peak_hrs_min),
      peak_hrs_max(other.peak_hrs_max), spp_depths(other.spp_depths),
      dhand_vals(other.dhand_vals), dhandid_vals(other.dhandid_vals) {
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
      hyd_result->push_back(new hydraulic_output(*res));
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

  dhand_depth_seq = other.dhand_depth_seq;
  c_from_s = other.c_from_s;
  hand = other.hand;
  handid = other.handid;
  dhand = other.dhand;
  dhandid = other.dhandid;
  out_rasters = other.out_rasters;
  streamnode_map = other.streamnode_map;
  flow = other.flow;
  peak_hrs_min = other.peak_hrs_min;
  peak_hrs_max = other.peak_hrs_max;
  spp_depths = other.spp_depths;
  dhand_vals = other.dhand_vals;
  dhandid_vals = other.dhandid_vals;

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
  CStreamnode *start_streamnode = get_streamnode_by_id(bbbc->nodeID);

  ExitGracefullyIf(!start_streamnode,
                   "Model.cpp: hyd_compute_profile(): boundary condition "
                   "streamnode id not represented in streamnodes",
                   BAD_DATA);

  // Initialize container for hydraulic result
  hyd_result = new std::vector<hydraulic_output *>(
      start_streamnode->output_flows.size() * bbsn->size());

  // Loop through each flow
  // Compute the corresponding hydraulic profile starting at the boundary condition streamnode
  for (int f = 0; f < start_streamnode->output_flows.size(); f++) {
    flow = f;
    compute_streamnode(start_streamnode, start_streamnode, hyd_result);
    WriteAdvisory("The range of required peak flood times ranges from " +
                      std::to_string(peak_hrs_min) + " to " +
                      std::to_string(peak_hrs_max) + " hours for flow " +
                      std::to_string(flow) +
                      " within the computed streamnodes",
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
/// \brief Reads GIS files required for model
//
void CModel::ReadGISFiles() {
  GDALAllRegister();
  
  if (std::filesystem::exists(bbopt->gis_path + "/" + bbopt->in_nc_name)) {
    bbopt->in_nc = true;
  }

  ReadRasterFile(bbopt->gis_path + "/bb_catchments_fromstreamnodes.tif", dynamic_cast<CRaster *>(c_from_s.get()));
  c_from_s->name = "Catchments from Streamnodes";
  if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_DHAND_WSLCORR ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
    ReadVectorFile(bbopt->gis_path + "/bb_snapped_pourpoints_hand.shp", spp);
    spp.name = "Snapped Pourpoints";
  }
  if (bbopt->interpolation_postproc_method == enum_ppi_method::CATCHMENT_HAND ||
      bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) { // no dhand
    ReadRasterFile(bbopt->gis_path + "/bb_hand.tif",  dynamic_cast<CRaster *>(hand.get()));
    hand->name = "HAND";
    if (bbopt->interpolation_postproc_method == enum_ppi_method::INTERP_HAND) {
      ReadRasterFile(bbopt->gis_path + "/bb_hand_pourpoint_id.tif",  dynamic_cast<CRaster *>(handid.get()));
      handid->name = "HAND ID";
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
        ReadRasterFile(bbopt->gis_path + "/bb_dhand_pourpoint_id_depth_" + stream.str() + "m.tif", dynamic_cast<CRaster *>(dhand.back().get()));
        dhandid.back()->name = "DHAND ID " + stream.str();
      }
    }
  }
  if (!bbopt->silent_run) {
    std::cout << "...raster data successfully read" << std::endl;
    std::cout << std::endl;
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Reads specified NetCDF file
//
void CModel::ReadNetCDFFile(std::string filename, CNetCDF *raster_obj) {
  netCDF::NcFile nc_file(filename, netCDF::NcFile::read);
  netCDF::NcVar var;
}

//////////////////////////////////////////////////////////////////
/// \brief Reads specified Raster file
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
  
  // loop for each flow_profiles
  for (int flow_ind = 0; flow_ind < bbsn->front()->output_flows.size(); flow_ind++) {
    if (!bbopt->silent_run) {
      std::cout << "post processing flood results for flow " + std::to_string(flow_ind + 1) << std::endl;
    }

    switch (bbopt->interpolation_postproc_method)
    {
    case (enum_ppi_method::CATCHMENT_HAND):
    {
      generate_out_raster(flow_ind, false, false);
      break;
    }
    case (enum_ppi_method::INTERP_HAND):
    {
      generate_spp_depths(flow_ind);
      generate_out_raster(flow_ind, true, false);
      break;
    }
    case (enum_ppi_method::CATCHMENT_DHAND):
    {
      generate_dhand_vals(flow_ind, false);
      generate_out_raster(flow_ind, false, true);
      break;
    }
    case (enum_ppi_method::INTERP_DHAND):
    {
      generate_spp_depths(flow_ind);
      generate_dhand_vals(flow_ind, true);
      generate_out_raster(flow_ind, true, true);
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
//
void CModel::compute_streamnode(CStreamnode *&sn, CStreamnode *&down_sn, std::vector<hydraulic_output *> *&res) {
  if (!bbopt->silent_run) {
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
              if (!bbopt->silent_run) {
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
                if (!bbopt->silent_run) {
                  std::cout << "setting to critical depth result on streamnode " << sn->nodeID << std::endl;
                }
              } else {
                if (!bbopt->silent_run) {
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
            if (!bbopt->silent_run) {
              std::cout << "Iterated on WSL at streamnode " << sn->nodeID
                        << " within tolerance on iteration " << i << std::endl;
            }
            break;
          } else {
            if (!bbopt->silent_run) {
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
                  if (!bbopt->silent_run) {
                    std::cout << "setting to supercritical" << std::endl;
                  }
                  break;
                } else {
                  if (!bbopt->silent_run) {
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

  // compute peak hours required and update min and max accordingly
  sn->mm->peak_hrs_required = (sn->mm->area * sn->mm->length_effectiveadjusted / sn->mm->flow) / 3600; // TODO check with rob
  if (peak_hrs_min == PLACEHOLDER || sn->mm->peak_hrs_required < peak_hrs_min) {
    peak_hrs_min = sn->mm->peak_hrs_required;
  }
  if (peak_hrs_max == PLACEHOLDER || sn->mm->peak_hrs_required > peak_hrs_max) {
    peak_hrs_max = sn->mm->peak_hrs_required;
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

  // loop through each snapped pourpoint
  for (int j = 0; j < spp.features.size(); j++) {
    auto feat = spp.features[j];
    if (sid != feat->GetFieldAsInteger(spp.get_index_by_fieldname("cpointid"))) { // snapped pourpoint in new streamnode/catachment
      // assing variables that only change when the streamnode/catchment changes
      sid = feat->GetFieldAsInteger(spp.get_index_by_fieldname("cpointid"));
      pSN = get_streamnode_by_id(sid);
      ExitGracefullyIf(
          pSN == NULL,
          "CModel.cpp: generate_spp_depths: spp references "
          "non-existant streamnode with id of " + sid,
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
                        "CModel.cpp: generate_spp_depths: only 1 "
                        "snapped pourpoint in headwater streamnode " + sid,
                        exitcode::BAD_DATA);
      double seqelev_j = head_elev +
        ((double)j - (double)head_ind) * (tail_elev - head_elev) / seqelev_divs;

      // find max change in depth (from change in elev)
      double max_change = std::abs((seqelev_j - temp_elev) / ho_depth);

      if (max_change > 0.5) {
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

      if (max_change > 0.5) {
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
  for (int j = 0; j < dhand[0].xsize * dhand[0].ysize; j++) {
    // grab the depth of the current raster cell and flow profile from the hydraulic output
    double curr_depth = (*hyd_result)[get_hyd_res_index(flow_ind, c_from_s.data[j])]->depth;

    // initialize variables and grab the corresponding dhand bounding depths
    double curr_dhand_val = PLACEHOLDER;
    int curr_dhandid_val = PLACEHOLDER;
    std::pair<int, int> bounds = dhand_bounding_depths(curr_depth);

    // assigns the dhand value of the current raster cell (and dhandid value if interp method)
    if (bounds.first == bounds.second) { // "depth" is equal to some dhand depth
      curr_dhand_val = dhand[bounds.first].data[j];
      if (is_interp) {
        curr_dhandid_val = dhandid[bounds.first].data[j];
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
          if (dhand[bounds.second].data[j] == dhand[bounds.second].na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            curr_dhand_val = dhand[bounds.second].data[j];
          }
          if (is_interp) {
            if (dhandid[bounds.second].data[j] == dhandid[bounds.second].na_val) {
              curr_dhandid_val = PLACEHOLDER;
            } else {
              curr_dhandid_val = dhandid[bounds.second].data[j];
            }
          }
        } else if (bounds.second == PLACEHOLDER) { // "depth" is higher than all dhand depths
          WriteWarning(
              "Depth of " + std::to_string(curr_depth) +
                  " is higher than all provided dhand depths. Using "
                  "closest available dhand, though results should be "
                  "re-run with more dhand rasters to cover this depth",
              bbopt->noisy_run);
          if (dhand[bounds.first].data[j] == dhand[bounds.first].na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            curr_dhand_val = dhand[bounds.first].data[j];
          }
          if (is_interp) {
            if (dhandid[bounds.first].data[j] == dhandid[bounds.first].na_val) {
              curr_dhandid_val = PLACEHOLDER;
            } else {
              curr_dhandid_val = dhandid[bounds.first].data[j];
            }
          }
        } else { // "depth" is between 2 dhand depths
          if (dhand[bounds.first].data[j] == dhand[bounds.first].na_val ||
              dhand[bounds.second].data[j] == dhand[bounds.second].na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            double r1 = dhand[bounds.first].data[j];
            double r2 = dhand[bounds.second].data[j];
            double d1 = dhand_depth_seq[bounds.first];
            double d2 = dhand_depth_seq[bounds.second];

            curr_dhand_val = r1 * ((d1 - curr_depth) / (d1 - d2)) +
                              r2 * ((curr_depth - d2) / (d1 - d2));
          }
        }
      } else { // enum_dh_method::FLOOR
        if (bounds.first == PLACEHOLDER) {
          if (dhand[bounds.second].data[j] == dhand[bounds.second].na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            curr_dhand_val = dhand[bounds.second].data[j];
          }
          if (is_interp) {
            if (dhandid[bounds.second].data[j] == dhandid[bounds.second].na_val) {
              curr_dhandid_val = PLACEHOLDER;
            } else {
              curr_dhandid_val = dhandid[bounds.second].data[j];
            }
          }
        } else {
          if (dhand[bounds.first].data[j] == dhand[bounds.first].na_val) {
            curr_dhand_val = PLACEHOLDER;
          } else {
            curr_dhand_val = dhand[bounds.first].data[j];
          }
          if (is_interp) {
            if (dhandid[bounds.first].data[j] == dhandid[bounds.first].na_val) {
              curr_dhandid_val = PLACEHOLDER;
            } else {
              curr_dhandid_val = dhandid[bounds.first].data[j];
            }
          }
        }
      }
    }
    // adds dhand value of current raster cell to dhand_vals and (and current dhandid to dhandid_vals if interp method)
    dhand_vals.push_back(curr_dhand_val);
    if (is_interp) {
      dhandid_vals.push_back(curr_dhandid_val);
    }
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Generates and saves a raster based on the post processing method for the "flow_ind"-th flow
/// \param flow_ind [in] index of the flow currently being considered
/// \param is_interp [in] boolean indicated whether post processing method is interp method
/// \param is_dhand [in] boolean indicated whether post processing method is dhand method
//
void CModel::generate_out_raster(int flow_ind, bool is_interp, bool is_dhand) {
  CRaster result;
  if (!is_dhand) {
    result = hand;
  } else {
    result = dhand[0];
  }
  result.name = "Result " + std::to_string(flow_ind + 1);
  std::fill(result.data, result.data + (result.xsize * result.ysize), 0.0);

  for (int j = 0; j < result.xsize * result.ysize; j++) {
    double curr_depth, curr_hand;
    // assign curr_depth based on whether post processing method is interp and/or dhand method
    if (!is_interp) {
      curr_depth =
          c_from_s.data[j] != c_from_s.na_val
              ? (*hyd_result)[get_hyd_res_index(flow_ind, c_from_s.data[j])]->depth
              : PLACEHOLDER;
    } else {
      if (!is_dhand) { // interp_hand
        if ((handid.data[j] != handid.na_val &&
             (handid.data[j] - 1 >= spp_depths.size() || handid.data[j] - 1 < 0))) {
          ExitGracefully(
              ("Model.cpp: postprocess_floodresults: handid specifies a "
               "pourpoint id of " + std::to_string(handid.data[j]) +
               " which does not exist in snapped pourpoints").c_str(),
              exitcode::BAD_DATA);
        }
        curr_depth = handid.data[j] != handid.na_val
                                ? spp_depths[handid.data[j] - 1]
                                : PLACEHOLDER;
      } else { // interp_dhand
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
    // assign curr_hand based on whether post processing method is dhand method
    if (!is_dhand) {
      curr_hand = hand.data[j] != hand.na_val ? hand.data[j] : PLACEHOLDER;
    } else {
      curr_hand = dhand_vals[j];
    }

    // assigning resulting value based on curr_depth and curr_hand
    if (curr_depth != PLACEHOLDER && curr_hand != PLACEHOLDER && curr_depth >= curr_hand) {
      result.data[j] = curr_depth - curr_hand;
    } else {
      result.data[j] = result.na_val;
    }
  }

  out_rasters.push_back(result); // save result for this flow profile to out_rasters
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