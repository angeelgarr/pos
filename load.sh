xcb_com=COM5
aip=../pkg/MiniPos.aip

log() {
	echo 
	echo log [start]
	xcb logcat -c
	xcb logcat -s debug
}

#main
if [ "$1" = "clean" ]; then
	xcb connect com:$xcb_com
	xcb installer uaip MAINAPP
	exit
fi

if [ "$1" = "sys" ]; then
	xcb connect com:$xcb_com
	xcb connect com:$xcb_com
	xcb telnetd
	exit
fi


if [ "$1" = "recover" ]; then
	xcb kill-server
	xcb start-server
	xcb connect com:$xcb_com
	xcb connect com:$xcb_com
	xcb telnetd
	exit
fi


if [ "$1" = "log" ]; then
	xcb connect com:$xcb_com
	xcb connect com:$xcb_com
	log
	exit
fi

if [ "$1" = "applog" ]; then
	xcb connect com:$xcb_com
	echo "start loading...."
	xcb installer aip $aip
	log
	exit
fi

xcb connect com:$xcb_com
echo "start loading...."
xcb installer aip $aip


if [ "$2" = "log" ]; then
	log
fi

if [ "$3" = "log" ]; then
	log
fi
