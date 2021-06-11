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


CHECKFILES = [  # 'src/scripting/lua_map.h',
    'src/scripting/lua_bases.h',
    'src/scripting/lua_game.h',
    # 'src/scripting/lua_root.h'
]

file_classes = {}
all_classes = []
for f_name in CHECKFILES:
    file_classes[f_name] = find_classes(f_name)
    all_classes.append(find_classes(f_name))

print("all classes:", all_classes)
for i in all_classes:
   for k,v in i.items():
      print(k,v)

print('file_classes:')
for f in file_classes:
    print('\nAll classes of:', f)
    for k, v in file_classes[f].items():
        print(k, 'child of: ', v)


def find_descendants(cls):
    descendants = [cls]
    for f in file_classes:
        for main_cls, descendant in file_classes[f].items():
            if descendant == cls:
                descendants.append([main_cls, f])
    return descendants


def get_main_classes():
    main_cls = []
    for f, l in file_classes.items():
        for k, v in l.items():
            if v == '':
                main_cls.append(k)
    return main_cls


def find_all_descendants():
    all_descendants = []
    for c in get_main_classes():
        for l in file_classes.values():
            print('c', c, 'l', l, c in l)
            if c in l:
                all_descendants.append([x for x in find_descendants(l[c])])
                print('found', l[c])
    return all_descendants


# print(get_main_classes())
desc = find_all_descendants()

for c in desc:
    print('descendants', c)


# graph_directive = """
    # .. graphviz::

    # graph dependency {{

    # bgcolor="transparent"
    # node [shape=box, style=filled, fillcolor=white]
    # edge [color=white]
    # {cur_cls} [color=green]
    # {main_cls} [shape=house]

    #{main_cls} -- {child_list}
    # }}""".format(cur_cls='Game',
    # main_cls='wl.bases.EditorGameBase',
    # child_list=create_cls_list('Game'),
    # )
# href="../autogen_wl_map.html#building", target="_parent"
# print(graph_directive)
