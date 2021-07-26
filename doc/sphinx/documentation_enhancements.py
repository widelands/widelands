#!/usr/bin/env python
# encoding: utf-8

import re
import os

HFILE_CLS_RE = re.compile(r'^class\s(\w+)\s+:\s+\w+\s+(\w+)[::]*(\w*)', re.M)
RSTDATA_CLS_RE = re.compile(r'.. class:: (\w+)')

MAX_CHILDS = 2
MAX_PARENTS = 2
MAX_NAME_LENGTH = 15

EXCLUDE_CLASSES = ['Market',
                   'MarketDescription',
                   ]


class LuaClass:

    def __init__(self, name, outfile, parent=None, is_base=False):

        def _prefix_name():
            prefix = '.'.join(outfile.rpartition('.')[0].split('_')[1:])
            return '{}.{}'.format(prefix, name)

        self.long_name = _prefix_name()
        self.name = name
        self.outfile = outfile
        self.parent = parent
        self.is_base = is_base
        self.children = []

    def get_parent_name(self):
        found = False
        if self.children:
            for c in self.children:
                if self.outfile != c.outfile:
                    found = True
        if found:
            return self.long_name
        return self.name

    def get_child_name(self):
        if self.parent:
            if self.parent.outfile != self.outfile:
                return self.long_name
        return self.name

    def get_graphviz_link(self):
        html_link = 'href="../{}#{}", target="_parent"'.format(
            self.outfile.replace('.rst', '.html'), self.name.lower())
        return html_link


    def print_data(self):
        parent = ''
        if self.parent:
            parent = self.parent.long_name

        print('long_name:', self.long_name,
              '\n  Name:', self.name,
              '\n  Parent:', parent, "type:", type(self.parent),
              '\n  Outfile:', self.outfile, 
              '\n  is_base:', self.is_base,
              '\n  children:', [x.long_name for x in self.children])
# End of LuaClass


class LuaClasses:
    """Stores all base and derived classes as LuaClass objects.

    This class contains also the methods for handling base and derived classes.
    """

    def __init__(self):
        self.bases = []
        self.derived = []
        self.all_cls = []

    def add_class(self, name, outfile, parent=None, is_base=False):
        if not is_base and not parent:
            raise Exception('Error: Derived class needs a parent class!')

        new_cls = LuaClass(name, outfile, parent, is_base)
        self.all_cls.append(new_cls)

        if is_base:
            # Append base class
            self.bases.append(LuaClass(name, outfile))
        else:
            # Append child class
            self.derived.append(LuaClass(name, outfile, parent=parent))

    def create_inherintances(self):
        for c in self.all_cls:
            if c.parent:
                parent = self.get_instance_new(c.parent)
                c.parent = parent
                c.parent.children.append(c)

    def get_instance_new(self, cls_name):
        c_list = []
        for c in self.all_cls:
            if c.name == cls_name:
                c_list.append(c)
        if len(c_list) == 0:
            raise Exception('Error: There is no class named:', cls_name)
        elif len(c_list) > 1:
            raise Exception('Error: Two classes of same name', cls_name)
        return c_list[0]


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
        return [c.name for c in self.all_cls if c.is_base]

    def get_parents(self, cls, tree=None):
        if tree == None:
            tree = []
        for c in self.all_cls:
            if c.name == cls:
                if c.parent:
                    if not c.parent in tree:
                        # This is only needed for double defined class Player.
                        tree.append(c.parent)
                    self.get_parents(c.parent.name, tree)
                else:
                    # No parent anymore, end of recursion
                    return tree
        return tree

    def get_children(self, cls, tree=None):
        if tree == None:
            tree = []
        for c in self.all_cls:
            if c.name == cls:
                if c.children:
                    l=[]
                    l.append(c)
                    l.append([x for x in c.children])
                    tree.append(l)
                    for child in c.children:
                        self.get_children(child.name, tree)
                else:
                    return tree
        return tree

    def print_classes(self):
        print('all classes:')
        for c in self.all_cls:
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
    # Read all class definitions
    for cpp_file, outfile in cpp_files:
        header = cpp_file.rpartition('.')[0] + '.h'
        h_path = os.path.join(base_dir, header)
        fill_data(h_path, outfile)
    # Apply inheritances. This can only be done after all classes are created because
    # some children are in different files.
    classes.create_inherintances()
    classes.print_classes()


def add_child_of(rst_data):
    """Adds the String 'Child of: …' to rst_data."""

    found_classes = RSTDATA_CLS_RE.findall(rst_data)
    for cls in found_classes:
        parents = classes.get_parents(cls)
        if parents:
            repl_str = '.. class:: {}\n\n'.format(cls)
            child_str = '{}   Child of:'.format(repl_str)
            for i, parent in enumerate(parents):
                child_str += ' :class:`{}`'.format(parent.get_parent_name())
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

    parents = list(reversed(classes.get_parents(cur_cls)))
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

def format_graphviz_children(cls):
    children = classes.get_children(cls)
    ret_str = ''
    last_row = 0

    if children:
        for child in children:
            last_row += 1
            ret_str += '{} -- {{'.format(child[0].name)
            for i, grandchild in enumerate(child[1]):
                ret_str += '"{name}"[{link}]'.format(
                    name=grandchild.get_child_name(),
                    link=grandchild.get_graphviz_link(),
                    )
                if i < len(child[1]) - 1:
                    # add space except after last entry
                    ret_str += ' '
            ret_str += '}\n    '
            if last_row >= MAX_CHILDS:
                break

    return ret_str


def create_directive(cls):
    children = format_graphviz_children(cls)
    base_cls, base_link, parents = format_graphviz_parents(cls)
    graph_directive = None
    if parents or children:
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
                   link=base_link,
                   parents=parents,
                   child_list=children,
                   )
    #print(graph_directive)
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
