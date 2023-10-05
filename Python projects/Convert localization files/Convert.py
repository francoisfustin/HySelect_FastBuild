import getopt
import json
import os
import re
import sys
import xml.etree.ElementTree as XmlElementTree

# Will contain a list of all language available in one xml file.
all_available_languages = []

# Will contain all opened output file.
# The key is the language code and the value is a list with in first position the handle on the file and in the
# second position the file name.
all_output_files = dict()

# Will contain all string for each country.
# The key is the language code and the value is a dict with the key as the string identifier and the string to
# export.
all_output_strings = dict()

# Set if we convert all files or only one defined by 'i' option.
convert_all_or_one = "all"

# Defined if the 'convert_all_or_one' variable is set to "one".
input_file = ""

# Default source directory.
source_dir = ".\\Localisation files\\Sources\\"

# Default destination directory.
destination_dir = ".\\Localisation files\\Converted\\"

# Default export format.
export_format = "xml"


def check_if_folder_or_file_exist(folder_path):
    if not os.path.exists(folder_path):
        print("'" + folder_path + "' folder doesn't exist.")
        sys.exit(-1)


# Get a list filed with the name of every immediate subdirectories.
def get_all_files_in_folder(a_dir):
    return [os.path.join(a_dir, name) for name in os.listdir(a_dir)
            if not os.path.isdir(os.path.join(a_dir, name))]


def read_all_available_languages(new_dataset):
    all_available_languages.clear()
    all_output_strings.clear()
    language_code_tab = new_dataset.find('Table')

    if language_code_tab is None or "name" not in language_code_tab.attrib or \
            language_code_tab.attrib["name"] != "LANGCODE_TAB":
        print("'LANGCODE_TAB' table not found. Bad file!\n")
        sys.exit(-1)

    for language in language_code_tab.findall('language'):
        code_node = language.find('code')

        if code_node is None:
            print("'code' xml tag not existing in 'language' from the 'LANGCODE_TAB' table. Bad file!\n")
            sys.exit(-1)

        english_name_node = language.find('enName')

        if english_name_node is None:
            print("'enName' xml tag not existing in 'language' from the 'LANGCODE_TAB' table. Bad file!\n")
            sys.exit(-1)

        all_available_languages.append((code_node.text, english_name_node.text))
        all_output_strings[code_node.text] = dict()


def create_one_output_file(filename):
    try:
        output_file = open(filename, 'w', encoding='utf-8')
    except OSError as e:
        print(" -> Can't create the '" + filename + "'file (error: " + e.strerror + ")")
        sys.exit(-1)

    return output_file


def create_all_output_files(filename):
    all_output_files.clear()

    # HyUpdateRc-compl.xml
    result = re.search(r'.+\\(.+)(-compl.xml)', filename)

    if not result:
        print(" -> The '" + filename + "' compare folder has not the good format (blabla-compl.xml).")
        sys.exit(-1)

    base_file_name = result.expand("\\1")

    for code, english_name in all_available_languages:
        output_file_name = destination_dir + base_file_name + "_" + english_name + "." + export_format
        output_file = create_one_output_file(output_file_name)

        if "xml" == export_format:
            output_file.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
            output_file.write("<resources>\n")
        else:
            output_file.write("[\n")

        all_output_files[code] = (output_file, output_file_name)


def save_string(id_number, string_definitions):

    for code in string_definitions:
        if code in all_output_files is False:
            print("'" + code + "' code language doesn't exist.\n")
            close_all_output_files()
            sys.exit(-1)

        string_output = ""

        if len(string_definitions[code]) > 0:

            if "xml" == export_format:
                string_output += "\t<string name=\"" + id_number + "\">"
                string_output += string_definitions[code] + "</string>\n"
            else:
                string_output = "  {\n    \"term\": \"" + id_number + "\",\n"
                string_output += "    \"definition\": \"" + string_definitions[code].replace("\"", "\\\"") + "\",\n"
                string_output += "    \"context\": \"\",\n"
                string_output += "    \"term_plural\": \"\",\n"
                string_output += "    \"reference\": \"\",\n"
                string_output += "    \"comment\": \"\"\n"
                string_output += "  }"

            all_output_strings[code][id_number] = string_output


def extract_one_node(node_to_extract):
    for one_string in node_to_extract.findall('string'):
        id_number_node = one_string.find('IDS')
        if id_number_node is None:
            print("'IDnr' tag file not found in the '" + one_string + "' tag.\n")
            close_all_output_files()
            sys.exit(-1)

        string_definitions = dict()

        for code in all_output_files:
            code_ts = code + "_TS"

            code_ts_node = one_string.find(code_ts)
            if code_ts_node is None:
                print("'" + code_ts + "' tag file not found in the '" + one_string + "' tag.\n")
                close_all_output_files()
                sys.exit(-1)

            code_text = ""

            if code_ts_node.text == "Translated":
                code_text_node = one_string.find(code)
                if code_text_node is None:
                    print("'" + code + "' tag file not found in the '" + one_string + "' tag.\n")
                    close_all_output_files()
                    sys.exit(-1)

                code_text = code_text_node.text

            string_definitions[code] = code_text

        save_string(id_number_node.text, string_definitions)


def extract_data(root):
    table_list = root.findall("Table")

    if len(table_list) == 1:
        extract_one_node(root)
    else:
        # It's the case for the 'TadbST-compl.xml' file.
        for table_node in table_list:
            if 'name' not in table_node.attrib:
                print("'name' attribute must be defined for each 'Table' xml tag.\n")
                close_all_output_files()
                sys.exit(-1)

            if table_node.attrib['name'] == 'LANGCODE_TAB':
                continue

            extract_one_node(table_node)


def write_all_strings():
    for code in all_output_strings:
        sorted_strings = sorted(all_output_strings[code])
        last_string_id = sorted_strings[len(sorted_strings) - 1]
        for string_id in sorted_strings:
            all_output_files[code][0].write(all_output_strings[code][string_id])

            if "json" == export_format:
                if string_id != last_string_id:
                    all_output_files[code][0].write(",")
                all_output_files[code][0].write("\n")


def close_all_output_files():
    if len(all_output_files) == 0:
        return

    for code in all_output_files:
        if "xml" == export_format:
            all_output_files[code][0].write("</resources>")
        else:
            all_output_files[code][0].write("]")

        all_output_files[code][0].close()


def convert_one_file(filename):
    print("Convert the file: " + filename + "\n")

    tree = XmlElementTree.parse(filename)
    root = tree.getroot()

    read_all_available_languages(root)
    create_all_output_files(filename)
    extract_data(root)
    write_all_strings()
    close_all_output_files()


def convert_all_files():
    all_source_files = get_all_files_in_folder(source_dir)

    if len(all_source_files) == 0:
        print("No source file found in the '" + source_dir + "' folder.")
        sys.exit(-1)

    for one_source_file in all_source_files:
        convert_one_file(one_source_file)


def start_conversion():
    # Verify source and destination folders.
    check_if_folder_or_file_exist(source_dir)
    check_if_folder_or_file_exist(destination_dir)

    if convert_all_or_one == "all":
        convert_all_files()
    else:
        # Verify if the input file exist.
        check_if_folder_or_file_exist(source_dir + input_file)
        convert_one_file(source_dir + input_file)


def main():
    # Start by parsing command line.
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hc:i:s:d:f:", ["help", "convert", "input", "sourcedir",
                                                                 "destdir", "format"])
    except getopt.GetoptError:
        print("Convert.py -h -c <opt> -i <opt> -s <opt> -d <opt> -f <opt>")
        sys.exit(-1)

    if 0 == opts.__len__():
        print("Convert.py -h -c <opt> -i <opt> -s <opt> -d <opt> -f <opt>")
        sys.exit(-1)

    if "h" in opts:
        print("Convert.py -h -c <opt> -i <opt> -s <opt> -d <opt> -f <opt>")
        print(" -h --help : this help.")
        print(" -c <opt> --convert <opt> : ")
        print("       <opt> = \"all\" : take all input files.")
        print("       <opt> = \"one\" : just one file defined by 'n' option.")
        print(" -i <opt> --input <opt> : ")
        print("       <opt> = name of the file if 'c' is 'one'.")
        print(" -s <opt> --sourcedir <opt> : ")
        print("       <opt> = path where are source files.")
        print(" -d <opt> --destdir <opt> : ")
        print("       <opt> = path where to put destination files.")
        print(" -f <opt> --format <opt> : ")
        print("       <opt> = xml (default).")
        print("       <opt> = json.")
        sys.exit()

    global convert_all_or_one
    global input_file
    global source_dir
    global destination_dir
    global export_format

    if "c" in opts:
        if "all" == opts["c"] or "one" == opts["c"]:
            convert_all_or_one = opts["c"]
        else:
            print("'-c' option must be 'all' or 'one'.")
            sys.exit()

    if convert_all_or_one == "one":
        if "i" in opts:
            input_file = opts["i"]
        else:
            print("If '-c' option is 'one', '-i' option must be defined.")
            sys.exit()

    if "s" in opts:
        source_dir = opts["s"]
        if len(source_dir) == 0:
            print("Source folder can't be empty if 's' option is defined!")
            sys.exit()

    if "d" in opts:
        destination_dir = opts["d"]
        if len(destination_dir) == 0:
            print("Destination folder can't be empty if 'd' option is defined!")
            sys.exit()

    if "f" in opts:
        if opts["f"] == "xml" or opts["f"] == "json":
            export_format = opts["f"]
        else:
            print("'-f' option can be 'xml' or 'json'.")
            sys.exit()

    start_conversion()


# main()

convert_all_or_one = "one"
input_file = "TasRc-compl.xml"
# source_dir
# destination_dir
export_format = "json"
start_conversion()
