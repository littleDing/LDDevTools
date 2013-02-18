from network import multi_ip
import logging
import sys
logging.basicConfig(level=0,format='%(asctime)s %(processName)s %(process)s %(message)s '+str(sys.argv))


#print multi_ip.get_page("http://172.18.217.117:8888/");

def main():
	logging.info('finished!')
	pass


if __name__ == '__main__' :
	main()


