#!/usr/bin/env python3
import gc
import os
import time
import math
import overpy
import socket
import requests
import threading
import numpy as np
# setup logging
import logging
import logging.handlers
import selfdrive.crash as crash
from common.params import Params
from collections import defaultdict
import cereal.messaging as messaging
from common.realtime import set_realtime_priority

from selfdrive.version import version, dirty

# OPKR 코드 참조.

class MapsdThread(threading.Thread):
    def __init__(self, threadID, name):
        threading.Thread.__init__(self)
        self.threadID =  threadID
        self.name  =  name
        self.logger = logging.getLogger( self.name )


        self.params = Params()
        self.params.put("OpkrMapEnable", "0")
        self.map_enabled = 0
        self.old_map_enable = 0        
        self.target_speed_map_counter2 = 0
        self.map_exec_status= False
        self.programRun = True

    # 1,3  map top
    # 2,   backgrand
    # 0,   kill
    def opkr_map_status_read(self):        
        self.map_enabled = int(self.params.get("OpkrMapEnable"))


        if self.map_enabled == 2 and self.target_speed_map_counter2 > 0:
            self.target_speed_map_counter2 -= 1
            print( " target_speed_map_counter2 = {}".format( self.target_speed_map_counter2  ))
            if self.target_speed_map_counter2  == 0:
                print( "am start --activity-task-on-home com.opkr.maphack/com.opkr.maphack.MainActivity" )
                os.system("am start --activity-task-on-home com.opkr.maphack/com.opkr.maphack.MainActivity")
                self.programRun = False
                return

        if( self.map_enabled == self.old_map_enable ):
            return

        self.old_map_enable = self.map_enabled

        if self.map_enabled == 0:
            self.map_exec_status = False
            os.system("pkill com.mnsoft.mappyobn")
        elif self.map_exec_status == False: 
            self.map_exec_status = True
            os.system("am start com.mnsoft.mappyobn/com.mnsoft.mappy.MainActivity &")  # map 실행.
            if self.map_enabled == 2: 
                self.target_speed_map_counter2 = 3   # map 실행후 3초후 backgrand로 전환합니다.
        elif self.map_enabled == 2:  # map backgrand
            os.system("am start --activity-task-on-home com.opkr.maphack/com.opkr.maphack.MainActivity") 
        elif self.map_enabled == 3:
            # map return 
            os.system("am start --activity-task-on-home com.mnsoft.mappyobn/com.mnsoft.mappy.MainActivity") 




              

    def run(self):
        # OPKR
        self.navi_on_boot = self.params.get_bool("OpkrRunNaviOnBoot")
        
        if self.navi_on_boot:
            self.params.put("OpkrMapEnable", "2")

        start = time.time()
        while True:
            if time.time() - start < 1.0:
              time.sleep(1.0)
              continue
            else:
              start = time.time()
              self.opkr_map_status_read()

          



def main():
    #gc.disable()
    #set_realtime_priority(1)
    params = Params()
    dongle_id = params.get("DongleId")
   

    mt = MapsdThread(1, "/data/media/0/videos/MapsdThread")
    mt.start()

if __name__ == "__main__":
    main()
