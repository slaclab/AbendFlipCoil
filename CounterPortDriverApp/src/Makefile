TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = CounterPortDriver
# CounterPortDriver.dbd will be created and installed
DBD += CounterPortDriver.dbd

# CounterPortDriver.dbd will be made up from these files:
CounterPortDriver_DBD += base.dbd
CounterPortDriver_DBD += devHP3458A.dbd
CounterPortDriver_DBD += drvAsynIPPort.dbd
CounterPortDriver_DBD += asyn.dbd
CounterPortDriver_DBD += register.dbd
# Include dbd files from all support applications:
#CounterPortDriver_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#CounterPortDriver_LIBS += xxx

# CounterPortDriver_registerRecordDeviceDriver.cpp derives from CounterPortDriver.dbd
CounterPortDriver_SRCS += CounterPortDriver_registerRecordDeviceDriver.cpp
CounterPortDriver_SRCS += flipcoil_util.cpp
CounterPortDriver_SRCS += asyn_read_write.cpp
CounterPortDriver_SRCS += flipcoil.cpp

# Build the main IOC entry point on workstation OSs.
CounterPortDriver_SRCS_DEFAULT += CounterPortDriverMain.cpp
CounterPortDriver_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#CounterPortDriver_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
CounterPortDriver_LIBS += devHP3458A
CounterPortDriver_LIBS += asyn
CounterPortDriver_LIBS += $(EPICS_BASE_IOC_LIBS)
#===========================

USR_CPPFLAGS += -std=c++11 -Wall -g

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

