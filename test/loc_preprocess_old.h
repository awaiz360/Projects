/*****************************************************************************
  INCLUDE PROTECTION
*****************************************************************************/
#ifndef _LOC_PREPROCESS_H_
#define _LOC_PREPROCESS_H_

/*****************************************************************************
  INCLUDES
*****************************************************************************/

#include "loc.h"

/*****************************************************************************
  TYPEDEFS
*****************************************************************************/

/*****************************************************************************
  FUNCTIONS
*****************************************************************************/

void LongCtrlPreProcess( const EgoMotion_t* const p_ego_motion, const LongPlan_t* const p_longPlan,
                         LocVector* const p_ctrl_req, LocVector* const p_ctrl_error,
                         CtrlPreProcessInternal_t* const p_ctrl_pre_process_internal,
                         LocPidCtrlPar* const p_dist_ctrl_par, LocPidCtrlPar* const p_velo_ctrl_par,
                         LocPidCtrlPar* const p_accel_ctrl_par );

#endif
