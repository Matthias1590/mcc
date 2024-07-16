#!/bin/bash

clear; make && valgrind -q --leak-check=full ./bin/mcc
