import sys

file1 = open(sys.argv[1])
file2 = open(sys.argv[2])

lines1 = []
lines2 = []

for line in file1:
    lines1.append(line)

for line in file2:
    lines2.append(line)

count = 0
for i in range(len(lines1)):
    if lines1[i] == lines2[i]:
        count += 1
    
print(count / 2500.0)