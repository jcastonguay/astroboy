#!/usr/bin/python

import os

path = 'coverage/'
listing = os.listdir(path)

goodfiles = {}
d = {}


for filename in listing:
#    print filename
    f = open (path+filename,'r')
    size = len(d)
    for line in f:
        d[line] = line

    f.close()

    if (len(d) > size):
        goodfiles[filename] = filename

for name in goodfiles.keys():
    print name


