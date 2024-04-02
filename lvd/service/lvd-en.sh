#!/bin/sh

case $1 in 
enable|ENABLE)
	nohup /usr/sbin/lvd > /dev/null 2>err.out &
	;;
disable|DISABLE)
	pkill lvd
	;;
*)  echo "Invalid parameter,only enable or disable"
	;;
esac

