from subprocess import Popen,PIPE
import httplib
import re
import urlparse
import urllib
from cache import Cache
import random
import sys

def get_ips():
	sss = Popen('ifconfig',stdout=PIPE).stdout.read()
	ret = re.findall('inet [a-z:]*?(\d+.\d+.\d+.\d+)',sss)	
	ret.pop(ret.index('127.0.0.1'))
	return ret

class IPPool():
	def __init__(self):
		self.ips = get_ips()
		self.cache = Cache()
		self.idx = 0
	def get_ip(self,domain):
		idx = self.cache[domain]
		if idx==None : idx = self.idx
		idx = 0	#warning! only for debug
		ret = self.ips[idx]
		idx = (idx+1) % len(self.ips)
		self.cache[domain] = idx
		self.idx = idx
		return ret

ip_pool = IPPool()
agents = [
	'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_4) AppleWebKit/537.4 (KHTML, like Gecko) Chrome/22.0.1229.94 Safari/537.4'
	,'Mozilla / 5.0 (Windows; U; Windows NT 5.2; en - US) AppleWebKit / 534.7 (KHTML, like Gecko) Chrome / 7.0.517.44 Safari / 534.7'
	,'Opera / 9.80 (Windows NT 5.2; U; zh - cn) Presto / 2.7.62 Version / 11.00'
	,'Mozilla / 5.0 (Windows; U; Windows NT 5.2; zh - CN; rv: 1.9.2.13) Gecko / 20101203 Firefox / 3.6.13'
	,'Mozilla / 4.0 (compatible; MSIE 8.0; Windows NT 5.2; Trident / 4.0; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)'
	,'Mozilla / 4.0 (compatible; MSIE 7.0; Windows NT 5.2; Trident / 4.0; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; Maxthon 2.0)'
	,'Mozilla / 5.0 (Windows; U; Windows NT 5.2; en - US) AppleWebKit / 533.9 (KHTML, like Gecko) Maxthon / 3.0 Safari / 533.9'
	,'Mozilla / 4.0 (compatible; MSIE 7.0; Windows NT 5.2; Trident / 4.0; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)'	];

agent_idx = 0
def get_agent():
	print agent_idx
	agent = agents[agent_idx]
	idx = (agent_idx+1) % len(agents)
	global agent_idx
	agent_idx = idx
	return agent

def get_page(url,headers={},timeout=None):
	headers['User-Agent'] = get_agent()
	urls = urlparse.urlsplit(url.encode('utf8'))	
	netloc = urls[1]	
	path = urls[2] +'?'+ urls[3] +'#' +urls[4]
	ip = ip_pool.get_ip(netloc)
	connect = httplib.HTTPConnection(netloc,source_address=(ip,random.randint(10000,20000)),timeout=timeout)
	connect.request('GET',path,headers=headers)
	res = connect.getresponse()
	page = res.read()
	print res.status,urls,ip
	connect.close()
	return page

def test():
	headers = {'User-Agent':'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_4) AppleWebKit/537.4 (KHTML, like Gecko) Chrome/22.0.1229.94 Safari/537.4'}
#	print get_page('http://www.baidu.com/s?wd=httplib&pn=10&tn=baiduhome_pg&ie=utf-8&rsv_page=1',headers);
	print get_page(sys.argv[1]);
	print get_page('http://172.18.218.28/',headers);
	print get_page('http://172.18.218.28:8888/',headers);
	print get_page('http://172.18.218.28:8888/',headers,10);
	print get_page('http://172.18.218.28:8888/',headers,3);	
	pass

def main():
	test()
	pass


if __name__ == '__main__':
	main()


