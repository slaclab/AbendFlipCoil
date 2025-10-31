#!../../bin/rhel9-x86_64/CounterPortDriver

#- You may have to change CounterPortDriver to something else
#- everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/CounterPortDriver.dbd"
CounterPortDriver_registerRecordDeviceDriver pdbbase

##Driver Launches
##CounterDriverConfigure("lujko")
##GetterDriverConfigure("lujko1")
##WaterDriverConfigure("lujko2")

#Beckoff add in

#drvAsynIPPortConfigure("beckhoff", "beckhoff_node", 0, 0, 1)
#modbusInterposeConfig("beckhoff", 0, 2000, 0)
#drvModbusAsynConfigure("BKHF1_3102", "beckhoff", 0,3, 0, 16, 0, 1000, "KL3102")


vxi11Configure("lujko1", "137.79.217.161", 0, "5.0", "gpib0", 0, 0)
FlipCoilDriverConfigure("lujko1", "134.79.217.161", 22)

## Load record instances
#dbLoadRecords("db/xxx.db","user=lujko")
#dbLoadRecords("db/counter.db", "USER=lujko,PORT=lujko,ADDR=0,TIMEOUT=0")
#dbLoadRecords("db/getter.db", "USER=lujko,PORT=lujko1,ADDR=0,TIMEOUT=0")
dbLoadRecords("db/flip.db", "P=lujko,PORT=lujko1,ADDR=0,TIMEOUT=0")
#dbLoadRecords("db/tank.db", "USER=lujko,PORT=lujko2,ADDR=0,TIMEOUT=0")
cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=lujko"
