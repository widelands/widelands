#!/usr/bin/env python
# encoding: utf-8

import re

child_class_re = re.compile(r'[\s|,]+(:class:`(\w+)`)')
class_name_re = re.compile(r'.*class::\s(\w+)')

tmp_classes = {}     # all classes with, if, a list of parent classes
cls_derived = {}     # derived classes with a list of parent classes
main_classes = []    # main classes

def parse_classes(file_name):
   with open(file_name) as f:
      for line in f:
         m = re.match(class_name_re, line)
         if m:
            class_name = m.group(1)
            tmp_classes[class_name]=''
         if line.startswith("   Child of:"):
            # sometimes the children span two lines because of max len of line = 100
            line2 = next(f)
            line = ''.join([line, line2])
            parent_classes = [ x[1] for x in re.findall(child_class_re, line) ]
            tmp_classes[class_name] = parent_classes

def split_classes():
   for cls, derived in tmp_classes.items():
      if not derived:
         main_classes.append(cls)
      else:
         cls_derived[cls]=derived

parse_classes('source/autogen_wl_map.rst')
split_classes()

def make_parent_tree(c, tree=None):
   if tree is None:
      tree = []
   if c in main_classes:
      tree.append(c)
      return tree
   elif c in list(cls_derived.keys()):
      tree.append(c)
      make_parent_tree(cls_derived[c][0], tree)
   return tree

parents = []
for cls in cls_derived['Warehouse']:
   print("make tree for ", cls)
   parents.append(make_parent_tree(cls))

print("\nresulting tree", parents)
for x in parents:
   print(x)

print("main classes:\n", main_classes)
print("derived classes:\n", cls_derived)
for k,v in cls_derived.items():
   print(k,v)



