#!/bin/sh

export PORT=8080
export ADDRESS=0.0.0.0

echo "Launching eiskaltdcpp-wt..."
eiskaltdcpp-wt --http-port=$PORT --http-address=$ADDRESS --docroot /usr/share/eiskaltdcpp/wt/
echo "eiskaltdcpp-wt was started on $ADDRESS:$PORT"

