#!/bin/bash

message="/me is listening silence now"
nowPlaying="$(qdbus org.mpris.clementine /Player GetMetadata 2>/dev/null)"

if [ -n "${nowPlaying}" ]                                                                                                                                                                                                             
then
    version="$(qdbus org.mpris.clementine / Identity)"
    nlength="$(qdbus org.mpris.clementine /Player PositionGet)"
    nlength=$((nlength/1000))
    title="$(echo "${nowPlaying}" | sed -ne 's/^title: \(.*\)$/\1/p')"                                                                                                                                                            
    artist="$(echo "${nowPlaying}" | sed -ne 's/^artist: \(.*\)$/\1/p')"
    if [ -n "$(echo "${nowPlaying}" | sed -ne 's/^audio\-bitrate: \(.*\)$/\1/p')" ]
    then
            bitrate="$(echo "${nowPlaying}" | sed -ne 's/^audio\-bitrate: \(.*\)$/\1/p')"
    else
        bitrate="0"
    fi
    album="$(echo "${nowPlaying}" | sed -ne 's/^album: \(.*\)$/\1/p')"
    if [ -n "$(echo "${nowPlaying}" | sed -ne 's/^year: \(.*\)$/\1/p')" ]
    then
        year="$(echo "${nowPlaying}" | sed -ne 's/^year: \(.*\)$/\1/p')"
    else
        year="0"
    fi
        if [ -n "$(echo "${nowPlaying}" | sed -ne 's/^time: \(.*\)$/\1/p')" ]
    then
        length="$(echo "${nowPlaying}" | sed -ne 's/^time: \(.*\)$/\1/p')" 
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
    else
        length="00"
        sec="00"
        min="00"
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


        if [ ${length} == "00" ]
        then
                progressbar=""
        else        
                # % of the elapsed time
                rate=$(((nlength*100)/length))
        
                # progressbar
                progressbar="["
                nrate=$((rate/10))
                for ((i=1;i<=${nrate};i++))
                do
                        progressbar="${progressbar}-"
                done
                progressbar="${progressbar}|"
                for((i=1;i<(10-nrate);i++))
                do
                        progressbar="${progressbar}-"
                done
                progressbar="${progressbar}] "
        fi


    message="/me is listening to: ${artist} - ${title}"
        if [ -n "${album}" ]
        then
                message="${message} (${album})"
        fi
        if [ ${year} -gt  0 ]
        then
                message="${message} from ${year}"
    fi
    message="${message} [${nmin}:${nsec}/${min}:${sec}] $progressbar[${bitrate} kbps](${version})"
fi
echo ${message}
