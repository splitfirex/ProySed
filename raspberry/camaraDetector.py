import datetime
import imutils
import cv2
import threading
from alarmaMonitor import alarmaMonitor

class camaraDetector (threading.Thread):

	def __init__(self,_nombre,_url,alarma):
		threading.Thread.__init__(self)
		self.URL = _url
		self.nombre = _nombre
		self.alarm = alarma
		self.firstFrame = None
		self.matar = False
		
	def tomarCamara(self):
		self.camera = cv2.VideoCapture(self.URL)
		if self.camera.isOpened() is False:
			print "La camara : {0} no ha sido tomada".format(self.nombre)
			return True
		print "La camara : {0}, ha sido tomada con exito".format(self.nombre)
		return False

	def run(self):
		
		self.matar = self.tomarCamara()
		
		while True:

			if self.matar is True:
				break

			(grabbed, frame) = self.camera.read()
			#print "{0} frame {1} ".format(self.nombre,datetime.datetime.now().strftime("%A %d %B %Y %I:%M:%S%p"))
			# valida si es posible tomar la ruta 
			if not grabbed:
				print "no se encuentra tomada la camara %s " %self.nombre 
				break

			frame = imutils.resize(frame, width=500)
			gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
			gray = cv2.GaussianBlur(gray, (21, 21), 0)

			# establece el primer cuadro contra que comprar
			if self.firstFrame is None:
				self.firstFrame = gray
				continue

			frameDelta = cv2.absdiff(self.firstFrame, gray)
			thresh = cv2.threshold(frameDelta, 25, 255, cv2.THRESH_BINARY)[1]

			thresh = cv2.dilate(thresh, None, iterations=2)
			(cnts, _) = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
			cv2.CHAIN_APPROX_SIMPLE)

			for c in cnts:
				if cv2.contourArea(c) < 500:
					continue

				(x, y, w, h) = cv2.boundingRect(c)
				cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
				self.alarm.setMovimiento(self.nombre,frame)
				self.firstFrame = gray			

			# show the frame and record if the user presses a key
			# cv2.imshow("Security Feed", frame)
			# cv2.imshow("Thresh", thresh)
			# cv2.imshow("Frame Delta", frameDelta)
		self.camera.release()
		
