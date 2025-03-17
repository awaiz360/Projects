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

static float32 LOC_TrjInterpUint(const AlgoDataTimeStamp_t tableX[], const float32 tableY[],
                                 const uint8 numPoints, const AlgoDataTimeStamp_t xValue) {
  // init local variables
  uint8 index = 0;
  float32 yValueOut;
  boolean idxFound;

  // find segment on xValue-table
  idxFound = FALSE;

  // index is increased once, even after idxFound = true
  //   therefore, index= index-1u is done after the for-loop
  //   this requires to allow index = max (instead of max-1)
  //   note that index=max does not access the table data according to if-condition inside the
  //   for-loop
  for (index = 0; (index < ((uint16)numPoints + 1U)) && (idxFound == FALSE); index++) {
    if (index < numPoints) {
      idxFound = (tableX[index] >= xValue) ? TRUE : FALSE;
    }
  }  // for
  index = index - 1U;

  if (index == 0U) {
    // begin of table
    yValueOut = tableY[0];
  } else if (index >= numPoints) {
    // end of table
    yValueOut = tableY[numPoints - 1U];
  } else {
    // inside table
    AlgoDataTimeStamp_t xPoint1 = tableX[index - 1U];
    float32 yPoint1 = tableY[index - 1U];
    AlgoDataTimeStamp_t xPoint2 = tableX[index];
    float32 yPoint2 = tableY[index];

    AlgoDataTimeStamp_t diffXtoX1 = xValue - xPoint1;
    AlgoDataTimeStamp_t diffX2toX1 = xPoint2 - xPoint1;

    float32 ratio = (float32)diffXtoX1 / ((float32)diffX2toX1);
    yValueOut = ratio * (yPoint2 - yPoint1);
    yValueOut += yPoint1;
  }  // if/else if /else

  return yValueOut;
}  // fcn trjInterpUint

static void LOC_CalcCtrlReq(const LongPlanTraj_t* const p_trajectory,
                            const LongPlanReq_t* const p_request, LocInternal_t* const p_Internal) {
  // init local variables
  AlgoDataTimeStamp_t addPrevTime = 0u;
  // preview time: if trajectory is outdated, add 1 sample of tratco cycle time to preview time
  if (p_trajectory->timeStamp[0] == p_Internal->CtrPreProcessInternal.prvSystemTime) {
    addPrevTime += (uint64)K_t_LocCycleTime_US;
  } else {
    addPrevTime = 0u;
    p_Internal->CtrPreProcessInternal.prvSystemTime = p_trajectory->timeStamp[0];
  }

  // effective preview time: start-timestamp of trajectory +  preview time parameter + addPrevTime
  // for outdated trajectory
  AlgoDataTimeStamp_t sumPreviewTime =
      p_trajectory->timeStamp[0] + K_t_LongPreviewTime_US + addPrevTime;

  // accelReq & veloReq
  // interpolate on timevector starting at 0, preview time = preview time parameter + addPrevTime
  // according to outdated trajectory data
  p_Internal->ctrlReq.fAccel = LOC_TrjInterpUint(&p_trajectory->timeStamp[0],
                                                 &p_trajectory->longAccel[0], 12u, sumPreviewTime);
  p_Internal->ctrlReq.fVelo = LOC_TrjInterpUint(&p_trajectory->timeStamp[0],
                                                &p_trajectory->longVelo[0], 12u, sumPreviewTime);
  //   if dist preview active -> interp demand value on trj, otherwise no preview (value[0])
  if (K_b_ActivateDistancePreview_1 == TRUE) {
    p_Internal->ctrlReq.fDist = LOC_TrjInterpUint(&p_trajectory->timeStamp[0],
                                                  &p_trajectory->longDist[0], 12u, sumPreviewTime);
  } else {
    p_Internal->ctrlReq.fDist = p_trajectory->longDist[0];
  }

  // fullbrake (EBA feedthrough)
  if (p_request->bFullBrkFlag == TRUE) {
    p_Internal->ctrlReq.fAccel = K_a_FullBrkAccel_M_S2;
  }
}

void LOC_CalcCtrlErr(const EgoMotion_t* const p_egoMotion, const LongPlanReq_t* const p_request,
                     LocInternal_t* const p_Internal) {
  float32 blndFac = p_Internal->CtrPreProcessInternal.prvBlndFac;
  // distErr before blending (preview)
  p_Internal->ctrlErr.fDist = p_Internal->ctrlReq.fDist - p_egoMotion->fTraveledDistance;

  // distErr blending
  // in case of new activation (or blendTime <= dt), avoid distErr blending
  //   -> works as inactive trade would reset statics to default (activatePrv static = false)
  //    use getLongTrjActivateCtrl as its a pure ACC function, not active for cpld atm (no ctrl prio
  //    in interface)
  if ((p_request->bActivateControl == TRUE) &&
          (p_Internal->CtrPreProcessInternal.prvActivateControl == FALSE) ||
      (K_t_CtrlBlendingTimeeCtrlPrio_S <= K_t_LocCycleTime_S)) {
    // no blending
    blndFac = 1.0f;
  } else {
    // no new activation (and blendTime > dt): reset blending if init happens
    if ((p_request->eCtrlPrio != p_Internal->CtrPreProcessInternal.prvCtrlPrio)) {
      blndFac = 0.0f;  // reset blndFac (=0)
      p_Internal->CtrPreProcessInternal.distErrBlndStart =
          p_Internal->CtrPreProcessInternal
              .prvDistErr;  // start value of blending of distErr = last distErr
    }
  }

  // increment blending factor
  blndFac += CmlProtectedDiv(K_t_LocCycleTime_S, K_t_CtrlBlendingTimeeCtrlPrio_S, 0.0f, 1.0f);
  // blending factor limits: 0 <= blndFac <= 1
  blndFac = MINMAX(0.0f, 1.0f, blndFac);

  // blending distance error
  p_Internal->ctrlErr.fDist = blndFac * p_Internal->ctrlErr.fDist +
                              (1.0f - blndFac) * p_Internal->CtrPreProcessInternal.distErrBlndStart;
  // p_Internal->ctrlErr.fVelo;     // For close loop, request must with feedback, so we can't
  // calculate error in this module p_Internal->ctrlErr.fAccel;    // For close loop, request must
  // with feedback, so we can't calculate error in this module

  // set "previous" statics
  p_Internal->CtrPreProcessInternal.prvCtrlPrio = p_request->eCtrlPrio;
  p_Internal->CtrPreProcessInternal.prvDistErr = p_Internal->ctrlErr.fDist;
  p_Internal->CtrPreProcessInternal.prvBlndFac = blndFac;
  p_Internal->CtrPreProcessInternal.prvActivateControl = p_request->bActivateControl;
}

static void LOC_AssignParameter(const LongPlanReq_t* const p_request,
                                LocPidCtrlPar* const p_distCtrlPar,
                                LocPidCtrlPar* const p_veloCtrlPar,
                                LocPidCtrlPar* const p_accelCtrlPar) {
  if (p_request->bFullBrkFlag == TRUE) {
    p_distCtrlPar->fKp = K_f_CtrlDistParamsKpDistAeb;
    p_distCtrlPar->fKi = 0.f;
    p_distCtrlPar->fKd = K_f_CtrlDistParamsKdDistAeb;
    p_distCtrlPar->fPt1 = K_f_CtrlDistParamsPt1DistAeb;
    p_distCtrlPar->fGainFeedForward = K_f_CtrlDistParamsveloKFFAeb;
    p_veloCtrlPar->fKp = K_f_CtrlDistParamsKpVeloAeb;
    p_veloCtrlPar->fKi = 0.f;
    p_veloCtrlPar->fKd = K_f_CtrlDistParamsKdVeloAeb;
    p_veloCtrlPar->fPt1 = K_f_CtrlDistParamsPt1VeloAeb;
    p_veloCtrlPar->fGainFeedForward = K_f_CtrlDistParamsAccelKFFAeb;
    p_accelCtrlPar->fKp = 0.f;
    p_accelCtrlPar->fKi = 0.f;
    p_accelCtrlPar->fKd = 0.f;
    p_accelCtrlPar->fPt1 = 0.f;
    p_accelCtrlPar->fGainFeedForward = 0.f;
  } else {
    if (p_request->eCtrlPrio == CTRL_PRIORITY_VELO) {
      switch (p_request->eDriveMode) {
        case DRIVEMODE_ECO:
        case DRIVEMODE_COMFORT:
        case DRIVEMODE_SNOW:
          p_distCtrlPar->fKp = K_f_CtrlVeloParamsKpDistAccLow;
          p_distCtrlPar->fKi = 0.f;
          p_distCtrlPar->fKd = K_f_CtrlVeloParamsKdDistAccLow;
          p_distCtrlPar->fPt1 = K_f_CtrlVeloParamsPt1DistAccLow;
          p_distCtrlPar->fGainFeedForward = K_f_CtrlVeloParamsveloKFFAccLow;
          p_veloCtrlPar->fKp = K_f_CtrlVeloParamsKpVeloAccLow;
          p_veloCtrlPar->fKi = 0.f;
          p_veloCtrlPar->fKd = K_f_CtrlVeloParamsKdVeloAccLow;
          p_veloCtrlPar->fPt1 = K_f_CtrlVeloParamsPt1VeloAccLow;
          p_veloCtrlPar->fGainFeedForward = K_f_CtrlVeloParamsAccelKFFAccLow;
          p_accelCtrlPar->fKp = 0.f;
          p_accelCtrlPar->fKi = 0.f;
          p_accelCtrlPar->fKd = 0.f;
          p_accelCtrlPar->fPt1 = 0.f;
          p_accelCtrlPar->fGainFeedForward = 0.f;
          break;
        case DRIVEMODE_SPORT:
        case DRIVEMODE_SPORT_PLUS:
          p_distCtrlPar->fKp = K_f_CtrlVeloParamsKpDistAccHigh;
          p_distCtrlPar->fKi = 0.f;
          p_distCtrlPar->fKd = K_f_CtrlVeloParamsKdDistAccHigh;
          p_distCtrlPar->fPt1 = K_f_CtrlVeloParamsPt1DistAccHigh;
          p_distCtrlPar->fGainFeedForward = K_f_CtrlVeloParamsveloKFFAccHigh;
          p_veloCtrlPar->fKp = K_f_CtrlVeloParamsKpVeloAccHigh;
          p_veloCtrlPar->fKi = 0.f;
          p_veloCtrlPar->fKd = K_f_CtrlVeloParamsKdVeloAccHigh;
          p_veloCtrlPar->fPt1 = K_f_CtrlVeloParamsPt1VeloAccHigh;
          p_veloCtrlPar->fGainFeedForward = K_f_CtrlVeloParamsAccelKFFAccHigh;
          p_accelCtrlPar->fKp = 0.f;
          p_accelCtrlPar->fKi = 0.f;
          p_accelCtrlPar->fKd = 0.f;
          p_accelCtrlPar->fPt1 = 0.f;
          p_accelCtrlPar->fGainFeedForward = 0.f;
          break;
        case DRIVEMODE_NORMAL:
        default:
          p_distCtrlPar->fKp = K_f_CtrlVeloParamsKpDistAccMed;
          p_distCtrlPar->fKi = 0.f;
          p_distCtrlPar->fKd = K_f_CtrlVeloParamsKdDistAccMed;
          p_distCtrlPar->fPt1 = K_f_CtrlVeloParamsPt1DistAccMed;
          p_distCtrlPar->fGainFeedForward = K_f_CtrlVeloParamsveloKFFAccMed;
          p_veloCtrlPar->fKp = K_f_CtrlVeloParamsKpVeloAccMed;
          p_veloCtrlPar->fKi = 0.f;
          p_veloCtrlPar->fKd = K_f_CtrlVeloParamsKdVeloAccMed;
          p_veloCtrlPar->fPt1 = K_f_CtrlVeloParamsPt1VeloAccMed;
          p_veloCtrlPar->fGainFeedForward = K_f_CtrlVeloParamsAccelKFFAccMed;
          p_accelCtrlPar->fKp = 0.f;
          p_accelCtrlPar->fKi = 0.f;
          p_accelCtrlPar->fKd = 0.f;
          p_accelCtrlPar->fPt1 = 0.f;
          p_accelCtrlPar->fGainFeedForward = 0.f;
          break;
      }
    } else if (p_request->eCtrlPrio == CTRL_PRIORITY_DIST) {
      switch (p_request->eDriveMode) {
        case DRIVEMODE_ECO:
        case DRIVEMODE_COMFORT:
        case DRIVEMODE_SNOW:
          p_distCtrlPar->fKp = K_f_CtrlDistParamsKpDistAccLow;
          p_distCtrlPar->fKi = 0.f;
          p_distCtrlPar->fKd = K_f_CtrlDistParamsKdDistAccLow;
          p_distCtrlPar->fPt1 = K_f_CtrlDistParamsPt1DistAccLow;
          p_distCtrlPar->fGainFeedForward = K_f_CtrlDistParamsveloKFFAccLow;
          p_veloCtrlPar->fKp = K_f_CtrlDistParamsKpVeloAccLow;
          p_veloCtrlPar->fKi = 0.f;
          p_veloCtrlPar->fKd = K_f_CtrlDistParamsKdVeloAccLow;
          p_veloCtrlPar->fPt1 = K_f_CtrlDistParamsPt1VeloAccLow;
          p_veloCtrlPar->fGainFeedForward = K_f_CtrlDistParamsAccelKFFAccLow;
          p_accelCtrlPar->fKp = 0.f;
          p_accelCtrlPar->fKi = 0.f;
          p_accelCtrlPar->fKd = 0.f;
          p_accelCtrlPar->fPt1 = 0.f;
          p_accelCtrlPar->fGainFeedForward = 0.f;
          break;
        case DRIVEMODE_SPORT:
        case DRIVEMODE_SPORT_PLUS:
          p_distCtrlPar->fKp = K_f_CtrlDistParamsKpDistAccHigh;
          p_distCtrlPar->fKi = 0.f;
          p_distCtrlPar->fKd = K_f_CtrlDistParamsKdDistAccHigh;
          p_distCtrlPar->fPt1 = K_f_CtrlDistParamsPt1DistAccHigh;
          p_distCtrlPar->fGainFeedForward = K_f_CtrlDistParamsveloKFFAccHigh;
          p_veloCtrlPar->fKp = K_f_CtrlDistParamsKpVeloAccHigh;
          p_veloCtrlPar->fKi = 0.f;
          p_veloCtrlPar->fKd = K_f_CtrlDistParamsKdVeloAccHigh;
          p_veloCtrlPar->fPt1 = K_f_CtrlDistParamsPt1VeloAccHigh;
          p_veloCtrlPar->fGainFeedForward = K_f_CtrlDistParamsAccelKFFAccHigh;
          p_accelCtrlPar->fKp = 0.f;
          p_accelCtrlPar->fKi = 0.f;
          p_accelCtrlPar->fKd = 0.f;
          p_accelCtrlPar->fPt1 = 0.f;
          p_accelCtrlPar->fGainFeedForward = 0.f;
          break;
        case DRIVEMODE_NORMAL:
        default:
          p_distCtrlPar->fKp = K_f_CtrlDistParamsKpDistAccMed;
          p_distCtrlPar->fKi = 0.f;
          p_distCtrlPar->fKd = K_f_CtrlDistParamsKdDistAccMed;
          p_distCtrlPar->fPt1 = K_f_CtrlDistParamsPt1DistAccMed;
          p_distCtrlPar->fGainFeedForward = K_f_CtrlDistParamsveloKFFAccMed;
          p_veloCtrlPar->fKp = K_f_CtrlDistParamsKpVeloAccMed;
          p_veloCtrlPar->fKi = 0.f;
          p_veloCtrlPar->fKd = K_f_CtrlDistParamsKdVeloAccMed;
          p_veloCtrlPar->fPt1 = K_f_CtrlDistParamsPt1VeloAccMed;
          p_veloCtrlPar->fGainFeedForward = K_f_CtrlDistParamsAccelKFFAccMed;
          p_accelCtrlPar->fKp = 0.f;
          p_accelCtrlPar->fKi = 0.f;
          p_accelCtrlPar->fKd = 0.f;
          p_accelCtrlPar->fPt1 = 0.f;
          p_accelCtrlPar->fGainFeedForward = 0.f;
          break;
      }
    }
  }
}

void LOC_CtrPreProcess(const EgoMotion_t* const p_egoMotion, const LongPlan_t* const p_longPlan,
                       LocInternal_t* const p_Internal) {
  // calculate request
  LOC_CalcCtrlReq(&p_longPlan->trajectory, &p_longPlan->request, p_Internal);

  // calculate error
  LOC_CalcCtrlErr(p_egoMotion, &p_longPlan->request, p_Internal);

  // assign ctrl parameter
  LOC_AssignParameter(&p_longPlan->request, &p_Internal->distCtrlPar, &p_Internal->veloCtrlPar,
                      &p_Internal->accelCtrlPar);
}