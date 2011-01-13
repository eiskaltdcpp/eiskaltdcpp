#!/bin/sh
# Modified script for amarok2

## Author: WiseLord
## License: Public Domain
## Version: 0.2

## Depens on sh, amarok2

## Script was patched by:
## Made by kTorpi << ktorpi@gmail.com >>
## and
## Made by Marcus << marcus@elitemail.hu >>

message="/me is listening silence now"
nowPlaying="$(qdbus org.kde.amarok /Player org.freedesktop.MediaPlayer.GetMetadata 2>/dev/null)"

if [ -n "${nowPlaying}" ]
then
	version="$(qdbus org.kde.amarok / org.freedesktop.MediaPlayer.Identity)"
	nlength="$(qdbus org.kde.amarok /Player org.freedesktop.MediaPlayer.PositionGet)"
	nlength=$((nlength/1000))

        title="$(echo "${nowPlaying}" | sed -ne 's/^title: \(.*\)$/\1/p')"
        artist="$(echo "${nowPlaying}" | sed -ne 's/^artist: \(.*\)$/\1/p')"
        bitrate="$(echo "${nowPlaying}" | sed -ne 's/^audio\-bitrate: \(.*\)$/\1/p')"
        album="$(echo "${nowPlaying}" | sed -ne 's/^album: \(.*\)$/\1/p')"
        length="$(echo "${nowPlaying}" | sed -ne 's/^time: \(.*\)$/\1/p')"
        year="$(echo "${nowPlaying}" | sed -ne 's/^year: \(.*\)$/\1/p')"
       
	 #The length sec part
	secv=$(($length%60))
        if [ ${secv} -lt 10 ]
        then
                sec="0${secv}"
        else
                sec="${secv}"
        fi
	
	#The lengt min part
        minv=$(($length/60))
        if [ ${minv} -lt 10 ]
        then
                min="0${minv}"
        else
                min="${minv}"
        fi

	#elapsed time sec part
        secn=$(($nlength%60))
        if [ ${secn} -lt 10 ]
        then
                nsec="0${secn}"
        else
                nsec="${secn}"
        fi

	#elapsed time min part
        minn=$(($nlength/60))
        if [ ${minn} -lt 10 ]
        then
                nmin="0${minn}"
        else
                nmin="${minn}"
        fi
        
        # % of the elapsed time
        rate=$(((nlength*100)/length))
        
        # progressbar
        progressbar="["
        nrate=$((rate/10))
        for ((i=1;i<=nrate;i++))
        do
                progressbar="${progressbar}-"
        done
        progressbar="${progressbar}|"
        for((i=1;i<(10-nrate);i++))
        do
                progressbar="${progressbar}-"
        done
        progressbar="${progressbar}]"
        
        message="${artist} - ${title} [${nmin}:${nsec}/${min}:${sec}] $progressbar [${bitrate} kbps] :: ${version}"

#The message in dc:[17:14:13] <[OP]Marcus> Kylie Minogue - Slow (Chemical Brothers Remix) [01:29/04:44] [---|------] [899 kbps] :: Amarok 2.3.90

##This part is from the original script.
                                                                                                                                                                                                                        
## Also you can send song magnet to chat:                                                                                                                                                                                                    
# [17:45:03] * WiseLord is listening now: Therion - Midgård (02. Midgård.mp3) (7.0 МиБ)                                                                                                                                                      
# You can use <magnet show=NAME_TO_SHOW>PATH_TO_FILE</magnet> or just <magnet>PATH_TO_FILE</magnet>                                                                                                                                          
# If you want to do this, uncomment 2 lines below:                                                                                                                                                                                           
                                                                                                                                                                                                                                             
#       location="$(echo "${nowPlaying}" | sed -ne 's/^location: file:\/\/\(.*\)$/\1/p' | sed -e s/\'/\\\\\'/g -e 's/%\([0-9A-Fa-f][0-9A-Fa-f]\)/\\\\\x\1/g' | xargs echo -e )"                                                              
#       message="/me is listening now: ${artist} - ${title} ( <magnet>${location}</magnet> )"                                                                                                                                                
fi                                                                                                                                                                                                                                           
                                                                                                                                                                                                                                             
echo "${message}" 

