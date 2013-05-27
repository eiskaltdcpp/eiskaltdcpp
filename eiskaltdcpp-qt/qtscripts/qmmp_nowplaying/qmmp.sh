#!/bin/bash

# NowPlaying QMMP Script
# Author: Gustavo Alvarez <sl1pkn07@gmail.com>
# License: GPL-2
# Date: 2012-05-28

##########################
# Define variables:
# Extrae de DBUS las cadenas necesarias.
player="$(qdbus org.mpris.qmmp / Identity 2>/dev/null)"
mdata="$(qdbus org.mpris.qmmp /Player GetMetadata 2>/dev/null)"
artist="$(echo "${mdata}" | sed -ne 's/^artist: \(.*\)$/\1/p')"
title="$(echo "${mdata}" | sed -ne 's/^title: \(.*\)$/\1/p')"
album="$(echo "${mdata}" | sed -ne 's/^album: \(.*\)$/\1/p')"
bitrate="$(echo "${mdata}" | sed -ne 's/^audio\-bitrate: \(.*\)$/\1/p')"
samplerate="$(echo "${mdata}" | sed -ne 's/^audio\-samplerate: \(.*\)$/\1/p')"
length="$(echo "${mdata}" | sed -ne 's/^mtime: \(.*\)$/\1/p')"
runtime="$(qdbus org.mpris.qmmp /Player PositionGet 2>/dev/null)"
type="$(echo "${mdata}"| sed -ne 's/^location: \(.*\)$/\1/p' | cut -d ":" -f1)"
ext="$(echo "${mdata}"| sed -ne 's/^location: \(.*\)$/\1/p' | awk -F . '{print $NF}')"

##########################
# Funciones:
# Función SELECT_LANG: Selecciona el lenguaje de la aplicación según la variable de sistema "${LANG}".
# Función SET_DEFAULT_LANG: Selecciona el lenguaje por defecto de la aplicación. por defecto "en_EN" (inglés)
# Función GET_DATA_FORMAT: Genera arrays asociativos para determinar tipo/formato de los archivos reproducidos.
# Función GENERATE_BAR: Crea una barra de progreso simple.
# Función GET_DATA_PLAY: se ejecuta cuando el player está en funcionamiento.
# Función GET_DATA_STOP: se ejecuta cuando el player está parado.
# Función GET_DATA_DOWN: se ejecuta cuando el player está apagado.
##########################

select_lang() {
# Selecciona el lenguaje del entorno, por defecto "en_EN" (inglés)
# Busca la carpeta "qmmp_langs", si la encuentra, selecciona el archivo del lenguaje según el lenguaje del sistema (seleccionado por "${lang})".
# si no existe el archivo del lenguaje. deja el lenguaje por defecto
set_default_lang
lang="$(echo "${LANG}" | cut -d "." -f1)"
lang_short="$(echo "${LANG}" | cut -d "." -f1 | cut -d "_" -f1)"
if [ -d qmmp_langs ]; then
    for l in $(ls -l `pwd`/qmmp_langs | awk 'NR!=1 && !/^d/ {print $NF}'); do
         [ "${l}" = "${lang}" ] && source `pwd`/qmmp_langs/"${l}" && break
         [ "$(echo ${l} | cut -d "_" -f1)" = "${lang_short}" ] && source `pwd`/qmmp_langs/"${l}" && break
    done
fi
    ##########################
}

set_default_lang() {
    # Lenguaje por defecto "en_EN"
    mess_listen="Listen"
    mess_artist="From Artist"
    mess_album="From Album"
    mess_duration="Duration"
    mess_player="Player"
    mess_stop="Qmmp Player Stopped"
    mess_down="Qmmp Player Down"
    ##########################
}

get_data_format() {
    # Determina el formato audio:
    # primero comprueba mediante "${type}" si es CD de Audio (cdda), CUE-sheet (cue) o si es un archivo simple (file).
    # Si coincide con "file", vuelve a compararlo usando "${ext}" para determinar el formato por su extensión.
    #
    # Genera el array "_type"
    declare -A _type=(\
    ["cdda"]="CDDA" \
    ["cue"]="CUE-sheet" \
    )
    # Genera el array "_ext"
    declare -A _ext=(\
    ["flac"]="FLAC" \
    ["ape"]="MonkeyAudio" \
    ["tta"]="TrueAudio" \
    ["mp3"]="MP3" \
    ["ogg"]="Ogg-Vorbis" \
    ["aac"]="AAC" \
    ["m4a"]="Mpeg4-Audio" \
    ["mpc"]="MusePack" \
    ["wav"]="WAVE" \
    ["wv"]="WavPack" \
    ["midi"]="MIDI" \
    ["wma"]="WMA" \
    ["ac3"]="AC-3" \
    ["dts"]="DTS" \
    ["ra"]="RealAudio" \
    ["truehd"]="TrueAudio-HD" \
    ["frog"]="OptimFROG" \
    ["shn"]="Shorten" \
    ["mlp"]="Meridian" \
    )
    # Compara los distintos arrays
    if [ "${type}" = "file" ]; then
        format="${_ext["${ext}"]}"
    else
        format="${_type["${type}"]}"
    fi
    ##########################
}

generate_bar() {
    # Genera una barra de progreso simple
    n=10
    let n=$((_rate/10))
    bar='['
    for i in `seq 1 "${n}"`; do bar+='|' ;done
    for i in `seq "${n}" 9`; do bar+='-' ;done
    bar+=']'
}

get_data_play() {
    # Porcentaje Transcurrido:
    # Genera el porcentaje reproducido
    _rate=$((runtime*100/length))
    ##########################
    # Pone 0 a la izquierda al porcentaje si el "${_rate}" está entre 0 y 9
    (("${_rate}" < "9")) && rate="0"${_rate}""
    ##########################
    # LLama a la función GENERATE_BAR
    generate_bar
    ##########################
    # Tiempo transcurrido / y Duración de la pista:
    # Pasa de milisegundos a segundos (dbus devuelve el resultado de "${runtime}" y "${length}" en milisegundos) para poder ser usado con el comando "date".
    # Comprueba si la duracíon excede de 1 hora (3600 segundos). mostrando la misma y el tiempo transcurrido en formato HH:MM:SS o MM:SS.
    runtime=$((runtime/1000))
    length=$((length/1000))
    if (( "${length}" < "3600" )) ; then
        runtime="$(date -ud @"${runtime}" +%M:%S)"
        length="$(date -ud @"${length}" +%M:%S)"
    else
        runtime="$(date -ud @"${runtime}" +%X)"
        length="$(date -ud @"${length}" +%X)"
    fi
    # LLama a la función GET_FORMAT.
    get_data_format
    ##########################
    # Forma los datos obtenidos y genera la variable "${message}"
    message=""${mess_listen}": *"${title}"* | "${mess_artist}": *"${artist}"* | "${mess_album}": *"${album}"* | "${mess_duration}": *"${runtime}"/"${length}"* *"${bar}" "${rate}"%* | Meta: *"${bitrate}"kbps/"${samplerate}"Hz/"${format}"* | "${mess_player}": *"${player}"*"
    ##########################
}

get_data_stop() {
    # muestra el mensaje de que el reproductor está Parado
    message="${mess_stop}"
    ##########################
}

get_data_down() {
    # muestra el mensaje de que el reproductor está Apagado
    message="${mess_down}"
    ##########################
}

# Comprueba el estado del reproductor y ejecuta las funciones
select_lang
if [ "${length}" = "0" ] || [ "${player}" = "" ] ; then
    get_data_stop
else
    get_data_play
fi
[ "${player}" = "" ] && get_data_down

##########################
# Muestra el el chat la salida de "${message}"
echo "/me ${message}"

##########################
# fin de programa
##########################

