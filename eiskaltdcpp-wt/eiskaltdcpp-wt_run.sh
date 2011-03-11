#!/bin/sh

if [ -z ${PORT} ]; then
  export PORT="8080"
fi

if [ -z ${ADDRESS} ]; then
  export ADDRESS="0.0.0.0"
fi

if [ -z ${DOCROOT} ]; then
  export DOCROOT="/usr/share/eiskaltdcpp/wt/"
fi

echo "Launching eiskaltdcpp-wt on http://${ADDRESS}:${PORT}..."
echo "DOCROOT = ${DOCROOT}"
eiskaltdcpp-wt --http-port=${PORT} --http-address=${ADDRESS} --docroot ${DOCROOT}
echo "Quit..."

