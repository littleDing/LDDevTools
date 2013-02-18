import logging
import pickle
from conf import *

class LineLogger():
	def __init__(self,name='',interval=1000,msg='lines done'):
		self.name = name
		self.interval = interval
		self.msg = msg
		self.cnt = 0
		logging.info(self.name+' begins')
	def inc(self,cnt=1):
		self.cnt += cnt
		if self.cnt % self.interval ==0 :
			logging.info('%s %d %s'%(self.name,self.cnt,self.msg))
	def end(self):
		logging.info(self.name+' ends')

import re		
def getFilename(path):
	return re.sub('^.*\/','',path) 

class DataLoader():
	def __init__(self,source,dumppath,headlines=0,loginterval=1000):
		self.source = source
		self.dumppath = dumppath
		self.headlines = headlines
		self.loginterval = loginterval
		self.data = []
		pass
	def load_data(self):
#if 0!=len(self.data):
#			return self.data
		try :
			logging.info('loading data from '+self.dumppath)
			fin = open(self.dumppath)
			self.data = pickle.load(fin)
	 	except Exception,e:
			logging.info('load data fail! try to construct from :' + self.source)
			self.data_source = self.load_data_source()
			logger = LineLogger(name = self.source,interval=self.loginterval)
			self.begin()
			for line in self.data_source :
				self.load_line(line)
				logger.inc()
			logger.end()
			self.end()
			with open(self.dumppath,'w') as fout :
				logging.info('saving to dump file : '+self.dumppath) 
				pickle.dump(self.data,fout)
		logging.info('load data finished!')
		return self.data
	def load_data_source(self):
		fin = open(self.source)
		for i in range(self.headlines):
			fin.readline()
		return fin
	def begin(self):
		pass
	def load_line(self,line):
		pass
	def end(self):
		pass

class SimpleLoader(DataLoader):
	def __init__(self,path,skip=0,dumppath=None,seperator=',',intid=True):
		DataLoader.__init__(self,path,dumppath,skip)
		self.data = {}
		self.seperator = seperator
		self.intid = intid
	def load_line(self,line):
		sp = line.split(self.seperator)
		uid = int(sp[0]) if self.intid else sp[0]
		self.data[uid] = sp[1:]

DUMP_PATH_BASE = TMP_DIR + '/JoinLoader/'

class JoinLoader(DataLoader):
	def __init__(self,path
					,keys=[(0,int)]
					,values={}
					,header=True
					,dumppath=None
					,seperator=','
					,drops=[]
					,length=0
					):
		name = path.replace('/','_')
		if dumppath == None :
			dumppath = DUMP_PATH_BASE + 'index.' + name  + '+' + '_'.join(map(str,drops)) +  '+' '_'.join([str(key) for key in keys])
		loginterval = 1000 if length==0 else length/100	
		DataLoader.__init__(self,path,dumppath,loginterval=loginterval)
		self.keys = keys
		self.data = {}
		self.seperator = seperator
		self.header = header
		self.values = values
		self.drops = drops
	def load_data_source(self):
		if self.header :
			fin = open(self.source)
			header = fin.readline()
			fields = header.split(self.seperator)
			self.fields = fields
			for i in range(len(self.keys)) :
				key = self.keys[i]
				self.keys[i] = (fields.index(key[0]),key[1])
			return fin
		else :
			return DataLoader.load_data_source(self)
	def load_line(self,line):
		fields = line.split(self.seperator)
		self.load_feathure(fields,self.data,0)
	def load_feathure(self,fields,data,key_index):
		key = self.keys[key_index]
		kid = key[1](fields[key[0]])
		if key_index < len(self.keys) -1 :
			if not kid in data :
				data[kid] = {}
			self.load_feathure(fields,data[kid],key_index+1)
		else :
			try :
				fields = [self.values[i](fields[i]) if i in self.values else fields[i]    for i in range(len(fields))]
				fields = [fields[i] for i in range(len(fields)) if not i in self.drops]
				data[kid] = fields	
			except Exception,e:
				print fields
				raise e
