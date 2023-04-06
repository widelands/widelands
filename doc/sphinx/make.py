#!/usr/bin/env python
# encoding: utf-8

import sys
import subprocess
import shutil

DOT_AVAILABLE = True

builder_help = {
    'html': 'to make standalone HTML files (preferred)',
    'dirhtml': 'to make HTML files named index.html in directories',
    'singlehtml': 'to make a single large index.html file',
}


def general_help():
    print('Create the source code documentation.')
    print('Dependencies: sphinxdoc, for class graphs: graphviz\n')
    usage()


def usage():
    print('Usage: ./make.py <builder> [-graphs] where <builder> is one of this builders:\n')
    for b, h in builder_help.items():
        print("  {} {}{}".format(b, '\t\t' if len(b) <= 4 else '\t', h))
    print('\nThe optional argument -graphs applies class dependency graphs to html files.')
    sys.exit()


def extract_rst(graphs, builder):
    if graphs and DOT_AVAILABLE:
        print('Adding class graphs to html files.')
        subprocess.run(['python', 'extract_rst.py', '-graphs', builder])
    else:
        subprocess.run(['python', 'extract_rst.py'])


def make_doc(**kwargs):
    print('Creating the source code documentation as {}.'.format(
        kwargs['builder'])
    )

    subprocess.run(['sphinx-build',
                    '-b', kwargs['builder'],
                    '-d', 'build/doctrees',
                    'source',
                    'build/{}'.format(kwargs['builder']),
                    ])


def main():
    opts = sys.argv[1:]
    graphs = False
    build_args = {}
    if not opts:
        general_help()

    for o in opts:
        if o in ['-h', '--help']:
            general_help()
        elif o == '-graphs':
            graphs = True
        elif o in builder_help:
            build_args['builder'] = o
        else:
            print('Unrecognized option... "{}"'.format(o))
            usage()

    if build_args.get('builder', '') != '':
        extract_rst(graphs, build_args['builder'])
        make_doc(**build_args)
    else:
        print('Please choose a builder...')
        usage()


if __name__ == '__main__':
    if not shutil.which('sphinx-build'):
        print('Please install sphinxdoc.')
        sys.exit(1)
    if not shutil.which('dot'):
        print('Program dot not found, creating class graphs not supported.')
        print('If you want class graphs to be supported install graphviz.')
        DOT_AVAILABLE = False
    main()
