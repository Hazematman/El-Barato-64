#include <iostream>
#include <cstdint>
#include "Vstate_controller.h"
using namespace std;

Vstate_controller *state_controller;

uint8_t spi_transfer(uint8_t data_send)
{
    for(int i = 0; i < 8; i++)
    {
        uint8_t bit = 7 - i;
        state_controller->MOSI = (data_send & (1 << bit)) >> bit;
    }
}

int main(int argc, char *argv[])
{
    Verilated::commandArgs(argc, argv);
    state_controller = new Vstate_controller;

    

    return 0;
}