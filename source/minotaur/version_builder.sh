#!/bin/bash

curDir=`pwd`
baseDir=$(cd "$(dirname "$0")"; pwd)


target_dir=$1
target_file=$2
echo $target_dir

cd $target_dir

COMMIT=`git log -1 --decorate=short | grep commit | head -1`
AUTHOR=`git log -1 --decorate=short | grep Author | head -1`
DATE=`git log -1 --decorate=short | grep Date | head -1`

cd $curDir

rm -f $target_file
echo "/* this file is generated by version_builder.sh */" >> $target_file
echo "/* please do not edit unless you know what you are doing */" >> $target_file
echo "#define _GIT_COMMIT_ (\"$COMMIT\")" >> $target_file
echo "#define _GIT_AUTHOR_ (\"$AUTHOR\")" >> $target_file
echo "#define _GIT_DATE_ (\"$DATE\")" >> $target_file
echo "#define GIT_CURRENT_INFO (\"$COMMIT\n$AUTHOR\n$DATE\n\")" >> $target_file

