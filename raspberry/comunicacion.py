import smbus
import time
import threading

class comunicacionArduino (threading.Thread):

	def __init__(self):
		threading.Thread.__init__(self)
		self.bus     = smbus.SMBus(1)
		self.address = 0x04
		self.matar = False
		self.data =""
		self.caracter =""
		

	def setAlarma(self,al):
		self.alarma  = al

	def notificarActivacion(self):
		print "Notificando activar alarma"
	
	def notificarPreactivacion(self):
		print "Notificando preactivacion Alarma"

	def notificarDesactivacion(self):
		print "Notificando deacticar Alarma"

	def notificarMovimiento(self):
		print "Notificando movimiento"
		#self.bus.write_i2c_block_data(self.address, ord('R'), [ord('3'), ord('T'), 0])
		
		value="R3T"
		for c in value:
			self.bus.write_byte(self.address, ord(c))

	def readNumber(self):
		caracter = ''
		while caracter is not ord('T'):
			caracter = self.bus.read_byte(self.address)
			time.sleep(1)
			if caracter is 0:
				continue
			self.data    +=str(chr(caracter))
			print "el caracter  es ", chr(caracter)
		print "la data es ", self.data
		return self.data

	def run(self):
		while True:

			valores = self.readNumber()
			if valores[0] is 'A' and valores[1] is '1':
				self.notificarMovimiento()

			if self.matar is True:
				break
			time.sleep(10)