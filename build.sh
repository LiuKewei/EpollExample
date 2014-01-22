#! /bin/bash
 
dir=build/obj
 
if [ ! -d  $dir ]; then
    mkdir -p $dir
fi
 
cd $dir
cmake ../..
make $1