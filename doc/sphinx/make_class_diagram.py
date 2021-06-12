#!/usr/bin/env python
# encoding: utf-8

import re

FIND_CLS_RE = re.compile(r'^class\s(\w+)\s+:\s+\w+\s+(\w+)[::]*(\w*)', re.M)


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

    tmp_classes = {}
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

            # feed tmp_classes
            if parent_cls1 == mod_name:
                tmp_classes[cls_name] = ''
            else:
                if parent_cls2:
                    tmp_classes[cls_name] = parent_cls2
                else:
                    tmp_classes[cls_name] = parent_cls1

    return tmp_classes


CHECKFILES = ['src/scripting/lua_map.h',
              'src/scripting/lua_bases.h',
              'src/scripting/lua_game.h',
              'src/scripting/lua_root.h'
              ]

all_classes = []
for f_name in CHECKFILES:
    all_classes.append(find_classes(f_name))

main_classes = []
derived_cls = []
def split_classes():
    for i in all_classes:
        for cls, ancestor in i.items():
            if ancestor == '':
                main_classes.append(cls)
            else:
                derived_cls.append([cls, ancestor])

split_classes()

print('Main classes:', main_classes)
print('Derived classes:', derived_cls)

def make_ancestor_tree(cls, tree=None):
   if tree is None:
      # Needed, otherwise the tree will survive between recursive calls
      tree = []
   if cls in main_classes:
      # End recursion
      tree.append(cls)
      return tree
   else:
      for i in derived_cls:
         if i[0] == cls:
            tree.append(cls)
            make_ancestor_tree(i[1], tree)
   return tree
   
print(make_ancestor_tree('MilitarySite'))
# graph_directive = """
    # .. graphviz::

    # graph dependency {{

    # bgcolor="transparent"
    # node [shape=box, style=filled, fillcolor=white]
    # edge [color=white]
    # {cur_cls} [color=green]
    # {main_classes} [shape=house]

    #{main_cls} -- {child_list}
    # }}""".format(cur_cls='Game',
    # main_cls='wl.bases.EditorGameBase',
    # child_list=create_cls_list('Game'),
    # )
# href="../autogen_wl_map.html#building", target="_parent"
# print(graph_directive)
