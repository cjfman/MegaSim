f = open("runnerArrayGen.txt")
lines = [line for line in f]
f.close()

d = {}

for i in xrange(len(lines)):
	line = lines[i].split('\t')
	name = line[0].split(' ')[1]
	index = int(line[1])
	d[index] = name

out = []

for i in xrange(128):
	line = None
	if i in d:
		line = "%s_run,\t// %i"%(d[i], i)

	else:
		line = "unhandled_run,\t// %i"%(i)
		
	out.append(line)

f = open("runnerArray.c", 'w')
f.write("\n".join(out))
f.close()
