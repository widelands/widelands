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


class LuaClass:

    def __init__(self, name, outfile, parent=None):
        self.name = name
        self.outfile = outfile
        self.parent = parent


    def get_prefixed_name(self):
        """Returns a string assembled form outfile and classname.

        E.g. if outfile = autogen_wl_map.rst and classname = MapObjectDescription
        the returned value is 'wl.map.MapObjectDescription'.
        """
        prefix = '.'.join(self.outfile.rpartition('.')[0].split('_')[1:])
        return '{}.{}'.format(prefix, self.name)


    def print_data(self):
        print('  ', self.name, "|", self.parent,'|', self.outfile, "|",
              self.get_prefixed_name())
# End of LuaClass

class LuaClasses:
    """Handles lists of base and derived classes."""

    def __init__(self):
        self.bases = []
        self.derived = []


    def add_class(self, name, outfile, is_base=False, parent=None):
        if not is_base and not parent:
            raise("Error: Derived class needs a parent class!")

        if is_base:
            self.bases.append(LuaClass(name, outfile))
        else:
            self.derived.append(LuaClass(name, outfile, parent=parent))


    def get_bases(self):
        """Returns a list of base class names."""
        return [c.name for c in self.bases]


    def get_name_parent(self):
        """Returns a list of all (name, parent)."""
        return [(c.name, c.parent) for c in self.derived]


    def get_parent(self, cls):
        """Returns the parent of cls."""
        for name, parent in self.get_name_parent():
            if name == cls:
                return parent
        return None


    def get_parent_tree(self, cls, tree=None):
        """Recursively find all parents of cls up to the base class."""
        if tree is None:
            tree = []
        if cls in self.get_bases():
            tree.append(cls)
            return tree
        p = self.get_parent(cls)
        if p:
            tree.append(cls)
            self.get_parent_tree(p, tree)
        return tree


    def get_children(self, cls):
        """Returns the children of cls."""
        children = []
        for name, parent in self.get_name_parent():
            if parent == cls:
                children.append(name)
        return children


    def get_children_tree(self, cls, max_children=0, tree=None):
        """Recursively find all children of cls."""
        if tree is None:
            tree = {cls: []} #tree = []
        children = self.get_children(cls)
        if not children or max_children == MAX_CHILDS:
            return tree
        else:
            max_children += 1
            tree[cls] = children
            for c in children:
                self.get_children_tree(c, max_children, tree)
        return tree


    def get_instance(self, cls):
        for c in self.bases:
            if c.name == cls:
                return c
        for c in self.derived:
            if c.name == cls:
                return c
        raise("Error: There is no class named:", cls)


    def format_cls(self, cls_to_format, cls_to_compare):
        """Returns a formatted name of cls_to_format if 
        cls_to_compare is in a different file.
        """
        inst1 = self.get_instance(cls_to_format)
        inst2 = self.get_instance(cls_to_compare)
        if inst1.outfile != inst2.outfile:
            return inst1.get_prefixed_name()
        return cls_to_format


    def print_classes(self):
        print("Main classes:")
        for c in self.bases:
            c.print_data()
        print("Derived classes:")
        for c in self.derived:
            c.print_data()


classes = LuaClasses()


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
            # The variables refer to a definition:
            # cls_name : public namespace_or_parent::parent_cls

            # Strip out leading 'Lua'. This can't be done in the regexp
            # because we need 'LunaClass' to get the parent class of all classes.
            cls_name = cls_name.replace('Lua', '')
            namespace_or_parent = namespace_or_parent.replace('Lua', '')
            parent_cls = parent_cls.replace('Lua', '')

            if cls_name in EXCLUDE_CLASSES:
                continue

            # Find main lua class (module name)
            if namespace_or_parent == 'LunaClass':
                mod_name = cls_name
                continue

            # Feed data models
            if namespace_or_parent == mod_name:
                classes.add_class(cls_name, outfile, is_base=True)
            else:
                if parent_cls:
                    # ... : public ...::parent_cls
                    classes.add_class(cls_name, outfile, parent=parent_cls)
                else:
                    # ... : public namespace_or_parent
                    classes.add_class(cls_name, outfile, parent=namespace_or_parent)


def init(base_dir, cpp_files):
    for cpp_file, outfile in cpp_files:
        header = cpp_file.rpartition('.')[0] + '.h'
        h_path = os.path.join(base_dir, header)
        fill_data(h_path, outfile)
    classes.print_classes()


def add_child_of(rst_data, outfile):
    """Adds the String 'Child of: …' to rst_data."""

    found_classes = RSTDATA_CLS_RE.findall(rst_data)
    for cls in found_classes:
        parents = classes.get_parent_tree(cls)[1:]
        if parents:
            child_str = '   Child of:'
            for i, parent in enumerate(parents):
                parent = classes.format_cls(parent, cls)
                child_str += ' :class:`{}`'.format(parent)
                if i < len(parents) - 1:
                    # add separator except after last entry
                    child_str += ', '

            repl_str = '.. class:: {}\n\n'.format(cls)
            child_str = '{}{}\n\n'.format(repl_str, child_str)
            rst_data = rst_data.replace(repl_str, child_str)

    return rst_data


def add_dependency_graph(rst_data, outfile):
    found_cls = RSTDATA_CLS_RE.findall(rst_data)
    for cls in found_cls:
        children = classes.get_children_tree(cls)
        print(children)
