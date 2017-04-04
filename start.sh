#!/bin/bash
nohup Xorg :0 -noreset -logfile /tmp/0.log -config /etc/xorg.conf &
nohup pulsar &
bash
