#!/usr/bin/python

import sys 

# Some verilog keyword 
MODULE = "module"
ASSIGN = "assign"

# Some special port after sythesis 
CLK = "CK" 
SI = "test_si" 
SO = "test_so" 
SE = "test_se"

# Some bench keyword 
COMMENT = "#"

INPUT = "INPUT"
OUTPUT = "OUTPUT" 
DFF = "DFF" 

NOT = "NOT" 
AND = "AND" 
NAND = "NAND" 
OR = "OR" 
NOR = "NOR"

# Some standard cell name 
INV = "INVX1"
SDFF = "SDFFXL" 

# ============================================================
# Array storing instance 
primary_input = [] 
primary_output = [] 
comb_gate = [] 
flip_flop = [] 

# Iterate through file 
with open(sys.argv[1]) as fin : 
    for line in fin : 
        line = line.replace(","," ")
        line = line.replace(")"," ")
        line = line.replace("("," ")
        line = line.replace("="," ")
        tokens = line.split()

        if len(tokens) == 0 :  
            continue 

        if tokens[0] == INPUT : 
            primary_input.append(tokens[1]); 
        elif tokens[0] == OUTPUT: 
            primary_output.append(tokens[1]); 
        elif tokens[0] == COMMENT : 
            continue 
        else :  
            if tokens[1] == AND or tokens[1] == NAND \
            or tokens[1] == OR  or tokens[1] == NOR \
            or tokens[1] == NOT : 
                comb_gate.append(tokens)
            elif tokens[1] == DFF : 
                flip_flop.append(tokens)
            else : 
                print "ERROR: gate type %s \
                currently not supported..." % (tokens[1]) 
                sys.exit(1) 

netlist_file = sys.argv[1].split("/")[-1] 
netlist_name = netlist_file.split(".")[0]

# print netlist_name
# write module & its ports  
fout = open(str(netlist_name) + ".v", "w")
fout.write(MODULE + " " \
+ netlist_name + "(" \
+ CLK + ", " \
+ SI + ", " \
+ SO + ", " \
+ SE) 

for i in range(0, len(primary_input)) :
    fout.write(", " + str(primary_input[i]))
for i in range(0, len(primary_output)) :
    fout.write(", " + str(primary_output[i]))
fout.write(");\n\n")

# write input instance 
for i in range(0, len(primary_input)) : 
    fout.write("input " + str(primary_input[i]) + "; \n") 

fout.write("input " + CLK + "; \n") 
fout.write("input " + SI  + "; \n") 
fout.write("input " + SE  + "; \n") 
fout.write("\n")

# write output instance 
for i in range(0, len(primary_output)) : 
    fout.write("output " + str(primary_output[i]) + "; \n") 

fout.write("output " + SO  + "; \n") 
fout.write("\n")

# write combinational gate 
for i in range(0, len(comb_gate)) : 
    g = comb_gate[i] 
    if g[1] == NOT : 
        fout.write(INV + " GNAME" + str(i) + "( " \
        + ".A(" + g[2] + "), " \
        + ".Y(" + g[0] + ") ); \n")
    else : 
        fan_in = len(g) - 2 # gate type & output 
        curr_fan_in = fan_in 
        if fan_in > 7 :  
            print "ERROR: fan-in count %d \
            currently not supported..." % (fan_in) 
            sys.exit(1) 
        elif fan_in > 4 : 
            curr_fan_in = 4 

        fout.write(g[1] + str(curr_fan_in) + "XL" \
        + " GNAME" + str(i) + "( ") 
        for j in range(0, curr_fan_in) : 
           fout.write("." + chr(ord('A') + j) + "(" + g[j + 2] + "), ") 

        if fan_in > 4 : 
            curr_fan_in = fan_in - curr_fan_in 
            fout.write(".Y(" + g[0] + "_1) ); \n")
            fout.write(g[1] + str(curr_fan_in + 1) + "XL" \
            + " GNAME" + str(i) + "_1( " \
            + ".A(" + g[0] + "_1), ")
            for j in range(0, curr_fan_in) : 
                fout.write("." + chr(ord('B') + j) + "(" + g[j + 6] + "), ") 

        fout.write(".Y(" + g[0] + ") ); \n")
fout.write("\n") 

# write flip_flops 
curr_si = SI 
for i in range(0, len(flip_flop)) : 
    ff = flip_flop[i]
    fout.write(SDFF + " DFF" + str(i) + "( " \
    + ".SI(" + curr_si + "), " \
    + ".SE(" + SE + "), " \
    + ".D(" + ff[2] + "), " \
    + ".Q(" + ff[0] + ") ); \n")  
    curr_si = ff[0]

# write assignment 
fout.write("\n") 
fout.write(ASSIGN + " " + SO + " = " + curr_si + ";\n\n") 
fout.write("endmodule\n")

fout.close() 
fin.close() 
