#!/usr/bin/env python
# encoding: utf-8

import re
import os

HFILE_CLS_RE = re.compile(r'^class\s(\w+)\s+:\s+\w+\s+(\w+)[::]*(\w*)', re.M)
RSTDATA_CLS_RE = re.compile(r'.. class:: (\w+)')

MAX_CHILDREN = 2
MAX_PARENTS = 1

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

    def name_hyphenated(self):
        """Return the hyphenated name split uppercase."""
        return '-\\n'.join(re.findall(r'[A-Z][a-z]*', self.name))

    def get_graphviz_link(self):
        builder = classes.builder
        if builder == 'dirhtml':
            html_link = 'href="../{folder}/index.html#{anchor}",\
                target="_parent"'.format(
                folder=self.outfile.replace('.rst', ''),
                anchor=self.name.lower()
            )
        elif builder == 'html':
            html_link = 'href="../{}#{}", target="_parent"'.format(
                self.outfile.replace('.rst', '.html'), self.name.lower()
            )
        elif builder == 'singlehtml':
            html_link = 'href="../index.html#{}", target="_parent"'.format(
                self.name.lower()
            )
        else:
            return 'href=""'
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
        self.class_list = []
        self.builder = ''

    def add_class(self, name, outfile, parent=None, is_base=False):
        if not is_base and not parent:
            raise Exception('Error: Derived class needs a parent class!')

        new_cls = LuaClass(name, outfile, parent, is_base)
        self.class_list.append(new_cls)

    def apply_inheritances(self):
        """Modify some LuaClass attributes.

        - Parent is just a string yet, apply the LuaClass instance instead
        - Add all children instances to the parent class
        """
        def _get_parent_instance(cls_name):
            c_list = []
            for c in self.class_list:
                if c.name == cls_name:
                    c_list.append(c)
            if len(c_list) == 0:
                raise Exception(
                    'Error: There is no parent class named:', cls_name)
            elif len(c_list) > 1:
                raise Exception(
                    'Error: Two parent classes with name:', cls_name)
            return c_list[0]

        for cls_inst in self.class_list:
            if cls_inst.parent:
                parent = _get_parent_instance(cls_inst.parent)
                cls_inst.parent = parent
                cls_inst.parent.children.append(cls_inst)

    def get_parent_tree(self, cls_inst, tree=None):
        """Recursively find all parents of cls_inst.

        Returns a list of LuaClass instances.
        """
        if tree is None:
            tree = []
        if cls_inst.is_base:
            return tree
        else:
            tree.append(cls_inst.parent)
            self.get_parent_tree(cls_inst.parent, tree)
        return tree

    def get_children_rows(self, cls_inst, count=0, rows=None):
        """Recursively find all children of cls_inst.

        Returns a dict where the keys are the rownumbers and the values
        are lists in form of [[parent,[children],],]
        """
        if rows is None:
            rows = {count: []}
        if count == MAX_CHILDREN:
            return rows
        if cls_inst.children:
            if count in rows:
                # Add a list to an existing row
                rows[count].append([cls_inst, cls_inst.children])
            else:
                # Create new row
                rows[count] = [[cls_inst, cls_inst.children]]
            count += 1
        for child in cls_inst.children:
            self.get_children_rows(child, count, rows)
        return rows

    def get_instance(self, cls_str, outfile):
        """Returns a unique LuaClass object."""
        for c in self.class_list:
            if cls_str == c.name and outfile == c.outfile:
                return c
        raise Exception('No class named "{}" found with outfile "{}": '.format(
            cls_str, outfile))

    def have_same_source(self, cls1, cls2):
        if cls1.outfile != cls2.outfile:
            return False
        return True

    def print_classes(self):
        print('all classes:')
        for c in self.class_list:
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
            # because we need 'LunaClass' to get the parent class of
            # all classes.
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
    # read because some class definitions are in different files.
    classes.apply_inheritances()

    # classes.print_classes()


def add_child_of(rst_data, outfile):
    """Adds the String 'Child of: …, …' to rst_data."""

    found_classes = RSTDATA_CLS_RE.findall(rst_data)
    for c_name in found_classes:
        cls_inst = classes.get_instance(c_name, outfile)
        parents = classes.get_parent_tree(cls_inst)
        if parents:
            repl_str = '.. class:: {}\n\n'.format(cls_inst.name)
            child_str = '{}   Child of:'.format(repl_str)
            for i, parent in enumerate(parents):
                if classes.have_same_source(parent, cls_inst):
                    cls_name = parent.name
                else:
                    # Apply the long name to make sphinx-links work across
                    # documents
                    cls_name = parent.long_name

                child_str += ' :class:`{}`'.format(cls_name)
                if i < len(parents) - 1:
                    # Add separator except after last entry
                    child_str += ', '

            child_str += '\n\n'
            rst_data = rst_data.replace(repl_str, child_str)

    return rst_data


def format_graphviz_parents(cls_inst):
    """Add nodes in graphviz syntax for the parents of cls_inst."""

    if cls_inst.is_base:
        return cls_inst.name, '', ''

    def _make_tooltip(p_list):
        via_list = 'Via: '
        for i, c in enumerate(p_list):
            via_list += c.name
            if i < len(p_list)-1:
                via_list += ' → '
        return via_list

    parents = list(reversed(classes.get_parent_tree(cls_inst)))
    ret_str = ''
    base_name = ''
    base_link = ''
    if parents:
        base = parents.pop(0)
        base_link = base.get_graphviz_link()
        base_name = base.name
        if parents:
            # Split parents into two lists
            tt_list = parents[:-MAX_PARENTS]
            show_list = parents[-MAX_PARENTS:]
            if tt_list:
                # Show big edge with tooltip.
                ret_str += '{base} -- {cls} [style=tapered, arrowhead=none, arrowtail=none dir=both,\
penwidth=15, edgetooltip="{tooltip}"]\n'.format(base=base_name,
                                                cls=show_list[0].name,
                                                tooltip=_make_tooltip(tt_list)
                                                )
            else:
                # No tooltip, normal edge
                ret_str += '{base} -- {n}\n'.format(base=base_name,
                                                    n=show_list[0].name,
                                                    )
            for i, p in enumerate(show_list):
                # Create the connections between parents
                # Add node options
                ret_str += '    {name} [{link}]\n'.format(
                    name=p.name,
                    link=p.get_graphviz_link(),
                )
                # Create connections
                try:
                    ret_str += '    {a} -- {b}\n'.format(
                        a=p.name,
                        b=show_list[i+1].name,
                    )
                except IndexError:
                    ret_str += '    {a} -- {b}\n'.format(
                        a=p.name,
                        b=cls_inst.name,
                    )
        else:
            # No intermediate parents
            ret_str = '{base} -- {cur}'.format(
                base=base_name,
                cur=cls_inst.name
            )
    return base_name, base_link, ret_str


def format_graphviz_children(cls_inst):
    """Add nodes in graphviz syntax for the children of cls_inst."""

    def _node_opts(cls, last_row):
        if classes.have_same_source(parent, cls):
            label = cls.name_hyphenated()
        else:
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
            ret_str += '\n    '
            if classes.have_same_source(child, child.parent):
                node_name = child.name
            else:
                # Using long_name to distuinguish between double
                # defined classnames
                node_name = child.long_name

            ret_str += '"{name}" {options}\n'.format(
                name=node_name,
                options=_node_opts(child, last_row),
            )
            ret_str += '    "{c1}" -- "{c2}"\n'.format(
                c1=parent.name,
                c2=node_name,
            )
        return ret_str

    ret_str = ''
    all_children = classes.get_children_rows(cls_inst)
    for row, row_items in all_children.items():
        for parent, children in row_items:
            last_row = False
            if row == len(all_children)-1:
                last_row = True
            ret_str += _create_row(parent, children, last_row)

    return ret_str


def create_directive(cls_inst):
    """Create a graphviz directive.

    Define the main look and feel here.
    """
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
    {cur_cls} [fillcolor="#118811", fontcolor=white, fontsize=13, shape=oval]
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


def add_dependency_graph(rst_data, outfile, builder):
    """Insert a dependency graph to rst_data."""
    classes.builder = builder
    found_cls = RSTDATA_CLS_RE.findall(rst_data)
    for cls_name in found_cls:
        cls_inst = classes.get_instance(cls_name, outfile)
        directive = create_directive(cls_inst)
        if directive:
            repl_str = '.. class:: {}\n\n'.format(cls_name)
            directive_str = '{}\n{}'.format(directive, repl_str)
            rst_data = rst_data.replace(repl_str, directive_str)

    return rst_data
