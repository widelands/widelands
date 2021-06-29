#!/usr/bin/env python
# encoding: utf-8

import re
import os

HFILE_CLS_RE = re.compile(r'^class\s(\w+)\s+:\s+\w+\s+(\w+)[::]*(\w*)', re.M)
RSTDATA_CLS_RE = re.compile(r'.. class:: (\w+)')

MAX_CHILDS = 2
MAX_PARENTS = 1#MAX_CHILDS

EXCLUDE_CLASSES = ['Market',
                   'MarketDescription',
                   ]

classes = {'Main': [], 'Derived': []}


def prefix_cls(cls, outfile):
    prefix = '.'.join(outfile.rpartition('.')[0].split('_')[1:])
    return '{}.{}'.format(prefix, cls)


def cls_name_from_prefixed(cls):
    return cls.rpartition('.')[-1]


def fill_data(file_name, outfile):
    """ Find class names in given File.
    Reads out each occurrence of "class x : public y::z" and fills
    main_classes and derived_classes. The values have to be cleaned
    which is made in two steps:
    - Strip leading 'Lua' from the name of classes.
      In contrast to classes in .h-files the Lua-classes have no leading
      string 'Lua'. Future implementations have to be exactly like this!
    -  The class derived from 'LunaClass'' is the base class for all other
      classes in this file. This class has no meaning for the Lua
      documentation, so we do not store it's name.
    """

    found_cls = False
    with open(file_name, 'r') as f:
        found_cls = HFILE_CLS_RE.findall(f.read())

    if found_cls:
        mod_name = ''
        for cls_name, namespace_or_parent, parent_cls in found_cls:
            # the variables refer to definition:
            # cls_name : public namespace_or_parent::parent_cls

            # strip out leading 'Lua'; this can't be done in the regexp
            # because we need 'LunaClass' to get the parent class of all classes
            cls_name = cls_name.replace('Lua', '')
            namespace_or_parent = namespace_or_parent.replace('Lua', '')
            parent_cls = parent_cls.replace('Lua', '')

            if cls_name in EXCLUDE_CLASSES:
                continue

            # find main lua class (module name)
            if namespace_or_parent == 'LunaClass':
                mod_name = cls_name
                continue

            # feed data models
            #cls_name = prefix_cls(outfile, cls_name)
            if namespace_or_parent == mod_name:
                classes['Main'].append([cls_name, outfile])
            else:
                if parent_cls:
                    # ... : public namespace_or_parent::parent_cls
                    #parent_cls = prefix_cls(outfile, parent_cls)
                    classes['Derived'].append([cls_name, parent_cls, outfile])
                else:
                    # ... : public namespace_or_parent
                    #namespace_or_parent = prefix_cls(outfile, namespace_or_parent)
                    classes['Derived'].append([cls_name, namespace_or_parent, outfile])


def init(base_dir, cpp_files):
    for cpp_file, outfile in cpp_files:
        header = cpp_file.rpartition('.')[0] + '.h'
        h_path = os.path.join(base_dir, header)
        fill_data(h_path, outfile)


def get_short_or_long(cls1, cls2):
    """Check if cls1 and cls2 are in different files."""
    cls1_outf = None
    cls2_outf = None
    for c,f in classes['Main']:
        if c == cls1:
            cls1_outf = f
        if c == cls2:
            cls2_outf = f
    if not cls1_outf or not cls2_outf:
        for c, p, f in classes['Derived']:
            if c == cls1:
                cls1_outf = f
            if c == cls2:
                cls2_outf = f
    if cls1_outf != cls2_outf:
        return True
    return False

def get_mainclass_names():
    """Returns a list of main class names."""
    return [c for c, f in classes['Main']]


def get_parent(cls):
    for cl, parent, f in classes['Derived']:
        if cls == cl:
            return parent
    return None


def get_parent_tree(cls, tree=None):
    """Recursively find all parents of cls."""
    if tree is None:
        tree = []
    if cls in get_mainclass_names():
        tree.append(cls)
        return tree
    a = get_parent(cls)
    if a:
        tree.append(cls)
        get_parent_tree(a, tree)
    return tree


def add_child_of(rst_data, outfile):
    """Adds the String 'Child of: …' to rst_data."""
    found_classes = RSTDATA_CLS_RE.findall(rst_data)
    for cls in found_classes:
        child_str = '   Child of (inserted):'
        parents = get_parent_tree(cls)[1:]
        if parents:
            child_str = '   Child of (inserted): '
            for i, parent in enumerate(parents):
                # using :any:`xy` to make linking between files work
                child_str += ' :any:`{a}`'.format(child_str, a=parent)
                if i < len(parents) - 1:
                    # add comma except after last entry
                    child_str += ', '
            repl_str = '.. class:: {}\n'.format(cls)
            rst_data = rst_data.replace(repl_str, child_str)
    return rst_data


def add_dep_graph():
    pass


def debug_classes_dict():
    print('Main classes: ([classname , outfile], assemled class name)')
    for data in classes['Main']:
        print('  ',data, prefix_cls(data[0], data[1]))
    print('Derived classes: ([derived class, parent, outfile], assembled class name)')
    for data in classes['Derived']:
        print('  ', data, prefix_cls(data[0], data[2]))
