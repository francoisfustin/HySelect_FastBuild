import os
import re
import sys


# Defined where is the HySelect folder containing all the sources.
hyselect_source_folder = "c:\\DocTA\\Software\\TAS Project\\TASelect-Sln\\TASelect\\"

hyselect_rc_file_name = "TASelect.rc"
hyselect_h_file_name = "resource.h"

# Dictionary with key the 'define' string and value the equivalent ID.
all_defines_in_h = dict()

# Dictionary with key the file name and value a dict with key line position and value the line.
all_source_files = dict()

# Result for each define.
# Dictionary with key the define name and value a pair with file name and line position.
defines_search_result = dict()


# Retrieve a list of all files in a folder.
def get_all_files_in_folder(a_dir, exclude_extension):
    list_files = list()
    for name in os.listdir(a_dir):
        if os.path.isdir(os.path.join(a_dir, name)):
            continue
        if len(exclude_extension):
            file_extension = os.path.splitext(os.path.join(a_dir, name))[1]
            for one_extension in exclude_extension:
                if file_extension == one_extension:
                    list_files.append(os.path.join(a_dir, name))
                    break

        else:
            list_files.append(os.path.join(a_dir, name))

    return list_files


def open_file(file_full_name, access_mode='r', encoding_mode='utf-8'):

    # Try to open the file.
    try:
        if encoding_mode is not '':
            file = open(file_full_name, access_mode, encoding=encoding_mode)

        else:
            file = open(file_full_name, access_mode)

    except OSError as e:
        print(" -> Can't open the '" + file_full_name + "'file (error: " + e.strerror + " )")
        sys.exit(-1)

    return file


def read_hyselect_h_file():
    if not os.path.exists(hyselect_source_folder + hyselect_h_file_name):
        print("'" + hyselect_source_folder + hyselect_h_file_name + "' file doesn't exist.")
        sys.exit(-1)

    hyselect_h_file = open_file(hyselect_source_folder + hyselect_h_file_name, "r", "utf-16")

    for one_line in hyselect_h_file:
        result = re.search(r'#define\s([^\s]+)\s+(\d+)$', one_line)
        if not result:
            continue

        all_defines_in_h[result.expand("\\1")] = result.expand("\\2")


def read_all_source_files():
    if not os.path.exists(hyselect_source_folder):
        print("'" + hyselect_source_folder + "' folder doesn't exist.")
        sys.exit(-1)

    list_source_files = get_all_files_in_folder(hyselect_source_folder, (".cpp", ".h", ".hpp"))
    list_source_files += get_all_files_in_folder(hyselect_source_folder + "SerialLibTAScope\\", (".cpp", ".h", ".hpp"))
    list_source_files += get_all_files_in_folder(hyselect_source_folder + "Tests\\TestRunner\\",
                                                 (".cpp", ".h", ".hpp"))
    list_source_files += get_all_files_in_folder(hyselect_source_folder + "Tests\\TestRunner\\DynamicWindow\\",
                                                 (".cpp", ".h", ".hpp"))
    list_source_files += get_all_files_in_folder(hyselect_source_folder + "Tests\\UnitTests\\",
                                                 (".cpp", ".h", ".hpp"))
    list_source_files.append(hyselect_source_folder + hyselect_rc_file_name)

    for one_source_file in list_source_files:
        if one_source_file == hyselect_source_folder + hyselect_rc_file_name:
            source_file = open_file(one_source_file, 'r', 'utf-16')
        else:
            source_file = open_file(one_source_file, 'r', 'cp1252')
        all_lines = dict()
        line_position = 0
        for one_line in source_file:
            all_lines[line_position] = one_line
            line_position += 1

        all_source_files[one_source_file] = all_lines
        source_file.close()


def analyze_source_files():
    for one_define_text, one_define_id in all_defines_in_h.items():
        print(one_define_text + " " + one_define_id)
        exclude_hyselect_rc = False
        if one_define_text[0:4] == "IDD_" or one_define_text[0:4] == "IDR_" or one_define_text[0:4] == "IDC_" or \
                one_define_text[0:4] == "IDV_":
            exclude_hyselect_rc = True

        for one_source_file, all_lines_in_file in all_source_files.items():
            if exclude_hyselect_rc and one_source_file == (hyselect_source_folder + hyselect_rc_file_name):
                continue
            found = False
            for one_line_pos, one_line_text in all_lines_in_file.items():
                if one_line_text.find(one_define_text) != -1:
                    defines_search_result[one_define_text] = (one_source_file, one_line_pos)
                    found = True
                    break

            if found is True:
                break
            else:
                defines_search_result[one_define_text] = ("", 0)


def export_results():
    output_defines_found_file = open_file(hyselect_source_folder + "defines_found.txt", "w", "cp1252")
    output_defines_not_found_file = open_file(hyselect_source_folder + "defines_notfound.txt", "w", "cp1252")
    for define_text, value in defines_search_result.items():
        if len(value[0]) is not 0:
            output_defines_found_file.write(define_text + "-> file: " + value[0] + " / pos: " + str(value[1]) + "\n")
        else:
            output_defines_not_found_file.write(define_text + "\n")

    output_defines_found_file.close()


def clean_hyselect_rc_file():
    input_defines_not_found_file = open_file(hyselect_source_folder + "defines_notfound.txt", "r", "cp1252")

    list_defines_not_found = list()
    for one_line in input_defines_not_found_file:
        one_line = one_line[:-1]
        list_defines_not_found.append(one_line)

    input_defines_not_found_file.close()

    input_hyselect_rc_file = open_file(hyselect_source_folder + hyselect_rc_file_name, "r", "utf-16")
    output_hyselect_rc_file = open_file(hyselect_source_folder + hyselect_rc_file_name + "-cleaned", "w", "utf-16")

    engine_state = 0
    engine_state_3_can_write = True

    for one_line in input_hyselect_rc_file:
        if 0 == engine_state:
            if one_line.find("STRINGTABLE") != -1:
                engine_state = 1
            output_hyselect_rc_file.write(one_line)

        elif 1 == engine_state:
            if one_line.find("BEGIN") == -1:
                print("\"STRINGTABLE\" must always be followed by \"BEGIN\".")
                sys.exit(-1)
            engine_state = 2
            output_hyselect_rc_file.write(one_line)

        elif 2 == engine_state:
            engine_state_3_can_write = False
            if one_line == "END\n":
                output_hyselect_rc_file.write(one_line)
                engine_state = 0
            else:
                # Check first if ID and text are on the same line.
                engine_state = 2
                result = re.search(r'[\s\t]+([\w]+)[\s\t]+(\".+\")$', one_line)

                if not result:
                    engine_state = 3
                    result = re.search(r'[\s\t]+([\w]+)', one_line)

                if not result:
                    print("Error in the file.")
                    sys.exit(-1)

                current_define = result.expand("\\1")
                if current_define not in list_defines_not_found:
                    output_hyselect_rc_file.write(one_line)
                    engine_state_3_can_write = True

        elif 3 == engine_state:
            if engine_state_3_can_write is True:
                output_hyselect_rc_file.write(one_line)
            engine_state = 2

    input_hyselect_rc_file.close()
    output_hyselect_rc_file.close()


def clean_hyselect_h_file():
    input_defines_not_found_file = open_file(hyselect_source_folder + "defines_notfound.txt", "r", "cp1252")

    list_defines_not_found = list()
    for one_line in input_defines_not_found_file:
        one_line = one_line[:-1]
        list_defines_not_found.append(one_line)

    input_defines_not_found_file.close()

    input_hyselect_h_file = open_file(hyselect_source_folder + hyselect_h_file_name, "r", "utf-16")
    output_hyselect_h_file = open_file(hyselect_source_folder + hyselect_h_file_name + "-cleaned", "w", "utf-16")

    for one_line in input_hyselect_h_file:
        result = re.search(r'#define\s([^\s]+)\s+(\d+)$', one_line)
        if not result:
            output_hyselect_h_file.write(one_line)
            continue

        current_define = result.expand("\\1")
        if current_define not in list_defines_not_found:
            output_hyselect_h_file.write(one_line)

    input_hyselect_h_file.close()
    output_hyselect_h_file.close()


def main():
    read_hyselect_h_file()
    read_all_source_files()
    analyze_source_files()
    export_results()
    clean_hyselect_rc_file()
    clean_hyselect_h_file()


main()
