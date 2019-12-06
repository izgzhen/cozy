import glob
import sys
import os
import random

files = glob.glob("/tmp/cozy_dumped_*")
for f in random.choices(files, k=200):
    os.system("mv " + f + " " + sys.argv[1])

os.system("rm /tmp/cozy_dumped_*")
