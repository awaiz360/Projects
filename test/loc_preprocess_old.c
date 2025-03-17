/*****************************************************************************
  INCLUDES
*****************************************************************************/

#include "loc_preprocess.h"

/*****************************************************************************
  GLOBAL VARIABLES
*****************************************************************************/

/*****************************************************************************
  STATIC FUNCTIONS DEFINE
*****************************************************************************/

/*****************************************************************************
  FUNCTIONS
*****************************************************************************/

static float32 TrajectoryInterpolationUnsigned( const AlgoDataTimeStamp_t table_x[],
                                                const float32 table_y[], const uint8 num_points,
                                                const AlgoDataTimeStamp_t x_in );
static void CalculateLongCtrlRequest( const LongPlanTraj_t* const p_trajectory,
                                      const LongPlanReq_t* const p_request,
                                      LocVector* const p_ctrl_req,
                                      CtrlPreProcessInternal_t* const p_ctrl_pre_process_internal );

static void CalculateLongCtrlError( const EgoMotion_t* const p_ego_motion,
                                    const LongPlanReq_t* const p_request,
                                    LocVector* const p_ctrl_req, LocVector* const p_ctrl_error,
                                    CtrlPreProcessInternal_t* const p_ctrl_pre_process_internal );

static void AssignLongCtrlParameter( const LongPlanReq_t* const p_request,
                                     LocPidCtrlPar* const p_dist_ctrl_par,
                                     LocPidCtrlPar* const p_velo_ctrl_par,
                                     LocPidCtrlPar* const p_accel_ctrl_par );

float32 TrajectoryInterpolationUnsigned( const AlgoDataTimeStamp_t table_x[],
                                         const float32 table_y[], const uint8 num_points,
                                         const AlgoDataTimeStamp_t x_in ) {
  // init local variables
  uint8 index = 0;
  float32 y_out;
  boolean idx_found;

  // find segment on x_in-table
  idx_found = FALSE;

  // index is increased once, even after idx_found = true
  //   therefore, index= index-1u is done after the for-loop
  //   this requires to allow index = max (instead of max-1)
  //   note that index=max does not access the table data according to if-condition inside the
  //   for-loop
  for ( index = 0; ( index < ( (uint16)num_points + 1U ) ) && ( idx_found == FALSE ); index++ ) {
    if ( index < num_points ) {
      idx_found = ( table_x[index] >= x_in ) ? TRUE : FALSE;
    }
  }  // for
  index = index - 1U;

  if ( index == 0U ) {
    // begin of table
    y_out = table_y[0];
  } else if ( index >= num_points ) {
    // end of table
    y_out = table_y[num_points - 1U];
  } else {
    // inside table
    AlgoDataTimeStamp_t x_point1 = table_x[index - 1U];
    float32 y_point1 = table_y[index - 1U];
    AlgoDataTimeStamp_t x_point2 = table_x[index];
    float32 y_point2 = table_y[index];

    AlgoDataTimeStamp_t diff_x_to_x1 = x_in - x_point1;
    AlgoDataTimeStamp_t diff_x2_to_x1 = x_point2 - x_point1;

    float32 ratio = (float32)diff_x_to_x1 / ( (float32)diff_x2_to_x1 );
    y_out = ratio * ( y_point2 - y_point1 );
    y_out += y_point1;
  }  // if/else if /else

  return y_out;
}  // fcn trjInterpUint

void CalculateLongCtrlRequest( const LongPlanTraj_t* const p_trajectory,
                               const LongPlanReq_t* const p_request, LocVector* const p_ctrl_req,
                               CtrlPreProcessInternal_t* const p_ctrl_pre_process_internal ) {
  // init local variables
  AlgoDataTimeStamp_t add_prev_time = 0u;
  // preview time: if trajectory is outdated, add 1 sample of tratco cycle time to preview time
  if ( p_trajectory->timestamp[0] == p_ctrl_pre_process_internal->prev_system_time ) {
    add_prev_time += (uint64)K_t_LocCycleTime_us;
  } else {
    add_prev_time = 0u;
    p_ctrl_pre_process_internal->prev_system_time = p_trajectory->timestamp[0];
  }

  // effective preview time: start-timestamp of trajectory +  preview time parameter + add_prev_time
  // for outdated trajectory
  AlgoDataTimeStamp_t sum_prev_time =
      p_trajectory->timestamp[0] + K_t_LongPreviewTime_us + add_prev_time;

  // accel_req & velo_req
  // interpolate on timevector starting at 0, preview time = preview time parameter + add_prev_time
  // according to outdated trajectory data
  p_ctrl_req->accel = TrajectoryInterpolationUnsigned(
      &p_trajectory->timestamp[0], &p_trajectory->long_accel[0], 12u, sum_prev_time );
  p_ctrl_req->velo = TrajectoryInterpolationUnsigned(
      &p_trajectory->timestamp[0], &p_trajectory->long_velo[0], 12u, sum_prev_time );
  //   if dist preview active -> interp demand value on trj, otherwise no preview (value[0])
  if ( K_b_ActivateDistancePreview == TRUE ) {
    p_ctrl_req->dist = TrajectoryInterpolationUnsigned(
        &p_trajectory->timestamp[0], &p_trajectory->long_dist[0], 12u, sum_prev_time );
  } else {
    p_ctrl_req->dist = p_trajectory->long_dist[0];
  }

  // fullbrake (EBA feedthrough)
  if ( p_request->full_brake_flag == TRUE ) {
    p_ctrl_req->accel = K_a_FullBrkAccel_mps2;
  }
}

void CalculateLongCtrlError( const EgoMotion_t* const p_ego_motion,
                             const LongPlanReq_t* const p_request, LocVector* const p_ctrl_req,
                             LocVector* const p_ctrl_error,
                             CtrlPreProcessInternal_t* const p_ctrl_pre_process_internal ) {
  float32 blend_factor = p_ctrl_pre_process_internal->prev_blend_factor;
  // distErr before blending (preview)
  p_ctrl_error->dist = p_ctrl_req->dist - p_ego_motion->traveled_distance;

  // distErr blending
  // in case of new activation (or blendTime <= dt), avoid distErr blending
  //   -> works as inactive trade would reset statics to default (activatePrv static = false)
  //    use getLongTrjActivateCtrl as its a pure ACC function, not active for cpld atm (no ctrl prio
  //    in interface)
  if ( ( p_request->activate_control == TRUE ) &&
           ( p_ctrl_pre_process_internal->prev_activate_control == FALSE ) ||
       ( K_t_CtrlBlendingTimeeCtrlPrio_s <= K_t_LocCycleTime_s ) ) {
    // no blending
    blend_factor = 1.0f;
  } else {
    // no new activation (and blendTime > dt): reset blending if init happens
    if ( ( p_request->control_priority != p_ctrl_pre_process_internal->prev_ctrl_prio ) ) {
      blend_factor = 0.0f;  // reset blend_factor (=0)
      p_ctrl_pre_process_internal->dist_error_blend_start =
          p_ctrl_pre_process_internal
              ->prev_dist_error;  // start value of blending of distErr = last distErr
    }
  }

  // increment blending factor
  blend_factor +=
      CmlProtectedDiv( K_t_LocCycleTime_s, K_t_CtrlBlendingTimeeCtrlPrio_s, 0.0f, 1.0f );
  // blending factor limits: 0 <= blend_factor <= 1
  blend_factor = MINMAX( 0.0f, 1.0f, blend_factor );

  // blending distance error
  p_ctrl_error->dist =
      blend_factor * p_ctrl_error->dist +
      ( 1.0f - blend_factor ) * p_ctrl_pre_process_internal->dist_error_blend_start;
  // p_ctrl_error->velo;     // For close loop, request must with feedback, so we can't
  // calculate error in this module p_ctrl_error->accel;    // For close loop, request
  // must with feedback, so we can't calculate error in this module

  // set "previous" statics
  p_ctrl_pre_process_internal->prev_ctrl_prio = p_request->control_priority;
  p_ctrl_pre_process_internal->prev_dist_error = p_ctrl_error->dist;
  p_ctrl_pre_process_internal->prev_blend_factor = blend_factor;
  p_ctrl_pre_process_internal->prev_activate_control = p_request->activate_control;
}

void AssignLongCtrlParameter( const LongPlanReq_t* const p_request,
                              LocPidCtrlPar* const p_dist_ctrl_par,
                              LocPidCtrlPar* const p_velo_ctrl_par,
                              LocPidCtrlPar* const p_accel_ctrl_par ) {
  if ( p_request->full_brake_flag == TRUE ) {
    p_dist_ctrl_par->kp = K_f_CtrlDistParamsKpDistAeb;
    p_dist_ctrl_par->ki = 0.f;
    p_dist_ctrl_par->kd = K_f_CtrlDistParamsKdDistAeb;
    p_dist_ctrl_par->pt1 = K_f_CtrlDistParamsPt1DistAeb;
    p_dist_ctrl_par->gain_feed_forward = K_f_CtrlDistParamsveloKFFAeb;
    p_velo_ctrl_par->kp = K_f_CtrlDistParamsKpVeloAeb;
    p_velo_ctrl_par->ki = 0.f;
    p_velo_ctrl_par->kd = K_f_CtrlDistParamsKdVeloAeb;
    p_velo_ctrl_par->pt1 = K_f_CtrlDistParamsPt1VeloAeb;
    p_velo_ctrl_par->gain_feed_forward = K_f_CtrlDistParamsAccelKFFAeb;
    p_accel_ctrl_par->kp = 0.f;
    p_accel_ctrl_par->ki = 0.f;
    p_accel_ctrl_par->kd = 0.f;
    p_accel_ctrl_par->pt1 = 0.f;
    p_accel_ctrl_par->gain_feed_forward = 0.f;
  } else {
    if ( p_request->control_priority == CTRL_PRIORITY_VELO ) {
      switch ( p_request->drive_mode ) {
        case DRIVEMODE_ECO:
        case DRIVEMODE_COMFORT:
        case DRIVEMODE_SNOW:
          p_dist_ctrl_par->kp = K_f_CtrlVeloParamsKpDistAccLow;
          p_dist_ctrl_par->ki = 0.f;
          p_dist_ctrl_par->kd = K_f_CtrlVeloParamsKdDistAccLow;
          p_dist_ctrl_par->pt1 = K_f_CtrlVeloParamsPt1DistAccLow;
          p_dist_ctrl_par->gain_feed_forward = K_f_CtrlVeloParamsveloKFFAccLow;
          p_velo_ctrl_par->kp = K_f_CtrlVeloParamsKpVeloAccLow;
          p_velo_ctrl_par->ki = 0.f;
          p_velo_ctrl_par->kd = K_f_CtrlVeloParamsKdVeloAccLow;
          p_velo_ctrl_par->pt1 = K_f_CtrlVeloParamsPt1VeloAccLow;
          p_velo_ctrl_par->gain_feed_forward = K_f_CtrlVeloParamsAccelKFFAccLow;
          p_accel_ctrl_par->kp = 0.f;
          p_accel_ctrl_par->ki = 0.f;
          p_accel_ctrl_par->kd = 0.f;
          p_accel_ctrl_par->pt1 = 0.f;
          p_accel_ctrl_par->gain_feed_forward = 0.f;
          break;
        case DRIVEMODE_SPORT:
        case DRIVEMODE_SPORT_PLUS:
          p_dist_ctrl_par->kp = K_f_CtrlVeloParamsKpDistAccHigh;
          p_dist_ctrl_par->ki = 0.f;
          p_dist_ctrl_par->kd = K_f_CtrlVeloParamsKdDistAccHigh;
          p_dist_ctrl_par->pt1 = K_f_CtrlVeloParamsPt1DistAccHigh;
          p_dist_ctrl_par->gain_feed_forward = K_f_CtrlVeloParamsveloKFFAccHigh;
          p_velo_ctrl_par->kp = K_f_CtrlVeloParamsKpVeloAccHigh;
          p_velo_ctrl_par->ki = 0.f;
          p_velo_ctrl_par->kd = K_f_CtrlVeloParamsKdVeloAccHigh;
          p_velo_ctrl_par->pt1 = K_f_CtrlVeloParamsPt1VeloAccHigh;
          p_velo_ctrl_par->gain_feed_forward = K_f_CtrlVeloParamsAccelKFFAccHigh;
          p_accel_ctrl_par->kp = 0.f;
          p_accel_ctrl_par->ki = 0.f;
          p_accel_ctrl_par->kd = 0.f;
          p_accel_ctrl_par->pt1 = 0.f;
          p_accel_ctrl_par->gain_feed_forward = 0.f;
          break;
        case DRIVEMODE_NORMAL:
        default:
          p_dist_ctrl_par->kp = K_f_CtrlVeloParamsKpDistAccMed;
          p_dist_ctrl_par->ki = 0.f;
          p_dist_ctrl_par->kd = K_f_CtrlVeloParamsKdDistAccMed;
          p_dist_ctrl_par->pt1 = K_f_CtrlVeloParamsPt1DistAccMed;
          p_dist_ctrl_par->gain_feed_forward = K_f_CtrlVeloParamsveloKFFAccMed;
          p_velo_ctrl_par->kp = K_f_CtrlVeloParamsKpVeloAccMed;
          p_velo_ctrl_par->ki = 0.f;
          p_velo_ctrl_par->kd = K_f_CtrlVeloParamsKdVeloAccMed;
          p_velo_ctrl_par->pt1 = K_f_CtrlVeloParamsPt1VeloAccMed;
          p_velo_ctrl_par->gain_feed_forward = K_f_CtrlVeloParamsAccelKFFAccMed;
          p_accel_ctrl_par->kp = 0.f;
          p_accel_ctrl_par->ki = 0.f;
          p_accel_ctrl_par->kd = 0.f;
          p_accel_ctrl_par->pt1 = 0.f;
          p_accel_ctrl_par->gain_feed_forward = 0.f;
          break;
      }
    } else if ( p_request->control_priority == CTRL_PRIORITY_DIST ) {
      switch ( p_request->drive_mode ) {
        case DRIVEMODE_ECO:
        case DRIVEMODE_COMFORT:
        case DRIVEMODE_SNOW:
          p_dist_ctrl_par->kp = K_f_CtrlDistParamsKpDistAccLow;
          p_dist_ctrl_par->ki = 0.f;
          p_dist_ctrl_par->kd = K_f_CtrlDistParamsKdDistAccLow;
          p_dist_ctrl_par->pt1 = K_f_CtrlDistParamsPt1DistAccLow;
          p_dist_ctrl_par->gain_feed_forward = K_f_CtrlDistParamsveloKFFAccLow;
          p_velo_ctrl_par->kp = K_f_CtrlDistParamsKpVeloAccLow;
          p_velo_ctrl_par->ki = 0.f;
          p_velo_ctrl_par->kd = K_f_CtrlDistParamsKdVeloAccLow;
          p_velo_ctrl_par->pt1 = K_f_CtrlDistParamsPt1VeloAccLow;
          p_velo_ctrl_par->gain_feed_forward = K_f_CtrlDistParamsAccelKFFAccLow;
          p_accel_ctrl_par->kp = 0.f;
          p_accel_ctrl_par->ki = 0.f;
          p_accel_ctrl_par->kd = 0.f;
          p_accel_ctrl_par->pt1 = 0.f;
          p_accel_ctrl_par->gain_feed_forward = 0.f;
          break;
        case DRIVEMODE_SPORT:
        case DRIVEMODE_SPORT_PLUS:
          p_dist_ctrl_par->kp = K_f_CtrlDistParamsKpDistAccHigh;
          p_dist_ctrl_par->ki = 0.f;
          p_dist_ctrl_par->kd = K_f_CtrlDistParamsKdDistAccHigh;
          p_dist_ctrl_par->pt1 = K_f_CtrlDistParamsPt1DistAccHigh;
          p_dist_ctrl_par->gain_feed_forward = K_f_CtrlDistParamsveloKFFAccHigh;
          p_velo_ctrl_par->kp = K_f_CtrlDistParamsKpVeloAccHigh;
          p_velo_ctrl_par->ki = 0.f;
          p_velo_ctrl_par->kd = K_f_CtrlDistParamsKdVeloAccHigh;
          p_velo_ctrl_par->pt1 = K_f_CtrlDistParamsPt1VeloAccHigh;
          p_velo_ctrl_par->gain_feed_forward = K_f_CtrlDistParamsAccelKFFAccHigh;
          p_accel_ctrl_par->kp = 0.f;
          p_accel_ctrl_par->ki = 0.f;
          p_accel_ctrl_par->kd = 0.f;
          p_accel_ctrl_par->pt1 = 0.f;
          p_accel_ctrl_par->gain_feed_forward = 0.f;
          break;
        case DRIVEMODE_NORMAL:
        default:
          p_dist_ctrl_par->kp = K_f_CtrlDistParamsKpDistAccMed;
          p_dist_ctrl_par->ki = 0.f;
          p_dist_ctrl_par->kd = K_f_CtrlDistParamsKdDistAccMed;
          p_dist_ctrl_par->pt1 = K_f_CtrlDistParamsPt1DistAccMed;
          p_dist_ctrl_par->gain_feed_forward = K_f_CtrlDistParamsveloKFFAccMed;
          p_velo_ctrl_par->kp = K_f_CtrlDistParamsKpVeloAccMed;
          p_velo_ctrl_par->ki = 0.f;
          p_velo_ctrl_par->kd = K_f_CtrlDistParamsKdVeloAccMed;
          p_velo_ctrl_par->pt1 = K_f_CtrlDistParamsPt1VeloAccMed;
          p_velo_ctrl_par->gain_feed_forward = K_f_CtrlDistParamsAccelKFFAccMed;
          p_accel_ctrl_par->kp = 0.f;
          p_accel_ctrl_par->ki = 0.f;
          p_accel_ctrl_par->kd = 0.f;
          p_accel_ctrl_par->pt1 = 0.f;
          p_accel_ctrl_par->gain_feed_forward = 0.f;
          break;
      }
    }
  }
}

void LongCtrlPreProcess( const EgoMotion_t* const p_ego_motion, const LongPlan_t* const p_longPlan,
                         LocVector* const p_ctrl_req, LocVector* const p_ctrl_error,
                         CtrlPreProcessInternal_t* const p_ctrl_pre_process_internal,
                         LocPidCtrlPar* const p_dist_ctrl_par, LocPidCtrlPar* const p_velo_ctrl_par,
                         LocPidCtrlPar* const p_accel_ctrl_par ) {
  // calculate request
  CalculateLongCtrlRequest( &p_longPlan->trajectory, &p_longPlan->request, p_ctrl_req,
                            p_ctrl_pre_process_internal );

  // calculate error
  CalculateLongCtrlError( p_ego_motion, &p_longPlan->request, p_ctrl_req, p_ctrl_error,
                          p_ctrl_pre_process_internal );

  // assign ctrl parameter
  AssignLongCtrlParameter( &p_longPlan->request, p_dist_ctrl_par, p_velo_ctrl_par,
                           p_accel_ctrl_par );
}