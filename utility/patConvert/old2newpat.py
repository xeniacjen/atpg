#!/ibmnas/427/r101/walts850221/local/bin/python3
#############################################################################
# File       [ conversion_beta.py ]                                         #
# Author     [ Howard Ho ]                                                  #
# Synopsis   [ ]                                                            #
# Date       [ 2014/06/06 created ]                                         #
#############################################################################


import sys
import re
import os
import os.path

PI = [ "" , "" ]
PO = [ "" , "" ]

print("\n")
print("####################################################")
print("####################################################")
print("###                                              ###")
print("###  Welcome to use pattern conversion tool !!   ###")
print("###                                              ###")
print("###    Vesion          : 1.0.0 Beta              ###")
print("###    Author          : Taoyuan De-Hua Liou     ###")
print("###    Last updated    : 2014/06/06              ###")
print("###    Current support :                         ###")
print("###                      Stuck-at-fault          ###")
print("###                      Launch-on-capture       ###")
print("###                                              ###")
print("####################################################")
print("####################################################")
print('\n')
print("Process start... \n")
print("Input File : ",sys.argv[1])
print("Output File : ",sys.argv[2])
scanChainFlag = 0;
inFile = open(sys.argv[1], 'r')
outFile = open(sys.argv[2], 'w')
print("Read PI order... ",end = ' ')
line = inFile.readline()
print("Done")

print("Read Scan order... ",end = ' ')
line_so = inFile.readline()
lineList = re.split(r'\s+', line_so)
if lineList[1] !='|' :
 scanChainFlag = 1;
print("Done")

print("Write PI order...",end = ' ')
print ("__PI_ORDER__",file = outFile,end = ' ')
if scanChainFlag == 1:
 print("CK test_si test_se",file = outFile,end = ' ')
lineList = re.split(r'\s+', line)
for element in lineList :
 if element !="" and element !='|' : 
  print(element,file = outFile,end = ' ')
print('',file = outFile)
print("Done")


print("Read and write PO order... ",end = ' ')
print("__PO_ORDER__",file = outFile,end = ' ')
if scanChainFlag == 1:
 print("test_so",file = outFile,end = ' ')
line = inFile.readline()
lineList = re.split(r'\s+', line)
for element in lineList :
 if element !="" and element !='|' :
  print(element,file = outFile,end = ' ')
print('',file = outFile)
print("Done")

print("Write Scan order... ",end = ' ')
print("__SCAN_ORDER__",file = outFile,end = ' ')
lineList = re.split(r'\s+', line_so)
for element in lineList :
 if element !="" and element !='|' :
  print(element,file = outFile,end = ' ')
print('',file = outFile)
print("Done")

print("Determine the type of pattern... ",end = ' ')
line = inFile.readline()
lineList = re.split(r'\s+',line)
type = lineList[0]
print("Done")
if type == '' : 
 print("### Error occur : Can't determine the type !!")
 sys.exit(0)
elif type != "BASIC_SCAN" and type != "LAUNCH_ON_CAPTURE" :
 print("### Error occur : Can't support conversion with type : ")
 sys.exit(0)
else:
 print("Original type is : ",type)

print("Determine the number of patterns.. .",end = ' ')
line=inFile.readline()
lineList = re.split('_',line)
number = lineList[4]
print("Done")

print("Start pattern conversion... ",end = ' ')
if type == "BASIC_SCAN" or type == "LAUNCH_ON_CAPTURE" :
 for i in range(0,int(number)) :
  #print("__PATTERN__",i,file = outFile)
  line = inFile.readline()
  lineList = re.split(r'[|]+',line)
  temp = re.split(r'\s+',lineList[0]) 
  lineList[0] = temp[1]
  for j in range(0,len(lineList)) :
   lineList[j] = lineList[j].replace(" ","")
  timeframe_num=0
  for j in range(0,2) :
   if lineList[j] != "" :
    PI[timeframe_num] =lineList[j]
    timeframe_num=timeframe_num+1 

  PPI = lineList[2]
  
  timeframe_num=0
  for j in range(4,6) :
   if lineList[j] != "" :
    PO[timeframe_num] =lineList[j]
    timeframe_num=timeframe_num+1
  
  PPO = lineList[6]
  PPO = PPO.replace("\n","")

  print("__PATTERN__",i,timeframe_num,file = outFile,end=' ')
  for j in range(0,timeframe_num) :
   if scanChainFlag ==1 : 
    print("000"+PI[j],"0"+PO[j],"__CAPT__ ",file = outFile,end=' ')
   else :
    print(PI[j],PO[j],"__CAPT__ ",file = outFile,end=' ')
  print(PPI,PPO,file = outFile)  

print("Done\n\nProcess finished !\n\n")  

print("####################################################")
print("###                                              ###")
print("###    Thank you for using conversion tool !!    ###")
print("###                                              ###")
print("####################################################")
print("\n")
 
