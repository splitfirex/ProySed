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
String caracter = " ";
int estadoAlarma = 2;
int contadorPreActivacion = 0;
int valorSensor1 = 0;
int valorSensor2 = 0;
int pirState = LOW;

void setup(){

  // Definimos el estado de los pines
  pinMode(sensor1Pin, INPUT);
  pinMode(sensor2Pin, INPUT);
  pinMode(sensorledPin, OUTPUT);    
  pinMode(zumbadorpin, OUTPUT);

  // Establecemos la comunicacion del I2C
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);

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

// Se cuenta hasta 15 segundos y luego se activa la alarma 
void estadoPreActivo(){

  digitalWrite(sensorledPin, HIGH);
  delay(500);
  digitalWrite(sensorledPin, LOW);
  delay(500);
  contadorPreActivacion++;
  if(contadorPreActivacion > 15){
    estadoAlarma = 2;  // activamos la alarma
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
      notificarMovimiento();
      pirState = HIGH;
    }

  } else {
    delay(300);    
    if (pirState == HIGH){
      pirState = LOW;
    }
  }

}

// duration in mSecs, frequency in hertz
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

void estadoSonando(){

  digitalWrite(sensorledPin, HIGH); // turn LED OFF
  playTone(300, 160);
  delay(500);
  digitalWrite(sensorledPin, LOW);
  delay(500);

}

// callback para la recepcion de la data
void receiveData(int byteCount){

  caracter = "";
  while(Wire.available()) {
    caracter += (char)Wire.read();
  }

  Serial.print("data recibida: ");
  Serial.println(data);
   if(caracter.charAt(0) != 'T'){ // Si el componente es Discovery reenviamos a la RASP
    data += caracter;
    return;
  }

  
  char componente   = data.charAt(0);
  char valorMensaje = data.charAt(1);
  
  //int index = 0;
  //while(Wire.available()) {
  //data[index] += (char)Wire.read();
  //  if(index >=4){ break; }
  //}
  
  //int componente = data[0];
  //int valorMensaje = data[1];
  
  Serial.print("componente: ");
  Serial.println(componente);
  Serial.print("valorMensaje: ");
  Serial.println(valorMensaje);
  

  if(componente == 'D'){ // Si el componente es Discovery reenviamos a la RASP
    sendData();
    return;
  }

  switch(estadoAlarma){
    case 0 :
      if(valorMensaje == '1'){
        estadoAlarma = 1;
        contadorPreActivacion = 0; // reinicializamos el contador de repeticiones
      }
      break;
    case 1 :
      if(valorMensaje == '2'){
        estadoAlarma = 2;
      }
      break;
    case 2 :
      if(valorMensaje == '3'){
        estadoAlarma = 3;
      }
      if(valorMensaje == '0'){
        estadoAlarma = 0;
      }
      break;
    case 3 :
      if(valorMensaje == '0'){
        estadoAlarma = 0;
      }
      break;
    default:
      estadoAlarma = 0;
      break;
  }
    data = "";

}

// metodo para notificar movimiento
void notificarMovimiento(){
  movimientoDetectado = 1;
  Serial.println("Movimiento detectado");
}

// callback for sending data
char* palabra = "A1T";
int index = 0;
void sendData(){
  
  if(movimientoDetectado == 1){
    Wire.write(palabra[index]);
    //Wire.write(palabra,3);
    ++index;
    if (index>=4){
      index=0;
       movimientoDetectado = 0;
      }
   
  }
 //Serial.println(data);

}
