#!/usr/bin/env python
# encoding: utf-8

import re

class_re=re.compile(r'^class Lua(\w+) : public Lua(\w+)')

tmp_classes = {}     # All classes in a file with classname as key. If this class is a child
                     # of other classes the value is a list of ancestors, otherwise ''.
cls_derived = {}     # derived classes with a list of parent classes
main_classes = []    # main classes

def parse_classes(file_name):
   with open(file_name) as f:
      for line in f:
         m = re.match(class_re, line)
         if m:
            tmp_classes[m.group(1)]=m.group(2)

parse_classes('/home/kaputtnik/Quellcode/widelands-repo/widelands/src/scripting/lua_map.h')

print("\nAll classes:")
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

