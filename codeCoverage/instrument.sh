#!/bin/bash
time ./code_coverage -s  libtensorflow_cc.so.2 inst/libtensorflow_cc.so.2
cp inst/libtensorflow_cc.so.2 /opt/python/pkgs/python/lib/python3.12/site-packages/tensorflow/libtensorflow_cc.so.2
