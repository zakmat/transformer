#!/bin/bash

FILES=`ls Examples | grep style`

if [ ! -d Results ]; then mkdir Results; fi
if [ ! -e transformer ]; then make; fi

for f in ${FILES}
do
	./transformer --xml Examples/cdcatalog.txt --xslt Examples/${f} --dest Results/${f/.txt/.html}
done

