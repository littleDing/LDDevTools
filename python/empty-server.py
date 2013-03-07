import tornado.httpserver
import tornado.ioloop
import tornado.options
import tornado.web
import json
import time

from tornado.options import define, options

define("port", default=8888, help="run on the given port", type=int)

html = ""
for i in range(1000):
	html += "<p>hello world<p>"

class MainHandler(tornado.web.RequestHandler):
	def post(self):
		print self.request
	def get(self):
		print self.request
		self.write('alert("hello world")')
#		self.write(html)
		

def main():
	tornado.options.parse_command_line()
	application = tornado.web.Application([
		(r"/.*", MainHandler),
		(r"/pull", MainHandler),
		(r"/ack", MainHandler),
		(r"/push", MainHandler),
		(r"/update_pages",MainHandler),
		(r"/get_pages",MainHandler),
	])
	http_server = tornado.httpserver.HTTPServer(application)
	http_server.listen(options.port)
	print 'running on ',options.port
	tornado.ioloop.IOLoop.instance().start()

if __name__ == "__main__":
	main()
