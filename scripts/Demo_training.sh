#!/bin/sh
path_ini=/cyberon/Data/Setting_NLU.ini

date -s '2026-01-01 12:00:00'
sleep 1
DSpotterNLU $path_ini -train2 Guest
