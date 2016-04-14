import argparse
import datetime
import imutils
import time
import cv2
import logging
import ConfigParser
import threading
import sys
from comunicacion import comunicacionArduino

format= '%(asctime)s - %(levelname)s - %(filename)s:%(lineno)s - %(funcName)s() - %(message)s'
format= '%(asctime)s - %(filename)s:%(lineno)s - %(message)s'
logging.basicConfig(level=logging.DEBUG, format=format)
logger = logging.getLogger(__name__)

class estadoAlarma:
	Inactiva  = 0
	Preactiva = 1
	Activa	  = 2
	Sonando	  = 3


class alarmaMonitor (threading.Thread):

	def __init__(self):
		threading.Thread.__init__(self)
		self.ea = estadoAlarma.Activa
		self.matar = False
		self.movimiento = False
		self.alarma = ''
		self.comm = comunicacionArduino()
		self.comm.setAlarma(self)
		self.comm.start()

	def setMovimiento(self,mov,frame):
		self.movimiento = True
		self.alarma = mov

		cv2.putText(frame, "Camara : {}".format(self.alarma), (10, 20),
		cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
		cv2.putText(frame, datetime.datetime.now().strftime("%A %d %B %Y %I:%M:%S%p"),
		(10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)
		print "/home/pi/shared/frames/{0}-{1}.jpg".format(self.alarma,datetime.datetime.now().strftime("%Y-%b-%d-%I-%M-%S%p"))
		cv2.imwrite("/home/pi/shared/frames/{0}-{1}.jpg".format(self.alarma,datetime.datetime.now().strftime("%Y-%b-%d-%I-%M-%S%p")),frame)

			# aprovecho a almacenar en el historico las alarmas correspondientes en el caso que vengan de sensores

	def run(self):
		while True:
			if self.matar is True:
				self.comm.matar = True
				break

			if self.movimiento is True and self.ea is estadoAlarma.Activa:
				logger.debug("Se detecto movimiento en : {} ; Estableciendo alarma !!!".format(self.alarma))
				self.comm.notificarMovimiento()
				self.ea = estadoAlarma.Sonando

			if self.ea is estadoAlarma.Sonando:
				logger.debug("ALARMA!!!")

			#logger.debug("tick alarma movimiento : {}, ea {}".format(self.movimiento,self.ea) )
			time.sleep(1)