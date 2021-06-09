#!/usr/bin/env python
# encoding: utf-8

import re

class_re=re.compile(r'^class\s+(\w+)\s+:\s+public\s+(\w+)')

tmp_classes = {}     # All classes in a file with classname as key. If this class is a child
                     # of other classes the value is a list of ancestors, otherwise ''.
descendant_cls = {}  # derived classes with a list of parent classes
main_cls = []    # main classes

def parse_classes(file_name):
   found_cls = False
   with open(file_name, 'r') as f:
      found_cls = re.findall(r'^class\s(\w+)\s:\s\w+\s(\w+)', f.read(), re.M)

   if found_cls:
      mod_name = ''
      for main_cls, descendant in found_cls:
         # Strip out leading 'Lua'; this can't be done in the regexp because we need 'LunaClass'
         main_cls = main_cls.replace('Lua','')
         descendant = descendant.replace('Lua','')

         # find module name
         if descendant == "LunaClass":
            mod_name = main_cls
            continue

         # feed tmp_classes
         print("chck: ", main_cls, list(tmp_classes.keys()))
         if main_cls not in list(tmp_classes.keys()):
            if descendant == mod_name:
               tmp_classes[main_cls] = []
            else:
               tmp_classes[main_cls]=[descendant]
         if descendant in list(tmp_classes.keys()):
            print("Appending: ", descendant)
            tmp_classes[main_cls].append(descendant)

parse_classes('/home/kaputtnik/Quellcode/widelands-repo/widelands/src/scripting/lua_map.h')

print("\nAll classes:", tmp_classes)
for k, v in tmp_classes.items():
  print(k, v)

#graph_directive = """
   #.. graphviz::

      #graph dependency {{

         #bgcolor="transparent"
         #node [shape=box, style=filled, fillcolor=white]
         #edge [color=white]
         #{cur_cls} [color=green]
         #{main_cls} [shape=house]

         #{main_cls} -- {child_list}
         #}}""".format(cur_cls='Game',
                      #main_cls='wl.bases.EditorGameBase',
                      #child_list=create_cls_list('Game'),
                      #)
#href="../autogen_wl_map.html#building", target="_parent"
#print(graph_directive)

