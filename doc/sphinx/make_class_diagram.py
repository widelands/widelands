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

main_classes = {}       # A dict with main class names as keys. The value
                        # will be the outfile as given by cpp_pairs in
                        # extract_rst.py.
derived_classes = {}    # A dict of derived classes. The key is a class name,
                        # the value is a list of [ancestor, outfile].


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
            if namespace_or_parent == mod_name:
                main_classes[cls_name] = outfile
            else:
                if parent_cls:
                    # ... : public namespace_or_parent::parent_cls
                    derived_classes[cls_name] = [parent_cls, outfile]
                else:
                    # ... : public namespace_or_parent
                    derived_classes[cls_name] = [namespace_or_parent, outfile]


def init(base_dir, cpp_files):
    for cpp_file, outfile in cpp_files:
        header = cpp_file.rpartition('.')[0] + '.h'
        h_path = os.path.join(base_dir, header)
        fill_data(h_path, outfile)


def get_ancestor(cls):
    # Helperfunction for get_ancestor_tree()
    for key, ancestor in derived_classes.items():
        if cls == key:
            return ancestor[0]
    return False


def get_ancestor_tree(cls, tree=None):
    # Recursively find all classes up to the main class
    if tree is None:
        tree = []
    if cls in main_classes.keys():
        tree.append(cls)
        return tree
    a = get_ancestor(cls)
    if a:
        tree.append(cls)
        get_ancestor_tree(a, tree)

    return tree


def get_children(cls):
    # Helper function for get_children_tree()
    children = []
    for key, ancestor in derived_classes.items():
        if cls == ancestor[0]:
            children.append(key)
    return children


def get_children_tree(cls, max_children=0, tree=None):
    """Recursively find all children of a class.

    The returned tree is a dict in form of:
    {class_name:[list_of_children], class_name1:[list_of_children],…}
    """
    if tree is None:
        tree = {cls: []}

    children = get_children(cls)

    if len(children) == 0 or max_children == MAX_CHILDS:
        return tree
    else:
        max_children += 1
        tree[cls] = children
        for c in children:
            get_children_tree(c, max_children, tree)

    return tree


def format_main_class(cls):
    main_class = get_ancestor_tree(cls)[-1]
    if main_class in main_classes.keys():
        html_link = '"../{}#{}", target="_parent"'.format(
            main_classes[main_class].replace('.rst', '.html'), main_class.lower())
        return main_class, html_link
    return ''


def format_ancestors(cls):
    if cls in main_classes.keys():
        return ''

    def _make_tooltip():
        via_list = 'Via: '
        for i, c in enumerate(ancestor_tree):
            via_list += c
            if i < len(ancestor_tree)-1:
                via_list += ' → '
        return via_list

    ancestor_tree = list(reversed(get_ancestor_tree(cls)))
    main_cls = ''
    if len(ancestor_tree):
        main_cls = ancestor_tree.pop(0)
        # pop cls
        ancestor_tree.pop()

    ret_str = ''

    if len(ancestor_tree) >= MAX_PARENTS:
        parent = ancestor_tree.pop()
        if len(ancestor_tree) >= MAX_PARENTS:
            # show a big edge with tooltipp
            ret_str = '{main_cls} -- {parent}\
[style=tapered, arrowhead=none, arrowtail=none dir=both,\
penwidth=15, edgetooltip="{tooltip}"]'.format(main_cls=main_cls,
                                              parent=parent,
                                              tooltip=_make_tooltip()
                                              )
            # add connection between parent and child
            ret_str += '\n    {{{parent}[{link}]}} -- {cls}'.format(
                parent=parent,
                link=get_child_html_link(parent),
                cls=cls)
        else:
            # only 2 parent classes
            ret_str = '{main_cls} -- {{{parent}[{link}]}} -- {cls}'.format(
                main_cls=main_cls,
                parent=parent,
                link=get_child_html_link(parent),
                cls=cls)
    else:
        ret_str = '{main_cls} -- {cls}'.format(main_cls=main_cls, cls=cls)

    return ret_str


def get_child_html_link(cls):
    f = derived_classes[cls][1]
    ret_str = 'href="../{}#{}", target="_parent"'.format(
        f.replace('.rst', '.html'), cls.lower())
    return ret_str


def format_child_lists(cls):
    """Create a graphviz formatted list of class(es) with children.
       This returns e.g.:
       MapObject -- {BaseImmovable[options] Bob[options]}
       BaseImmovable -- {PlayerImmovable[options]]}
       …
    """
    ret_str = ''
    last_row = 0
    for cl, children in get_children_tree(cls).items():
        if not children:
            return ''

        ret_str += '{cl} -- {{'.format(cl=cl)
        last_row += 1
        for i, child in enumerate(children):
            link = get_child_html_link(child)
            child_name = child

            if len(children) >= 4 and len(child_name) > 15:
                # Truncate long words to make the resulting graph smaller in width
                child_name = child_name[0:15] + '…'

            label = '"{}"'.format(child_name)
            tooltip = '"{}"'.format(child)

            grandchildren = get_children(child)
            if grandchildren and last_row >= MAX_CHILDS:
                # Provide a label which indicates more child classes
                label = '"{}\\n… more …"'.format(child_name)
                # A tooltip is made from the label, but can't handle '\n'
                tooltip = '"{} has more children…"'.format(child)

            ret_str = '{}{child}[{link}, label={label}, tooltip={tt}]'.format(ret_str,
                                                                              child=child,
                                                                              link=link,
                                                                              label=label,
                                                                              tt=tooltip
                                                                              )
            if i < len(children) - 1:
                # add space except after last entry
                ret_str += ' '
        ret_str += '}'

    return ret_str


def create_directive(cls):
    ancestors = format_ancestors(cls)
    child_list = format_child_lists(cls)
    graph_directive = None
    if ancestors or child_list:
        main_cls, link = format_main_class(cls)
        graph_directive = """
.. graphviz::
    :alt: Dependency graph for class: {cur_cls}

    graph {cur_cls} {{


    bgcolor="transparent"
    node [shape=box, style=filled, fillcolor=white,
          fontsize=12, fontname="Helvetica", margin="0.05, 0.0"]
    edge [color=gray]
    {cur_cls} [fillcolor=green, fontcolor=white, fontsize=13, shape=oval]
    {main_cls} [shape=house, href={link}]
    {ancestors}
    {child_list}
    }}\n""".format(cur_cls=cls,
                   ancestors=ancestors,
                   main_cls=main_cls,
                   link=link,
                   child_list=child_list,
                   )
    return graph_directive


def debug_graph():
    for cls, outfile in main_classes.items():
        print('cls/outfile:', cls, '/', outfile, create_directive(cls))

    for cls, data in derived_classes.items():
        print('cls/outfile:', cls, '/', data[1], create_directive(cls))


def debug_global_dicts():
    for cls, outfile in main_classes.items():
        print('cls/outfile:', cls, '/', outfile)

    for cls, data in derived_classes.items():
        print('cls/outfile:', cls, '/', data[1])

    for checkfor, v in derived_classes.items():
        print('\nDATA FOR:', checkfor)
        print('  ancestors of: '+ checkfor +'\n   ', get_ancestor_tree(checkfor))

        print('  children of: ' + checkfor +':')
        for cl, ch in get_children_tree(checkfor).items():
            print('    class:', cl, ' children:', ch)
