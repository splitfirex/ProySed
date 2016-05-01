import argparse
import datetime
import imutils
import time
import cv2
import logging
import ConfigParser
import threading
import sys

from comunicacion import comunicacion

format= '%(asctime)s - %(levelname)s - %(filename)s:%(lineno)s - %(funcName)s() - %(message)s'
format= '%(asctime)s - %(filename)s:%(lineno)s - %(message)s'
logging.basicConfig(level=logging.DEBUG, format=format)
logger = logging.getLogger(__name__)

class estadoAlarma:
	Inactiva  = 0
	Preactiva = 1
	Activa	  = 2
	Presonando= 3
	Sonando	  = 4


class alarmaMonitor (threading.Thread):

	def __init__(self,params):
		threading.Thread.__init__(self)
		self.ea = estadoAlarma.Activa
		self.matar = False
		self.movimiento = False
		self.alarma = ''
		self.framesFolder = params["framesFolder"]
		self.urlFramesfolder = params["urlFramesfolder"]
		self.url = params["weburl"]
		self.contadorSonador = 0;
		self.password = '';

	def setMovimiento(self,mov,frame):
		
		if self.ea is estadoAlarma.Activa and frame is not None:
			self.movimiento = True
			self.alarma = mov
			cv2.putText(frame, "Camara : {}".format(self.alarma), (10, 20),
			cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
			cv2.putText(frame, datetime.datetime.now().strftime("%A %d %B %Y %I:%M:%S%p"),
			(10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)
			fechaImagen = datetime.datetime.now().strftime("%Y-%b-%d-%I-%M-%S%p")
			print "{2}{0}-{1}.jpg".format(self.alarma,fechaImagen,self.framesFolder)
			cv2.imwrite("{2}{0}-{1}.jpg".format(self.alarma,fechaImagen,self.framesFolder),frame)
			comunicacion.notificaI2C("R3T")
			comunicacion.notificaHTTP("/api?action=insertarmovimiento&tipo=camara&nombre={0}&fecha={1}&imagen={2}".format(mov,fechaImagen,"{2}{0}-{1}.jpg".format(self.alarma,fechaImagen,self.urlFramesfolder)))
			self.ea = estadoAlarma.Presonando
			comunicacion.notificaHTTP("/api?action=cambiarestado&estado=Presonando")
			# aprovecho a almacenar en el historico las alarmas correspondientes en el caso que vengan de sensores
		if self.ea is estadoAlarma.Activa and frame is None:
			self.movimiento = True
			self.alarma = mov
			fechaImagen = datetime.datetime.now().strftime("%Y-%b-%d-%I-%M-%S%p")
			comunicacion.notificaI2C("R3T")
			comunicacion.notificaHTTP("/api?action=insertarmovimiento&tipo=Infrarojo&nombre={0}&fecha={1}&imagen=None".format(mov,fechaImagen))
			self.ea = estadoAlarma.Presonando
			comunicacion.notificaHTTP("/api?action=cambiarestado&estado=Presonando")
		

	def consultaEstadoWeb(self):
		webdata = comunicacion.consultaHTTP("/rest?action=consultarestado")

		if webdata["estado"] == "Inactiva":
			print "setea el estado inactiva"
			self.ea = estadoAlarma.Inactiva
			comunicacion.notificaI2C("R0T")
		if webdata["estado"] == "Preactiva":
			print "setea el estado Preactiva"
			self.ea = estadoAlarma.Preactiva
			comunicacion.notificaI2C("R1T")
		if webdata["estado"] == "Activa":
			print "setea el estado Activa"
			self.ea = estadoAlarma.Activa
			comunicacion.notificaI2C("R2T")
		if webdata["estado"] == "Presonando":
			print "setea el estado Presonando"
			self.ea = estadoAlarma.Presonando
			comunicacion.notificaI2C("R3T")
		if webdata["estado"] == "Sonando":
			print "setea el estado Sonando"
			self.ea = estadoAlarma.Sonando
			comunicacion.notificaI2C("R4T")

	def notificaEstadoWeb(self):
		webdata = "Sin Conexion"
		if self.ea is estadoAlarma.Inactiva:
			webdata = "Inactiva"
		if self.ea is estadoAlarma.Preactiva:
			webdata = "Preactiva"
		if self.ea is estadoAlarma.Activa:
			webdata = "Activa"
		if self.ea is estadoAlarma.Presonando:
			webdata = "Presonando"
		if self.ea is estadoAlarma.Sonando:
			webdata = "Sonando"
		comunicacion.notificaHTTP("/api?action=cambiarestado&estado="+webdata)
		

	def ingresaContrasena(self,char):
		self.password = char.split("D")[1].split("T")[0]
		webdata = comunicacion.consultaHTTP("/rest?action=consultarpassword");
		print "el password consultado fue: ", webdata
		print "el password entregado fue: ", self.password
		if webdata["password"] == self.password:
			if self.ea is estadoAlarma.Inactiva:
				self.ea = estadoAlarma.Preactiva
				comunicacion.notificaI2C("R1T")
				self.notificaEstadoWeb()
			if self.ea is estadoAlarma.Activa or self.ea is estadoAlarma.Sonando:
				self.ea = estadoAlarma.Inactiva
				comunicacion.notificaI2C("R0T")
				self.notificaEstadoWeb()

	# Solo esperamos los mensajes de la discovery para validar que la contrasena es correcta
	def estadoInactivo(self):
		data = comunicacion.consultaI2C()
		if data[0] is '1':
			if data[1] is 'D':
				self.ingresaContrasena(data)
	
	# Se esperan 15 segundos y luego cambiamos el estado a activa (Notificar el I2C ?).
	def estadoPreactiva(self):
		time.sleep(15)
		comunicacion.notificaI2C("R2T")
		self.ea = estadoAlarma.Activa
		self.notificaEstadoWeb()
		pass
		
	# Consultamos el I2C por movimientos o por mensajes desde la discovery para desconectar la alarma o ponerla a sonar
	def estadoActiva(self):
		data = comunicacion.consultaI2C()
		if data[0] is '1':
			if data[1] is 'D':
				self.ingresaContrasena(data)
			if data[1] is 'A':
				self.setMovimiento("Sensor IR",None)
	
	def estadoPresonando(self):
		data = comunicacion.consultaI2C()
		if data[0] is '1':
			if data[1] is 'D':
				self.ingresaContrasena(data)
		self.contadorSonador+=1
		print "el contador esta en", self.contadorSonador
		if(self.contadorSonador > 15):
			self.contadorSonador = 0
			self.ea = estadoAlarma.Sonando
			comunicacion.notificaI2C("R4T")
			self.notificaEstadoWeb()
				
	
	# consultamos unicamente el I2C para validar que nos entregan datos desde la discovery
	def estadoSonando(self):
		data = comunicacion.consultaI2C()
		if data[0] is '1':
			if data[1] is 'D':
				self.ingresaContrasena(data)

	def run(self):
		while True:
			if self.matar is True:
				break
			# Obtenemos el estado de la web para validar si hay que realizar un cambio previamente
			self.consultaEstadoWeb()
				
			if self.ea is estadoAlarma.Inactiva:
				self.estadoInactivo()
			elif self.ea is estadoAlarma.Preactiva:
				self.estadoPreactiva()
			elif self.ea is estadoAlarma.Activa:
				self.estadoActiva()
			elif self.ea is estadoAlarma.Presonando:
				self.estadoPresonando()
			elif self.ea is estadoAlarma.Sonando:
				self.estadoSonando()

			time.sleep(1)