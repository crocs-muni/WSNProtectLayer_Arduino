#!/bin/bash

PATH_FILE=$EDU_HOC_HOME/config/relay_legit_paths.txt
JAR=/home/martin/Desktop/Edu-hoc/JeeTool/JeeTool/dist/JeeTool.jar
CLASS=cz.muni.fi.crocs.EduHoc.Main
#PROJECT=/home/martin/git/wsnpl_arduino/ProtectLayer/BS_slave/
PROJECT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "$J6" > $PATH_FILE

#echo "$J1" > $PATH_FILE
#echo "$J2" >> $PATH_FILE
#echo "$J3" >> $PATH_FILE
#echo "$J4" >> $PATH_FILE

#echo "/dev/ttyUSB0" >> $PATH_FILE


java -cp $JAR $CLASS -a $PATH_FILE -u $PROJECT

rm $PATH_FILE
