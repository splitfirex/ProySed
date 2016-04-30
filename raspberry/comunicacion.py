import smbus
import time
import requests
import logging

logging.getLogger("requests").setLevel(logging.WARNING)
logging.getLogger("urllib3").setLevel(logging.WARNING)

class comunicacion:
	url = "http://localhost:8888"
	bus = smbus.SMBus(1)
	address = 0x04
	params = {"verbose": False}


	@classmethod
	def consultaI2C(cls):
		data = "000T"
		try:
			caracteres = cls.bus.read_i2c_block_data(cls.address,0)
			return str(bytearray(caracteres))
		except:
			pass
		return data
		
	@classmethod
	def notificaI2C(cls,notificacion):
		print "notificamos a la arduino :", notificacion
		try:
			cls.bus.write_i2c_block_data(cls.address,0x31, [ord(c) for c in notificacion])
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
