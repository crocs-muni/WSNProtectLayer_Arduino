#!/bin/bash

PATH_FILE=$EDU_HOC_HOME/config/relay_ar_paths.txt
JAR=/home/martin/Desktop/Edu-hoc/JeeTool/JeeTool/dist/JeeTool.jar
CLASS=cz.muni.fi.crocs.EduHoc.Main
PROJECT=/home/martin/git/arduino_config/Arduino_app
#PROJECT=/home/martin/Desktop/Edu-hoc/src/EepromCheck

> $PATH_FILE

for f in `cat config_files/devices.cfg`; do
    printf "/dev/serial/by-id/%s\n" $f >> $PATH_FILE;
done



#echo "$J0" > $PATH_FILE
#echo "$J1" >> $PATH_FILE
#echo "$J2" >> $PATH_FILE
#echo "$J3" >> $PATH_FILE
#echo "$J4" >> $PATH_FILE
#echo "$J5" >> $PATH_FILE
#echo "$J6" >> $PATH_FILE
#echo "$J7" >> $PATH_FILE

java -cp $JAR $CLASS -a $PATH_FILE -u $PROJECT

rm $PATH_FILE

