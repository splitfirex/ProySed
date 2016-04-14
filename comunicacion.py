import smbus
import time
import threading

class comunicacionArduino (threading.Thread):

	def __init__(self):
		threading.Thread.__init__(self)
		#self.bus     = smbus.SMBus(1)
		self.address = 0x04
		self.matar = False
		

	def setAlarma(self,al):
		self.alarma  = al

	def notificarActivacion(self):
		print "Notificando activar alarma"
	
	def notificarPreactivacion(self):
		print "Notificando preactivacion Alarma"

	def notificarMovimiento(self):
		print "Notificando movimiento"

	def notificarDesactivacion(self):
		print "Notificando deacticar Alarma"

	def writeNumber(value):
		bus.write_byte(address, value)
		return -1

	def readNumber():
		number = bus.read_byte(address)
		return number

	def run(self):
		while True:

			if self.matar is True:
				break
			time.sleep(10)