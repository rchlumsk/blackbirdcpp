#include "Reach.h"

// Default constructor
CReach::CReach() : CStreamnode() {
  // Default constructor implementation (inherits from CStreamnode)
}

//////////////////////////////////////////////////////////////////
/// \brief Compute basic flow properties
///
/// \param flow [in] flow to be used in computations
/// \param *&bbopt [in] Global model options information
//
void CReach::compute_basic_flow_properties(double flow, COptions *&bbopt) {
  mm->flow = flow;

  // --- Zero‑flow exception block ---
  if (mm->flow == 0.0) {
    mm->velocity = 0.0;
    mm->velocity_head = 0.0;
    mm->pressure_head = 0.0;
    mm->energy_total = mm->wsl; // water surface level stays as energy
    mm->froude = 0.0;
    mm->sf = 0.0;
    return;
  }

  /*
  if (is_structure) {
    
      // determine flow regime type for structure
    flow_regime fr; // default regime is OPEN_CHANNEL_FLOW
    // fr.regime = enum_flow_regimes::OPEN_CHANNEL_FLOW;


    // xxx - can move common elements like mm->velocity and mm->sf, mm->froude out of here if same between all
    // xxx to do

    fr.regime = hydstructure.get_flow_regime(mm->depth);

    switch (fr.regime) {
        case OPEN_CHANNEL_FLOW: {
          // Standard hydraulics through effective opening
          // equivalent to non-structure computation, but uses the pre-processed properties for structures
          fr.open_channel_flow = mm->flow; // all flow is open_channel

          // typical non-structure computation
          mm->velocity = mm->area != 0 && mm->flow / mm->area != DBL_MAX
                            ? mm->flow / mm->area
                            : 0;
          mm->velocity_head =
            (mm->alpha * mm->velocity * mm->velocity / 2.) / GRAVITY;
          mm->energy_total = mm->velocity_head + mm->wsl;
          mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
          mm->sf = mm->k_total != 0 && pow(mm->flow / mm->k_total, 2.) != DBL_MAX
                        ? pow(mm->flow / mm->k_total, 2.)
                        : 0;
          break;
        }

        case PRESSURE_FLOW: {
          fr.pressure_flow = mm->flow; // all flow is pressurized

          mm->velocity = mm->area != 0 && mm->flow / mm->area != DBL_MAX
                            ? mm->flow / mm->area
                            : 0;
          mm->velocity_head =
            (mm->alpha * mm->velocity * mm->velocity / 2.) / GRAVITY;
          mm->pressure_head = hydstructure.calc_pressure_head(mm->depth);
          mm->energy_total = mm->velocity_head + mm->wsl + mm->pressure_head; 
          // mm->froude = 0.0;  // Froude not really meaningful in pressurized flow
          // mm->sf = 0.0; // Manning friction slope generally not used
          mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
          mm->sf =
              mm->k_total != 0 && pow(mm->flow / mm->k_total, 2.) != DBL_MAX
                 ? pow(mm->flow / mm->k_total, 2.)
                 : 0; 
          break;
        }

        case WEIR_FLOW: {
          fr.weir_flow = mm->flow; // all flow is over the weir

          WriteWarning("Weir flow not currently supported", true);

            // xxx not currently used until we add weir structures
            // will need to update this
            mm->velocity = mm->area != 0 && mm->flow / mm->area != DBL_MAX
                               ? mm->flow / mm->area
                               : 0;
            mm->velocity_head =
                (mm->alpha * mm->velocity * mm->velocity / 2.) / GRAVITY;
            mm->energy_total = mm->velocity_head + mm->wsl;
            mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
            mm->sf = mm->k_total != 0 && pow(mm->flow / mm->k_total, 2.) != DBL_MAX
                         ? pow(mm->flow / mm->k_total, 2.)
                         : 0;
            // froude and sf meaningful here?
            // mm->froude = 0.0;  // Froude not really meaningful in pressurized flow
            // mm->sf = 0.0; // Manning friction slope generally not used
          break;
        }

        case COMBINED_WEIR_PRESSURE_FLOW: {
          fr.pressure_flow = mm->flow / 2;
          fr.weir_flow = mm->flow / 2; // xxx update, flow is split between pressure and weir flow. To be determined from proper calculations flow/2 is a placeholder

          // update velocity? what is the velocity with two flow paths? xxx
          // AI suggests using just the pressure flow velocity and ignoring weir, but that doesnt make sense to me
          mm->velocity = mm->area != 0 && mm->flow / mm->area != DBL_MAX
                            ? mm->flow / mm->area
                            : 0;
          mm->velocity_head =
            (mm->alpha * mm->velocity * mm->velocity / 2.) / GRAVITY;
          mm->pressure_head = hydstructure.calc_pressure_head(mm->depth);
          mm->energy_total = mm->velocity_head + mm->wsl + mm->pressure_head; 
          // mm->froude = 0.0;  // Froude not really meaningful in pressurized flow
          // mm->sf = 0.0; // Manning friction slope generally not used
          mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
          mm->sf =
              mm->k_total != 0 && pow(mm->flow / mm->k_total, 2.) != DBL_MAX
                 ? pow(mm->flow / mm->k_total, 2.)
                 : 0; 
          break;
        }
    }
  }*/

  // else {
      // typical non-structure computation
    mm->velocity = mm->area != 0 && mm->flow / mm->area != DBL_MAX
                       ? mm->flow / mm->area
                       : 0;
    mm->velocity_head =
        (mm->alpha * mm->velocity * mm->velocity / 2.) / GRAVITY;
    mm->energy_total = mm->velocity_head + mm->wsl ;  // + mm->pressure_head
    mm->froude = mm->velocity / std::sqrt(GRAVITY * mm->hyd_depth);
    mm->sf = mm->k_total != 0 && pow(mm->flow / mm->k_total, 2.) != DBL_MAX
                 ? pow(mm->flow / mm->k_total, 2.)
                 : 0;
  // }
}
