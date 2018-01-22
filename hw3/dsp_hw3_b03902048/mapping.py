#!/usr/bin/python
# -*- coding: utf-8 -*-
# mapping.py

import sys
import codecs

try:
  infilename = sys.argv[1]
  outfilename = sys.argv[2]
  # infile = open(infilename, 'r', encoding='BIG5-HKSCS')
  # outfile = open(outfilename, 'w', encoding='BIG5-HKSCS')
  infile = codecs.open(infilename, 'r', encoding='BIG5-HKSCS')
  outfile = codecs.open(outfilename, 'wb')
except Exception as e:
  print('Error:', str(e))
  exit(1)

big2zhu = {}

for line in infile:
  big5_zhu = line.split()
  big5 = big5_zhu[0]
  zhu = big5_zhu[1]
  zhu_list = zhu.split('/')

  for zhuyin in zhu_list:
    z = zhuyin[0]
    if z in big2zhu:
      if big5 not in big2zhu[z]:
        big2zhu[z].append(big5)
    else:
      big2zhu[z] = [big5]

infile.close()

for key, value in sorted(big2zhu.items()):
  lastline = ''

  for v in value:
    lastline += (v + ' ' + v + "\n")
  firstline = key + ' ' + ' '.join(value)

  outfile.write( (firstline + "\n").encode('big5-hkscs') )
  outfile.write( lastline.encode('big5-hkscs') )

outfile.close()

