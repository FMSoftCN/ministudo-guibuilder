#!/usr/bin/python
#Filename: cfg2c.py

import string, sys

if sys.version_info < (3, 0):
    import ConfigParser
else:
    import configparser

def printHeader(fp, c_name):
    fp.write("/*\n"
         " * Filename: %s\n" 
         " * This file is generated automatically by tools, please don't edit.\n"
         " * \n"
         " *    Copyright (C) 2009 Feynman Software\n"
         " *    All rights reserved by Feynman Software.\n"
         " */\n\n\n"
         "#include <stdio.h>\n"
         "#include <stdlib.h>\n"
         "#include <string.h>\n"
         "#include <minigui/common.h>\n"
         "#include <minigui/minigui.h>\n"
         "#include <minigui/gdi.h>\n"
         "#include <minigui/window.h>\n\n\n" % c_name)

def printSection(fp, cf):
    section = {} #dict

    for sect in cf.sections():
        size = 0
        prefix = sect.replace('.', '_');
        keyname = '_%s_keys' % prefix
        valuename = '_%s_values' % prefix

        fp.write('static char* %s[] = {\n' % keyname);
        for key, value in cf.items(sect):
            fp.write('     \"%s\",\n' % key);
            size += 1
        fp.write('};\n\n');

        fp.write('static char* %s[] = {\n' % valuename);
        for key, value in cf.items(sect):
            fp.write('     \"%s\",\n' % value);
        fp.write('};\n\n');

        sect_info = (size, keyname, valuename)
        section[sect] = sect_info;

    fp.write("static ETCSECTION ncs_etc_section[] = {\n")
    for key, info in section.items():
        fp.write('    { 0, %s,\n'
              '      \"%s\",\n'
              '      %s,\n'
              '      %s },\n\n' 
              % (info[0], key, info[1], info[2]))
    fp.write('};\n\n');

    fp.write("static ETC_S ncs_etc = {\n"
                 "    0,\n"
                 "    sizeof(ncs_etc_section)/sizeof(ETCSECTION),\n"
                 "    ncs_etc_section\n"
                 "};\n\n\n"
                 "GHANDLE ncsGetEtcCfg(void)\n"
                 "{\n"
                 "    return (GHANDLE)&ncs_etc;\n"
                 "}\n")

def help():
    help_txt = '''
This program convert configuration file to c file.
Usage:
    ./cfg2c [file.cfg] [dest.c]
Options include:
    --version : Prints the version number
    --help    : Display this help'''

    print('%s' % help_txt)

def parse_argument():
    global cfg_name
    global c_name
    if len(sys.argv) < 2:
        help()
        sys.exit()

    if sys.argv[1].startswith('--'):
        option = sys.argv[1][2:]
        if option == 'version':
            print('Version 1.0')
        elif option == 'help':
            help()
        sys.exit()
    else:
        if len(sys.argv) < 3:
            help()
            sys.exit()

        cfg_name = sys.argv[1]
        if cfg_name.endswith('.cfg'):
            c_name = sys.argv[2]
        else:
            help()
            sys.exit()
        
def cfg2c():
    parse_argument()

    if sys.version_info < (3, 0):
        cf = ConfigParser.ConfigParser()
    else:
        cf = configparser.ConfigParser()

    cf.read(cfg_name)
    fp = open(c_name, 'w+')
    printHeader(fp, c_name)
    printSection(fp, cf)
    fp.close()

cfg2c()
