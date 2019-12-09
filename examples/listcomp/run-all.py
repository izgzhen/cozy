#!/usr/bin/env python3

import os
import sys
from multiprocessing import Pool

mode = sys.argv[1]

work_list = []

for target in ["ListSums", "SumMul"]:
  for TIMEOUT in [30, 90, 150, 300, 600, 900, 1500]:
    hpp = target + "_" + str(TIMEOUT) + ".hpp"
    binary = target + "_" + str(TIMEOUT)
    os.system("rm -f " + binary + ".txt")
    if mode == "cozy":
      os.system("cozy --simple --allow-big-sets --c++ " + hpp + " --resume synthesized/" + target + ".ds." + str(TIMEOUT) + ".synthesized")
    elif mode == "g++":
      os.system("g++ -std=c++11 -O3 -Werror -Wno-parentheses-equality --include " + hpp + " " + target + ".cpp -o " + binary)
    work_list.append(binary)

def work(binary):
  os.system("./" + binary + " 100 > " + binary + ".txt")
  print("Done: " + str(binary))

pool = Pool(processes=64)
pool.map(work, work_list)
pool.close()
pool.join()
