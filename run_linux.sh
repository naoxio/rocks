#!/bin/bash

xmake clean

xmake 

cd build/linux/x86_64/release/

./image_viewer
