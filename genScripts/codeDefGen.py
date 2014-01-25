f = open("codes.txt")
num = 0
lines = f.read().split('\n')
f.close()
res = ""
for line in lines:
	if line[0] == '/':
		res += line + "\r\n"
		continue

	res += "#define " + line + "\t%d\r\n"%(num)
	num += 1

f = open("codes.h", "w")
f.write(res)
f.close
