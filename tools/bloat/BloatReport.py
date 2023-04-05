# Works on Linux .sym files generated using the nm command
# Like this:
# nm -CSr --size-sort StereoKitC.sym > size.txt

import re

data = {}

file1 = open('size.txt', 'r')
while True:
    line = file1.readline()
 
    # if line is empty end of file is reached
    if not line:
        break

    matches = re.search("(\w*) (\w*) (\w*) ([^_:<(]*)(?:[_:<(]*)([^_:<(]*)", line)

    size = int(matches[2], 16)
    namespace = matches[4]

    if ' ' in namespace:
        namespace = namespace.split(' ')[1]
    namespace = namespace.replace('\n', '')
    
    if namespace not in data:
        data[namespace] = size
    else:
        data[namespace] += size
file1.close()

data = dict(sorted(data.items(), key=lambda item: item[1]))
for key in data:
    size = data[key]
    if size < 1024: continue
    size = round( size / 1024 )
    print(key + " " + str(size) + "kb")
