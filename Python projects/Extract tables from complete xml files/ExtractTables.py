# This little python program allows us to export all tables and IDS.
# Each IDS for a table are exported in a separate file with the table name.

import fileinput
import re

def Process( line, TableFile ):
    #global TableFile
    result = re.search(r'<Table name="(.+)"\s{0,1}TAScope', line )
    
    if result:
        # Create a file with the table name.
        table_name = result.expand("\\1")
        TableFile = open( table_name, 'w' )
        return TableFile
    
    result = re.search(r'</Table>', line )
    
    if result:
        # Close the file.
        TableFile.close()
        return TableFile
    
    result = re.search(r'<IDS>(.+)</IDS>', line )
    
    if result:
        string_name = result.expand("\\1\n")
        TableFile.write( string_name )
        return TableFile
    
    return TableFile

with open('TadbST-compl.xml', 'r', encoding="utf_16") as f:
    TableFile = 0
    for line in f:
        TableFile = Process( line, TableFile )
