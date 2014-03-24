f = open("codes.txt")
codes = f.read().split('\n')
f.close()
codes.sort()
res = ""
for code in codes:
	if code[0] == '/':
		continue

	res += "int %s_run(Instruction *inst);\r\n"%(code) 

f = open("runners.h", "w")
f.write(res)
f.close
