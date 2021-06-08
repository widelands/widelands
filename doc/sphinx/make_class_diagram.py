#!/usr/bin/env python
# encoding: utf-8

import re

class_re=re.compile(r'^class\s+(\w+)\s+:\s+public\s+(\w+)')

tmp_cls = {}     # All classes in a file with classname as key. If this class is a child
                     # of other classes the value is a list of ancestors, otherwise ''.
descendant_cls = {}     # derived classes with a list of parent classes
main_cls = []    # main classes

def parse_classes(file_name):
   found_cls = False
   with open(file_name, 'r') as f:
      found_cls = re.findall(r'^class\s(\w+)\s:\s\w+\s(\w+)', f.read(), re.M)

   if found_cls:
      mod_name = ''
      for main_cl, der_cl in found_cls:
         # Strip out leading 'Lua'; this can't be done in the regexp because we need 'LunaClass'
         main_cl = main_cl.replace('Lua','')
         der_cl = der_cl.replace('Lua','')

         # find module name
         if der_cl == "LunaClass":
            mod_name = main_cl
            continue

         # feed tmp_cls
         if der_cl == mod_name:
            tmp_cls[main_cl]=''
         else:
            tmp_cls[main_cl]=der_cl

parse_classes('/home/kaputtnik/Quellcode/widelands-repo/widelands/src/scripting/lua_map.h')

print("\nAll classes:")
for k, v in tmp_cls.items():
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

