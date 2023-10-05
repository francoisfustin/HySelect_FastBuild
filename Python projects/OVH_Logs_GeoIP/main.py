import os
import sys
import re
from geoip import geolite2

path_file = "c:/Temp/HySelect/Statistics/"


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


def main():
    list_files = get_all_files_in_folder(path_file, ".txt")
    if len(list_files) == 0:
        print("No file in the '" + path_file + "' folder.")
        sys.exit(-1)

    ip_statistics = dict()
    ip_already_verified = []
    ip_not_found = []

    for one_file in list_files:
        print("Working on file: " + one_file + "...\n")
        file = open_file(path_file+one_file)

        line_count = 0
        for one_line in file:
            result = re.search(r'(^\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})', one_line)
            if not result:
                print("Error in line: " + str(line_count) + " in the file: " + one_file + "\n")
                print("No IP found.\n")
                sys.exit(-1)

            ip_address = result.expand("\\1")

            if ip_already_verified.count(ip_address) == 0:
                ip_already_verified.append(ip_address)
                match = geolite2.lookup(ip_address)
                if match is None:
                    if ip_not_found.count(ip_address) == 0:
                        ip_not_found.append(ip_address)
                else:
                    if match.country in ip_statistics:
                        if ip_address in ip_statistics[match.country]:
                            ip_statistics[match.country][ip_address] += 1
                        else:
                            ip_statistics[match.country].update({ip_address: 1})
                    else:
                        ip_statistics[match.country] = {ip_address: 1}

        file.close()

    output_file = open_file("result.txt", "w")
    for country, ip_addresses in ip_statistics.items():
        if country is not None:
            output_file.write("Country: " + country + " [" + str(len(ip_addresses)) + "]\n")
        else:
            output_file.write("Country: unknown\n")

        for ip_address, ip_count in ip_addresses.items():
            output_file.write("\t" + ip_address + ": " + str(ip_count) + "\n")

        output_file.write("\n")

    file.close


main()
