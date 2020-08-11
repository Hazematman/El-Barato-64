#!/usr/bin/env python3
import serial
import sys
import os
from time import sleep

def wait_cmd(ser):
    if 0:
        last_char = 'a'
        while True:
            new_char = ser.read().decode("ascii")
            if last_char == ":" and new_char == " ":
                break
            last_char = new_char
    elif 1:
        while True:
            try:
                byte = ser.read(1).decode("ascii")
                print(byte)
                if  byte == ":" or byte == "":
                    if byte == ":":
                        ser.read(1)
                    return
            except serial.SerialTimeoutException:
                print("Timeout")
                return
    else:
        sleep(1 / 100)

def read_new(ser):
    while True:
        try:
            byte = ser.read(1).decode("ascii")
            if  byte == "D" or byte == "":
                return
        except serial.SerialTimeoutException:
            print("Timeout")
            return

def main():
    if len(sys.argv) < 2:
        print("serial_prog.py <binary file>")
        return 0

    addr = 0
    ser = serial.Serial("/dev/ttyUSB0", 115200, bytesize=8, timeout=1, stopbits=serial.STOPBITS_ONE)

    size = os.path.getsize(sys.argv[1]) // 2
    f = open(sys.argv[1], "rb")

    for i in range(size):
        #wait_cmd(ser)
        val = int.from_bytes(f.read(2), "big")
        print("Read {}".format(val))
        print("  1 {} {}\r".format(addr, val))
        ser.write("1 {} {}\r".format(addr, val).encode("ascii"))
        addr += 1
        read_new(ser)

    f.close()
    print("Done writing")
    return 0

if __name__ == "__main__":
    sys.exit(main())