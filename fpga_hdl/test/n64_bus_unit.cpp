#include <iostream>
#include <cstdint>
#include "Vn64_bus.h"
#include "verilated_vcd_c.h"
using namespace std;

Vn64_bus *n64_bus;
VerilatedVcdC *tfp;

uint32_t ram[1*1024*1024];

void clk()
{
    static int main_time = 0;
    n64_bus->clk = true;
    n64_bus->eval();
    tfp->dump(main_time);
    main_time += 1;
    n64_bus->clk = false;
    n64_bus->eval();
    tfp->dump(main_time);
    main_time += 1;
}

uint32_t n64_bus_trans(uint32_t addr, uint32_t data)
{
    uint32_t result = 0;
    n64_bus->ALE_H = 1;

    for(int i=0; i < 100; i++)
        clk();

    n64_bus->ALE_L = 1;
    n64_bus->AD = (addr >> 16);

    for(int i=0; i < 110; i++)
        clk();

    n64_bus->ALE_H = 0;

    for(int i=0; i < 110; i++)
        clk();

    n64_bus->AD = addr & 0x0000FFFF;
    n64_bus->ALE_L = 0;

    for(int i=0; i < 100; i++)
        clk();

    n64_bus->data = data >> 16;
    n64_bus->READ = 0;

    for(int i=0; i < 100; i++)
        clk();

    result = n64_bus->AD << 16;
    n64_bus->READ = 1;

    for(int i=0; i < 60; i++)
        clk();

    n64_bus->data = data & 0x0000FFFF;
    n64_bus->READ = 0;

    for(int i=0; i < 100; i++)
        clk();

    result |= n64_bus->AD;
    n64_bus->READ = 1;

    return result;
}

void init()
{
    /* TODO actually test ram, this is place holder for now */
    ram[0] = 0xDEADBEEF;

    n64_bus->READ = 1;
    n64_bus->WRITE = 1;
}

int main(int argc, char *argv[])
{
    Verilated::commandArgs(argc, argv);
    n64_bus = new Vn64_bus();
    Verilated::traceEverOn(true);
    tfp = new VerilatedVcdC;
    n64_bus->trace(tfp, 99);
    tfp->open("n64_bus.vcd");

    init();

    /* Run 100 clock cycles to just initalize stuff */
    for(int i=0; i < 100; i++)
        clk();


    cout << "0x" << hex << n64_bus_trans(0, 0xDEADBEEF) << dec << endl;

    tfp->close();
    return 0;
}