#!/bin/bash

cd webserver &&
python -m SimpleHTTPServer 8080 && 
cd .. && cd raspberry && 
python motion_detector.py && python restServer.py 8081
