#!/bin/bash

level=$1

function poweroff_codesys(){
    dpkg -l | grep -q "ed-codesys-[1-2][0-9][0-9]"
    if [ $? -ne 0 ];then
        return 0
    fi
    systemctl is-active codesyscontrol.service > /dev/null
    if [ $? -eq 0 ];then
        systemctl stop codesyscontrol.service
        sync
    fi
}

function poweron_codesys(){
    dpkg -l | grep -q "ed-codesys-[1-2][0-9][0-9]"
    if [ $? -ne 0 ];then
        return 0
    fi
    status=$(systemctl is-enabled codesyscontrol.service 2>/dev/null)
    if [ "${status}" == "enabled" ];then
        systemctl is-active codesyscontrol.service > /dev/null
        if [ $? -ne 0 ];then
            systemctl start codesyscontrol.service
        fi
    fi
}


function run(){
    case "$level" in
   "rising")
        echo "Rising edge is triggered."
        poweron_codesys
        ;;
   "falling")
        echo "Falling  edge is triggered."
        poweroff_codesys
        ;;
   *)
        echo "Unknown parameters"
        ;;
esac
}

run
