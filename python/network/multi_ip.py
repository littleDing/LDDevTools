from subprocess import Popen,PIPE
import httplib
import re
import urlparse
import urllib
from cache import Cache

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
		ret = self.ips[idx]
		idx = (idx+1) % len(self.ips)
		self.cache[domain] = idx
		self.idx = idx
		return ret

ip_pool = IPPool()

def get_page(url,headers={}):
	urls = urlparse.urlsplit(url.encode('utf8'))	
	netloc = urls[1]	
	path = urls[2] +'?'+ urls[3] +'#' +urls[4]
	ip = ip_pool.get_ip(netloc)
	connect = httplib.HTTPConnection(netloc)
	connect.request('GET',path,headers=headers)
	res = connect.getresponse()
	page = res.read()
	print res.status,urls,ip
	return page

def test():
	headers = {'User-Agent':'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_4) AppleWebKit/537.4 (KHTML, like Gecko) Chrome/22.0.1229.94 Safari/537.4'}
#	print get_page('http://www.baidu.com/s?wd=httplib&pn=10&tn=baiduhome_pg&ie=utf-8&rsv_page=1',headers);
	print get_page('http://172.18.217.117/',headers);
	print get_page('http://172.18.217.117:8888/',headers);
	print get_page('http://172.18.217.117:8888/',headers);
	print get_page('http://172.18.217.117:8888/',headers);	
	pass

def main():
	test()
	pass


if __name__ == '__main__':
	main()


