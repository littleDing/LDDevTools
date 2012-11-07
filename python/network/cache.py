class Cache():
	def __init__(self,max_size=10000,clean_round=1000):
		self.data = {}
		self.last_access = {}
		self.max_size = max_size
		self.clean_round = clean_round
		self.time_stamp = 0
		self.cnt = 0
	def clean_some(self):
		keys = sorted(self.recent_cnt.items(),key=lambda x:x[1])[:len(self.data)/2]
		for k in keys :
			del self.data 
			del self.last_access
	def update_cnt(self,key):
		if self.cnt == self.clean_round :
			self.time_stamp +=1
			self.cnt = 0
		self.cnt +=1
		self.last_access[key] = self.time_stamp
	def __getitem__(self,key):
		if key in self.data :
			self.update_cnt(key)
			return self.data[key]
		else : 
			return None
	def __setitem__(self,key,value):
		if key in self.data :
			self.data[key] = value
			self.update_cnt(key)
		else:
			if len(self.data) >= self.max_size and self.cnt%(self.max_size/10) ==0:
				self.clean_some()
			self.data[key] = value 
			self.update_cnt(key)

def main():
	test()
	pass


if __name__ == '__main__':
	main()


