import os
import re
import sys


# Try to open the file.
try:
    input_file = open("input.txt", "r")

except OSError as e:
    print(" -> Can't open the 'input.txt' file (error: " + e.strerror + " )")
    sys.exit(-1)

try:
    output_file = open("output.txt", "w")

except OSError as e:
    print(" -> Can't create the 'output.txt' file (error: " + e.strerror + " )")
    sys.exit(-1)

for one_line in input_file:
    if -1 == one_line.find("NEWPRODPIC"):
        output_file.write(one_line)
    else:
        anchoring_point_list = re.findall("_Pt\(\s*\d+\s*,\s*\d+\s*\)", one_line)
        angle_list = re.findall("\),(\s*\d+)", one_line)
        output_file.write(one_line[:one_line.find("_Pt")])
        anchoring_point_count = len(anchoring_point_list)
        angle_count = len(angle_list)

        for i in range(anchoring_point_count):
            if i == 0:
                output_file.write("{ APDf( _I, " + anchoring_point_list[i] + " )")
            elif i == 1:
                output_file.write("APDf( _O, " + anchoring_point_list[i] + " )")
            elif i == 2:
                if ( anchoring_point_count == 3 and angle_count == 0 ) or anchoring_point_count == 5:
                    output_file.write("APDf( _B, " + anchoring_point_list[i] + " )")
                else:
                    output_file.write("APDf( _CP, " + anchoring_point_list[i] + "," + angle_list[0] + " )")
            elif i == 3:
                output_file.write("APDf( _CS, " + anchoring_point_list[i] + "," + angle_list[1] + " )")

            if i < anchoring_point_count - 1:
                output_file.write(", ")
            else:
                output_file.write(" } );\n")
