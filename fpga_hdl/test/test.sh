#!/bin/bash

verilator --trace -cc ../state_controller.sv ../spi.sv --exe main.cpp
make -C ./obj_dir/ -f Vstate_controller.mk

verilator --trace -cc ../n64_bus.sv --exe n64_bus_unit.cpp
make -C ./obj_dir/ -f Vn64_bus.mk