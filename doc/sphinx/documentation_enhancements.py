#!/usr/bin/env python
# encoding: utf-8

import re
import os

HFILE_CLS_RE = re.compile(r'^class\s(\w+)\s+:\s+\w+\s+(\w+)[::]*(\w*)', re.M)
RSTDATA_CLS_RE = re.compile(r'.. class:: (\w+)')

MAX_CHILDREN = 2
MAX_PARENTS = 1
MAX_NAME_LENGTH = 15

EXCLUDE_CLASSES = ['Market',
                   'MarketDescription',
                   ]


class LuaClass:

    def __init__(self, name, outfile, parent, is_base):

        def _prefix_name():
            prefix = '.'.join(outfile.rpartition('.')[0].split('_')[1:])
            return '{}.{}'.format(prefix, name)

        self.long_name = _prefix_name()
        self.name = name
        self.outfile = outfile
        self.parent = parent
        self.is_base = is_base
        self.children = []

    def hyphenated(self):
        # Hyphenation by uppercase
        return '-\\n'.join(re.findall(r'[A-Z][a-z]*', self.name))

    def get_graphviz_link(self):
        html_link = 'href="../{}#{}", target="_parent"'.format(
            self.outfile.replace('.rst', '.html'), self.name.lower())
        return html_link

    def print_data(self):
        parent = ''
        if self.parent:
            parent = self.parent.long_name

        print('Name:', self.name,
              '\n  Long Name:', self.long_name,
              '\n  Parent:', parent, 'type:', type(self.parent),
              '\n  Outfile:', self.outfile,
              '\n  is_base:', self.is_base,
              '\n  children:', [x.name for x in self.children])
# End of LuaClass


class LuaClasses:
    """Stores and handles LuaClass objects."""

    def __init__(self):
        self.all_classes = []

    def add_class(self, name, outfile, parent=None, is_base=False):
        if not is_base and not parent:
            raise Exception('Error: Derived class needs a parent class!')

        new_cls = LuaClass(name, outfile, parent, is_base)
        self.all_classes.append(new_cls)

    def create_inheritances(self):
        """Modify some LuaClass attributes.

        - Parent is just a name, we apply the LuaClass instance instead
        - Add all children instances to the parent class
        """
        def _get_parent_instance(cls_name):
            c_list = []
            for c in self.all_classes:
                if c.name == cls_name:
                    c_list.append(c)
            if len(c_list) == 0:
                raise Exception('Error: There is no parent class named:', cls_name)
            elif len(c_list) > 1:
                raise Exception('Error: Two parent classes with name:', cls_name)
            return c_list[0]

        for c in self.all_classes:
            if c.parent:
                parent = _get_parent_instance(c.parent)
                c.parent = parent
                c.parent.children.append(c)

    def get_base_names(self):
        """Returns a list of base class names."""
        return [c.name for c in self.all_classes if c.is_base]

    def get_parents(self, cls_inst, tree=None):
        """Recursively find all parents of cls_inst.

        Returns a list of LuaClass instances.
        """
        if tree == None:
            tree = []
        for c in self.all_classes:
            if c == cls_inst:
                if c.parent:
                    if not c.parent in tree:
                        # This is only needed for double defined class Player.
                        tree.append(c.parent)
                    self.get_parents(c.parent, tree)
                else:
                    # No parent anymore, end of recursion
                    return tree
        return tree

    def is_double(self, cls_inst):
        found = 0
        for c in self.all_classes:
            if c.name == cls_inst.name:
                found += 1
        if found > 1:
            return True
        return False

    def get_instance(self, cls_str, outfile):
        # Returns a LuaClass object
        for c in self.all_classes:
            if cls_str == c.name and outfile == c.outfile:
                return c
        raise Exception('No class named "{}" found with outfile "{}": '.format(
            cls_str, outfile))


    def print_classes(self):
        print('all classes:')
        for c in self.all_classes:
            c.print_data()
# End of LuaClasses


classes = LuaClasses()


def fill_data(file_name, outfile):
    """Find class names in given File.

    Reads out each occurrence of "class x : public y::z".
    Each found class will be an instance of LuaClass and added to a list
    of classes.
    The values have to be cleaned which is made in two steps:
    - Strip leading 'Lua' from the name of classes.
      In contrast to classes in .h-files the Lua-classes have no leading
      string 'Lua'. Future implementations have to be exactly like this!
    -  The class derived from 'LunaClass' is the base class for all other
      classes in this file. This class has no meaning for the Lua
      documentation, so we do not store it's name.
    """

    found_cls = False
    with open(file_name, 'r') as f:
        found_cls = HFILE_CLS_RE.findall(f.read())

    if found_cls:
        mod_name = ''
        for cls_name, namespace_or_parent, parent_cls in found_cls:
            # The variables refer to a definition of:
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

            # Store found classes
            if namespace_or_parent == mod_name:
                classes.add_class(cls_name, outfile, is_base=True)
            else:
                if parent_cls:
                    # ... : public ...::parent_cls
                    classes.add_class(cls_name, outfile, parent=parent_cls)
                else:
                    # ... : public namespace_or_parent
                    classes.add_class(cls_name, outfile,
                                      parent=namespace_or_parent)


def init(base_dir, cpp_files):
    # Read all class definitions
    for cpp_file, outfile in cpp_files:
        # Assuming each .cc file has a corresponding .h file
        header = cpp_file.rpartition('.')[0] + '.h'
        h_path = os.path.join(base_dir, header)
        fill_data(h_path, outfile)

    # Apply inheritances. This can only be done after all classes are
    # created because some class definitions are in different files.
    classes.create_inheritances()

    #classes.print_classes()


def add_child_of(rst_data, outfile):
    """Adds the String 'Child of: …, …' to rst_data."""

    def _long_or_short(cls):
        # If the parent is defined in a different file than the child
        # return it's long_name
        if cls.children:
            for c in cls.children:
                if c.outfile != cls.outfile:
                    return cls.long_name
        return cls.name

    found_classes = RSTDATA_CLS_RE.findall(rst_data)
    for c_name in found_classes:
        cls_inst = classes.get_instance(c_name, outfile)
        parents = classes.get_parents(cls_inst)
        if parents:
            repl_str = '.. class:: {}\n\n'.format(cls_inst.name)
            child_str = '{}   Child of:'.format(repl_str)
            for i, parent in enumerate(parents):
                child_str += ' :class:`{}`'.format(_long_or_short(parent))
                if i < len(parents) - 1:
                    # add separator except after last entry
                    child_str += ', '

            child_str += '\n\n'
            rst_data = rst_data.replace(repl_str, child_str)

    return rst_data


def format_graphviz_parents(cls_inst):

    if cls_inst.name in classes.get_base_names():
        # This is already a base class
        return cls_inst.name, '', ''

    def _make_tooltip(p_list):
        via_list = 'Via: '
        for i, c in enumerate(p_list):
            via_list += c.name
            if i < len(p_list)-1:
                via_list += ' → '
        return via_list

    parents = list(reversed(classes.get_parents(cls_inst)))
    ret_str = ''
    base_name = ''
    base_link = ''
    if parents:
        base = parents.pop(0)
        base_link = base.get_graphviz_link()
        base_name = base.name
        if parents:
            # Split parents into two lists depending on MAX_PARENTS
            tt_list = parents[:-MAX_PARENTS]
            show_list = parents[-MAX_PARENTS:]
            if tt_list:
                # Show big edge with tooltip.
                ret_str += '{base} -- {n} [style=tapered, arrowhead=none, arrowtail=none dir=both,\
penwidth=15, edgetooltip="{tooltip}"]\n'.format(base=base_name,
                                                n=show_list[-1].name,
                                                tooltip=_make_tooltip(tt_list)
                                                )
            else:
                # No tooltip, normal edge
                ret_str += '{base} -- {n}\n'.format(base=base_name,
                                                n=show_list[-1].name,
                                                )
            for i, p in enumerate(show_list):
                # Create the connections between parents
                try:
                    ret_str += '    {{{a}[{link}]}} -- {b}\n'.format(
                        a=show_list[i+1].name,
                        link=show_list[i+1].get_graphviz_link(),
                        b=show_list[i].name
                        )
                except:
                    ret_str += '    {{{a}[{link}]}} -- {b}\n'.format(
                        a=show_list[i-1].name,
                        link=show_list[i-1].get_graphviz_link(),
                        b=cls_inst.name
                        )
        else:
            # No parents
            ret_str = '{base} -- {cur}'.format(base=base_name,
                                               cur=cls_inst.name
                                               )
    return base_name, base_link, ret_str

def get_children_rows(cls_inst, count=0, rows=None):
    # Recursively find all children of cls_inst.
    # Returns a dict where the keys are the rownumbers and the values are
    # lists in form of [[parent,[children],],]
    if rows == None:
        rows = {count: []}
    if count == MAX_CHILDREN:
        return rows
    if cls_inst.children:
        if count in rows:
            # add children
            rows[count].append([cls_inst, cls_inst.children])
        else:
            # create row
            rows[count] = [[cls_inst, cls_inst.children]]
        count += 1
    for child in cls_inst.children:
        get_children_rows(child, count, rows)
    return rows


def format_graphviz_children(cls_inst):

    def _node_opts(cls, last_row):
        label = cls.hyphenated()
        if classes.is_double(cls):
            label = cls.long_name
        tt = cls.name
        if last_row and cls.children:
            label = '{}\\n… more …'.format(label)
            tt = '{} has more children'.format(cls.name)

        opts = '[{url}, label="{label}", tooltip="{tt}"]'.format(
            url=cls.get_graphviz_link(),
            label=label,
            tt=tt)
        return opts

    def _create_row(parent, children, last_row):
        ret_str = ''
        for child in children:
            node_name = child.name
            if classes.is_double(child):
                node_name = child.long_name
            ret_str += '"{}" {options}'.format(node_name,
                                               options=_node_opts(child, last_row))
            # Add spaces to make sphinx happy
            ret_str += '\n    '
            ret_str += '"{}" -- "{}"\n    '.format(parent.name, node_name)
        return ret_str

    ret_str = ''
    all_children = get_children_rows(cls_inst)
    for row, row_items in all_children.items():
        for parent, children in row_items:
            last_row = False
            if row == len(all_children)-1:
                last_row = True
            ret_str += _create_row(parent, children, last_row)

    return ret_str


def create_directive(cls_inst):
    children = format_graphviz_children(cls_inst)
    base_cls, base_link, parents = format_graphviz_parents(cls_inst)
    graph_directive = None

    if parents or children:
        graph_directive = """
.. graphviz::
    :alt: Dependency graph for class: {cur_cls}

    graph {cur_cls} {{

    bgcolor="transparent"
    node [shape=box, style=filled, fillcolor=white,
          fontsize=12, fontname="Helvetica"]
    edge [color=gray]
    {cur_cls} [fillcolor=green, fontcolor=white, fontsize=13, shape=oval]
    {base_cls} [shape=house, {link}]
    {parents}
    {children}
    }}\n""".format(cur_cls=cls_inst.name,
                   base_cls=base_cls,
                   link=base_link,
                   parents=parents,
                   children=children,
                   )

    return graph_directive


def add_dependency_graph(rst_data, outfile):
    found_cls = RSTDATA_CLS_RE.findall(rst_data)
    for cls_name in found_cls:
        cls_inst = classes.get_instance(cls_name, outfile)
        directive = create_directive(cls_inst)
        if directive:
            repl_str = '.. class:: {}\n\n'.format(cls_name)
            directive_str = '{}\n{}'.format(directive, repl_str)
            rst_data = rst_data.replace(repl_str, directive_str)

    return rst_data
