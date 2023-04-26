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
import hashlib


def hash_file(fn):
    return hashlib.sha1(open(fn, 'rb').read()).hexdigest()


def get_dependencies(loading_binary):
    out = subprocess.check_output(['/usr/bin/otool', '-L', loading_binary])
    interesting_lines = (l.strip()
                         for l in out.splitlines() if l.startswith('\t'))
    binary_hash = hash_file(loading_binary)
    dependencies = []
    for line in interesting_lines:
        if '.framework' in line:  # We cannot handle frameworks
            continue
        if line.startswith('/usr/lib'):  # Do not mess with system libraries.
            continue
        if '@executable_path' in line:
            continue
        dependency = line[:line.find('(')].strip()
        dependencies.append(dependency)
    dep_dict = {}
    for dep in dependencies:
        file_name = dep.replace('@loader_path', p.dirname(loading_binary))
        dependency_hash = hash_file(file_name)
        if binary_hash == file_name:
            # This is a dylib and the first line is not a dependency, but the
            # ID of this dependency. We ignore it.
            continue
        dep_dict[dep] = os.path.realpath(file_name)
    return dep_dict


def change_id(binary):
    subprocess.check_call(['/usr/bin/install_name_tool', '-id',
                           '@executable_path/' + p.basename(binary), binary])


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
        dependencies = get_dependencies(b)
        for (dep_name, dep_path) in dependencies.items():
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
        if binary.endswith('.dylib'):
            change_id(binary)
        for (dep_name, dep_path) in all_dependencies.items():
            subprocess.check_call(['/usr/bin/install_name_tool', '-change',
                                   dep_name, '@executable_path/' + p.basename(dep_path), binary])


if __name__ == '__main__':
    main()
