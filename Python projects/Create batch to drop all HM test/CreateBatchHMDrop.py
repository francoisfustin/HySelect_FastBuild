import os
import sys


# Get all direct subdirectories.
def get_all_sub_folders(a_dir):
    return [os.path.join(a_dir, name) for name in os.listdir(a_dir)
            if os.path.isdir(os.path.join(a_dir, name))]


# Get all ".tsp" in current directory.
def get_all_files_in_folder(a_dir):
    return [os.path.join(a_dir, name) for name in os.listdir(a_dir)
            if not os.path.isdir(os.path.join(a_dir, name)) and name.find(".tsp") != -1]


def create_one_output_file(file_name):
    try:
        output_file = open(file_name, 'w', encoding='utf-8')
    except OSError as e:
        print(" -> Can't create the '" + file_name + "'file (error: " + e.strerror + ")")
        sys.exit(-1)

    return output_file


all_tsp_sub_folders = get_all_sub_folders(".")
if len(all_tsp_sub_folders) == 0:
    print("No sub folder found in the current folder.")
    sys.exit(-1)

all_drop_files = []
for one_tsp_sub_folder in all_tsp_sub_folders:
    print("Run the sub folder: " + one_tsp_sub_folder)
    all_tsp_files = get_all_files_in_folder(one_tsp_sub_folder)

    if len(all_tsp_files) > 0:
        folder_name = one_tsp_sub_folder.split("\\").pop()
        output_file_name = one_tsp_sub_folder + "\\DropAll" + folder_name + ".txt"
        print("Create output file: " + output_file_name)
        output_file = create_one_output_file(output_file_name)
        all_drop_files.append(output_file_name)

        for one_tsp_file in all_tsp_files:
            one_tsp_file_name = one_tsp_file.split("\\").pop()
            print("\t" + one_tsp_file)
            output_file.write("OPENTSPFILE;" + one_tsp_file_name + "\n")
            tsp_file_name = one_tsp_file_name.split(".").pop(0)
            output_file.write("DROPHMCONTENT;" + tsp_file_name + ".txt;CREATE\n")

        output_file.close()

output_file = create_one_output_file("DropAll.txt")
for one_drop_file in all_drop_files:
    output_file.write("ADDBATCHFILE;" + one_drop_file + "\n")

output_file.close()
