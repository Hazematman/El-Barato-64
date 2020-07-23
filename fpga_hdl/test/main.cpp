#include <iostream>
#include <cstdint>
#include "Vstate_controller.h"
using namespace std;

Vstate_controller *state_controller;
uint16_t ram[16*1024*1024];

void process_memory_cmd()
{
    static bool do_read = false;
    static bool do_write = false;

    if(state_controller->wr_enable || state_controller->rd_enable)
    {
        do_write = state_controller->wr_enable;
        do_read = state_controller->rd_enable;
        state_controller->rd_ready = false;
        state_controller->busy = true;
    }
    else if(do_write)
    {
        ram[state_controller->wr_addr] = state_controller->wr_data;
        state_controller->busy = false;
        do_write = false;
    }
    else if(do_read)
    {
        state_controller->rd_data = ram[state_controller->rd_addr];
        state_controller->busy = false;
        state_controller->rd_ready = true;
        do_read = false;
    }
}

void clk()
{
    state_controller->clk = true;
    state_controller->eval();
    state_controller->clk = false;
    state_controller->eval();

    process_memory_cmd();
}

uint8_t spi_transfer(uint8_t data_send)
{
    uint8_t data_in = 0;
    state_controller->SSEL = false;
    for(int i = 0; i < 8; i++)
    {
        uint8_t bit = 7 - i;
        state_controller->MOSI = (data_send & (1 << bit)) >> bit;
        data_in |= (state_controller->MISO << bit);
        state_controller->SCK = true;
        clk();
        clk();
        clk();
        clk();

        state_controller->SCK = false;

        clk();
        clk();
        clk();
        clk();
    }

    state_controller->SSEL = true;
    clk();
    clk();
    clk();
    clk();

    return data_in;
}

int main(int argc, char *argv[])
{
    Verilated::commandArgs(argc, argv);
    state_controller = new Vstate_controller;

    state_controller->SSEL = false;
    for(int i=0; i < 1000; i++)
        clk(); /* Run 1000 cycles just to init */

    spi_transfer(1);
    spi_transfer(0);
    spi_transfer(0);
    spi_transfer(0);
    spi_transfer(1);
    spi_transfer(2);

    spi_transfer(2);
    spi_transfer(0);
    spi_transfer(0);
    spi_transfer(0);

    spi_transfer(3);
    uint8_t b1 = spi_transfer(0);
    uint8_t b2 = spi_transfer(0);

    cout << (int)b1 << " " << (int)b2 << endl;

    return 0;
}