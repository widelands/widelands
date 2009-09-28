#!/usr/bin/python
# -*- coding: utf-8 -*-

import subprocess
import sys
import shutil
import re
import os

summ = 'result.png'
shutil.copy(sys.argv[1], summ)

for arg in sys.argv:
    print arg
    if re.match('.*\\.png$', arg) :
      subprocess.call(['convert', '-composite', '-compose', 'dst-over',  summ, arg, summ])

#res = subprocess.getoutput(['convert', '-trim',  summ, summ])
res = subprocess.Popen(['convert', '-trim', '-verbose',  summ, summ], stdout=subprocess.PIPE).communicate()[0]
#print res

res_change = re.findall('\\d+x\\d+=>(\\d+x\\d+) \\d+x\\d+(\\+\\d+\\+\\d+)', res)
#print res_change

crop_amount= res_change[0][0]+res_change[0][1]
#print crop_amount

for arg in sys.argv:
    print arg
    if re.match('.*\\.png$', arg) :
      res = subprocess.Popen(['convert', '-verbose', '-crop', crop_amount,  arg, arg], stdout=subprocess.PIPE).communicate()[0] 
      print res

os.remove(summ)