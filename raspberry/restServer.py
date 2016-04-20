import web
import xml.etree.ElementTree as ET
import datetime
from xml.etree.ElementTree import Element

urls = (
    '/consultar/historico', 'list_alarma',
    '/consultar/estado', 'get_state',
    '/insertar/movimiento', 'insert_alarma',
    '/cambiar/estado/(.*)', 'update_state',
	'/cambiar/password/(.*)', 'update_pass'
)

app = web.application(urls, globals())

class list_alarma:
	def GET(self):
		web.header("Access-Control-Allow-Origin", "*");
		tree = ET.parse('alarm_data.xml')
		root = tree.getroot()
		output = '['
		for nodo in root:
			output += str(nodo.attrib) + ','
		output = output[:-1]
		output += ']';
		return output.replace("'",'\"')

class get_state:
	def GET(self):
		web.header("Access-Control-Allow-Origin", "*");
		tree = ET.parse('alarm_data.xml')
		root = tree.getroot()
		state = root.get('estado')
		state_since = root.get('estado-desde')
		output = '{ '
		output += '"estado" : "{0}" , "since" : "{1}"'.format(state, state_since)
		output += '}'
		return output

class insert_alarma:        
    def GET(self):
    	web.header("Access-Control-Allow-Origin", "*");
    	tree = ET.parse('alarm_data.xml')
    	root = tree.getroot()    	
    	raw_data = web.input()
    	e = Element('movimiento')
    	if raw_data:
	    	e.set('tipo',raw_data.tipo)
	    	e.set('nombre', raw_data.nombre)
	    	e.set('fecha',raw_data.fecha)
	    	if raw_data.imagen is not None:
	    		e.set('imagen',raw_data.imagen)
	    	root.insert(0,e)
	        tree.write('user_data.xml', xml_declaration=False)
	        return 'OK'

class update_state:
    def GET(self, state):
    	web.header("Access-Control-Allow-Origin", "*");
    	tree = ET.parse('alarm_data.xml')
    	root = tree.getroot()
    	root.set('estado',state)
    	root.set('estado-desde', datetime.datetime.now().strftime("%Y/%B/%d %I:%M:%S.%p"))
    	tree.write('alarm_data.xml', xml_declaration=False)
    	return 'OK'
		
class update_pass:
    def GET(self, password):
    	web.header("Access-Control-Allow-Origin", "*");
    	tree = ET.parse('alarm_data.xml')
    	root = tree.getroot()
    	root.set('password',password)
    	tree.write('alarm_data.xml', xml_declaration=False)
    	return 'OK'
    
if __name__ == "__main__":
    app.run()