# USAGE
# python motion_detector.py
# python motion_detector.py --video videos/example_01.mp4

# import the necessary packages
import ConfigParser
from camaraDetector import camaraDetector
from alarmaMonitor import alarmaMonitor

Alarma = False

def main_loop():
	config = ConfigParser.RawConfigParser()
	config.read('camarasConfig.properties')
	rutas = config.get('Camara', 'camara.rutas').split()
	nombres = config.get('Camara', 'camara.nombres').split()
	folder = config.get('Camara', 'camara.folder')

	threads = list()
	
	Alarma = alarmaMonitor(folder)
	threads.append(Alarma)
	Alarma.start()


	for i in range(len(rutas)):
		camara = camaraDetector(nombres[i],rutas[i],Alarma)
		threads.append(camara)
		camara.start()

	raw_input("Press Enter to continue...")
	for hilos in threads:
		hilos.matar = True
	#while len(threads) > 0:
	#	try:
	#		threads = [t for t in threads if t is not None and t.isAlive()]
	#	except KeyboardInterrupt:
	#		print "Ctrl-c received! Sending kill to threads..."
	#		for hilos in threads:
	#			hilos.matar = True

if __name__ == '__main__':
    main_loop()

