#!/bin/sh

for i in *.po; do
 echo "Updating file: $i"
 msgmerge $i *.pot > $i.new
 mv $i.new $i
done
