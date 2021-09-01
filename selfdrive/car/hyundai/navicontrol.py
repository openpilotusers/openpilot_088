import math
import numpy as np



from selfdrive.config import Conversions as CV
from selfdrive.car.hyundai.values import Buttons
from selfdrive.controls.lib.lane_planner import TRAJECTORY_SIZE
from common.numpy_fast import clip, interp
import cereal.messaging as messaging

import common.loger as trace1
import common.MoveAvg as mvAvg




class NaviControl():
  def __init__(self, p = None ):
    self.p = p
    
    self.sm = messaging.SubMaster(['liveNaviData','lateralPlan','radarState']) 

    self.btn_cnt = 0
    self.seq_command = 0
    self.target_speed = 0
    self.set_point = 0
    self.wait_timer2 = 0
    self.wait_timer3 = 0

    self.moveAvg = mvAvg.MoveAvg()



  def update_lateralPlan( self ):
    self.sm.update(0)
    path_plan = self.sm['lateralPlan']
    return path_plan



  def button_status(self, CS ): 
    if not CS.acc_active or CS.cruise_buttons != Buttons.NONE: 
      self.wait_timer2 = 50 
    elif self.wait_timer2: 
      self.wait_timer2 -= 1
    else:
      return 1
    return 0




  # buttn acc,dec control
  def switch(self, seq_cmd):
      self.case_name = "case_" + str(seq_cmd)
      self.case_func = getattr( self, self.case_name, lambda:"default")
      return self.case_func()

  def reset_btn(self):
      if self.seq_command != 3:
        self.seq_command = 0


  def case_default(self):
      self.seq_command = 0
      return None

  def case_0(self):
      self.btn_cnt = 0
      self.target_speed = self.set_point
      delta_speed = self.target_speed - self.VSetDis
      if delta_speed > 1:
        self.seq_command = 1
      elif delta_speed < -1:
        self.seq_command = 2
      return None

  def case_1(self):  # acc
      btn_signal = Buttons.RES_ACCEL
      self.btn_cnt += 1
      if self.target_speed == self.VSetDis:
        self.btn_cnt = 0
        self.seq_command = 3            
      elif self.btn_cnt > 10:
        self.btn_cnt = 0
        self.seq_command = 3
      return btn_signal


  def case_2(self):  # dec
      btn_signal = Buttons.SET_DECEL
      self.btn_cnt += 1
      if self.target_speed == self.VSetDis:
        self.btn_cnt = 0
        self.seq_command = 3            
      elif self.btn_cnt > 10:
        self.btn_cnt = 0
        self.seq_command = 3
      return btn_signal

  def case_3(self):  # None
      btn_signal = None  # Buttons.NONE
      
      self.btn_cnt += 1
      #if self.btn_cnt == 1:
      #  btn_signal = Buttons.NONE
      if self.btn_cnt > 5: 
        self.seq_command = 0
      return btn_signal


  def ascc_button_control( self, CS, set_speed ):
    self.set_point = max(30,set_speed)
    self.curr_speed = CS.out.vEgo * CV.MS_TO_KPH
    self.VSetDis   = CS.VSetDis
    btn_signal = self.switch( self.seq_command )

    return btn_signal

  def get_forword_car_speed( self, CS,  cruiseState_speed):
    self.lead_0 = self.sm['radarState'].leadOne
    self.lead_1 = self.sm['radarState'].leadTwo
    cruise_set_speed_kph = cruiseState_speed

    if self.lead_0.status:
      dRel = self.lead_0.dRel
      vRel = self.lead_0.vRel
    else:
      dRel = 150
      vRel = 0

    dRelTarget = 110 #interp( CS.clu_Vanz, [30, 90], [ 30, 70 ] )
    if dRel < dRelTarget and CS.clu_Vanz > 20:
      dGap = interp( CS.clu_Vanz, [30, 40,  70], [ 20, 10, 5 ] )
      cruise_set_speed_kph = CS.clu_Vanz + dGap
    else:
      self.wait_timer3 = 0


    cruise_set_speed_kph = self.moveAvg.get_avg(cruise_set_speed_kph, 30)
    return  cruise_set_speed_kph


  def get_navi_speed(self, sm, CS, cruiseState_speed ):
    cruise_set_speed_kph = cruiseState_speed
    v_ego_kph = CS.out.vEgo * CV.MS_TO_KPH    
    self.liveNaviData = sm['liveNaviData']
    speedLimit = self.liveNaviData.speedLimit
    speedLimitDistance = self.liveNaviData.speedLimitDistance
    safetySign  = self.liveNaviData.safetySign
    mapValid = self.liveNaviData.mapValid
    trafficType = self.liveNaviData.trafficType
    
    if not mapValid or trafficType == 0:
      return  cruise_set_speed_kph

    elif CS.is_highway or speedLimit < 30:
      return  cruise_set_speed_kph
    elif speedLimitDistance >= 50:
      if speedLimit <= 60:
        spdTarget = interp( speedLimitDistance, [50, 600], [ speedLimit, speedLimit + 50 ] )
      else:
        spdTarget = interp( speedLimitDistance, [150, 900], [ speedLimit, speedLimit + 30 ] )
    else:
      spdTarget = speedLimit



    if v_ego_kph < speedLimit:
      v_ego_kph = speedLimit

    cruise_set_speed_kph = min( spdTarget, v_ego_kph )
    return  cruise_set_speed_kph

  def update(self,  CS ):  
    # send scc to car if longcontrol enabled and SCC not on bus 0 or ont live
    btn_signal = None
    if not self.button_status( CS  ):
      pass
    elif CS.acc_active:
      cruiseState_speed = CS.out.cruiseState.speed * CV.MS_TO_KPH      
      kph_set_vEgo = self.get_navi_speed(  self.sm , CS, cruiseState_speed )
      self.ctrl_speed = min( cruiseState_speed, kph_set_vEgo)

      #if CS.cruise_set_mode == 1:
      if CS.cruise_set_mode == 1:
        kph_fwd_speed = self.get_forword_car_speed( CS,  cruiseState_speed)
        self.ctrl_speed = min( self.ctrl_speed, kph_fwd_speed)
      btn_signal = self.ascc_button_control( CS, self.ctrl_speed )
 

    return btn_signal
