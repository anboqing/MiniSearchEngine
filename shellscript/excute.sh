#!/bin/bash
cd /home/anboqing/workspace/miniSearchEngine/
echo $PWD
cd shellscript
echo $PWD


function start 
{
	cd /home/anboqing/workspace/miniSearchEngine/bin/
	./server.exe
}

function stop
{
	cd /home/anboqing/workspace/miniSearchEngine/bin/
	killall -9 server.exe
}

function stat
{
	ps -ef | grep server.exe
}

# like switch
case $1 in
	start) start ;;
	stop) stop ;;
	restart) stop ; start ;;
	stat) stat ;;
	*) echo "Igorn.." ;;
esac