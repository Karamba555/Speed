#!/bin/bash
make clean 
./scripts/feeds update -a
./scripts/feeds install -a
git restore .config 
make -j2
