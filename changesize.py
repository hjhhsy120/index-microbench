import sys
f = open('index.h', 'r')
ls = f.readlines()
f.close()
f = open('index.h', 'w')
f.writelines(['#define MYGENERICKEYSIZE ', sys.argv[1], '\n'])
f.writelines(ls[1:])
f.close()
