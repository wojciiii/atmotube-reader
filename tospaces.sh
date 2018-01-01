#!/bin/bash

FILES=$(find . -name \*.h -o -name \*.c)

for FILE in $(echo ${FILES}); do
    echo $FILE
    expand -t4 ${FILE} > ${FILE}.converted && \
	mv -v ${FILE}.converted ${FILE}
done
