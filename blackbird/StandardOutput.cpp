#include "Model.h"

#if defined(_WIN32)
#include <direct.h>
#elif defined(__linux__)
#include <sys/stat.h>
#elif defined(__unix__)
#include <sys/stat.h>
#elif defined(__APPLE__)
#include <sys/stat.h>
#endif

//////////////////////////////////////////////////////////////////
/// \brief Write output file headers
/// \details Called prior to simulation (but after initialization) from CModel::Initialize()
/// \param &Options [in] Global model options information
//
void CModel::WriteOutputFileHeaders(const optStruct& Options)
{
  int i, j, p;
  string tmpFilename;

  if (Options.noisy) { cout << "  Writing Output File Headers..." << endl; }

  if (Options.output_format == OUTPUT_STANDARD)
  {

    //WatershedStorage.csv
    //--------------------------------------------------------------
    if (Options.write_watershed_storage)
    {
      tmpFilename = FilenamePrepare("WatershedStorage.csv", Options);
      _STORAGE.open(tmpFilename.c_str());
      if (_STORAGE.fail()) {
        ExitGracefully(("CModel::WriteOutputFileHeaders: unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
      }

      int iAtmPrecip = GetStateVarIndex(ATMOS_PRECIP);
      _STORAGE << "time [d],date,hour,rainfall [mm/day],snowfall [mm/d SWE],Channel Storage [mm],Reservoir Storage [mm],Rivulet Storage [mm]";
      for (i = 0;i < GetNumStateVars();i++) {
        if (CStateVariable::IsWaterStorage(_aStateVarType[i])) {
          if (i != iAtmPrecip) {
            _STORAGE << "," << CStateVariable::GetStateVarLongName(_aStateVarType[i],
              _aStateVarLayer[i],
              _pTransModel) << " [mm]";
            //_STORAGE<<","<<CStateVariable::SVTypeToString(_aStateVarType[i],_aStateVarLayer[i])<<" [mm]";
          }
        }
      }
      _STORAGE << ", Total [mm], Cum. Inputs [mm], Cum. Outflow [mm], MB Error [mm]" << endl;
    }

    //Hydrographs.csv
    //--------------------------------------------------------------
    tmpFilename = FilenamePrepare("Hydrographs.csv", Options);
    _HYDRO.open(tmpFilename.c_str());
    if (_HYDRO.fail()) {
      ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }

    CSubBasin* pSB;
    _HYDRO << "time,date,hour";
    _HYDRO << ",precip [mm/day]";
    for (p = 0;p < _nSubBasins;p++)
    {
      pSB = _pSubBasins[p];
      if (pSB->IsGauged() && pSB->IsEnabled())
      {
        if (pSB->GetName() == "") { _HYDRO << ",ID=" << pSB->GetID() << " [m3/s]"; }
        else { _HYDRO << "," << pSB->GetName() << " [m3/s]"; }

        for (i = 0; i < _nObservedTS; i++) {
          if (IsContinuousFlowObs(_pObservedTS[i], pSB->GetID()))
          {
            if (pSB->GetName() == "") { _HYDRO << ",ID=" << pSB->GetID() << " (observed) [m3/s]"; }
            else { _HYDRO << "," << pSB->GetName() << " (observed) [m3/s]"; }
          }
        }
        if (Options.write_localflow) {
          if (pSB->GetName() == "") { _HYDRO << ",ID=" << pSB->GetID() << " (local) [m3/s]"; }
          else { _HYDRO << "," << pSB->GetName() << " (local) [m3/s]"; }
        }
        if (pSB->GetReservoir() != NULL)
        {
          if (pSB->GetName() == "") { _HYDRO << ",ID=" << pSB->GetID() << " (res. inflow) [m3/s]"; }
          else { _HYDRO << "," << pSB->GetName() << " (res. inflow) [m3/s]"; }
          for (i = 0; i < _nObservedTS; i++) {
            if (IsContinuousInflowObs(_pObservedTS[i], pSB->GetID()))
            {
              if (pSB->GetName() == "") { _HYDRO << ",ID=" << pSB->GetID() << " (obs. res. inflow) [m3/s]"; }
              else { _HYDRO << "," << pSB->GetName() << " (obs. res. inflow) [m3/s]"; }
            }
          }
        }
      }
    }
    _HYDRO << endl;

    //WaterLevels.csv
    //--------------------------------------------------------------
    if (Options.write_waterlevels) {
      tmpFilename = FilenamePrepare("WaterLevels.csv", Options);
      _LEVELS.open(tmpFilename.c_str());
      if (_LEVELS.fail()) {
        ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
      }

      CSubBasin* pSB;
      _LEVELS << "time,date,hour";
      _LEVELS << ",precip [mm/day]";
      for (p = 0;p < _nSubBasins;p++)
      {
        pSB = _pSubBasins[p];
        if (pSB->IsGauged() && pSB->IsEnabled())
        {
          if (pSB->GetName() == "") { _LEVELS << ",ID=" << pSB->GetID() << " [m]"; }
          else { _LEVELS << "," << pSB->GetName() << " [m]"; }

          for (i = 0; i < _nObservedTS; i++) {
            if (IsContinuousLevelObs(_pObservedTS[i], pSB->GetID()))
            {
              if (pSB->GetName() == "") { _LEVELS << ",ID=" << pSB->GetID() << " (observed) [m]"; }
              else { _LEVELS << "," << pSB->GetName() << " (observed) [m]"; }
            }
          }
        }
      }
      _LEVELS << endl;
    }

    //ReservoirStages.csv
    //--------------------------------------------------------------
    if (Options.write_reservoir)
    {
      tmpFilename = FilenamePrepare("ReservoirStages.csv", Options);
      _RESSTAGE.open(tmpFilename.c_str());
      if (_RESSTAGE.fail()) {
        ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
      }

      _RESSTAGE << "time,date,hour";
      _RESSTAGE << ",precip [mm/day]";
      for (p = 0;p < _nSubBasins;p++)
      {
        if ((_pSubBasins[p]->IsGauged()) && (_pSubBasins[p]->IsEnabled()) && (_pSubBasins[p]->GetReservoir() != NULL)) {
          if (_pSubBasins[p]->GetName() == "") { _RESSTAGE << ",ID=" << _pSubBasins[p]->GetID() << " "; }
          else { _RESSTAGE << "," << _pSubBasins[p]->GetName() << " "; }
        }

        for (i = 0; i < _nObservedTS; i++) {
          if (IsContinuousStageObs(_pObservedTS[i], _pSubBasins[p]->GetID()))
          {
            if (_pSubBasins[p]->GetName() == "") { _RESSTAGE << ",ID=" << _pSubBasins[p]->GetID() << " (observed) [m]"; }
            else { _RESSTAGE << "," << _pSubBasins[p]->GetName() << " (observed) [m]"; }
          }
        }

      }
      _RESSTAGE << endl;
    }

    //ReservoirMassBalance.csv
    //--------------------------------------------------------------
    if (Options.write_reservoirMB)
    {
      ofstream RES_MB;
      string name;
      tmpFilename = FilenamePrepare("ReservoirMassBalance.csv", Options);
      RES_MB.open(tmpFilename.c_str());
      if (RES_MB.fail()) {
        ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
      }
      RES_MB << "time,date,hour";
      RES_MB << ",precip [mm/day]";
      for (p = 0;p < _nSubBasins;p++) {
        if ((_pSubBasins[p]->IsGauged()) && (_pSubBasins[p]->IsEnabled()) && (_pSubBasins[p]->GetReservoir() != NULL)) {

          if (_pSubBasins[p]->GetName() == "") { name = to_string(_pSubBasins[p]->GetID()) + "=" + to_string(_pSubBasins[p]->GetID()); }
          else { name = _pSubBasins[p]->GetName(); }
          RES_MB << "," << name << " stage [m]";
          RES_MB << "," << name << " inflow [m3]";
          RES_MB << "," << name << " outflow [m3]"; //from main outlet
          for (int i = 0; i < _pSubBasins[p]->GetReservoir()->GetNumControlStructures(); i++) {
            string name2 = _pSubBasins[p]->GetReservoir()->GetControlName(i);
            RES_MB << "," << name << " ctrl outflow " << name2 << " [m3]";
            RES_MB << "," << name << " ctrl regime " << name2;
          }
          RES_MB << "," << name << " precip [m3]";
          RES_MB << "," << name << " evap [m3]";
          RES_MB << "," << name << " seepage [m3]";
          RES_MB << "," << name << " volume [m3]";
          RES_MB << "," << name << " losses [m3]";
          RES_MB << "," << name << " MB error [m3]";
          RES_MB << "," << name << " constraint";
        }
      }
      RES_MB << endl;
      RES_MB.close();
    }

    //ForcingFunctions.csv
    //--------------------------------------------------------------
    if (Options.write_forcings)
    {
      tmpFilename = FilenamePrepare("ForcingFunctions.csv", Options);
      _FORCINGS.open(tmpFilename.c_str());
      if (_FORCINGS.fail()) {
        ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
      }
      _FORCINGS << "time [d], date, hour, day_angle,";
      _FORCINGS << " rain [mm/d], snow [mm/d], temp [C], temp_daily_min [C], temp_daily_max [C],temp_daily_ave [C],temp_monthly_min [C],temp_monthly_max [C],";
      _FORCINGS << " air_density [kg/m3], air_pressure [KPa], rel_humidity [-],";
      _FORCINGS << " cloud_cover [-],";
      _FORCINGS << " ET_radiation [MJ/m2/d], SW_radiation [MJ/m2/d], net_SW_radiation [MJ/m2/d], LW_incoming [MJ/m2/d], net_LW_radiation [MJ/m2/d], wind_speed [m/s],";
      _FORCINGS << " PET [mm/d], OW_PET [mm/d],";
      _FORCINGS << " daily_correction [-], potential_melt [mm/d]";
      _FORCINGS << endl;
    }
  }
  else if (Options.output_format == OUTPUT_ENSIM)
  {
    WriteEnsimStandardHeaders(Options);
  }
  else if (Options.output_format == OUTPUT_NETCDF)
  {
    WriteNetcdfStandardHeaders(Options);  // creates NetCDF files, writes dimensions and creates variables (without writing actual values)
  }

  //Demands.csv
  //--------------------------------------------------------------
  if ((Options.write_demandfile) && (Options.output_format != OUTPUT_NONE))
  {
    tmpFilename = FilenamePrepare("Demands.csv", Options);
    _DEMANDS.open(tmpFilename.c_str());
    if (_DEMANDS.fail()) {
      ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }

    _DEMANDS << "time,date,hour";
    for (p = 0;p < _nSubBasins;p++) {
      if ((_pSubBasins[p]->IsEnabled()) && (_pSubBasins[p]->IsGauged()) && (_pSubBasins[p]->HasIrrigationDemand())) {
        string name;
        if (_pSubBasins[p]->GetName() == "") { name = "ID=" + to_string(_pSubBasins[p]->GetID()); }
        else { name = _pSubBasins[p]->GetName(); }
        _DEMANDS << "," << name << " [m3/s]";
        _DEMANDS << "," << name << " (demand) [m3/s]";
        _DEMANDS << "," << name << " (min.) [m3/s]";
        _DEMANDS << "," << name << " (unmet) [m3/s]";
      }
    }
    _DEMANDS << endl;
  }

  //WatershedMassEnergyBalance.csv
  //--------------------------------------------------------------
  if (Options.write_mass_bal)
  {
    ofstream MB;
    tmpFilename = FilenamePrepare("WatershedMassEnergyBalance.csv", Options);
    MB.open(tmpFilename.c_str());
    if (MB.fail()) {
      ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }
    MB << "time [d],date,hour";
    for (j = 0;j < _nProcesses;j++) {
      for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
        MB << "," << GetProcessName(_pProcesses[j]->GetProcessType());
        MB << "[" << CStateVariable::GetStateVarUnits(_aStateVarType[_pProcesses[j]->GetFromIndices()[q]]) << "]";
      }
    }
    MB << endl;
    MB << ",,from:";
    for (j = 0;j < _nProcesses;j++) {
      for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
        sv_type typ = GetStateVarType(_pProcesses[j]->GetFromIndices()[q]);
        int     ind = GetStateVarLayer(_pProcesses[j]->GetFromIndices()[q]);
        MB << "," << _pStateVar->SVTypeToString(typ, ind);
      }
    }
    MB << endl;
    MB << ",,to:";
    for (j = 0;j < _nProcesses;j++) {
      for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
        sv_type typ = GetStateVarType(_pProcesses[j]->GetToIndices()[q]);
        int     ind = GetStateVarLayer(_pProcesses[j]->GetToIndices()[q]);
        MB << "," << _pStateVar->SVTypeToString(typ, ind);
      }
    }
    MB << endl;
    MB.close();
  }

  //WatershedMassEnergyBalance.csv
  //--------------------------------------------------------------
  if (Options.write_group_mb != DOESNT_EXIST)
  {
    int kk = Options.write_group_mb;
    ofstream HGMB;
    tmpFilename = FilenamePrepare(_pHRUGroups[kk]->GetName() + "_MassEnergyBalance.csv", Options);

    HGMB.open(tmpFilename.c_str());
    if (HGMB.fail()) {
      ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }
    HGMB << "time [d],date,hour";
    for (j = 0;j < _nProcesses;j++) {
      for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
        HGMB << "," << GetProcessName(_pProcesses[j]->GetProcessType());
        HGMB << "[" << CStateVariable::GetStateVarUnits(_aStateVarType[_pProcesses[j]->GetFromIndices()[q]]) << "]";
      }
    }
    HGMB << endl;
    HGMB << ",,from:";
    for (j = 0;j < _nProcesses;j++) {
      for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
        sv_type typ = GetStateVarType(_pProcesses[j]->GetFromIndices()[q]);
        int     ind = GetStateVarLayer(_pProcesses[j]->GetFromIndices()[q]);
        HGMB << "," << _pStateVar->SVTypeToString(typ, ind);
      }
    }
    HGMB << endl;
    HGMB << ",,to:";
    for (j = 0;j < _nProcesses;j++) {
      for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
        sv_type typ = GetStateVarType(_pProcesses[j]->GetToIndices()[q]);
        int     ind = GetStateVarLayer(_pProcesses[j]->GetToIndices()[q]);
        HGMB << "," << _pStateVar->SVTypeToString(typ, ind);
      }
    }
    HGMB << endl;
    HGMB.close();
  }

  //ExhaustiveMassBalance.csv
  //--------------------------------------------------------------
  if (Options.write_exhaustiveMB)
  {
    ofstream MB;
    tmpFilename = FilenamePrepare("ExhaustiveMassBalance.csv", Options);
    MB.open(tmpFilename.c_str());
    if (MB.fail()) {
      ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }
    MB << "time[d],date,hour";
    bool first;
    for (i = 0;i < _nStateVars;i++) {
      if (CStateVariable::IsWaterStorage(_aStateVarType[i]))
      {
        MB << "," << _pStateVar->SVTypeToString(_aStateVarType[i], _aStateVarLayer[i]);
        first = true;
        for (j = 0;j < _nProcesses;j++) {
          for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
            if (_pProcesses[j]->GetFromIndices()[q] == i) {
              if (!first) { MB << ","; }first = false;
            }
          }
        }
        for (j = 0;j < _nProcesses;j++) {
          for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
            if (_pProcesses[j]->GetToIndices()[q] == i) {
              if (!first) { MB << ","; }first = false;
            }
          }
        }
        for (j = 0;j < _nProcesses;j++) {
          for (int q = 0;q < _pProcesses[j]->GetNumLatConnections();q++) {
            CLateralExchangeProcessABC* pProc = static_cast<CLateralExchangeProcessABC*>(_pProcesses[j]);
            if (pProc->GetLateralToIndices()[q] == i) {
              if (!first) { MB << ","; }first = false;break;
            }
            if (pProc->GetLateralFromIndices()[q] == i) {
              if (!first) { MB << ","; }first = false;break;
            }
          }
        }
        MB << ",,,";//cum, stor, error
      }
    }
    MB << endl;
    MB << ",,";//time,date,hour
    for (i = 0;i < _nStateVars;i++) {
      if (CStateVariable::IsWaterStorage(_aStateVarType[i]))
      {
        for (j = 0;j < _nProcesses;j++) {
          for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
            if (_pProcesses[j]->GetFromIndices()[q] == i) { MB << "," << GetProcessName(_pProcesses[j]->GetProcessType()); }
          }
        }
        for (j = 0;j < _nProcesses;j++) {
          for (int q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
            if (_pProcesses[j]->GetToIndices()[q] == i) { MB << "," << GetProcessName(_pProcesses[j]->GetProcessType()); }
          }
        }
        for (j = 0;j < _nProcesses;j++) {
          for (int q = 0;q < _pProcesses[j]->GetNumLatConnections();q++) {
            CLateralExchangeProcessABC* pProc = static_cast<CLateralExchangeProcessABC*>(_pProcesses[j]);
            if (pProc->GetLateralToIndices()[q] == i) { MB << "," << GetProcessName(_pProcesses[j]->GetProcessType());break; }
            if (pProc->GetLateralFromIndices()[q] == i) { MB << "," << GetProcessName(_pProcesses[j]->GetProcessType());break; }
          }
        }
        MB << ",cumulative,storage,error";
      }
    }
    MB << endl;
    MB.close();
  }

  // HRU Storage files
  //--------------------------------------------------------------
  if (_pOutputGroup != NULL) {
    for (int kk = 0; kk < _pOutputGroup->GetNumHRUs();kk++)
    {
      ofstream HRUSTOR;
      tmpFilename = "HRUStorage_" + to_string(_pOutputGroup->GetHRU(kk)->GetID()) + ".csv";
      tmpFilename = FilenamePrepare(tmpFilename, Options);
      HRUSTOR.open(tmpFilename.c_str());
      if (HRUSTOR.fail()) {
        ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
      }
      int iAtmPrecip = GetStateVarIndex(ATMOS_PRECIP);
      HRUSTOR << "time [d],date,hour,rainfall [mm/day],snowfall [mm/d SWE]";
      for (i = 0;i < GetNumStateVars();i++) {
        if (CStateVariable::IsWaterStorage(_aStateVarType[i])) {
          if (i != iAtmPrecip) {
            HRUSTOR << "," << CStateVariable::GetStateVarLongName(_aStateVarType[i],
              _aStateVarLayer[i],
              _pTransModel) << " [mm]";
            //HRUSTOR<<","<<CStateVariable::SVTypeToString(_aStateVarType[i],_aStateVarLayer[i])<<" [mm]";
          }
        }
      }
      HRUSTOR << ", Total [mm]" << endl;
      HRUSTOR.close();
    }
  }

  // Custom output files
  //--------------------------------------------------------------
  for (int c = 0;c < _nCustomOutputs;c++)
  {
    _pCustomOutputs[c]->WriteFileHeader(Options);
  }

  // Transport output files
  //--------------------------------------------------------------
  _pTransModel->WriteOutputFileHeaders(Options);

  if (Options.management_optimization) {
    _pDO->WriteOutputFileHeaders(Options);
  }

  //raven_debug.csv
  //--------------------------------------------------------------
  if (Options.debug_mode)
  {
    ofstream DEBUG;
    tmpFilename = FilenamePrepare("raven_debug.csv", Options);
    DEBUG.open(tmpFilename.c_str());
    if (DEBUG.fail()) {
      ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }
    DEBUG << "time[d],date,hour,debug1,debug2,debug3,debug4,debug5,debug6,debug7,debug8,debug9,debug10" << endl;
    DEBUG.close();
  }

  //opens and closes diagnostics.csv so that this warning doesn't show up at end of simulation
  //--------------------------------------------------------------
  if ((_nObservedTS > 0) && (_nDiagnostics > 0))
  {
    ofstream DIAG;
    tmpFilename = FilenamePrepare("Diagnostics.csv", Options);
    DIAG.open(tmpFilename.c_str());
    if (DIAG.fail()) {
      ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }
    DIAG.close();
  }

}

//////////////////////////////////////////////////////////////////
/// \brief Writes minor output to file at the end of each timestep (or multiple thereof)
/// \note only thing this modifies should be output streams
/// \param &Options [in] Global model options information
/// \param &tt [in] Local (model) time *at the end of* the pertinent time step
//
void CModel::WriteMinorOutput(const optStruct& Options, const time_struct& tt)
{

  int     i, iCumPrecip, k;
  double  output_int = 0.0;
  double  mod_final = 0.0;
  double  S, currentWater;
  string  thisdate;
  string  thishour;
  bool    silent = true; //for debugging
  bool    quiet = true;  //for debugging
  double  t;

  CSubBasin* pSB;

  string tmpFilename;

  if ((tt.model_time == 0) && (Options.suppressICs)) { return; }

  if ((_pEnsemble != NULL) && (_pEnsemble->DontWriteOutput())) { return; } //specific to EnKF

  //converts the 'write every x timesteps' into a 'write at time y' value
  output_int = Options.output_interval * Options.timestep;
  mod_final = ffmod(tt.model_time, output_int);

  iCumPrecip = GetStateVarIndex(ATMOS_PRECIP);

  if (fabs(mod_final) <= 0.5 * Options.timestep)  //checks to see if sufficiently close to timestep
    //(this should account for any roundoff error in timestep calcs)
  {
    thisdate = tt.date_string;                   //refers to date and time at END of time step
    thishour = DecDaysToHours(tt.julian_day);
    t = tt.model_time;

    time_struct prev;
    JulianConvert(t - Options.timestep, Options.julian_start_day, Options.julian_start_year, Options.calendar, prev); //get start of time step, prev

    double usetime = tt.model_time;
    string usedate = thisdate;
    string usehour = thishour;
    if (Options.period_starting) {
      usedate = prev.date_string;
      usehour = DecDaysToHours(prev.julian_day);
      usetime = tt.model_time - Options.timestep;
    }

    // Console output
    //----------------------------------------------------------------
    if ((quiet) && (!Options.silent) && (tt.day_of_month == 1) && ((tt.julian_day) - floor(tt.julian_day + TIME_CORRECTION) < Options.timestep / 2))
    {
      cout << thisdate << endl;
    }
    if (!silent)
    {
      cout << thisdate << " " << thishour << ":";
      if (t != 0) { cout << " | P: " << setw(6) << setiosflags(ios::fixed) << setprecision(2) << GetAveragePrecip(); }
      else { cout << " | P: ------"; }
    }

    //Write current state of water storage in system to WatershedStorage.csv (ALWAYS DONE if not switched OFF)
    //----------------------------------------------------------------
    if (Options.output_format == OUTPUT_STANDARD)
    {
      if (Options.write_watershed_storage)
      {
        double snowfall = GetAverageSnowfall();
        double precip = GetAveragePrecip();
        double channel_stor = GetTotalChannelStorage();
        double reservoir_stor = GetTotalReservoirStorage();
        double rivulet_stor = GetTotalRivuletStorage();

        _STORAGE << tt.model_time << "," << thisdate << "," << thishour; //instantaneous, so thishour rather than usehour used.

        if (t != 0) { _STORAGE << "," << precip - snowfall << "," << snowfall; }//precip
        else { _STORAGE << ",---,---"; }
        _STORAGE << "," << channel_stor << "," << reservoir_stor << "," << rivulet_stor;

        currentWater = 0.0;
        for (i = 0;i < GetNumStateVars();i++)
        {
          if ((CStateVariable::IsWaterStorage(_aStateVarType[i])) && (i != iCumPrecip))
          {
            S = GetAvgStateVar(i);
            if (!silent) { cout << "  |" << setw(6) << setiosflags(ios::fixed) << setprecision(2) << S; }
            _STORAGE << "," << FormatDouble(S);
            currentWater += S;
          }
        }
        currentWater += channel_stor + rivulet_stor + reservoir_stor;
        if (t == 0) {
          // \todo [fix]: this fixes a mass balance bug in reservoir simulations, but there is certainly a more proper way to do it
          // JRC: I think somehow this is being double counted in the delta V calculations in the first timestep
          for (int p = 0;p < _nSubBasins;p++) {
            if (_pSubBasins[p]->GetReservoir() != NULL) {
              currentWater += _pSubBasins[p]->GetIntegratedReservoirInflow(Options.timestep) / 2.0 / _WatershedArea * MM_PER_METER / M2_PER_KM2;
              currentWater -= _pSubBasins[p]->GetIntegratedOutflow(Options.timestep) / 2.0 / _WatershedArea * MM_PER_METER / M2_PER_KM2;
            }
            //currentWater-=_pSubBasins[p]->GetIntegratedSpecInflow(0,Options.timestep)/2.0/_WatershedArea*MM_PER_METER/M2_PER_KM2;
          }
        }

        _STORAGE << "," << currentWater << "," << _CumulInput << "," << _CumulOutput << "," << FormatDouble((currentWater - _initWater) + (_CumulOutput - _CumulInput));
        _STORAGE << endl;
      }

      //Write hydrographs for gauged watersheds to Hydrographs.csv (ALWAYS DONE)
      //----------------------------------------------------------------
      if (Options.ave_hydrograph)
      {
        _HYDRO << usetime << "," << usedate << "," << usehour;
        if (t != 0) { _HYDRO << "," << GetAveragePrecip(); }//watershed-wide precip
        else { _HYDRO << ",---"; }

        for (int p = 0;p < _nSubBasins;p++)
        {
          pSB = _pSubBasins[p];
          if (pSB->IsGauged() && (pSB->IsEnabled()))
          {
            _HYDRO << "," << pSB->GetIntegratedOutflow(Options.timestep) / (Options.timestep * SEC_PER_DAY);

            for (i = 0; i < _nObservedTS; i++)
            {
              if (IsContinuousFlowObs(_pObservedTS[i], pSB->GetID()))
              {
                double val = _pObservedTS[i]->GetAvgValue(tt.model_time, Options.timestep); //time shift handled in CTimeSeries::Parse
                if ((val != RAV_BLANK_DATA) && (tt.model_time > 0)) { _HYDRO << "," << val; }
                else { _HYDRO << ","; }
              }
            }
            if (Options.write_localflow) {
              _HYDRO << "," << pSB->GetIntegratedLocalOutflow(Options.timestep) / (Options.timestep * SEC_PER_DAY);
            }
            if (pSB->GetReservoir() != NULL) {
              _HYDRO << "," << pSB->GetIntegratedReservoirInflow(Options.timestep) / (Options.timestep * SEC_PER_DAY);
              for (i = 0; i < _nObservedTS; i++)
              {
                if (IsContinuousInflowObs(_pObservedTS[i], pSB->GetID()))
                {
                  double val = _pObservedTS[i]->GetAvgValue(tt.model_time, Options.timestep); //time shift handled in CTimeSeries::Parse
                  if ((val != RAV_BLANK_DATA) && (tt.model_time > 0)) { _HYDRO << "," << val; }
                  else { _HYDRO << ","; }
                }
              }
            }
          }
        }
        _HYDRO << endl;
      }
      else //point value hydrograph or t==0
      {
        if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
        else {
          _HYDRO << t << "," << thisdate << "," << thishour;
          if (t != 0) { _HYDRO << "," << GetAveragePrecip(); }//watershed-wide precip
          else { _HYDRO << ",---"; }
          for (int p = 0;p < _nSubBasins;p++)
          {
            pSB = _pSubBasins[p];
            if (pSB->IsGauged() && (pSB->IsEnabled()))
            {
              _HYDRO << "," << pSB->GetOutflowRate();

              for (i = 0; i < _nObservedTS; i++) {
                if (IsContinuousFlowObs(_pObservedTS[i], pSB->GetID()))
                {
                  double val = _pObservedTS[i]->GetAvgValue(tt.model_time, Options.timestep);
                  if ((val != RAV_BLANK_DATA) && (tt.model_time > 0)) { _HYDRO << "," << val; }
                  else { _HYDRO << ","; }
                }
              }
              if (Options.write_localflow) {
                _HYDRO << "," << pSB->GetLocalOutflowRate();
              }
              if (pSB->GetReservoir() != NULL) {
                _HYDRO << "," << pSB->GetReservoirInflow();
                for (i = 0; i < _nObservedTS; i++)
                {
                  if (IsContinuousInflowObs(_pObservedTS[i], pSB->GetID()))
                  {
                    double val = _pObservedTS[i]->GetAvgValue(tt.model_time, Options.timestep); //time shift handled in CTimeSeries::Parse
                    if ((val != RAV_BLANK_DATA) && (tt.model_time > 0)) { _HYDRO << "," << val; }
                    else { _HYDRO << ","; }
                  }
                }
              }
            }
          }
          _HYDRO << endl;
        }
      }
    }
    else if (Options.output_format == OUTPUT_ENSIM)
    {
      WriteEnsimMinorOutput(Options, tt);
    }
    else if (Options.output_format == OUTPUT_NETCDF)
    {
      WriteNetcdfMinorOutput(Options, tt);
    }

    //Write cumulative mass balance info to HRUGroup_MassEnergyBalance.csv
    //----------------------------------------------------------------
    if (Options.write_group_mb != DOESNT_EXIST)
    {
      if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
      else {
        double sum;
        int kk = Options.write_group_mb;
        ofstream HGMB;
        tmpFilename = FilenamePrepare(_pHRUGroups[kk]->GetName() + "_MassEnergyBalance.csv", Options);
        HGMB.open(tmpFilename.c_str(), ios::app);
        HGMB << usetime << "," << usedate << "," << usehour;
        double areasum = 0.0;
        for (k = 0; k < _nHydroUnits; k++) {
          if (_pHRUGroups[kk]->IsInGroup(k)) {
            areasum += _pHydroUnits[k]->GetArea();
          }
        }
        for (int js = 0;js < _nTotalConnections;js++)
        {
          sum = 0.0;
          for (k = 0; k < _nHydroUnits; k++) {
            if (_pHRUGroups[kk]->IsInGroup(k)) {
              sum += _aCumulativeBal[k][js] * _pHydroUnits[k]->GetArea();
            }
          }
          HGMB << "," << sum / areasum;
        }
        HGMB << endl;
        HGMB.close();
      }
    }

    //Write cumulative mass balance info to WatershedMassEnergyBalance.csv
    //----------------------------------------------------------------
    if (Options.write_mass_bal)
    {
      if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
      else {
        double sum;
        ofstream MB;
        tmpFilename = FilenamePrepare("WatershedMassEnergyBalance.csv", Options);
        MB.open(tmpFilename.c_str(), ios::app);

        MB << usetime << "," << usedate << "," << usehour;
        for (int js = 0;js < _nTotalConnections;js++)
        {
          sum = 0.0;
          for (k = 0;k < _nHydroUnits;k++) {
            if (_pHydroUnits[k]->IsEnabled())
            {
              sum += _aCumulativeBal[k][js] * _pHydroUnits[k]->GetArea();
            }
          }
          MB << "," << sum / _WatershedArea;
        }
        MB << endl;
        MB.close();
      }
    }

    //WaterLevels.csv
    //Write hydrographs for gauged watersheds (ALWAYS DONE)
    //----------------------------------------------------------------
    if (Options.write_waterlevels)
    {
      if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
      else {
        _LEVELS << t << "," << thisdate << "," << thishour;
        if (t != 0) { _LEVELS << "," << GetAveragePrecip(); }//watershed-wide precip
        else { _LEVELS << ",---"; }
        for (int p = 0;p < _nSubBasins;p++)
        {
          pSB = _pSubBasins[p];
          if (pSB->IsGauged() && (pSB->IsEnabled()))
          {
            _LEVELS << "," << pSB->GetWaterLevel();

            for (i = 0; i < _nObservedTS; i++) {
              if (IsContinuousLevelObs(_pObservedTS[i], pSB->GetID()))
              {
                double val = _pObservedTS[i]->GetAvgValue(tt.model_time, Options.timestep);
                if ((val != RAV_BLANK_DATA) && (tt.model_time > 0)) { _LEVELS << "," << val; }
                else { _LEVELS << ","; }
              }
            }
          }
        }
        _LEVELS << endl;
      }
    }



    //ReservoirStages.csv
    //--------------------------------------------------------------
    if ((Options.write_reservoir) && (Options.output_format == OUTPUT_STANDARD))
    {
      int nn = (int)((tt.model_time + TIME_CORRECTION) / Options.timestep);//current timestep index

      if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
      else {
        _RESSTAGE << t << "," << thisdate << "," << thishour << "," << GetAveragePrecip();
        for (int p = 0;p < _nSubBasins;p++)
        {
          pSB = _pSubBasins[p];
          if ((pSB->IsGauged()) && (pSB->IsEnabled()) && (pSB->GetReservoir() != NULL)) {
            _RESSTAGE << "," << pSB->GetReservoir()->GetResStage();
          }
          for (i = 0; i < _nObservedTS; i++) {
            if (IsContinuousStageObs(_pObservedTS[i], pSB->GetID()))
            {
              double val = _pObservedTS[i]->GetAvgValue(tt.model_time, Options.timestep);
              if ((val != RAV_BLANK_DATA) && (tt.model_time > 0)) { _RESSTAGE << "," << val; }
              else { _RESSTAGE << ","; }
            }
          }
        }
        _RESSTAGE << endl;
      }
    }

    //Demands.csv
    //----------------------------------------------------------------
    if ((Options.write_demandfile) && (Options.output_format != OUTPUT_NONE))
    {
      if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
      else {
        _DEMANDS << t << "," << thisdate << "," << thishour;
        for (int p = 0;p < _nSubBasins;p++)
        {
          pSB = _pSubBasins[p];
          if ((pSB->IsEnabled()) && (pSB->IsGauged()) && (pSB->HasIrrigationDemand()))
          {
            double irr = pSB->GetIrrigationDemand(tt.model_time);
            double eF = pSB->GetEnviroMinFlow(tt.model_time);
            double Q = pSB->GetOutflowRate(); //AFTER irrigation removed
            double Qd = pSB->GetDemandDelivery();
            double unmet = max(irr - Qd, 0.0);
            _DEMANDS << "," << Q << "," << irr << "," << eF << "," << unmet;
          }
        }
        _DEMANDS << endl;
      }
    }

    //ReservoirMassBalance.csv
    //----------------------------------------------------------------
    if ((Options.write_reservoirMB) && (Options.output_format != OUTPUT_NONE))
    {
      if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
      else {
        ofstream RES_MB;
        tmpFilename = FilenamePrepare("ReservoirMassBalance.csv", Options);
        RES_MB.open(tmpFilename.c_str(), ios::app);
        if (RES_MB.fail()) {
          ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
        }

        RES_MB << usetime << "," << usedate << "," << usehour << "," << GetAveragePrecip();
        double in, out, loss, stor, oldstor, precip, evap, seepage, stage;
        for (int p = 0;p < _nSubBasins;p++)
        {
          pSB = _pSubBasins[p];
          if ((pSB->IsGauged()) && (pSB->IsEnabled()) && (pSB->GetReservoir() != NULL))
          {
            string name, constraint_str;
            if (pSB->GetName() == "") { name = to_string(pSB->GetID()); }
            else { name = pSB->GetName(); }

            stage = pSB->GetReservoir()->GetResStage();//m
            in = pSB->GetIntegratedReservoirInflow(Options.timestep);//m3
            out = pSB->GetIntegratedOutflow(Options.timestep);//m3
            RES_MB << "," << stage;
            RES_MB << "," << in << "," << out;
            for (int i = 0; i < pSB->GetReservoir()->GetNumControlStructures(); i++) {
              double Qc = pSB->GetReservoir()->GetIntegratedControlOutflow(i, Options.timestep);
              RES_MB << "," << Qc;//m3
              RES_MB << "," << pSB->GetReservoir()->GetRegimeName(i, prev);//evaluated at start of timestep
              if (pSB->GetReservoir()->GetControlFlowTarget(i) != pSB->GetDownstreamID()) { out += Qc; }
            }
            stor = pSB->GetReservoir()->GetStorage();//m3
            oldstor = pSB->GetReservoir()->GetOldStorage();//m3
            loss = pSB->GetReservoir()->GetReservoirLosses(Options.timestep);//m3 = GW+ET
            precip = pSB->GetReservoir()->GetReservoirPrecipGains(Options.timestep);//m3
            evap = pSB->GetReservoir()->GetReservoirEvapLosses(Options.timestep);//m3
            seepage = pSB->GetReservoir()->GetReservoirGWLosses(Options.timestep);//m3
            constraint_str = pSB->GetReservoir()->GetCurrentConstraint();
            if (tt.model_time == 0.0) { in = 0.0; }
            RES_MB << "," << precip << "," << evap << "," << seepage;
            RES_MB << "," << stor << "," << loss << "," << in - out - loss + precip - (stor - oldstor) << "," << constraint_str;
          }
        }
        RES_MB << endl;
        RES_MB.close();
      }
    }


    // ExhaustiveMassBalance.csv
    //--------------------------------------------------------------
    if (Options.write_exhaustiveMB)
    {
      if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
      else {
        int j, js, q;
        double cumsum;
        double sum;

        ofstream MB;
        tmpFilename = FilenamePrepare("ExhaustiveMassBalance.csv", Options);
        MB.open(tmpFilename.c_str(), ios::app);

        MB << usetime << "," << usedate << "," << usehour;
        for (i = 0;i < _nStateVars;i++)
        {
          if (CStateVariable::IsWaterStorage(_aStateVarType[i]))
          {
            cumsum = 0.0;
            js = 0;
            for (j = 0;j < _nProcesses;j++) {
              for (q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
                if (_pProcesses[j]->GetFromIndices()[q] == i)
                {
                  sum = 0.0;
                  for (k = 0;k < _nHydroUnits;k++) {
                    if (_pHydroUnits[k]->IsEnabled())
                    {
                      sum += _aCumulativeBal[k][js] * _pHydroUnits[k]->GetArea();
                    }
                  }
                  MB << "," << -sum / _WatershedArea;
                  cumsum -= sum / _WatershedArea;
                }
                js++;
              }
            }
            js = 0;
            for (j = 0;j < _nProcesses;j++) {
              for (q = 0;q < _pProcesses[j]->GetNumConnections();q++) {
                if (_pProcesses[j]->GetToIndices()[q] == i)
                {
                  sum = 0.0;
                  for (k = 0;k < _nHydroUnits;k++) {
                    if (_pHydroUnits[k]->IsEnabled())
                    {
                      sum += _aCumulativeBal[k][js] * _pHydroUnits[k]->GetArea();
                    }
                  }
                  MB << "," << sum / _WatershedArea;
                  cumsum += sum / _WatershedArea;
                }
                js++;
              }
            }
            js = 0;
            bool found;
            for (j = 0;j < _nProcesses;j++) {
              sum = 0;
              found = false;
              for (q = 0;q < _pProcesses[j]->GetNumLatConnections();q++) {
                CLateralExchangeProcessABC* pProc = static_cast<CLateralExchangeProcessABC*>(_pProcesses[j]);
                if (pProc->GetLateralToIndices()[q] == i) {
                  sum += _aCumulativeLatBal[js];found = true;
                }
                if (pProc->GetLateralFromIndices()[q] == i) {
                  sum -= _aCumulativeLatBal[js];found = true;
                }
                js++;
              }
              if ((_pProcesses[j]->GetNumLatConnections() > 0) && (found == true)) {
                MB << "," << sum / _WatershedArea;
                cumsum += sum / _WatershedArea;
              }
            }

            //Cumulative, storage, error
            double Initial_i = 0.0; //< \todo [bug] need to evaluate and store initial storage actross watershed!!!
            MB << "," << cumsum << "," << GetAvgStateVar(i) << "," << cumsum - GetAvgStateVar(i) - Initial_i;
          }
        }
        MB << endl;
        MB.close();
      }
    }

    // ForcingFunctions.csv
    //----------------------------------------------------------------
    if ((Options.write_forcings) && (Options.output_format == OUTPUT_STANDARD))
    {
      if ((Options.period_starting) && (t == 0)) {}//don't write anything at time zero
      else {
        force_struct* pFave;
        force_struct faveStruct = GetAverageForcings();
        pFave = &faveStruct;
        _FORCINGS << usetime << "," << usedate << "," << usehour << ",";
        _FORCINGS << pFave->day_angle << ",";
        _FORCINGS << pFave->precip * (1 - pFave->snow_frac) << ",";
        _FORCINGS << pFave->precip * (pFave->snow_frac) << ",";
        _FORCINGS << pFave->temp_ave << ",";
        _FORCINGS << pFave->temp_daily_min << ",";
        _FORCINGS << pFave->temp_daily_max << ",";
        _FORCINGS << pFave->temp_daily_ave << ",";
        _FORCINGS << pFave->temp_month_min << ",";
        _FORCINGS << pFave->temp_month_max << ",";
        _FORCINGS << pFave->air_dens << ",";
        _FORCINGS << pFave->air_pres << ",";
        _FORCINGS << pFave->rel_humidity << ",";
        _FORCINGS << pFave->cloud_cover << ",";
        _FORCINGS << pFave->ET_radia << ",";
        _FORCINGS << pFave->SW_radia << ",";
        _FORCINGS << pFave->SW_radia_net << ",";
        //_FORCINGS<<pFave->SW_radia_subcan<<",";
        _FORCINGS << pFave->LW_incoming << ",";
        _FORCINGS << pFave->LW_radia_net << ",";
        _FORCINGS << pFave->wind_vel << ",";
        _FORCINGS << pFave->PET << ",";
        _FORCINGS << pFave->OW_PET << ",";
        _FORCINGS << pFave->subdaily_corr << ",";
        _FORCINGS << pFave->potential_melt;
        _FORCINGS << endl;
      }
    }

    // Transport output files
    //--------------------------------------------------------------
    _pTransModel->WriteMinorOutput(Options, tt);

    if (Options.management_optimization) {
      _pDO->WriteMinorOutput(Options, tt);
    }

    // raven_debug.csv
    //--------------------------------------------------------------
    if (Options.debug_mode)
    {
      ofstream DEBUG;
      tmpFilename = FilenamePrepare("raven_debug.csv", Options);
      DEBUG.open(tmpFilename.c_str(), ios::app);
      DEBUG << t << "," << thisdate << "," << thishour;
      for (i = 0;i < 10;i++) { DEBUG << "," << g_debug_vars[i]; }
      DEBUG << endl;
      DEBUG.close();
    }

    // HRU storage output
    //--------------------------------------------------------------
    if (_pOutputGroup != NULL)
    {
      for (int kk = 0;kk < _pOutputGroup->GetNumHRUs();kk++)
      {
        ofstream HRUSTOR;
        tmpFilename = "HRUStorage_" + to_string(_pOutputGroup->GetHRU(kk)->GetID()) + ".csv";
        tmpFilename = FilenamePrepare(tmpFilename, Options);
        HRUSTOR.open(tmpFilename.c_str(), ios::app);

        const force_struct* F = _pOutputGroup->GetHRU(kk)->GetForcingFunctions();

        HRUSTOR << tt.model_time << "," << thisdate << "," << thishour;//instantaneous -no period starting correction

        if (t != 0) { HRUSTOR << "," << F->precip * (1 - F->snow_frac) << "," << F->precip * (F->snow_frac); }//precip
        else { HRUSTOR << ",---,---"; }

        currentWater = 0;
        for (i = 0;i < GetNumStateVars();i++)
        {
          if ((CStateVariable::IsWaterStorage(_aStateVarType[i])) && (i != iCumPrecip))
          {
            S = _pOutputGroup->GetHRU(kk)->GetStateVarValue(i);
            HRUSTOR << "," << S;
            currentWater += S;
          }
        }
        HRUSTOR << "," << currentWater;
        HRUSTOR << endl;
        HRUSTOR.close();
      }
    }
  } // end of write output interval if statement

  // Custom output files
  //--------------------------------------------------------------
  for (int c = 0;c < _nCustomOutputs;c++)
  {
    _pCustomOutputs[c]->WriteCustomOutput(tt, Options);
  }

  // Write major output, if necessary
  //--------------------------------------------------------------
  if ((_nOutputTimes > 0) && (_currOutputTimeInd < _nOutputTimes) && (tt.model_time > _aOutputTimes[_currOutputTimeInd] - 0.5 * Options.timestep))
  {
    string thishour = DecDaysToHours(tt.julian_day);
    _currOutputTimeInd++;
    tmpFilename = "state_" + tt.date_string.substr(0, 4) + tt.date_string.substr(5, 2) + tt.date_string.substr(8, 2) + "_" + thishour.substr(0, 2) + thishour.substr(3, 2);
    WriteMajorOutput(Options, tt, tmpFilename, false);
  }

}


//////////////////////////////////////////////////////////////////
/// \brief Replaces the WriteOutputFileHeaders function by not requiring the Options structure as an argument
/// \param &tt [in] Local (model) time *at the end of* the pertinent time step
/// \param solfile [in] Name of the solution file to be written
/// \param final [in] Whether this is the final solution file to be written
//
void CModel::WriteMajorOutput(const time_struct& tt, string solfile, bool final) const
{
  int i, k;
  string tmpFilename;
  const optStruct* Options = this->_pOptStruct;  // just to make the code more readable

  if (Options->output_format == OUTPUT_NONE) { return; } //:SuppressOutput is on

  // WRITE {RunName}_solution.rvc - final state variables file
  ofstream RVC;
  tmpFilename = FilenamePrepare(solfile + ".rvc", *_pOptStruct);
  RVC.open(tmpFilename.c_str());
  if (RVC.fail()) {
    WriteWarning(("CModel::WriteMajorOutput: Unable to open output file " + tmpFilename + " for writing.").c_str(),
      Options->noisy);
  }
  RVC << ":TimeStamp " << tt.date_string << " " << DecDaysToHours(tt.julian_day) << endl;

  //Header--------------------------
  //write in blocks of 80 state variables
  int mini, maxi;
  int M = 80;
  for (int j = 0; j < ceil(GetNumStateVars() / (double)(M)); j++) {
    mini = j * M;
    maxi = min(GetNumStateVars(), (j + 1) * M);
    RVC << ":HRUStateVariableTable" << endl;
    RVC << "  :Attributes,";
    for (i = mini;i < maxi;i++)
    {
      RVC << _pStateVar->SVTypeToString(_aStateVarType[i], _aStateVarLayer[i]);
      if (i != GetNumStateVars() - 1) { RVC << ","; }
    }
    RVC << endl;
    RVC << "  :Units,";
    for (i = mini;i < maxi;i++)
    {
      RVC << CStateVariable::GetStateVarUnits(_aStateVarType[i]);
      if (i != GetNumStateVars() - 1) { RVC << ","; }
    }
    RVC << endl;
    //Data----------------------------
    for (k = 0;k < _nHydroUnits;k++)
    {
      RVC << std::fixed; RVC.precision(5);
      RVC << "  " << _pHydroUnits[k]->GetID() << ",";
      for (i = mini;i < maxi;i++)
      {
        RVC << _pHydroUnits[k]->GetStateVarValue(i);
        if (i != GetNumStateVars() - 1) { RVC << ","; }
      }
      RVC << endl;
    }
    RVC << ":EndHRUStateVariableTable" << endl;
  }
  //By basin------------------------
  RVC << ":BasinStateVariables" << endl;
  for (int p = 0;p < _nSubBasins;p++) {
    RVC << "  :BasinIndex " << _pSubBasins[p]->GetID() << ",";
    _pSubBasins[p]->WriteToSolutionFile(RVC);
  }
  RVC << ":EndBasinStateVariables" << endl;

  _pTransModel->WriteMajorOutput(RVC);

  RVC.close();

  // SubbasinProperties.csv
  //--------------------------------------------------------------
  if (Options->write_basinfile) {
    ofstream BAS;
    tmpFilename = FilenamePrepare("SubbasinProperties.csv", *_pOptStruct);
    BAS.open(tmpFilename.c_str());
    if (BAS.fail()) {
      WriteWarning(("CModel::WriteMinorOutput: Unable to open output file " + tmpFilename + " for writing.").c_str(), Options->noisy);
    }
    BAS << "ID,Qref[m3/s],reach_length[m],area[km2],drainage_area[km2],t_conc[d],t_peak[d],gamma_sh,gamma_sc[1/d],celerity[m/s],diffusivity[m2/s],N,UH[0],UH[1],UH[2],..." << endl;
    for (int pp = 0;pp < _nSubBasins;pp++) {
      BAS << _pSubBasins[pp]->GetID() << ",  " << _pSubBasins[pp]->GetReferenceFlow();
      BAS << "," << _pSubBasins[pp]->GetReachLength();
      BAS << "," << _pSubBasins[pp]->GetBasinArea();
      BAS << "," << _pSubBasins[pp]->GetDrainageArea();
      BAS << "," << _pSubBasins[pp]->GetBasinProperties("TIME_CONC");
      BAS << "," << _pSubBasins[pp]->GetBasinProperties("TIME_TO_PEAK");
      BAS << "," << _pSubBasins[pp]->GetBasinProperties("GAMMA_SHAPE");
      BAS << "," << _pSubBasins[pp]->GetBasinProperties("GAMMA_SCALE");
      BAS << "," << _pSubBasins[pp]->GetBasinProperties("CELERITY");
      BAS << "," << _pSubBasins[pp]->GetBasinProperties("DIFFUSIVITY");
      BAS << "," << _pSubBasins[pp]->GetLatHistorySize();
      for (int i = 0; i < _pSubBasins[pp]->GetLatHistorySize(); i++) {
        BAS << "," << _pSubBasins[pp]->GetUnitHydrograph()[i];
      }
      BAS << endl;
    }
    BAS.close();
  }

  // rating_curves.csv
  //--------------------------------------------------------------
  if (Options->write_channels) {
    WriteRatingCurves(*Options);
  }
}


//////////////////////////////////////////////////////////////////
/// \brief Writes major output to file at the end of simulation
/// \details Writes:
/// - Solution file of all state variables; and
/// - Autogenerated parameters
///
/// \param &Options [in] Global model options information
//
void CModel::WriteMajorOutput(const optStruct& Options, const time_struct& tt, string solfile, bool final) const
{
  WriteMajorOutput(tt, solfile, final);
}
//////////////////////////////////////////////////////////////////
/// \brief Writes simple output to file
/// \note  written at start of time step before external script is read, after forcings processed.
/// \param &Options      [in] Global model options information
/// \param &tt           [in] time structure
//
void CModel::WriteSimpleOutput(const optStruct& Options, const time_struct& tt)
{
  if (Options.write_simpleout)
  {
    string thisdate = tt.date_string;                   //refers to date and time at START of time step
    string thishour = DecDaysToHours(tt.julian_day);
    double thistime = tt.model_time;

    force_struct* pFave;
    force_struct faveStruct = GetAverageForcings();
    pFave = &faveStruct;

    ofstream _SIMPLE;
    string tmpFilename = FilenamePrepare("simple_output.csv", Options);
    _SIMPLE.open(tmpFilename.c_str());
    if (_SIMPLE.fail()) {
      ExitGracefully(("CModel::WriteSimpleOutput: unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }

    _SIMPLE << thistime << ", " << thisdate << ", " << thishour << ", ";
    _SIMPLE << pFave->precip << ", ";
    _SIMPLE << pFave->temp_daily_min << ", ";
    _SIMPLE << pFave->temp_daily_max << ", ";
    _SIMPLE << pFave->SW_radia << ", ";
    _SIMPLE << pFave->day_length << ", ";
    _SIMPLE << endl;

    _SIMPLE.close();
  }
}
//////////////////////////////////////////////////////////////////
/// \brief Writes progress file in JSON format (mainly for PAVICS runs)
///        Looks like:
///               {
///               	"% progress": 65,
///               	"seconds remaining": 123
///               }
///
/// \note  Does not account for initialization (reading) and final writing of model outputs. Only pure modeling time.
///
/// \param &Options      [in] Global model options information
/// \param &elapsed_time [in] elapsed time  (computational time markers)
/// \param elapsed_steps [in] elapsed number of simulation steps to perform (to determine % progress)
/// \param total_steps   [in]   total number of simulation steps to perform (to determine % progress)
//
void CModel::WriteProgressOutput(const optStruct& Options, clock_t elapsed_time, int elapsed_steps, int total_steps)
{
  if (Options.pavics)
  {
    ofstream PROGRESS;
    PROGRESS.open((Options.main_output_dir + "Raven_progress.txt").c_str());
    if (PROGRESS.fail()) {
      PROGRESS.close();
      ExitGracefully("ParseInput:: Unable to open Raven_progress.txt. Bad output directory specified?", RUNTIME_ERR);
    }

    float total_time = (float(total_steps) * float(elapsed_time) / float(elapsed_steps)) / CLOCKS_PER_SEC;
    if (Options.benchmarking) { total_time = float(elapsed_time) / CLOCKS_PER_SEC; }
    PROGRESS << "{" << endl;
    PROGRESS << "       \"% progress\": " << int(float(elapsed_steps) * 100.0 / float(total_steps)) << "," << endl;
    PROGRESS << "       \"seconds remaining\": " << total_time - float(elapsed_time) / CLOCKS_PER_SEC << endl;
    PROGRESS << "}" << endl;

    PROGRESS.close();
  }
}

//////////////////////////////////////////////////////////////////
/// \brief Writes model summary information to screen
/// \param &Options [in] Global model options information
//
void CModel::SummarizeToScreen(const optStruct& Options) const
{
  int rescount = 0;
  for (int p = 0; p < _nSubBasins; p++) {
    if (_pSubBasins[p]->GetReservoir() != NULL) { rescount++; }
  }
  int disablecount = 0;
  double allarea = 0.0;
  for (int k = 0;k < _nHydroUnits; k++) {
    if (!_pHydroUnits[k]->IsEnabled()) { disablecount++; }
    allarea += _pHydroUnits[k]->GetArea();
  }
  int SBdisablecount = 0;
  for (int p = 0;p < _nSubBasins; p++) {
    if (!_pSubBasins[p]->IsEnabled()) { SBdisablecount++; }
  }
  time_struct tt, tt2;
  double day;
  int year;
  JulianConvert(0, Options.julian_start_day, Options.julian_start_year, Options.calendar, tt);
  AddTime(Options.julian_start_day, Options.julian_start_year, Options.duration, Options.calendar, day, year);
  JulianConvert(0, day, year, Options.calendar, tt2);

  if (!Options.silent)
  {
    cout << "==MODEL SUMMARY=======================================" << endl;
    cout << "       Model Run: " << Options.run_name << endl;
    cout << "      Start time: " << tt.date_string << endl;
    cout << "        End time: " << tt2.date_string << " (duration=" << Options.duration << " days)" << endl;
    cout << "    rvi filename: " << Options.rvi_filename << endl;
    cout << "Output Directory: " << Options.main_output_dir << endl;
    cout << "     # SubBasins: " << GetNumSubBasins() << " (" << rescount << " reservoirs) (" << SBdisablecount << " disabled)" << endl;
    cout << "          # HRUs: " << GetNumHRUs() << " (" << disablecount << " disabled)" << endl;
    cout << "        # Gauges: " << GetNumGauges() << endl;
    cout << "#State Variables: " << GetNumStateVars() << endl;
    for (int i = 0;i < GetNumStateVars();i++) {
      //don't write if convolution storage or advection storage?
      cout << "                - ";
      cout << CStateVariable::GetStateVarLongName(_aStateVarType[i],
        _aStateVarLayer[i],
        _pTransModel) << " (";
      cout << _pStateVar->SVTypeToString(_aStateVarType[i], _aStateVarLayer[i]) << ")" << endl;
    }
    cout << "     # Processes: " << GetNumProcesses() << endl;
    for (int j = 0;j < GetNumProcesses();j++)
    {
      cout << "                - ";
      cout << GetProcessName(GetProcessType(j)) << endl;
    }
    cout << "    #Connections: " << _nTotalConnections << endl;
    cout << "#Lat.Connections: " << _nTotalLatConnections << endl;
    cout << "        Duration: " << Options.duration << " d" << endl;
    cout << "       Time step: " << Options.timestep << " d (" << (Options.timestep * MIN_PER_DAY) << " min)" << endl;
    cout << "  Watershed Area: " << _WatershedArea << " km2 (simulated) of " << allarea << " km2" << endl;
    cout << "======================================================" << endl;
    cout << endl;

  }
}

//////////////////////////////////////////////////////////////////
/// \brief run model diagnostics (at end of simulation)
///
/// \param &Options [in] global model options
//
void CModel::RunDiagnostics(const optStruct& Options)
{
  if ((_nObservedTS == 0) || (_nDiagnostics == 0)) { return; }

  ofstream DIAG;
  string tmpFilename;
  tmpFilename = FilenamePrepare("Diagnostics.csv", Options);
  DIAG.open(tmpFilename.c_str());
  if (DIAG.fail()) {
    ExitGracefully(("CModel::WriteOutputFileHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
  }
  //header
  DIAG << "observed_data_series,filename,";
  for (int j = 0; j < _nDiagnostics;j++) {
    DIAG << _pDiagnostics[j]->GetName() << ",";
  }
  DIAG << endl;
  //body
  bool skip;
  for (int d = 0; d < _nDiagPeriods; d++) {
    double starttime = _pDiagPeriods[d]->GetStartTime();
    double endtime = _pDiagPeriods[d]->GetEndTime();
    comparison compare = _pDiagPeriods[d]->GetComparison();
    double     thresh = _pDiagPeriods[d]->GetThreshold();
    for (int i = 0;i < _nObservedTS;i++)
    {
      skip = false;
      string datatype = _pObservedTS[i]->GetName();
      if ((datatype == "HYDROGRAPH") || (datatype == "RESERVOIR_STAGE") ||
        (datatype == "RESERVOIR_INFLOW") || (datatype == "RESERVOIR_NETINFLOW") || (datatype == "WATER_LEVEL") ||
        (datatype == "STREAM_CONCENTRATION") || (datatype == "STREAM_TEMPERATURE"))
      {
        CSubBasin* pBasin = GetSubBasinByID(_pObservedTS[i]->GetLocID());
        if ((pBasin == NULL) || (!pBasin->IsEnabled())) { skip = true; }
      }

      if (!skip)
      {

        DIAG << _pObservedTS[i]->GetName() << "_" << _pDiagPeriods[d]->GetName() << "[" << _pObservedTS[i]->GetLocID() << "]," << _pObservedTS[i]->GetSourceFile() << ",";//append to end of name for backward compatibility
        for (int j = 0; j < _nDiagnostics;j++) {
          DIAG << _pDiagnostics[j]->CalculateDiagnostic(_pModeledTS[i], _pObservedTS[i], _pObsWeightTS[i], starttime, endtime, compare, thresh, Options) << ",";
        }
        DIAG << endl;
      }
    }
    //compute aggregate diagnostics
    for (int ii = 0; ii < _nAggDiagnostics; ii++)
    {
      int            kk = _pAggDiagnostics[ii]->kk;
      string agg_datatype = _pAggDiagnostics[ii]->datatype;

      string name = "";//"Average_HYDROGRAPH_GroupX"
      switch (_pAggDiagnostics[ii]->aggtype)
      {
      case AGG_AVERAGE:               name += "Average"; break;
      case AGG_MAXIMUM:               name += "Maximum"; break;
      case AGG_MINIMUM:               name += "Minimum"; break;
      case AGG_MEDIAN:                name += "Median";  break;
      }
      //name+=AggStatToString(_pAggDiagnostics[ii]->aggtype); //replace above with this
      name += "_" + agg_datatype;
      name += "_" + _pDiagPeriods[d]->GetName();
      if (kk != DOESNT_EXIST) {
        if (agg_datatype == "HYDROGRAPH") { name += "_" + _pSBGroups[kk]->GetName(); } // \todo[funct]- handle more datatypes
      }

      DIAG << name << ",[multiple],";
      for (int j = 0; j < _nDiagnostics;j++) {
        DIAG << CalculateAggDiagnostic(ii, j, starttime, endtime, compare, thresh, Options) << ",";
      }
      DIAG << endl;
    }
  }

  DIAG.close();

  //reset for ensemble mode
  for (int i = 0;i < _nObservedTS;i++)
  {
    _aObsIndex[i] = 0;
  }
}


//////////////////////////////////////////////////////////////////
/// \brief run model diagnostics (at end of simulation)
///
/// \param &Options [in] global model options
//
double CModel::CalculateAggDiagnostic(const int ii, const int j, const double& starttime, const double& endtime, const comparison compare, const double& thresh, const optStruct& Options)
{
  bool skip;
  double val;
  double stat = 0;
  int N = 0;
  agg_stat       type = _pAggDiagnostics[ii]->aggtype;
  int            kk = _pAggDiagnostics[ii]->kk;
  string agg_datatype = _pAggDiagnostics[ii]->datatype;

  double* data = new double[_nObservedTS]; //maximum number of diagnostics that could be considered

  if (type == AGG_AVERAGE) { stat = 0; }
  else if (type == AGG_MAXIMUM) { stat = -ALMOST_INF; }
  else if (type == AGG_MINIMUM) { stat = ALMOST_INF; }
  else if (type == AGG_MEDIAN) { stat = 0; }
  for (int i = 0;i < _nObservedTS;i++)
  {
    data[i] = 0;
    skip = false;
    string datatype = _pObservedTS[i]->GetName();
    if (datatype != agg_datatype) { skip = true; }
    else if ((datatype == "HYDROGRAPH") || (datatype == "RESERVOIR_STAGE") ||
      (datatype == "RESERVOIR_INFLOW") || (datatype == "RESERVOIR_NETINFLOW") || (datatype == "WATER_LEVEL") ||
      (datatype == "STREAM_CONCENTRATION") || (datatype == "STREAM_TEMPERATURE")) //subbasin-linked metrics
    {
      CSubBasin* pBasin = GetSubBasinByID(_pObservedTS[i]->GetLocID());
      if ((pBasin == NULL) || (!pBasin->IsEnabled())) { skip = true; }
      if ((pBasin != NULL) && (kk != DOESNT_EXIST) && (!IsInSubBasinGroup(pBasin->GetID(), _pSBGroups[kk]->GetName()))) { skip = true; }
    }
    else { //HRU-linked
      if ((kk != DOESNT_EXIST) && (!IsInHRUGroup(_pObservedTS[i]->GetLocID(), _pHRUGroups[kk]->GetName()))) { skip = true; }
    }

    if (!skip)
    {
      val = _pDiagnostics[j]->CalculateDiagnostic(_pModeledTS[i], _pObservedTS[i], _pObsWeightTS[i], starttime, endtime, compare, thresh, Options);

      if (type == AGG_AVERAGE) { stat += val; N++; }
      else if (type == AGG_MAXIMUM) { upperswap(stat, val);N++; }
      else if (type == AGG_MINIMUM) { lowerswap(stat, val);N++; }
      else if (type == AGG_MEDIAN) { data[N] = val; N++; }
    }
  }
  int n;
  if (type == AGG_MEDIAN) { quickSort(data, 0, N - 1);n = (int)rvn_floor((double)(N) / 2.0 + 0.01); }

  if (N == 0) { return -ALMOST_INF; }
  if (type == AGG_AVERAGE) { stat /= N; }
  else if ((type == AGG_MEDIAN) && (N % 2 == 1)) { stat = data[n]; } //odd N
  else if ((type == AGG_MEDIAN) && (N % 2 == 0)) { stat = 0.5 * (data[n] + data[n - 1]); } //even N
  delete[] data;
  return stat;
}

//////////////////////////////////////////////////////////////////
/// \brief Writes output headers for WatershedStorage.tb0 and Hydrographs.tb0
///
/// \param &Options [in] global model options
//
void CModel::WriteEnsimStandardHeaders(const optStruct& Options)
{
  int i;
  time_struct tt, tt2;

  JulianConvert(0.0, Options.julian_start_day, Options.julian_start_year, Options.calendar, tt);//start of the timestep
  JulianConvert(Options.timestep, Options.julian_start_day, Options.julian_start_year, Options.calendar, tt2);//end of the timestep

  //WatershedStorage.tb0
  //--------------------------------------------------------------
  int iAtmPrecip = GetStateVarIndex(ATMOS_PRECIP);
  string tmpFilename;

  if (Options.write_watershed_storage)
  {
    tmpFilename = FilenamePrepare("WatershedStorage.tb0", Options);

    _STORAGE.open(tmpFilename.c_str());
    if (_STORAGE.fail()) {
      ExitGracefully(("CModel::WriteEnsimStandardHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
    }
    _STORAGE << "#########################################################################" << endl;
    _STORAGE << ":FileType tb0 ASCII EnSim 1.0" << endl;
    _STORAGE << "#" << endl;
    _STORAGE << ":Application   Raven" << endl;
    if (!Options.benchmarking) {
      _STORAGE << ":Version       " << Options.version << endl;
      _STORAGE << ":CreationDate  " << GetCurrentMachineTime() << endl;
    }
    _STORAGE << "#" << endl;
    _STORAGE << "#------------------------------------------------------------------------" << endl;
    _STORAGE << "#" << endl;
    _STORAGE << ":RunName       " << Options.run_name << endl;
    _STORAGE << ":Format         Instantaneous" << endl;
    _STORAGE << "#" << endl;

    if (Options.suppressICs) {
      _STORAGE << ":StartTime " << tt2.date_string << " " << DecDaysToHours(tt2.julian_day) << endl;
    }
    else {
      _STORAGE << ":StartTime " << tt.date_string << " " << DecDaysToHours(tt.julian_day) << endl;
    }
    if (Options.timestep != 1.0) { _STORAGE << ":DeltaT " << DecDaysToHours(Options.timestep) << endl; }
    else { _STORAGE << ":DeltaT 24:00:00.00" << endl; }
    _STORAGE << "#" << endl;

    _STORAGE << ":ColumnMetaData" << endl;
    _STORAGE << "  :ColumnName rainfall snowfall \"Channel storage\" \"Rivulet storage\"";
    for (i = 0;i < GetNumStateVars();i++) {
      if ((CStateVariable::IsWaterStorage(_aStateVarType[i])) && (i != iAtmPrecip)) {
        _STORAGE << " \"" << CStateVariable::GetStateVarLongName(_aStateVarType[i],
          _aStateVarLayer[i],
          _pTransModel) << "\"";
      }
    }
    _STORAGE << " \"Total storage\" \"Cum. precip\" \"Cum. outflow\" \"MB error\"" << endl;

    _STORAGE << "  :ColumnUnits mm/d mm/d mm mm ";
    for (i = 0;i < GetNumStateVars();i++) {
      if ((CStateVariable::IsWaterStorage(_aStateVarType[i])) && (i != iAtmPrecip)) { _STORAGE << " mm"; }
    }
    _STORAGE << " mm mm mm mm" << endl;

    _STORAGE << "  :ColumnType float float float float";
    for (i = 0;i < GetNumStateVars();i++) {
      if ((CStateVariable::IsWaterStorage(_aStateVarType[i])) && (i != iAtmPrecip)) { _STORAGE << " float"; }
    }
    _STORAGE << " float float float float" << endl;

    _STORAGE << "  :ColumnFormat -1 -1 0 0";
    for (i = 0; i < GetNumStateVars(); i++) {
      if ((CStateVariable::IsWaterStorage(_aStateVarType[i])) && (i != iAtmPrecip)) {
        _STORAGE << " 0";
      }
    }
    _STORAGE << " 0 0 0 0" << endl;

    _STORAGE << ":EndColumnMetaData" << endl;

    _STORAGE << ":EndHeader" << endl;
  }

  //Hydrographs.tb0
  //--------------------------------------------------------------
  tmpFilename = FilenamePrepare("Hydrographs.tb0", Options);
  _HYDRO.open(tmpFilename.c_str());
  if (_HYDRO.fail()) {
    ExitGracefully(("CModel::WriteEnsimStandardHeaders: Unable to open output file " + tmpFilename + " for writing.").c_str(), FILE_OPEN_ERR);
  }
  _HYDRO << "#########################################################################" << endl;
  _HYDRO << ":FileType tb0 ASCII EnSim 1.0" << endl;
  _HYDRO << "#" << endl;
  _HYDRO << ":Application   Raven" << endl;
  if (!Options.benchmarking) {
    _HYDRO << ":Version       " << Options.version << endl;
    _HYDRO << ":CreationDate  " << GetCurrentMachineTime() << endl;
  }
  _HYDRO << "#" << endl;
  _HYDRO << "#------------------------------------------------------------------------" << endl;
  _HYDRO << "#" << endl;
  _HYDRO << ":RunName       " << Options.run_name << endl;
  _HYDRO << "#" << endl;

  if (Options.ave_hydrograph) {
    _HYDRO << ":Format         PeriodEnding" << endl;
  }
  else {
    _HYDRO << ":Format         Instantaneous" << endl;
  }
  if (((Options.period_ending) && (Options.ave_hydrograph)) || (Options.suppressICs)) {
    _HYDRO << ":StartTime " << tt2.date_string << " " << DecDaysToHours(tt2.julian_day) << endl;
  }
  else {
    _HYDRO << ":StartTime " << tt.date_string << " " << DecDaysToHours(tt.julian_day) << endl;
  }

  if (Options.timestep != 1.0) { _HYDRO << ":DeltaT " << DecDaysToHours(Options.timestep) << endl; }
  else { _HYDRO << ":DeltaT 24:00:00.00" << endl; }
  _HYDRO << "#" << endl;

  double val = 0; //snapshot hydrograph
  double val2 = 1;
  if (Options.ave_hydrograph) { val = 1; } //continuous hydrograph
  if (Options.period_ending) { val *= -1; val2 *= -1; }//period ending

  _HYDRO << ":ColumnMetaData" << endl;
  _HYDRO << "  :ColumnName precip";
  for (int p = 0;p < _nSubBasins;p++) { _HYDRO << " Q_" << _pSubBasins[p]->GetID(); }_HYDRO << endl;
  _HYDRO << "  :ColumnUnits mm/d";
  for (int p = 0;p < _nSubBasins;p++) { _HYDRO << " m3/s"; }_HYDRO << endl;
  _HYDRO << "  :ColumnType float";
  for (int p = 0;p < _nSubBasins;p++) { _HYDRO << " float"; }_HYDRO << endl;
  _HYDRO << "  :ColumnFormat " << val2;
  for (int p = 0;p < _nSubBasins;p++) { _HYDRO << " " << val; }_HYDRO << endl;
  _HYDRO << ":EndColumnMetaData" << endl;
  _HYDRO << ":EndHeader" << endl;
}
//////////////////////////////////////////////////////////////////
/// \brief creates specified output directory, if needed
///
/// \param &Options [in] global model options
//
void PrepareOutputdirectory(const optStruct& Options)
{
  if (Options.output_dir != "")
  {
#if defined(_WIN32)
    _mkdir(Options.output_dir.c_str());
#elif defined(__linux__)
    mkdir(Options.output_dir.c_str(), 0777);
#elif defined(__APPLE__)
    mkdir(Options.output_dir.c_str(), 0777);
#elif defined(__unix__)
    mkdir(Options.output_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
  }
  g_output_directory = Options.main_output_dir;//necessary evil
}

//////////////////////////////////////////////////////////////////
/// \brief returns directory path given filename
///
/// \param fname [in] filename, e.g., C:\temp\thisfile.txt returns c:\temp
//
string GetDirectoryName(const string& fname)
{
  size_t pos = fname.find_last_of("\\/");
  if (std::string::npos == pos) { return ""; }
  else { return fname.substr(0, pos); }
}

//////////////////////////////////////////////////////////////////
/// \brief returns file extension given filename
///
/// \param filename [in] , e.g., C:\temp\thisfile.txt returns txt
//
string GetFileExtension(string filename)
{
  if (filename == "") { return ""; }
  return filename.substr(filename.find_last_of(".") + 1);
}

//////////////////////////////////////////////////////////////////
/// \brief returns directory path given filename and relative path
///
/// \param filename [in] filename, e.g., C:/temp/thisfile.txt returns c:/temp
/// \param relfile [in] filename of reference file
/// e.g.,
///       absolute path of reference file is adopted
///       if filename = something.txt         and relfile= c:/temp/myfile.rvi,  returns c:/temp/something.txt
///
///       relative path of reference file is adopted
///       if filename = something.txt         and relfile= ../dir/myfile.rvi,   returns ../dir/something.txt
///
///       if path of reference file is same as file, then nothing changes
///       if filename = ../temp/something.txt and relfile= ../temp/myfile.rvi,  returns ../temp/something.txt
///
///       if absolute paths of file is given, nothing changes
///       if filename = c:/temp/something.txt and relfile= ../temp/myfile.rvi,  returns c:/temp/something.txt
//
string CorrectForRelativePath(const string filename, const string relfile)
{
  string filedir = GetDirectoryName(relfile); //if a relative path name, e.g., "/path/model.rvt", only returns e.g., "/path"

  if (StringToUppercase(filename).find(StringToUppercase(filedir)) == string::npos)//checks to see if absolute dir already included in redirect filename
  {
    string firstchar = filename.substr(0, 1);   // if '/' --> absolute path on UNIX systems
    string secondchar = filename.substr(1, 1);   // if ':' --> absolute path on WINDOWS system

    if ((firstchar.compare("/") != 0) && (secondchar.compare(":") != 0)) {
      // cout << "This is not an absolute filename!  --> " << filename << endl;
      //+"//"
      //cout << "StandardOutput: corrected filename: " << filedir + "//" + filename << endl;
      return filedir + "//" + filename;
    }
  }
  // cout << "StandardOutput: corrected filename: " << filename << endl;
  return filename;
}