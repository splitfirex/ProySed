from tornado import websocket, web, ioloop
import json
from xml.etree.ElementTree import Element
import xml.etree.ElementTree as ET
import datetime

cl = []

class IndexHandler(web.RequestHandler):
    def get(self):
        self.render("index.html")

class SocketHandler(websocket.WebSocketHandler):
    def check_origin(self, origin):
        return True

    def open(self):
        if self not in cl:
            cl.append(self)
            
            # Estado
            tree = ET.parse('alarm_data.xml')
            root = tree.getroot()
            state = root.get('estado')
            state_since = root.get('estado-desde')
            
            # Alarmas actuales
            output = []
            for nodo in root:
                output.append(nodo.attrib)

            data = {"action": "estado", "estado": state, "since" : state_since}
            data = json.dumps(data)
            self.write_message(data)
            data = {"action": "movimiento", "movimientos": output  }
            data = json.dumps(data)
            self.write_message(data)

    def on_close(self):
        if self in cl:
            cl.remove(self)

class ApiHandler(web.RequestHandler):

    @web.asynchronous
    def get(self, *args):
        self.finish()
        
        action = self.get_argument("action")
        
        if(action == "cambiarestado"):
            tree = ET.parse('alarm_data.xml')
            root = tree.getroot()
            state = self.get_argument("estado");
            root.set('estado',state)
            root.set('estado-desde', datetime.datetime.now().strftime("%Y/%B/%d %I:%M:%S.%p"))
            tree.write('alarm_data.xml', xml_declaration=False)
            movimiento = {"action": "estado","estado" : state}
            data = json.dumps(movimiento)
            for c in cl:
                c.write_message(data)
                        
        if(action == "cambiarpassword"):
            tree = ET.parse('alarm_data.xml')
            root = tree.getroot()
            password = self.get_argument("password");
            root.set('password',password)
            tree.write('alarm_data.xml', xml_declaration=False)

        if(action == "insertarmovimiento"):
            tree = ET.parse('alarm_data.xml')
            root = tree.getroot()
            e = Element('movimiento')
            e.set('tipo',self.get_argument("tipo"))
            e.set('nombre',self.get_argument("nombre"))
            e.set('fecha',self.get_argument("fecha"))
            if self.get_argument("imagen"):
                e.set('imagen',self.get_argument("imagen"))
            root.insert(0,e)
            tree.write('alarm_data.xml', xml_declaration=False)
            # notifico al resto
            movimiento = [{"tipo": self.get_argument("tipo"), "nombre" :self.get_argument("nombre"), "fecha" : self.get_argument("fecha") }]
            data = {"action": "movimiento", "movimientos": movimiento  }
            data = json.dumps(data)
            for c in cl:
                c.write_message(data)

    @web.asynchronous
    def post(self):
        pass

class RestHandler(web.RequestHandler):

    def get(self,*args):
        action = self.get_argument("action")
        
        if(action == "consultarestado"):
            tree = ET.parse('alarm_data.xml')
            root = tree.getroot()
            state = root.get('estado')
            state_since = root.get('estado-desde')                
            data = {"estado": state, "since" : state_since}
            data = json.dumps(data)
            self.write(data)
          
        if(action == "consultarpassword"):
            tree = ET.parse('alarm_data.xml')
            root = tree.getroot()
            password = root.get('password')               
            data = {"password": password}
            data = json.dumps(data)
            self.write(data)
            
app = web.Application([
    (r'/', IndexHandler),
    (r'/ws', SocketHandler),
    (r'/api', ApiHandler),
    (r'/rest', RestHandler),
    (r'/(.*)', web.StaticFileHandler, {'path': './'}),
])

if __name__ == '__main__':
    app.listen(8888)
    ioloop.IOLoop.instance().start()
