import getopt
import os
import re
import sys

# This list allow us to match language code from catmaker files to UNICODE language code used in HySelect.
language_code = ["en", "zh_cn", "zh-tw", "cs", "da", "nl", "en-us", "fi", "fr", "de", "hu", "it", "ja", "ko", "no",
                 "pl", "pt", "pt-br", "ro", "ru", "es", "sv", "tr", "hr", "sl", "rs", "lt"]

# Set the language code that we want to create.
language_code_to_create = ""

# Set the mode of the operation:
#   "new":  the new language code will be added and all strings will empty with 'Untranslated' translation state.
#   "copy": the new language code will be added and we will copy the existing translation with the corresponding
#           translation found for the "language_code_to_copy" code if existing.
add_mode = "new"

# Set the language code to retrieve the translation mode is "copy".
language_code_to_copy = ""

# Folders where are the XML files to operate.
# If not defined, "source_file" must be defined!
source_dir = ""

# Defines the XML file to operate.
# If not defined, "source_dir" must be defined!
input_file = ""

# Folders where the new XML files must be written.
# If not defined, the path is the same as the "source_dir" or "input_file".
destination_dir = ""


# Retrieve a list of all files in a folder.
def get_all_files_in_folder(a_dir, extension):
    list_files = list()
    for name in os.listdir(a_dir):
        if os.path.isdir(os.path.join(a_dir, name)):
            continue
        if len(extension):
            file_name, file_extension = os.path.splitext(name)
            if file_extension == extension:
                list_files.append(name)
        else:
            list_files.append(name)

    return list_files


def open_file(file_full_name, access_mode='r', encoding_mode='utf-8'):

    # Try to open the file.
    try:
        if encoding_mode != '':
            file = open(file_full_name, access_mode, encoding=encoding_mode)

        else:
            file = open(file_full_name, access_mode)

    except OSError as e:
        print(" -> Can't open the '" + file_full_name + "'file (error: " + e.strerror + " )")
        sys.exit(-1)

    return file


def execute_one_file(src_file_path_name, dst_file_path_name):
    print("Start parsing file '" + src_file_path_name + "'.")

    if not os.path.exists(src_file_path_name):
        print("'" + src_file_path_name + "' file doesn't exist.")
        sys.exit(-1)

    src_file = open_file(src_file_path_name, "r", "utf-16")

    # Try to create the destination file.
    dst_file = None
    try:
        dst_file = open(dst_file_path_name, "w", encoding='utf-16')
    except OSError as e:
        print(" -> Can't create the '" + dst_file_path_name + "'file (error: " + e.strerror + " )")
        sys.exit(-1)

    engine_state = 0
    line_number = 0
    previous_regex_result = None
    code_to_copy_regex_result = None
    translation_state_written = False
    global add_mode
    global language_code_to_copy

    for one_line in src_file:
        # Searching the first "string" tag.
        if 0 == engine_state:
            translation_state_written = False

            if one_line.find("<string>") != -1:
                engine_state = 1

            dst_file.write(one_line)

        # Scan all codes and take the translation from "language_code_to_copy" if "add_mode" = "copy".
        # Until we reach the first <xx_TS> tag.
        elif 1 == engine_state:
            regex_result = re.search(r'(.*)(<[\s"=:\w-]*>)(.*)(</[\w-]*>)', one_line)

            if not regex_result:
                print("Error at line " + str(line_number) + ": a tag is missing.")
                sys.exit(-1)

            current_tag = regex_result.expand("\\2")

            if current_tag.find("_TS") != -1 or current_tag.find("NTBTrans") != -1:
                # We reach the end of the first part of the string definition.
                # If we are with the add_mode "copy" and we have a translation available, we need to add it.
                if add_mode == "copy" and code_to_copy_regex_result is not None:
                    dst_file.write(code_to_copy_regex_result.expand("\\1"))
                    dst_file.write("<" + language_code_to_create + ">")
                    dst_file.write(code_to_copy_regex_result.expand("\\3"))
                    dst_file.write("</" + language_code_to_create + ">\n")
                    code_to_copy_regex_result = None

                engine_state = 2

            elif add_mode == "copy" and current_tag.find(language_code_to_copy) != -1:
                # We must copy the current translation that will be the same for the new one.
                # It has been the case for Slovak that is very similar to Czech.
                code_to_copy_regex_result = regex_result

            dst_file.write(one_line)
            previous_regex_result = regex_result

        # We are now on the second part of the string definition.
        # We need to parse the translation states to find the one from "language_code_to_copy" if "add_mode" = "copy".
        # Until we reach the </string> tag.
        elif 2 == engine_state:
            regex_result = re.search(r'(.*)(<[\w-]*>)(.*)(</[\w-]*>)', one_line)

            if not regex_result or regex_result.expand("\\2") == "<comment>":
                # We are probably at the end of the string definition.
                if not regex_result:
                    if one_line.find("</string>") == -1:
                        print("Error at line " + str(line_number) + ": a string definition must end with </string>.")
                        sys.exit(-1)
                    else:
                        # If we are on a </string> tag, we can reset the engine_state.
                        engine_state = 0

                # No need of "else". If we are on a <comment> tag, we don't change yet the engine state because
                # the </string> tag comes just after.
                # But if we are on a </string> or <comment> tag we do the following code for both cases.

                if translation_state_written is False:
                    if add_mode == "copy" and code_to_copy_regex_result is not None:
                        # We are with the add_mode "copy" and we need to copy the translation state of the
                        # "language_code_to_copy" code.
                        dst_file.write(code_to_copy_regex_result.expand("\\1"))
                        dst_file.write("<" + language_code_to_create + "_TS>")
                        dst_file.write(code_to_copy_regex_result.expand("\\3"))
                        dst_file.write("</" + language_code_to_create + "_TS>\n")
                        code_to_copy_regex_result = None
                    else:
                        # We are not with the add_mode "copy" or we don't found the translation state corresponding to
                        # the "language_code_to_copy". We simply force then "Untranslated.
                        dst_file.write(previous_regex_result.expand("\\1"))
                        dst_file.write("<" + language_code_to_create + "_TS>Untranslated</" + language_code_to_create
                                       + "_TS>\n")

                    translation_state_written = True

                dst_file.write(one_line)

            else:
                # We can have here either the code language tag or the "comment" tag.
                current_tag = regex_result.expand("\\2")

                if add_mode == "copy" and current_tag.find(language_code_to_copy) != -1:
                    # We must copy the current translation that will be the same for the new one.
                    # It has been the case for Slovak that is very similar to Czech.
                    code_to_copy_regex_result = regex_result

                dst_file.write(one_line)

            previous_regex_result = regex_result

        line_number = line_number + 1

    src_file.close()
    dst_file.close()
    print("Parse end!")


def execute():
    global source_dir
    global destination_dir

    # Check first if source path is good.
    if len(source_dir) > 0:
        if not os.path.exists(source_dir):
            print("The '" + source_dir + "' source path defined with the option 's' doesn't exist.")
            sys.exit(-1)

    # If destination path is not defined...
    if len(destination_dir) == 0:
        if len(source_dir) > 0:
            destination_dir = source_dir
        else:
            destination_dir = os.path.dirname(input_file)

    # Check now the destination folder.
    if not os.path.exists(destination_dir):
        print("The '" + destination_dir + "' destination path doesn't exist.")
        sys.exit(-1)

    if len(source_dir) > 0:
        list_files = get_all_files_in_folder(source_dir, ".xml")
        if len(list_files) == 0:
            print("No XML file in the '" + source_dir + "' folder.")
            sys.exit(-1)

        for one_file in list_files:
            filename, extension = os.path.splitext(one_file)
            execute_one_file(source_dir + one_file, destination_dir + "/"+ filename + "-NEW" + extension)
    else:
        path, file = os.path.split(input_file)
        filename, extension = os.path.splitext(file)
        execute_one_file(input_file, destination_dir + "/" + filename + "-NEW" + extension)


def print_language_codes():
    for code in language_code:
        if code != language_code[len(language_code) - 1]:
            print(code + ",")
        else:
            print(code + "\n")


def main():
    # Start by parsing command line.
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hl:m:c:s:f:d", ["help", "languagecode" "mode", "copycode", "srcdir",
                                                                "filepath", "destdir"])
    except getopt.GetoptError:
        print("AddLanguage.py -h -l <opt> -m <opt> -c <opt> -s <opt> -f <opt> -d <opt>")
        sys.exit(-1)

    if 0 == opts.__len__():
        print("AddLanguage.py -h -l <opt> -m <opt> -c <opt> -s <opt> -f <opt> -d <opt>")
        sys.exit(-1)

    global language_code_to_create
    global add_mode
    global language_code_to_copy
    global source_dir
    global input_file
    global destination_dir

    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print("AddLanguage.py -h -l <opt> -m <opt> -c <opt> -s <opt> -f <opt> -d <opt>")
            print(" -h --help : this help.")
            print(" -l <opt> --languagecode <opt> : ")
            print("       <opt> = \"language_code\" : language code to create.")
            print(" -m <opt> --mode <opt> : ")
            print("       <opt> = \"new\"  : set all to 'Untranslated.'")
            print("       <opt> = \"copy\" : copy translation from 'copycode'.")
            print(" -c <opt> --copycode <opt> : ")
            print("       <opt> = \"language_code\" : language code to copy when 'mode' is 'copy'.")
            print(" -s <opt> --srcdir <opt> : ")
            print("       <opt> = path where are source files.")
            print(" -f <opt> --filepath <opt> : ")
            print("       <opt> = path where is the source file.")
            print(" -d <opt> --destdir <opt> : ")
            print("       <opt> = path where to put destination files.")
            sys.exit()

        if opt in ("-l", "--languagecode"):
            language_code_to_create = arg

        if opt in ("-m", "--mode"):
            if "new" == arg or "copy" == arg:
                add_mode = arg
            else:
                print("'-c' option must be 'all' or 'one'.")
                sys.exit()

        if opt in ("-c", "--copycode"):
            language_code_to_copy = arg

        if opt in ("-s", "--srcdir"):
            source_dir = arg

        if opt in ("-f", "--filepath"):
            input_file = arg

        if opt in ("-d", "--destdir"):
            input_file = arg

    if len(language_code_to_create) == 0:
        print("Language to create can't be empty. Please define it with the '-l' option.")
        sys.exit()

    if add_mode == "copy":
        if len(language_code_to_copy) == 0:
            print("Language to copy can't be empty if '-m' is 'copy'. Please define it with the '-c' option.")
            sys.exit()

        if language_code_to_copy not in language_code:
            print(language_code_to_copy + " code doesn't exist. Here are the available codes:\n")
            print_language_codes()

    if len(source_dir) == 0 and len(input_file) == 0:
        print("One of these two options must be defined: '-s' or '-f'.")
        sys.exit()
    elif len(source_dir) > 0 and len(input_file) > 0:
        print("Only one of these two options must be defined: '-s' or '-f'.")
        sys.exit()

    execute()


main()
