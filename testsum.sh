#!/bin/bash

client=0
clientFail=0
totOp=0
totFail=0
con=0
dis=0
disFail==0
str=0
strOp=0
strOk=0
strFail=0
strData=0
ret=0
retOp=0
retOk=0
retFail=0
retData=0
del=0
delOp=0
delOk=0
delFail=0
disOk=0

exec {FD}<testout.log   

while IFS=" " read -u ${FD} LINE
do
	read -r -a elem <<< "$LINE"

	case ${elem[0]} in
		("REPORT")
			((totOp+=${elem[1]}))
			((totFail+=${elem[2]}))
			if [ ${elem[2]} -gt 0 ]; then ((clientFail++)); fi
		;;
		("CONNECT")
			((client++))
			if [ ${elem[1]} -eq 1 ]; then ((con++)); fi
			;;
		("STORE")
			((str++))
			((strOp+=${elem[1]}))
			((strOk+=${elem[1]} - ${elem[2]}))
			((strData+=${elem[3]}))
			if [ ${elem[2]} -gt 0 ]; then ((strFail++)); fi
		;;
		("RETRIEVE")
			((ret++))
			((retOp+=${elem[1]}))
			((retOk+=${elem[1]} - ${elem[2]}))
			((retData+=${elem[3]}))
			if [ ${elem[2]} -gt 0 ]; then ((retFail++)); fi
		;;
		("DELETE")
			((del++))
			((delOp+=${elem[1]}))	
			((delOk+=${elem[1]} - ${elem[2]}))
			if [ ${elem[2]} -gt 0 ]; then ((delFail++)); fi
		;;
		("LEAVE")
			((dis++))		
			if [ ${elem[1]} -eq 1 ]; then ((disOk++)); fi
		;;
	esac
    
done

exec {FD}<&-   

echo "*****************************"
echo "*****************************"
echo "********             ********"
echo "******** TEST REPORT ********"
echo "********             ********"
echo "*****************************"
echo "*****************************"
echo ""

echo "-----------------------------" 
echo "---------- CLIENTS ----------" 
echo "-----------------------------" 
echo ""
echo "LAUNCHED: $client"
echo "CONNECTED: $con"
echo "DISCONNECTED: $disOk"
anomaly=0
if [ $client -gt 0 ]; then anomaly=$(($clientFail/$client*100)); fi
echo "ANOMALY: $anomaly%"
echo ""
echo "----------------------------" 
echo ""

echo "-----------------------------" 
echo "-------- TEST SUITES --------" 
echo "-----------------------------" 
echo ""

echo "--> SUITE 1" 
echo "      CASE: $str"
anomaly=0
if  [ $str -gt 0 ]; then anomaly=$(($strFail/$str*100)); fi
echo "      ANOMALY: $anomaly%"
echo ""

echo "--> SUITE 2" 
echo "      CASE: $ret"
anomaly=0
if  [ $ret -gt 0 ]; then anomaly=$(($retFail/$ret*100)); fi
echo "      ANOMALY: $anomaly%"
echo ""

echo "--> SUITE 3" 
echo "      CASE: $del"
anomaly=0
if  [ $del -gt 0 ]; then anomaly=$(($delFail/$del*100)); fi
echo "      ANOMALY: $anomaly%"
echo ""
echo "-----------------------------" 
echo ""


echo "-----------------------------" 
echo "-------- OPERATIONS ---------" 
echo "-----------------------------" 
echo ""

echo "TOTAL: $totOp"
echo "FAILED: $totFail"
echo ""

echo "--> CONNECTION" 
echo "	    TOTAL: $client"
echo "	    SUCCESS: $con"
echo ""
echo "--> STORE" 
echo "	    TOTAL: $strOp"
echo "	    SUCCESS: $strOk"
echo "	    BYTE STORED: $strData"
echo ""

echo "--> RETRIEVE" 
echo "	    TOTAL: $retOp"
echo "	    SUCCESS: $retOk"
echo "	    BYTE RETRIEVED: $retData"
echo ""

echo "--> DELETE" 
echo "	    TOTAL: $delOp"
echo "	    SUCCESS: $delOk"
echo ""

echo "--> LEAVE" 
echo "	    TOTAL: $dis"
echo "	    SUCCESS: $disOk"
echo "-----------------------------" 
echo ""

pids=$(pidof -c server)

for pid in $pids
do
	kill -SIGUSR1 $pid
done

















