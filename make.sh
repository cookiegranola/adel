#!/bin/sh
set -x
cd lib/irrlicht/source/Irrlicht
make
cd ../../../..
make
