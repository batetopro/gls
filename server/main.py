import tornado.ioloop
import tornado.web
import tornado.template
import os, os.path, time

from util import *


class PageHandler(tornado.web.RequestHandler):
    def build(self, template, **kwargs):
        base = os.path.dirname(os.path.realpath(__file__))
        loader = tornado.template.Loader(os.path.join(base, 'template'))
        self.write(loader.load("header.html").generate())
        self.write(loader.load(template + ".html").generate(**kwargs))
        self.write(loader.load("footer.html").generate())


class JSONHandler(tornado.web.RequestHandler):
    def get(self, *args, **kwargs):
        self.set_header('Access-Control-Allow-Origin', self.request.headers.get('Origin', '*'))
        self.set_header('Access-Control-Allow-Methods', "GET, POST")
        self.set_header('Access-Control-Allow-Credentials', "true")
        self.set_header('Content-Type', "application/json")
        code, result = self.run(*args, **kwargs)
        if code == 0:
            resp = {"result": 0, "message": "OK", "data": result}
        else:
            resp = {"result": code, "message": "ERROR"}
        self.write(resp)

    def run(self, *args, **kwargs):
        return 0


class FileHandler(tornado.web.RequestHandler):
    def get(self, *args, **kwargs):
        self.set_header('Access-Control-Allow-Origin', self.request.headers.get('Origin', '*'))
        self.set_header('Access-Control-Allow-Methods', "GET, POST")
        self.set_header('Access-Control-Allow-Credentials', "true")

        result = self.run(*args, **kwargs)
        if result == 0:
            self.set_status(200)
        else:
            self.set_status(404)

    def run(self, *args, **kwargs):
        return 0

    def send(self, path):
        if not os.path.exists(path):
            return 1

        mimes = {"png": "image/png", "csv": "text"}
        ext = path.split(os.path.extsep)[-1]
        mime = ""#""application/octet-stream"
        if ext in mimes:
            mime = mimes[ext]

        self.set_header('Content-Type', mime)

        with open(path, 'rb') as f:
            while 1:
                data = f.read(16384)
                if not data: break
                self.write(data)

        self.finish()
        return 0

class IndexHandler(PageHandler):
    def get(self):
        self.build('home')


class APIHandler(PageHandler):
    def get(self):
        self.build('api')


class GraphsHandler(JSONHandler):
    def run(self, *args, **kwargs):
        return 0, G


class ColoringHandler(FileHandler):
    def run(self, *args, **kwargs):
        if len(args) < 2:
            return 1

        path = os.path.join(db, "coloring", args[0], args[1])
        return self.send(path)


class StrategyHandler(FileHandler):
    def run(self, *args, **kwargs):
        if len(args) < 2:
            return 1

        path = os.path.join(exp, args[0], args[1], "strategy.csv")
        return self.send(path)


class MovesHandler(FileHandler):
    def run(self, *args, **kwargs):
        if len(args) < 4:
            return 1

        path = os.path.join(exp, args[0], args[1], "moves." + args[2] + "." + args[3] + ".png")
        return self.send(path)


class MetaHandler(FileHandler):
    def run(self, *args, **kwargs):
        if len(args) < 4:
            return 1

        path = os.path.join(exp, args[0], args[1], "meta." + args[2] + "." + args[3] + ".png")
        return self.send(path)


def make_app():
    global G
    E = {}
    if os.path.exists(exp):
        for e in os.listdir(exp):
            E[e] = {}
            for f in os.listdir(os.path.join(exp, e)):
                E[e][f] = 1

    G = {}

    for g in read_csv(os.path.join(db, "graphs.csv")):
        if g["source"] not in G:
            G[g["source"]] = {"graphs": [], "comment": "", "author": "", "mail": ""}
        g["experiment"] = False

        if g["source"] in E and g["graph"] in E[g["source"]]:
            g["experiment"] = True

        G[g["source"]]["graphs"].append(g)
        print(g["source"], g["graph"], g["experiment"])

    for g in read_csv(os.path.join(db, "groups.csv")):
        if g["name"] in G:
            G[g["name"]]["comment"] = g["comment"]
            G[g["name"]]["author"] = g["author"]
            G[g["name"]]["mail"] = g["mail"]

    assets = os.path.join(base, 'assets')
    api = os.path.join(base, 'api')
    return tornado.web.Application([
        (r"/", IndexHandler),

        (r"/api", APIHandler),
        (r"/api/graphs", GraphsHandler),
        (r"/api/coloring/(.*)/(.*)", ColoringHandler),
        (r"/api/moves/(.*)/(.*)/(.*)/(.*)", MovesHandler),
        (r"/api/meta/(.*)/(.*)/(.*)/(.*)", MetaHandler),
        (r"/api/strategy/(.*)/(.*)", StrategyHandler),
        (r"/api/(.*)", tornado.web.StaticFileHandler, {'path': api}),
        (r"/assets/(.*)", tornado.web.StaticFileHandler, {'path': assets}),

        #(r"/feeding", FeedingHandler),
        #(r"/temperature", TemperatureHandler),
        #(r"/brightness", BrightnessHandler),
        #(r"/water-level", WaterLevelHandler),
        #(r"/budget", BudgetHandler),
        #(r"/receive", ReceiveHandler),
    ])


if __name__ == "__main__":
    base = os.getcwd()
    db = os.path.join(base, "db")
    exp = os.path.join(db, "experiment")

    G = {}
    app = make_app()
    app.listen(8890)
    tornado.ioloop.IOLoop.current().start()
