#!/bin/bash

# NowPlaying QMMP Script
# Author: Gustavo Alvarez <sl1pkn07@gmail.com>
# License: GPL-2
# Date: 2013-05-08

# Funciones:
# Función set_default_lang(): Selecciona el lenguaje por defecto de la aplicación. por defecto "en_EN" (inglés)
# Función set_lang(): Selecciona el lenguaje de la aplicación según la variable de sistema "${LANG}".
# Función get_data_format(): Genera arrays asociativos para determinar tipo/formato de los archivos reproducidos.
# Función generate_bar(): Crea una barra de progreso simple.
# Función get_data_play(): se ejecuta cuando el player está en funcionamiento.
# Función get_data_stop(): se ejecuta cuando el player está parado.
# Función get_data_down(): se ejecuta cuando el player está apagado.

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

set_default_lang() {
   source `pwd`/qmmp_langs/en_EN
}

set_lang() {
  # Llama a la función set_default_lang()
  set_default_lang

  # Selecciona el archivo del lenguaje según el lenguaje del sistema (seleccionado por "${lang})".
  # si no existe el archivo del lenguaje. deja el lenguaje por defecto
  lang="$(echo "${LANG}" | cut -d "." -f1)"
  lang_short="$(echo "${LANG}" | cut -d "." -f1 | cut -d "_" -f1)"
  for l in $(ls -l `pwd`/qmmp_langs | awk 'NR!=1 && !/^d/ {print $NF}'); do
    [ "${l}" = "${lang}" ] && source `pwd`/qmmp_langs/"${l}" && break
    [ "$(echo ${l} | cut -d "_" -f1)" = "${lang_short}" ] && source `pwd`/qmmp_langs/"${l}" && break
  done
}

get_data_format() {
  # Determina el formato audio:
  # primero comprueba mediante "${type}" si es CD de Audio (cdda), CUE-sheet (cue), URL Stream (http) o si es un archivo simple (file).
  # Si coincide con "file", vuelve a compararlo usando "${ext}" para determinar el formato por su extensión.

  # Genera el array "_type"
  declare -A _type=(\
  ["cdda"]="CDDA" \
  ["cue"]="CUE-sheet" \
  ["http"]="URL Stream" \
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
    format="${_ext[${ext}]}"
  else
    format="${_type[${type}]}"
  fi
}

generate_bar() {
  # Genera una barra de progreso simple
  n=10
  let n=$((rate/10))
  bar='['
  for i in $(seq 1 "${n}"); do bar+='|'; done
  for i in $(seq "${n}" 9); do bar+='-'; done
  bar+=']'
}

get_data_play() {
  # LLama a la función GET_FORMAT.
  get_data_format

  # Como los Streams URL no se puede determinar la duracón, necesita generar otro tipo de ${out} para adaptarse a la nueva circustancia
  if [ "${type}" != "http" ]; then
    # Porcentaje Transcurrido:
    # Genera el porcentaje reproducido
    rate="$((runtime*100/length))"

    # Pone 0 a la izquierda al porcentaje si el "${_rate}" está entre 0 y 9
   (("${rate}" < "10")) && rate="0${rate}"

    # LLama a la función GENERATE_BAR
    generate_bar
    # Tiempo transcurrido / y Duración de la pista:
    # Pasa de milisegundos a segundos (dbus devuelve el resultado de "${runtime}" y "${length}" en milisegundos) para poder ser usado con el comando "date".
    runtime="$((runtime/1000))"
    length="$((length/1000))"

    # Comprueba si la duracíon excede de 1 hora (3600 segundos). mostrando la misma y el tiempo transcurrido en formato HH:MM:SS o MM:SS.
    if (( "${length}" < "3601" )) ; then
      runtime="$(date -ud @${runtime} +%M:%S)"
      length="$(date -ud @${length} +%M:%S)"
    else
      runtime="$(date -ud @${runtime} +%X)"
      length="$(date -ud @${length} +%X)"
    fi
    # Forma los datos obtenidos y genera la variable "${out}"
    out="${msg_listen}: ${title} | ${msg_artist}: ${artist} | ${msg_album}: ${album} | ${msg_duration}: ${runtime}/${length} ${bar} ${rate}% | Meta: ${bitrate}kbps/${samplerate}Hz/${format} | ${msg_player}: ${player}"
  else
    # Extrae la URL del streeam
    stream="$(echo "${mdata}"| sed -ne 's/^location: \(.*\)$/\1/p')"
    # Al ser Stream URL, necesita otro tipo de datos, forma los datos obtenidos y genera la variable "${out}"
    out="${msg_listen}: ${title} | ${msg_artist}: ${artist} | ${msg_album}: ${album} | ${msg_stream}: ${stream} | Meta: ${bitrate}kbps/${samplerate}Hz/${format} | ${msg_player}: ${player}"
  fi
}

get_data_stop() {
  # muestra el mensaje de que el reproductor está Parado
  out="${msg_stop}"
}

get_data_down() {
  # muestra el mensaje de que el reproductor está Apagado
  out="${msg_down}"
}

# Comprueba el estado del reproductor y ejecuta las funciones
set_lang
if [ "${length}" = "0" -o "${player}" = "" ] ; then
  get_data_stop
  [ "${type}" = "http" ] && get_data_play
else
  get_data_play
fi
[ "${player}" = "" ] && get_data_down

# Muestra el el chat la salida de "${out}"
echo "/me ${out}"

# fin de programa
