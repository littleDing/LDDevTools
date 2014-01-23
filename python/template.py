from optparse import OptionParser
import random,logging,traceback
logging.basicConfig(level=logging.WARN,format='%(asctime)s %(funcName)s@%(filename)s#%(lineno)d %(levelname)s %(message)s')

def main():
	parser = OptionParser()
	parser.add_option('', '--job', dest='job',help='jobname');
	parser.add_option('', '--seed', dest='seed',type=int,default=123,help='random seed');
	parser.add_option('', '--options', dest='options',type=int,default=1
		,help='if set to 1, function will call with optparser\'s option as first argument');
	(options, args) = parser.parse_args()
	logging.warn((options,args))
	random.seed(options.seed)
	try:
		job = globals()[options.job]
		if options.options :
			job(options,*args)
		else :
			job(*args)
	except Exception,e:
		traceback.print_exc()
		logging.error(e)
		raise e

if __name__ == '__main__':
	main()