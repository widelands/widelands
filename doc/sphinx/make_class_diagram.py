#!/usr/bin/env python
# encoding: utf-8

import re

FIND_CLS_RE = re.compile(r'^class\s(\w+)\s+:\s+\w+\s+(\w+)[::]*(\w*)', re.M)

CHECKFILES = ['src/scripting/lua_map.h',
              'src/scripting/lua_bases.h',
              'src/scripting/lua_game.h',
              'src/scripting/lua_root.h',
              'src/graphic/text/rt_parse.cc'
              ]

main_classes = []       # a list of main classes without ancestors
derived_classes = {}    # a dict with class names as key and his ancestor as value


def find_classes(file_name):
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
        found_cls = FIND_CLS_RE.findall(f.read())

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
                main_classes.append(cls_name)
            else:
                if parent_cls2:
                    # ... : public parent_cls1::parent_cls2
                    derived_classes[cls_name] = parent_cls2
                else:
                    # ... : public parent_cls1
                    derived_classes[cls_name] = parent_cls1


def init(cpp_files):
    for cpp_file, out in cpp_files:
        header = cpp_file.rpartition('.')[0] + '.h'
        find_classes(header)


for f in CHECKFILES:
    find_classes(f)


def get_ancestor(cls):
    # Helperfunction for get_ancestor_tree()
    for key, ancestor in derived_classes.items():
        if cls == key:
            return ancestor
    return False

def get_ancestor_tree(cls, tree=None):
    # Recursively find all classes up to the main class 
    if tree is None:
        tree = []
    if cls in main_classes:
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
        if cls == ancestor:
            children.append(key)
    return children

# Recursively find all children of a class.
# The returned tree is a dict in form of
#   {class_name:[list_of_children], class_name1:[list_of_children],…}
def get_children_tree(cls, tree=None):
    if tree is None:
        #print("creating new dict", cls)
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
    if main_class in main_classes:
        return main_class
    return ''


def format_ancestors(cls):
    if cls in main_classes:
        return ''

    def _make_tooltip():
        via_list = 'Via: '
        for i, c in enumerate(ancestor_tree):
            via_list += c
            if i < len(ancestor_tree)-1:
                via_list += ' → '
        return via_list

    ancestor_tree = list(reversed(get_ancestor_tree(cls)))
    # pop first
    main_cls = ancestor_tree.pop(0)
    # pop last
    if len(ancestor_tree):
        ancestor_tree.pop()

    ret_str = ''
    if len(ancestor_tree) > 0:
        # show a big dashed edge with tooltip
        ret_str = """{main_cls} -- {cls} [style=dashed, penwidth=10, edgetooltip="{tooltip}"]
                  """.format(main_cls=main_cls, cls=cls, tooltip=_make_tooltip())
    else:
        ret_str = """{main_cls} -- {cls}""".format(main_cls=main_cls, cls=cls)

    return ret_str


def format_child_lists(cls):
    """Create a formatted list of class(es) with children.
       This returns e.g.:
       MapObject -- {BaseImmovable Bob}
       BaseImmovable -- {PlayerImmovable}
       …
    """
    ret_str = ''
    for cl, children in get_children_tree(cls).items():
        if not children:
            return ''
        # spaces needed to make sphinxdoc happy
        ret_str += '\n    {} -- {{'.format(cl)
        for i, child in enumerate(children):
            ret_str = '{}{}'.format(ret_str, child)
            if i < len(children) - 1:
                # add space except after last entry
                ret_str += ' '
        ret_str += '}'
    return ret_str

#for checkfor, v in derived_classes.items():
    #print("\nDATA FOR:", checkfor)
    #print('  ancestor_tree:\n   ', get_ancestor_tree(checkfor))
       
    #print('  children_tree:')
    #for cl, ch in get_children_tree(checkfor).items():
        #print('    class:', cl, ' children:', ch)

def create_directive(cls):
    ancestors=format_ancestors(cls)
    child_list=format_child_lists(cls)
    graph_directive = None
    if ancestors or child_list:
        graph_directive = """
.. graphviz::
    
    graph {cur_cls} {{
    
    bgcolor="transparent"
    node [shape=box, style=filled, fillcolor=white]
    edge [color=white]
    {cur_cls} [fillcolor=green]
    {main_cls} [shape=house]
    {ancestors}
    {child_list}
    }}""".format(cur_cls=cls,
                 ancestors=ancestors,
                 main_cls=get_main_class(cls),
                 child_list=child_list,
                )
    return graph_directive
#href="../autogen_wl_map.html#building", target="_parent"
#for cls in main_classes:
#    print("cls", cls, create_directive(cls))
#
#for cls in derived_classes.keys():
#    print("cls", cls, create_directive(cls))
