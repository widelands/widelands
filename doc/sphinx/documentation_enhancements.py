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

    def get_graphviz_link(self):
        html_link = 'href="../{}#{}", target="_parent"'.format(
            self.outfile.replace('.rst', '.html'), self.name.lower())
        return html_link


    def print_data(self):
        print('  ', self.name, '|', self.parent, '|', self.outfile, '|',
              self.get_prefixed_name())
# End of LuaClass


class LuaClasses:
    """Stores all base and derived classes as LuaClass objects.

    This class contains also the methods for handling base and derived classes.
    """

    def __init__(self):
        self.bases = []
        self.derived = []

    def add_class(self, name, outfile, is_base=False, parent=None):
        if not is_base and not parent:
            raise Exception('Error: Derived class needs a parent class!')

        if is_base:
            self.bases.append(LuaClass(name, outfile))
        else:
            self.derived.append(LuaClass(name, outfile, parent=parent))

    def get_instance(self, cls_name):
        for c in self.bases:
            if c.name == cls_name:
                return c
        for c in self.derived:
            if c.name == cls_name:
                return c
        raise Exception('Error: There is no class named:', cls_name)

    def get_base_names(self):
        """Returns a list of base class names."""
        return [c.name for c in self.bases]

    def get_parent_tree(self, cli, tree=None):
        """Recursively find all parent instances of cli up to the base class."""
        if not isinstance(cli, LuaClass):
            raise Exception("Class must be an instance of LuaClass but got:", type(cli))

        if tree is None:
            tree = []
        if cli.name in self.get_base_names():
            return tree
        parent = classes.get_instance(cli.parent)
        if parent:
            tree.append(parent)
            self.get_parent_tree(parent, tree)
        return tree

    def get_name_parent(self):
        """Returns a list of (name, parent) pairs from all derived classes."""
        return [(c.name, c.parent) for c in self.derived]

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
            tree = {cls: []}
        children = self.get_children(cls)
        if not children or max_children == MAX_CHILDS:
            return tree
        else:
            max_children += 1
            tree[cls] = children
            for c in children:
                self.get_children_tree(c, max_children, tree)
        return tree

    def format_cls(self, cli_to_format, cli_to_compare):
        """Returns a formatted name of cli_to_format if 
        cli_to_compare is in a different file.
        """
        if cli_to_format.outfile != cli_to_compare.outfile:
            return cli_to_format.get_prefixed_name()
        return cli_to_format.name

    def print_classes(self):
        print("Base classes:")
        for c in self.bases:
            c.print_data()
        print("Derived classes:")
        for c in self.derived:
            c.print_data()
# End of LuaClasses


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


def add_child_of(rst_data):
    """Adds the String 'Child of: …' to rst_data."""

    found_classes = RSTDATA_CLS_RE.findall(rst_data)
    for cls in found_classes:
        cli = classes.get_instance(cls)
        parents = classes.get_parent_tree(cli)
        if parents:
            repl_str = '.. class:: {}\n\n'.format(cli.name)
            child_str = '{}   Child of:'.format(repl_str)
            for i, parent_inst in enumerate(parents):
                parent_name = classes.format_cls(parent_inst, cli)
                child_str += ' :class:`{}`'.format(parent_name)
                if i < len(parents) - 1:
                    # add separator except after last entry
                    child_str += ','

            child_str += '\n\n'
            rst_data = rst_data.replace(repl_str, child_str)

    return rst_data

def format_graphviz_parents(cur_cls):

    def _make_tooltip():
        via_list = 'Via: '
        for i, c in enumerate(parents):
            via_list += c.name
            if i < len(parents)-1:
                via_list += ' → '
        return via_list

    cli = classes.get_instance(cur_cls)
    parents = list(reversed(classes.get_parent_tree(cli)))
    ret_str = ''
    base_name = ''
    base_link = ''
    if parents:
        base = parents.pop(0)
        base_link = base.get_graphviz_link()
        base_name = base.name
        if parents:
            ret_str += '{base} -- {cur} [style=tapered, arrowhead=none, arrowtail=none dir=both,\
penwidth=15, edgetooltip="{tooltip}"]\n'.format(base=base_name,
                                                cur=cur_cls,
                                                tooltip=_make_tooltip()
                                              )
        else:
            # Only one parent -> no big edge
            ret_str = '{base} -- {cur}'.format(base=base_name,
                                               cur=cur_cls
                                               )
    return base_name, base_link, ret_str


def create_directive(cls):
    children = (classes.get_children_tree(cls))
    base_cls, base_link, parents = format_graphviz_parents(cls)
    graph_directive = None
    if parents:# or children:
        graph_directive = """
.. graphviz::
    :alt: Dependency graph for class: {cur_cls}

    graph {cur_cls} {{


    bgcolor="transparent"
    node [shape=box, style=filled, fillcolor=white,
          fontsize=12, fontname="Helvetica", margin="0.05, 0.0"]
    edge [color=gray]
    {cur_cls} [fillcolor=green, fontcolor=white, fontsize=13, shape=oval]
    {base_cls} [shape=house, {link}]
    {parents}
    {child_list}
    }}\n""".format(cur_cls=cls,
                   base_cls=base_cls,
                   parents=parents,
                   link=base_link,
                   child_list='' #children,
                   )
    print(graph_directive)
    return graph_directive


def add_dependency_graph(rst_data):
    found_cls = RSTDATA_CLS_RE.findall(rst_data)
    for cls in found_cls:
        cli = classes.get_instance(cls)
        directive = create_directive(cls)
        if directive:
            repl_str = '.. class:: {}\n\n'.format(cli.name)
            directive_str = '{}\n{}'.format(directive, repl_str)
            rst_data = rst_data.replace(repl_str, directive_str)

    return rst_data
