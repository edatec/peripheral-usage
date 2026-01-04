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

function backlight_off(){
    echo "[DEBUG] Turning off backlight..."
    if command -v ed-ddc-server >/dev/null 2>&1; then
        echo "[DEBUG] Using ed-ddc-server"
        ed-ddc-server brightness write -v 0
    fi
    echo "[DEBUG] Using sysfs method"
    # echo 1 | tee /sys/class/backlight/10-0027/bl_power 2>/dev/null || \
    echo 1 | tee /sys/class/backlight/*/bl_power 2>/dev/null || \
    echo 0 | tee /sys/class/backlight/*/brightness 2>/dev/null
}

function backlight_on(){
    echo "[DEBUG] Turning on backlight..."
    if command -v ed-ddc-server >/dev/null 2>&1; then
        echo "[DEBUG] Using ed-ddc-server"
        sleep 1
        ed-ddc-server brightness write -v 100
    fi
    echo "[DEBUG] Using sysfs method"
    # echo 0 | tee /sys/class/backlight/10-0027/bl_power 2>/dev/null || \
    echo 0 | tee /sys/class/backlight/*/bl_power 2>/dev/null || \
    echo 255 | tee /sys/class/backlight/*/brightness 2>/dev/null
}

function cpu_freq_min(){
    echo powersave | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2>/dev/null
    cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_setspeed 2>/dev/null
}

function cpu_freq_restore(){
    echo ondemand | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2>/dev/null
}


function run(){
    case "$level" in
   "rising")
        echo "Rising edge is triggered."
        poweron_codesys
        backlight_on
        cpu_freq_restore
        ;;
   "falling")
        echo "Falling  edge is triggered."
        poweroff_codesys
        backlight_off
        cpu_freq_min
        ;;
   *)
        echo "Unknown parameters"
        ;;
esac
}

run
