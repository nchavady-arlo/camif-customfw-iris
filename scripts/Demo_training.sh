#!/bin/sh
path_ini=/cyberon/Data/Setting.ini

date -s '2025-10-05 12:00:00'
sleep 1
CClever $path_ini -train2 Guest
