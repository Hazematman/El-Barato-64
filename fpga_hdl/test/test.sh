#!/bin/bash

verilator --trace -cc ../state_controller.sv ../spi.sv --exe main.cpp
make -C ./obj_dir/ -f Vstate_controller.mk
