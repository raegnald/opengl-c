#! /usr/bin/env python3

import sys
import os

if len(sys.argv) < 2:
    sys.stderr.write("No file provided\n")
    exit(1)

shader_file = sys.argv[1]
output_file = shader_file + '.inc'

with open(shader_file, 'r') as inp:
    with open(output_file, 'w') as out:
        for line in inp:
            out.write(f'"{line[:-1]}\\n"\n')
