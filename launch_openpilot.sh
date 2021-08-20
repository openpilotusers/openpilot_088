#!/usr/bin/bash

GET_PROP_TZ=$(getprop persist.sys.timezone)

if [ "$GET_PROP_TZ" != "Asia/Seoul" ]; then
    setprop persist.sys.timezone Asia/Seoul
fi

export PASSIVE="0"
exec ./launch_chffrplus.sh

