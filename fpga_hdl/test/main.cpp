#include <iostream>
#include <cstdint>
#include "Vstate_controller.h"
#include "verilated_vcd_c.h"
using namespace std;

Vstate_controller *state_controller;
VerilatedVcdC *tfp;
uint16_t ram[16*1024*1024];

void process_memory_cmd()
{
    static bool do_read = false;
    static bool do_write = false;
    static int wait = -1;
    static uint16_t read_data = 0;

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
        //state_controller->rd_data = ram[state_controller->rd_addr];
        read_data = ram[state_controller->rd_addr];
        state_controller->rd_data = 0xFFFF;
        state_controller->busy = false;
        //state_controller->rd_ready = true;
        wait = 100;
        do_read = false;
    }

    if(wait > 0) 
    {
        wait -= 1;
    }
    else if(wait == 0)
    {
        state_controller->rd_data = ram[state_controller->rd_addr];
        state_controller->rd_ready = true;
        wait = -1;
    }
}

void clk()
{
    static int main_time = 0;
    state_controller->clk = true;
    state_controller->eval();
    tfp->dump(main_time);
    main_time += 1;
    state_controller->clk = false;
    state_controller->eval();
    process_memory_cmd();
    tfp->dump(main_time);
    main_time += 1;
}

uint8_t spi_transfer(uint8_t data_send)
{
    uint8_t data_in = 0;
    state_controller->SSEL = false;
    clk();
    clk();
    clk();
    clk();
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
    uint8_t result;
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);
    tfp = new VerilatedVcdC;
    state_controller = new Vstate_controller;
    state_controller->trace(tfp, 99);
    tfp->open("trace.vcd");

    state_controller->SSEL = true;
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

    while(true)
    {
        spi_transfer(4);
        result = spi_transfer(0);
        cout << "Read result " << (int) result << endl;
        if(result == 1)
        {
            break;
        }
    }

    spi_transfer(3);
    uint8_t b1 = spi_transfer(0);
    uint8_t b2 = spi_transfer(0);

    cout << (int)result << " " << (int)b1 << " " << (int)b2 << endl;

    spi_transfer(2);
    spi_transfer(0);
    spi_transfer(0);
    spi_transfer(1);

    while(true)
    {
        spi_transfer(4);
        result = spi_transfer(0);
        if(result == 1)
        {
            break;
        }
    }
    
    spi_transfer(3);
    b1 = spi_transfer(0);
    b2 = spi_transfer(0);

    cout << (int)result << " " << (int)b1 << " " << (int)b2 << endl;

    tfp->close();

    return 0;
}