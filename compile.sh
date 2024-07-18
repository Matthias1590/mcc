#!/bin/bash

set -xe

# Make qbe
make -C ./qbe-1.2

# Make mcc
make -C .

# Run mcc
./bin/mcc > ./bin/out.qbe

# Translate outputted qbe code into assembly
./qbe-1.2/qbe -o ./bin/out.s ./bin/out.qbe

# Assemble
as -o ./bin/out.o ./bin/out.s

# Link
cc -o ./bin/out ./bin/out.o
