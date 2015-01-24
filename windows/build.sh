#!/bin/sh

number_of_processors=`grep -c ^processor /proc/cpuinfo`
make -j $number_of_processors
