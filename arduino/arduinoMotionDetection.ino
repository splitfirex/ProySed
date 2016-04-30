#include <Wire.h>
#define SLAVE_ADDRESS 0x04

//Sensores, zumbador y led
int sensorledPin = 13;          
int sensor1Pin = 2;     
int sensor2Pin = 6;
int zumbadorpin = 10;

// Estado de la alarma
int movimientoDetectado = 0;
String data = "";
// char data[4];
String caracter = "";
int estadoAlarma = 2;
int contadorPreActivacion = 0;
int valorSensor1 = 0;
int valorSensor2 = 0;
int pirState = LOW;
// variables comunicacion discovery
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup(){

  // Definimos el estado de los pines
  pinMode(sensor1Pin, INPUT);
  pinMode(sensor2Pin, INPUT);
  pinMode(sensorledPin, OUTPUT);    
  pinMode(zumbadorpin, OUTPUT);

  // Establecemos la comunicacion UART con stm32
  Serial.begin(9600);
  inputString.reserve(200); 
    
  // Establecemos la comunicacion I2C con la RASP
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);

}

// Lectura de eventos de UART
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == 'T') {
      stringComplete = true;
    }
  }
}

void loop(){

  switch(estadoAlarma){
    case 0 :
      // No se hace nada en el estado inactivo
    break;
    case 1 :
      estadoPreActivo();
    break;
    case 2 :
      estadoActivo();
    break;
    case 3 :
      estadoSonando();
    break;
    default:
    estadoAlarma = 0;
    break;
  }
}

// Ejecuta el sonido de la alarma
void playTone(long duration, int freq) {
    duration *= 1000;
    int period = (1.0 / freq) * 1000000;
    long elapsed_time = 0;
    while (elapsed_time < duration) {
        digitalWrite(zumbadorpin,HIGH);
        delayMicroseconds(period / 2);
        digitalWrite(zumbadorpin, LOW);
        delayMicroseconds(period / 2);
        elapsed_time += (period);
    }
}

// Solo suena y prende los led correspondientes
void estadoSonando(){

  digitalWrite(sensorledPin, HIGH); // turn LED OFF
  playTone(300, 160);
  delay(500);
  digitalWrite(sensorledPin, LOW);
  delay(500);

}

// Se cuenta hasta 15 segundos y luego se activa la alarma 
void estadoPreActivo(){

  digitalWrite(sensorledPin, HIGH);
  delay(500);
  digitalWrite(sensorledPin, LOW);
  delay(500);
  contadorPreActivacion++;
  if(contadorPreActivacion > 15){
    estadoAlarma = 2;  // activamos la alarma
    Serial.print("nuevo estado: Preactivo ");
  }

}

// Leemos los sensores de movimiento y realizamos la notificacion a la RASP
void estadoActivo(){

  valorSensor1 = digitalRead(sensor1Pin); 
  valorSensor2 = digitalRead(sensor2Pin);

  //Validamos si en alguno de ellos hay movimiento
  if (valorSensor1 | valorSensor2 == HIGH) {  
    delay(150);
    if (pirState == LOW) {
      movimientoDetectado = 1;
      pirState = HIGH;
    }

  } else {
    delay(300);    
    if (pirState == HIGH){
      pirState = LOW;
    }
  }

}

// callback para la recepcion de la data
void receiveData(int byteCount){

  caracter = "";
  while(Wire.available()) {
    caracter += (char)Wire.read();
  }
  Serial.print("mensaje: ");
  Serial.println(caracter);
  char valorMensaje = caracter.charAt(2);
  Serial.print("valorMensaje: ");
  Serial.println(valorMensaje);
 

  switch(estadoAlarma){
    case 0 :
      if(valorMensaje == '1'){
        estadoAlarma = 1;
        contadorPreActivacion = 0; // reinicializamos el contador de repeticiones
        Serial.print("nuevo estado: Preactivo ");
      }
      break;
    case 1 :
      if(valorMensaje == '2'){
        estadoAlarma = 2;
        Serial.print("nuevo estado: activo ");
      }
      break;
    case 2 :
      if(valorMensaje == '3'){
        estadoAlarma = 3;
        Serial.print("nuevo estado: sonando ");
      }
      if(valorMensaje == '0'){
        estadoAlarma = 0;
        Serial.print("nuevo estado: inactivo ");
      }
      break;
    case 3 :
      if(valorMensaje == '0'){
        estadoAlarma = 0;
        Serial.print("nuevo estado: inactivo ");
      }
      break;
    default:
      break;
  }

}


void sendData(){
  
  if(stringComplete){
    char respuesta[32] = "000T";
    inputString.toCharArray(respuesta,32);
    Wire.write(respuesta);
    inputString = "";
    stringComplete = false;   
  }
  else if(movimientoDetectado == 1){
    Wire.write("1A1T");   
    movimientoDetectado =0;
  }
  else{
    Wire.write("000T");    
  }
  
}
