#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This tool was written out of frustration with
https://github.com/auriamg/macdylibbundler not doing the job right.

Unfortunately the above tool is not maintained, so we roll our own.

"""

import argparse
import os.path as p
import os
import shutil
import subprocess


def get_dependencies(loading_binary):
    out = subprocess.check_output(['/usr/bin/otool', '-L', loading_binary])
    interesting_lines = (l.strip()
                         for l in out.splitlines() if l.startswith('\t'))
    dependencies = []
    for line in interesting_lines:
        if '.framework' in line:  # We cannot handle frameworks
            continue
        if line.startswith('/usr/lib'):  # Do not mess with system libraries.
            continue
        if '@executable_path' in line:
            continue
        dependencies.append(line[:line.find('(')].strip())
    dep_dict = {}
    for dep in dependencies:
        file_name = dep.replace('@loader_path', p.dirname(loading_binary))
        dep_dict[dep] = os.path.realpath(file_name)
    return dep_dict


def fix_dependencies(top_level_binary):
    top_level_binary = p.realpath(top_level_binary)
    todo = {(top_level_binary, top_level_binary)}
    done = set()
    path = p.dirname(top_level_binary)
    while todo:
        (binary_in_path, binary_in_system) = todo.pop()
        print('Fixing %s' % binary_in_path)
        done.add(binary_in_path)
        dependencies = get_dependencies(binary_in_system)
        for (dep_name, dep_file) in dependencies.items():
            in_directory = p.join(path, p.basename(dep_file))
            todo.add((in_directory, dep_file))
            shutil.copy(dep_file, in_directory)
            os.chmod(in_directory, 0644)
            subprocess.check_call(['/usr/bin/install_name_tool', '-change',
                                   dep_name, '@executable_path/' + p.basename(dep_file), binary_in_path])


def parse_args():
    p = argparse.ArgumentParser(description='Fix dependencies in executables or libraries.'
                                )
    p.add_argument('binary', type=str, nargs='+', help='The binaries to fix.')

    return p.parse_args()


def main():
    args = parse_args()

    output_dir = p.realpath(p.dirname(args.binary[0]))
    binaries = set(args.binary)
    done = set()

    all_dependencies = {}
    while binaries:
        b = binaries.pop()
        if b in done:
            continue
        done.add(b)
        for (dep_name, dep_path) in get_dependencies(b).items():
            if dep_name in all_dependencies and all_dependencies[dep_name] != dep_path:
                raise RuntimeError('{} was already seen with a different path: {} != {}' % (
                    dep_name, dep_path, all_dependencies[dep_name]))
            all_dependencies[dep_name] = dep_path
            binaries.add(dep_path)

    to_fix = set(args.binary)
    for (dep_name, dep_path) in all_dependencies.items():
        in_directory = p.join(output_dir, p.basename(dep_path))
        shutil.copy(dep_path, in_directory)
        print('Copying %s to %s' % (dep_path, in_directory))
        to_fix.add(in_directory)
        os.chmod(in_directory, 0644)

    for binary in to_fix:
        print('Fixing binary: %s' % binary)
        for (dep_name, dep_path) in all_dependencies.items():
            subprocess.check_call(['/usr/bin/install_name_tool', '-change',
                                   dep_name, '@executable_path/' + p.basename(dep_path), binary])

if __name__ == '__main__':
    main()
