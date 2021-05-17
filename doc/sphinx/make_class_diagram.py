#!/usr/bin/env python
# encoding: utf-8

import re

class_name_re = re.compile(r'.*class::\s(\w+)')

# Should match:
# :class:`wl.bases.EditorGameBase`
# :class:`EditorGameBase`
child_class_re = re.compile(r'[\s|,]+(:class:`([\w+.]+)`)')

tmp_classes = {}     # All classes in a file with classname as key. If this class is a child
                     # of other classes the value is a list of ancestors, otherwise ''.
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

parse_classes('source/autogen_wl.rst')
parse_classes('source/autogen_wl_bases.rst')
print("\nAll classes:")
for k, v in tmp_classes.items():
  print(k, v)

split_classes()

# Recursively find all ancestors of a class
def make_ancestor_tree(c, tree=None):
   if tree is None:
      # Needed, otherwise the tree will survive between recursive calls
      tree = []
   if c in main_classes:
      # End recursion
      tree.append(c)
      return tree
   elif c in list(cls_derived.keys()):
      tree.append(c)
      make_ancestor_tree(cls_derived[c][0], tree)
   return tree

single_child_trees = {}
def make_single_child_trees():
   for cls, ancestors in tmp_classes.items():
      l = []
      for cls_name, children in tmp_classes.items():
         l1=[]
         for c in children:
            l1.append(c.rpartition('.')[2])
         if cls in l1:
            #print("found ", c)
            l.append(cls_name)
            single_child_trees[cls]=l

make_single_child_trees()
print("\nChildren: ")
for k,v in single_child_trees.items():
   print(k,v)

def create_cls_list(name):
   s = '{'
   for c in single_child_trees[name]:
      #s = s + c + " "
      s='{} {}'.format(s,c)
   return s + '}'

all_ancestors = {}
for cls_name, child_list in cls_derived.items():
   ancestors = []
   for cls in cls_derived[cls_name]:
      ancestors.append(make_ancestor_tree(cls))
   all_ancestors[cls_name] = ancestors

print("\nall ancestors")
for k,v in all_ancestors.items():
   print(k,v)

#print("main classes:\n", main_classes)
print("derived classes:")
for k,v in cls_derived.items():
   print(k,v)

graph_directive = """
   .. graphviz::

      graph dependency {{

         bgcolor="transparent"
         node [shape=box, style=filled, fillcolor=white]
         edge [color=white]
         {cur_cls} [color=green]
         {main_cls} [shape=house]

         {main_cls} -- {child_list}
         }}""".format(cur_cls='Game',
                      main_cls='wl.bases.EditorGameBase',
                      child_list=create_cls_list('Game'),
                      )
#href="../autogen_wl_map.html#building", target="_parent"
print(graph_directive)

