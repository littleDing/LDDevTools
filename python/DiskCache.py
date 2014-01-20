import cPickle
import marshal
import os
import logging

def next_safe(iterable):
	item = None
	for item in iterable:
		break
	return item

def DiskCacheReader(filename,serializer=cPickle):
	with open(filename) as fin:
		try:
			while True:
				buff = serializer.load(fin)
				for item in buff:
					yield item
		except:
			pass

def DiskCacheWriter(filename,iterable,buffsize=1024,serializer=cPickle):
	with open(filename,'w') as fout:
		buff = []
		for item in iterable:
			buff.append(item)
			if len(buff) ==buffsize:
				serializer.dump(buff,fout)
				buff = []
			yield item	
		if len(buff) != 0:
			serializer.dump(buff,fout)

def writeToDiskCache(filename,iterable,buffsize=1024,serializer=cPickle):
	with open(filename,'w') as fout:
		buff = []
		for item in iterable:
			buff.append(item)
			if len(buff) ==buffsize:
				serializer.dump(buff,fout)
				buff = []
		if len(buff) != 0:
			serializer.dump(buff,fout)

class CachedDataLoader():
	def __init__(self,memcached=True,serializer=cPickle):
		self.memcache = {} if memcached else None
		self.serializer = serializer
	def load_data(self,path,loader,reuse=True,diskOperators=None):
		return self.load_object(path,loader,reuse,diskOperators)
	def load_object(self,path,loader,reuse=True,diskOperators=None):
		logging.warn(('loading_object',path,loader,reuse))
		ret = None
		if diskOperators == None:
			diskOperators = (lambda s:self.serializer.load(open(s))),(lambda x,s:self.serializer.dump(x,open(s,'w')))
		reader,writer = diskOperators
		if reuse and self.memcache !=None and path in self.memcache:
			ret = self.memcache[path]
		elif reuse and os.path.exists(path):
			ret = reader(path)
		else :
			data = loader()
			writer(data,path)
			if self.memcache !=None :
				self.memcache[path] = data
			ret = data
		logging.warn(('load_object_done',path,loader,reuse))
		return ret
	def load_enumerate(self,path,loader,reuse=True,buffsize=1024,diskOperators=None):
		logging.warn(('loading_enumerate',path))
		if reuse and self.memcache !=None and path in self.memcache:
			for item in self.memcache[path]:
				yield item
		elif reuse and os.path.exists(path):
			return DiskCacheReader(path,serializer=self.serializer)
			for item in DiskCacheReader(path,serializer=self.serializer):
				yield item 
		else :
			data = []
			with open(path,'w') as fout:
				buff = []
				if self.memcache !=None:
					for item in loader():
						data.append(item)
						buff.append(item)
						if len(buff) ==buffsize:
							self.serializer.dump(buff,fout)
							buff = []
						yield item
				else :
					for item in loader():
						buff.append(item)
						if len(buff) ==buffsize:
							self.serializer.dump(buff,fout)
							buff = []
						yield item
				if len(buff) != 0:
					self.serializer.dump(buff,fout)
			if self.memcache !=None :
				self.memcache[path] = data

disk_cache = CachedDataLoader(False)
mem_cache = CachedDataLoader()
def load_cached_data(path,loader,reuse=True,memcached=True):
	if memcached :
		return mem_cache.load_data(path,loader,reuse)
	else :
		return disk_cache.load_data(path,loader,reuse)
def load_diskcached_data(path,loader,reuse=True):
	return load_cached_data(path,loader,reuse,False)
def load_memcached_data(path,loader,reuse=True):
	return load_cached_data(path,loader,reuse,True)
def diskcached(path,reuse=True):
	def make_loader(loader):
		def _loader():
			return load_diskcached_data(path,loader,reuse)
		return _loader
	return make_loader
class Database():
	def __init__(self,path,memcached=True,buffsize=1024,**arg):
		self.path = path
		self.buffsize = buffsize
		self.datas = CachedDataLoader(memcached,**arg)
		self.loaders = {}
	def add_data(self,name,loader,_type='enumerate',_diskOperators=None):
		self.loaders[name] = (loader,_type,_diskOperators)
	def load_data(self,name,*arg):
		logging.debug(self.loaders)
		loader,_type,_ops = self.loaders[name]
		path = self.path + name + str(arg).replace('/','__').replace('\'','__')
		if _type == 'object':
			data = self.datas.load_object(path,lambda:loader(*arg),diskOperators=_ops)
		elif _type == 'enumerate':
			data = self.datas.load_enumerate(path,lambda:loader(*arg), buffsize=self.buffsize, diskOperators=_ops)
		return data
		
import conf
database = Database(conf.PICKLE_DIR)
def load_data(name,arg):
	return database.load_data(name,arg)
def dbcached(db=database,_type='enumerate',_diskOperators=None):
	def _dbcached(loader):
		def _load_data(*arg):
			return db.load_data(loader.__name__,*arg)
		db.add_data(loader.__name__,loader,_type,_diskOperators)
		return _load_data
	return _dbcached

import tempfile
class DiskSorter():
	def __init__(self,dtemp=None):
		self.dtemp = dtemp if dtemp!=None else tempfile.mkdtemp
	def sort(self,iterable,_cmp=None, key=None, reverse=False): 
		if _cmp == None: _cmp = cmp
		if key == None: key = lambda x:x
		do_cmp = (lambda x,y:_cmp(key(y),key(x))) if reverse else (lambda x,y:_cmp(key(x),key(y)))

		def merge(it1,it2):
			j = next_safe(it2)
			i = next_safe(it1)
			while True:
				if do_cmp(i,j)<=0: 
					yield i
					i = next_safe(it1)
				else : 
					yield j
					j = next_safe(it2)
				if i==None or j==None:
					if i!=None: yield i
					if j!=None: yield j
					break
			for i in it1:
				yield i
			for j in it2:
				yield j
		
		def make_it(it,f,t):
			for i in range(f):
				it.next()
			for i in range(f,t):
				yield it.next()

		def do_sort(cache,l):
			buff = []
			for item in DiskCacheReader(cache):
				buff.append(item)
			if l<5:
				for item in sorted(DiskCacheReader(cache),do_cmp):
					yield item
			else:
				dtemp = tempfile.mkdtemp() + '/'
				it1,l1 = dtemp+'it1',l/2
				it2,l2 = dtemp+'it2',l-l1
				writeToDiskCache(it1,make_it(DiskCacheReader(cache),0,l1))
				writeToDiskCache(it2,make_it(DiskCacheReader(cache),l1,l))
				for item in merge(do_sort(it1,l1),do_sort(it2,l2)):
					yield item
			
		dtemp = tempfile.mkdtemp() + '/'
		cache = dtemp + 'source'
		l = 0
		for i in DiskCacheWriter(cache,iterable):
			l += 1
		return do_sort(cache,l)

def dsorted(iterable, cmp=None, key=None, reverse=False):
	'''
		sorted(iterable, cmp=None, key=None, reverse=False) --> new sorted iterable
	'''	
	return DiskSorter().sort(iterable,cmp,key,reverse)


import random

def test_dsorted():
	def do_test(iterable):
		ans = []
		for it in dsorted(iterable):
			ans.append(it)
		print iterable,ans
	do_test(range(100))
	haha = range(100)
	random.shuffle(haha)
	do_test(haha)

def main():
	test_dsorted()

if __name__ == '__main__':
	main()


