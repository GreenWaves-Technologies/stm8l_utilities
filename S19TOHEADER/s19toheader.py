#!/usr/bin/python

import sys
import math

###############################################################################
# Start of file
###############################################################################
if(len(sys.argv) < 2):
    print("Usage s19toheader.py FILENAME")
    quit()

###############################################################################
# Parse s19 file
###############################################################################
s19_file = open(sys.argv[1], 'r')
s19_addr = {}
s19_data = {}
byte_counter=0;

for line in s19_file:
    rec_field = line[:2]
    prefix    = line[:4]

    if rec_field == "S0" or rec_field == "S9":
        continue

    size = int(line[2:4], 16)
    addr = int(line[4:8], 16)
    
    for i in range(0,size-3):
        data = line[8+i*2:10+i*2]
        s19_addr[byte_counter] = addr+i
        s19_data[byte_counter] = data
        byte_counter=byte_counter+1
        

s19_file.close()


###############################################################################
# open files
###############################################################################
stm8l_data      = open("stm8l_data.h",    'w')

###############################################################################
# write the header
###############################################################################


# WRITE NON COMPRESSED HEADER FILE
stm8l_data.write("int stm8l_code_size = %d;\n\n" % byte_counter)

stm8l_data.write("int stm8l_code_data[%d] = {\n" % (byte_counter*2))

for i in range(0,byte_counter):
    stm8l_data.write("0x%02X, 0x%s,\n" % ((s19_addr[i]), s19_data[i]))
    
stm8l_data.write("};\n\n")

stm8l_data.close()
