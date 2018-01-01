#!/bin/bash

FILES=$(find . -name \*.h -o -name \*.c)

for FILE in $(echo ${FILES}); do
    echo $FILE

    indent -nbad -bap -nbc -bbo -bl -bli2 -bls -ncdb -nce -cp1 -cs -di2 -ndj -nfc1 -nfca -hnl -i2 -ip5 -lp \
           -pcs -nprs -psl -saf -sai -saw -nsc -nsob ${FILE} -o $FILE.converted && \
        mv -v ${FILE}.converted ${FILE}

#    expand -t4 ${FILE} > ${FILE}.converted && \

done
