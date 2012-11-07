import hashlib
def simhash(vec,bits=64,hasher=(lambda x:int(hashlib.md5(str(x)).hexdigest(),16)),weight=None):
	if weight==None :
		weight = map(lambda x:1,vec)
	ans = [ 0  for i in range(bits) ]
	for i in range(len(vec)):
		h = hasher(vec[i])
		w = weight[i]
		for b in range(bits):
			bit = h & (1<<b)
			bit = -w if bit==0 else w
			ans[b] +=bit
	ret = 0
	for i in range(bits):
		ret += 1<<i  if ans[i]>0 else 0	
	return ret

def bits(a):
	ans = []
	while a!=0 :
		ans.append(a&1)
		a>>=1
	return ans	

def hamming_distance(a,b):
	a = bits(a)
	b = bits(b)
	la = len(a)
	lb = len(b)
	ans = 0
	for i in range(max(la,lb)):
		if i<la and i<lb:
			if a[i]!=b[i]:
				ans +=1
		elif i<la:
			if a[i]==1:
				ans +=1
		else :
			if b[i]==1:
				ans +=1
	return ans

def test():
	a = [1,2,3,4,5]
	b = [1,2,3,4,6]
	sa = simhash(a)
	sb = simhash(b)
	print a,sa
	print b,sb
	print hamming_distance(sa,sb)
	pass

def main():
	test()
	pass

if __name__ == '__main__':
	main()


