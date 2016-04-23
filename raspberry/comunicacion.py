#import smbus
import time
import requests

class comunicacion:
	url = "http://localhost:8081/"
	bus = '1'
	address = 1234
	params = {"verbose": False}

	@classmethod
	def consultaI2C(cls):
		data = [0,'D',0,0]
		try:
			caracter = ''
			while caracter is not ord('T'):
				caracter = cls.bus.read_byte(cls.address)
				time.sleep(1)
				if caracter is 0:
					continue
				data    +=str(chr(caracter))
				print "el caracter es ", chr(caracter)
			print "la data es ", data
		except:
			pass
		return data
		
	@classmethod
	def notificaI2C(cls,notificacion):
		print "notificamos a la arduino :", notificacion
		try:
			cls.bus.write_i2c_block_data(cls.address,0,notificacion)
		except:
			pass
			
	@classmethod
	def consultaHTTP(cls,tipo):
		try:
			contenido = requests.get(cls.url+ '' +tipo)
		except Exception,e:
			print str(e)
			return {}
		return contenido.json()
	
	@classmethod
	def notificaHTTP(cls,tipo):
		try:
			contenido = requests.get(cls.url+ '' +tipo)
			return contenido.json()
		except:
			return {}
