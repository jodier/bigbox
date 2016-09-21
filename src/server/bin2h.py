#!/usr/bin/env python

#############################################################################

from argparse import ArgumentParser

#############################################################################

__version__ = "1.0"

#############################################################################

def main():
    #########################################################################

    parser = ArgumentParser(
        description = "Binary to Header converter",
        epilog = "Copyright (C) 2016 Jerome ODIER <odier@hypnos-solutions.com>",
    )

    #########################################################################

    parser.add_argument("--version", action = "version", version = "%(prog)s " + __version__)
    parser.add_argument("file", help = "file to convert")
    parser.add_argument("id", help = "variable to use")

    args = parser.parse_args()

    #########################################################################

    with open(args.file, "rb") as fd:
        data_in = bytearray(fd.read())

    #########################################################################

    data_len = len(data_in)
    data_out = (((("\n"))))

    for part in range(0, data_len, 8):

        data_out += '\t' + ', '.join(["0x%02X" % b for b in data_in[part: part + 8]]) + ',\n'

    #########################################################################

    print("#include <stdint.h>\n")

    print("/* file: %s */\n" % args.file)

    print("#define %s_SIZE %d\n" % (args.id.upper(), data_len))

    print("uint8_t %s_buff[] = {%s};" % (args.id.lower(), data_out))

#############################################################################

if __name__ == "__main__":
    main()

#############################################################################
