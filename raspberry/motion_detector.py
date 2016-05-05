# USAGE
# python motion_detector.py
# python motion_detector.py --video videos/example_01.mp4

# import the necessary packages
import ConfigParser
from camaraDetector import camaraDetector
from alarmaMonitor import alarmaMonitor
import time
import os

Alarma = False

def main_loop():
	os.system("cd /home/pi/ProySed/webserver/ && python app.py &")
	time.sleep(2)
	config = ConfigParser.RawConfigParser()
	config.read('camarasConfig.properties')
	rutas = config.get('Camara', 'camara.rutas').split()
	nombres = config.get('Camara', 'camara.nombres').split()
	params = {"framesFolder" : config.get('Camara', 'camara.framesfolder'),"weburl" : config.get('Camara', 'camara.weburl'), "urlFramesfolder": config.get('Camara', 'camara.urlFramesfolder') }
	#framesFolder = config.get('Camara', 'camara.framesfolder')
	#urlweb = config.get('Camara', 'camara.weburl')

	threads = list()
	
	Alarma = alarmaMonitor(params)
	threads.append(Alarma)
	Alarma.start()


	for i in range(len(rutas)):
		camara = camaraDetector(nombres[i],rutas[i],Alarma)
		threads.append(camara)
		camara.start()

	raw_input("Press Enter to continue...")
	for hilos in threads:
		hilos.matar = True
	os.system("pkill -9 python")

if __name__ == '__main__':
    main_loop()

