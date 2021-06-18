#!/usr/bin/env python
# encoding: utf-8

import re
import os

HFILE_CLS_RE = re.compile(r'^class\s(\w+)\s+:\s+\w+\s+(\w+)[::]*(\w*)', re.M)
RSTDATA_CLS_RE = re.compile(r'.. class:: (\w+)')

main_classes = {}       # a dict with main class names as keys. The value will be outfile
                        # as given by cpp_pairs in extract_rts.py
derived_classes = {}    # a dict with derived class names as key and his ancestor as value


def fill_data(file_name, outfile):
    """ Find class names in given File.
    Reads out each occurrence of "class x : public y::z" and returns a
    'cleaned' dict of {[class_name]=ancestor, ...}. The 'cleaning' is made
    in two steps:
    - Strip leading 'Lua' from the name of classes.
      In contrast to classes in .h-files the Lua-classes have no leading
      string 'Lua'.
    -  The class derived from 'LunaClass'' is the base class for all other
      classes in this file. This class has no meaning for the Lua
      documentation, so we do not store it's name, either as key nor
      as value.
    """

    found_cls = False
    with open(file_name, 'r') as f:
        found_cls = HFILE_CLS_RE.findall(f.read())

    if found_cls:
        mod_name = ''
        for cls_name, parent_cls1, parent_cls2 in found_cls:
            # strip out leading 'Lua'; this can't be done in the regexp
            # because we need 'LunaClass' to get the parent class of all classes
            cls_name = cls_name.replace('Lua', '')
            parent_cls1 = parent_cls1.replace('Lua', '')
            parent_cls2 = parent_cls2.replace('Lua', '')

            # find main lua class (module name)
            if parent_cls1 == 'LunaClass':
                mod_name = cls_name
                continue

            # feed data models
            if parent_cls1 == mod_name:
                main_classes[cls_name] = outfile
            else:
                if parent_cls2:
                    # ... : public parent_cls1::parent_cls2
                    derived_classes[cls_name] = [parent_cls2, outfile]
                else:
                    # ... : public parent_cls1
                    derived_classes[cls_name] = [parent_cls1, outfile]


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

# Recursively find all children of a class.
# The returned tree is a dict in form of
#   {class_name:[list_of_children], class_name1:[list_of_children],…}
def get_children_tree(cls, tree=None):
    if tree is None:
        tree = {cls: []}
    children = get_children(cls)

    if len(children) == 0:
        return tree
    else:
        tree[cls] = children
        for c in children:
            get_children_tree(c, tree)

    return tree

    
def get_main_class(cls):
    main_class = get_ancestor_tree(cls)[-1]
    if main_class in main_classes.keys():
        html_link = '../{}#{}", target="_parent"'.format(
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
        # pop first
        main_cls = ancestor_tree.pop(0)
        # pop last
        ancestor_tree.pop()

    ret_str = ''
    if len(ancestor_tree) > 0:
        # show a big dashed edge with tooltip
        ret_str = """{main_cls} -- {cls} [style=dashed, penwidth=10, edgetooltip="{tooltip}"]
                  """.format(main_cls=main_cls, cls=cls, tooltip=_make_tooltip())
    else:
        ret_str = """{main_cls} -- {cls}""".format(main_cls=main_cls, cls=cls)

    return ret_str


def get_child_html_link(cls):
    f = derived_classes[cls][1]
    ret_str= 'href="../{}#{}", target="_parent"'.format(f.replace('.rst', '.html'), cls.lower())
    return ret_str


def format_child_lists(cls):
    """Create a formatted list of class(es) with children.
       This returns e.g.:
       MapObject -- {BaseImmovable[link] Bob[link]}
       BaseImmovable -- {PlayerImmovable[link]}
       …
    """
    ret_str = ''
    for cl, children in get_children_tree(cls).items():
        if not children:
            return ''
        # spaces needed to make sphinxdoc happy
        ret_str += '    {} -- {{'.format(cl)
        for i, child in enumerate(children):
            ret_str = '{}{}[{link}]'.format(ret_str, child, link=get_child_html_link(child))
            if i < len(children) - 1:
                # add space except after last entry
                ret_str += ' '
        ret_str += '}'
    return ret_str


def create_directive(cls):
    ancestors=format_ancestors(cls)
    child_list=format_child_lists(cls)
    graph_directive = None
    if ancestors or child_list:
        main_cls, link=get_main_class(cls)
        graph_directive = """
.. graphviz::
    
    graph {cur_cls} {{
    
    bgcolor="transparent"
    node [shape=box, style=filled, fillcolor=white, fontsize=10]
    edge [color=white]
    {cur_cls} [fillcolor=green, fontcolor=white]
    {main_cls} [shape=house, href="{link}]
    {ancestors}
    {child_list}
    }}\n""".format(cur_cls=cls,
             ancestors=ancestors,
             main_cls = main_cls,
             link=link,
             child_list=child_list,
            )
    return graph_directive


def debug_graph():
    for cls, infile in main_classes.items():
        print("cls/infile:", cls,"/", infile, create_directive(cls))
    
    for cls, data in derived_classes.items():
        print("cls/infile:", cls,"/", data[1], create_directive(cls))


def debug_global_dicts():
    for cls, infile in main_classes.items():
        print("cls/infile:", cls,"/", infile)

    for cls, data in derived_classes.items():
        print("cls/infile:", cls,"/", data[1])

    for checkfor, v in derived_classes.items():
        print("\nDATA FOR:", checkfor)
        print('  ancestors of: '+checkfor+'\n   ', get_ancestor_tree(checkfor))
           
        print('  children of: ' + checkfor +':')
        for cl, ch in get_children_tree(checkfor).items():
            print('    class:', cl, ' children:', ch)
