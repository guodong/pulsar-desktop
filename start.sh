#!/bin/bash
xfconf-query -c xsettings -p /Net/ThemeName -s "Adwaita"
nohup Xorg :0 -noreset -logfile /tmp/0.log -config /etc/xorg.conf &
sleep 1
nohup xfce4-session &
nohup pulsar &
bash
