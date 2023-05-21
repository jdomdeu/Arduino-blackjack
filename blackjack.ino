#include <Wire.h>

/* Definimos los modos ---------------------------- */
#define MODO_CARTERA             0
#define MODO_JUGAR               1
#define MODO_JUGANDO             2
#define MODO_PLANTARSE           3
#define DEALER_JUEGA             4
#define PERDER                   5
#define COMPARAR                 6
#define GANAR                    7
#define EMPATE                   8
#define GAME_OVER                9

/* Todo lo necesario para el funcionamiento del LCD */
#define LCD_I2C_ADDR        0x3E

#define LCD_MODE_COMMAND    0x00
#define LCD_MODE_DATA       0x40

#define LCD_CLEARDISPLAY    0x01
#define LCD_RETURNHOME      0x02
#define LCD_ENTRYMODESET    0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT     0x10
#define LCD_FUNCTIONSET     0x20
#define LCD_SETCGRAMADDR    0x40
#define LCD_SETDDRAMADDR    0x80

#define LCD_8BITMODE        0x10
#define LCD_4BITMODE        0x00
#define LCD_2LINE           0x08
#define LCD_1LINE           0x00
#define LCD_5x10DOTS        0x04
#define LCD_5x8DOTS         0x00

#define LCD_DISPLAYON       0x04
#define LCD_DISPLAYOFF      0x00
#define LCD_CURSORON        0x02
#define LCD_CURSOROFF       0x00
#define LCD_BLINKON         0x01
#define LCD_BLINKOFF        0x00

#define LCD_ENTRYRIGHT      0x00
#define LCD_ENTRYLEFT       0x02
#define LCD_ENTRYSHIFTINC   0x01
#define LCD_ENTRYSHIFTDEC   0x00

/* Variables ---------------------------------------------------------- */
const int start_button = 5;
int valorPulsador1 = 0;

const int boton_pedir = 8;
int valorPulsador2 = 0;

const int boton_plantarse = 7;
int valorPulsador3 = 0;

int cartera = 100;            // Dinero con el que se empieza
int apuesta = 10;             // Apuesta inicial

String valor_dinero;
String valor_dinero2;

int apuesta2;

int mode = 0; 

/* Variables BlackJack ---------------------------------------------------------- */
String carta_str;
String carta1_str;
String carta2_str;

String carta_d_str;
String carta1_d_str;

int resultado;
String resultado_str;

int resultado_dealer;
String resultado_dealer_str;

/* LCD ---------------------------------------------------------- */

void lcdSend(unsigned char value, unsigned char mode) {
  Wire.beginTransmission(LCD_I2C_ADDR);
  Wire.write(mode);
  Wire.write(value);
  Wire.endTransmission();
  delayMicroseconds(50);
}

void lcdSetup() {
  Wire.begin();
  delay(50);

  lcdSend(LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS, LCD_MODE_COMMAND);
  lcdSend(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF, LCD_MODE_COMMAND);
  lcdSend(LCD_CLEARDISPLAY, LCD_MODE_COMMAND);
  delayMicroseconds(2000);
  lcdSend(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDEC, LCD_MODE_COMMAND);

  Serial.begin(9600);
}

void lcdClear() {
  lcdSend(LCD_CLEARDISPLAY, LCD_MODE_COMMAND);
  delayMicroseconds(2000);
}

void lcdSetCursor( int x, int y) {
  unsigned char value = 0;

  if (y > 0) {
    value = LCD_SETCGRAMADDR;
  }
  value += x;

  lcdSend(LCD_SETDDRAMADDR | value, LCD_MODE_COMMAND);
}

void lcdPrint(const char *str) {
  while (*str != 0) {
    lcdSend(*str, LCD_MODE_DATA);
    str++;
  }
}
/** Crear cartas aleatorias --------------------------------------------------- **/

int aleatorio_en_rango(int minimo, int maximo) {
  return minimo + rand() / (RAND_MAX / (maximo - minimo + 1) + 1);
}

int numero;
int numero1;
int numero_d;
int numero1_d;
int numero_d2;

String numero_d2_str;


/** Empiezan las funciones --------------------------------------------------- **/

void bajar_apuesta(){                 // Las apuestas se bajan de 10 en 10
  apuesta = apuesta - 10;
  if (apuesta < 10) apuesta = 10;     // No se puede apostar menos de 10
}

void subir_apuesta(){                 // Las apuestas se suben de  10 en 10
  apuesta = apuesta + 10;
  if (apuesta > cartera) apuesta = cartera;     // No se puede apostar más de la cartera
}


void modo_cartera(){                  // Enseña la cartera y se apuesta
  lcdSetCursor(0, 0);
  String dinero_str;
  dinero_str = "Tienes "+(String)cartera+" $";
  lcdPrint(dinero_str.c_str());

  lcdSetCursor(0, 1);
  String apuesta_str;
  apuesta_str = "Apuestas: "+(String)apuesta;
  lcdPrint(apuesta_str.c_str());
  
  if(valorPulsador1==LOW){    // BAJAR APUESTA
    bajar_apuesta();
    delay(100);
    lcdClear();
    }
  if (valorPulsador2==LOW){  // SUBIR APUESTA
    subir_apuesta();
    delay(100);
    lcdClear();
  }
  if (valorPulsador3 == LOW) {   // EMPEZAMOS A JUGAR
    lcdClear();
    mode = MODO_JUGAR;
  }
  
}

void pinta(){                   // Ahorrar suciedad visual
  lcdSetCursor(0, 0);
  lcdPrint("J: ");
  lcdSetCursor(3, 0);
  lcdPrint(carta_str.c_str());
  lcdSetCursor(6, 0);
  lcdPrint(carta1_str.c_str());
  lcdSetCursor(14, 0);
  lcdPrint(resultado_str.c_str());

  lcdSetCursor(0, 1);
  lcdPrint("D: ");
  lcdSetCursor(3, 1);
  lcdPrint(carta_d_str.c_str());
  
  lcdSetCursor(6, 1);
  lcdPrint("?");
  
  lcdSetCursor(14, 1);
  lcdPrint(resultado_dealer_str.c_str());
  
}

void pinta_sin_dealer(){         // Ahorrar suciedad visual
  lcdSetCursor(0, 0);
  lcdPrint("J: ");
  lcdSetCursor(3, 0);
  lcdPrint(carta_str.c_str());
  lcdSetCursor(6, 0);
  lcdPrint(carta1_str.c_str());
  lcdSetCursor(9, 0);
  lcdPrint(carta2_str.c_str());
  lcdSetCursor(14, 0);
  lcdPrint(resultado_str.c_str());

  lcdSetCursor(0, 1);
  lcdPrint("D: ");
  lcdSetCursor(3, 1);
  lcdPrint(carta_d_str.c_str());
}

void modo_plantarse() {    // hay que borrar,volver a poner todo y el dealer juega
  lcdClear();
  pinta_sin_dealer();     //  Cartas del jugador y la primera del dealer (no tienen que cambiar)
  lcdSetCursor(6, 1);
  carta1_d_str = numero1_d;   // Nueva carta del dealer
  
  lcdPrint(carta1_d_str.c_str());
  resultado_dealer = resultado_dealer + numero1_d;
  delay(100);
  
  resultado_dealer_str = resultado_dealer;
  lcdSetCursor(14, 1);
  lcdPrint(resultado_dealer_str.c_str());
  
  mode = DEALER_JUEGA;          // El dealer decidirá que hacer
}

void comparar(){               // Ver resultados
  delay(700);                                     // Que se pueda ver que ha pasado
  lcdClear();
  if (resultado_dealer > 21){                     // El dealer ha superado 21
      mode = GANAR;}
  if (resultado_dealer == 21){                    // BlackJack del dealer
      mode = PERDER;}
  if (resultado_dealer > resultado && resultado_dealer<=21){   // Dealer nos gana y tiene menos de 21
    mode = PERDER;
  }
  if (resultado > resultado_dealer){                      // Ganamos al dealer (no hace falta poner que tenemos <21)
    mode = GANAR;
  }
  if (resultado == resultado_dealer){                     // Mismo resultado
    mode = EMPATE;
  }
  
}

void perder(){ 
  valor_dinero = apuesta;                // Se pierde lo apostado

  lcdSetCursor(0, 0);
  String perdida;
  perdida = "Perdiste "+valor_dinero+" $";
  lcdPrint(perdida.c_str());

  lcdSetCursor(0, 1);
  lcdPrint("Pulse boton 3");

  
  if ((cartera-apuesta) < 10){          // No se puede hacer la apuesta mínima
      delay(250);
      lcdClear();
      mode = GAME_OVER;
    }
  
  if(valorPulsador3==LOW){    // Volver a jugar
    lcdClear();
    cartera = cartera - apuesta;
    apuesta = 10;
    resultado = 0;
    resultado_dealer = 0;
    delay(100);
    
    mode = MODO_CARTERA;
    }
}

void ganar(){
  apuesta2 = apuesta*2;       // Se gana lo doble de lo apostado
  valor_dinero2 = apuesta2;

  lcdSetCursor(0, 0);
  String ganaste;
  ganaste = "Ganaste "+valor_dinero2+" $";
  lcdPrint(ganaste.c_str());

  lcdSetCursor(0, 1);
  lcdPrint("Pulse boton 3");
  
  if(valorPulsador3==LOW){    // Volver a jugar
    lcdClear();
    cartera = cartera + apuesta2;
    apuesta = 10;
    resultado = 0;
    resultado_dealer = 0;
    delay(100);
    
    mode = MODO_CARTERA;
    }
  
}

void empate(){               // Ni se gana ni se pierde
  
  lcdSetCursor(0, 0);
  lcdPrint("Empate");

  lcdSetCursor(0, 1);
  lcdPrint("Pulse boton 3");
  
  if(valorPulsador3==LOW){    // Volver a jugar
    lcdClear();
    apuesta = 10;
  
    resultado = 0;
    resultado_dealer = 0;
    delay(100);
    
    mode = MODO_CARTERA;
    }
}

void game_over(){          // Se piede del todo
  lcdSetCursor(4, 0);
  lcdPrint("GAME OVER");
  
  lcdSetCursor(0, 1);
  lcdPrint("Jeff Bezos gana");
}

void dealer_juega(){    // Automatizar

  if (resultado_dealer >= resultado){         // DEALER > JUGADOR
            Serial.println("dealer mayor");
            mode = COMPARAR;              
      }
  
   if (resultado_dealer >= 19){          // NO SE SABE QUIEN HA GANADO
      Serial.println("más de 19");
      mode = COMPARAR;
    }
  
  while (resultado_dealer < resultado && resultado_dealer <= 19){    // Seguirá jugando mientras tenga menos que el jugador y menos de 19
    numero_d2 = aleatorio_en_rango(1,10);
    delay(2000);                                                     // 2 segundos para poner tensión y que se vea lo que pasa
    Serial.println("---------------");
    numero_d2_str = numero_d2;
    
    lcdSetCursor(9, 1);                                             // Cada vez va jugando una carta
    lcdPrint(numero_d2_str.c_str());
    resultado_dealer = resultado_dealer+numero_d2;
    Serial.println(resultado_dealer);
    resultado_dealer_str = resultado_dealer;
    
    pinta_sin_dealer();                                             // Para que se vean las cartas de antes
    lcdSetCursor(6, 1);
    lcdPrint(carta1_d_str.c_str());
    lcdSetCursor(14, 1);
    lcdPrint(resultado_dealer_str.c_str());
  }
  
}

void modo_jugando() {
  pinta();                                                           // Cartas del jugador y la secreta del dealer

  if (resultado > 21){                                               // No podemos superar 21
    lcdClear();
    mode = PERDER;
  }
  
  if(valorPulsador2==LOW){                                    // Nueva carta
    carta2_str = numero;
    lcdSetCursor(9, 0);
    lcdPrint(carta2_str.c_str());

    resultado = resultado+numero;
    resultado_str = resultado;
    Serial.println(numero);
    Serial.println(resultado);
    lcdSetCursor(14, 0);
    lcdPrint(resultado_str.c_str());
    delay(100);                         //De esta forma solo sale un número (si no hay delay salen 10 a la vez)
    }

  if (resultado == 21){                // BlackJack
    lcdClear();
    mode = GANAR;
  }

  if(valorPulsador3==LOW){    // No más cartas
    mode = MODO_PLANTARSE;
    }

  
}

void modo_jugar(){
  lcdSetCursor(0, 0);
  lcdPrint("Barajar cartas");
  lcdSetCursor(0, 1);
  lcdPrint("Pulse boton 1");
  
  if(valorPulsador1==LOW){    // EMPIEZAS A JUGAR
    lcdClear();
    
    carta_str = numero;
    lcdSetCursor(0, 0);
    lcdPrint("J: ");
    lcdSetCursor(3, 0);
    lcdPrint(carta_str.c_str());
    resultado = resultado + numero;
    Serial.println(numero);
  

   
    carta1_str = numero1;
    lcdSetCursor(6, 0);
    lcdPrint(carta1_str.c_str());
    resultado = resultado + numero1;
    lcdSetCursor(14, 0);
    resultado_str = resultado;
    lcdPrint(resultado_str.c_str());
    Serial.println(numero1);
    Serial.println(resultado);

    carta_d_str = numero_d;
    lcdSetCursor(0, 1);
    lcdPrint("D: ");
    lcdSetCursor(3, 1);
    lcdPrint(carta_d_str.c_str());
    
    lcdSetCursor(6, 1);
    lcdPrint("?");
    
    resultado_dealer = resultado_dealer + numero_d;
    
    resultado_dealer_str = resultado_dealer;
    lcdSetCursor(14, 1);
    lcdPrint(resultado_dealer_str.c_str());
    
    mode = MODO_JUGANDO;
    }
}

/** setup y loop --------------------------------------------------- **/
void setup() {
  Serial.begin(9600);
  lcdSetup();
  pinMode(start_button, INPUT_PULLUP);
  pinMode(boton_pedir, INPUT_PULLUP);
  pinMode(boton_plantarse, INPUT_PULLUP);
  randomSeed(analogRead(0));                    // Esto creo que no se usa, NO BORRAR por si las moscas
}

void loop() {
  numero = aleatorio_en_rango(1,10);
  numero1 = aleatorio_en_rango(1,10);
  
  numero_d = aleatorio_en_rango(1,10);
  numero1_d = aleatorio_en_rango(1,10);
  
  valorPulsador1 = digitalRead(start_button);
  valorPulsador2 = digitalRead(boton_pedir);
  valorPulsador3 = digitalRead(boton_plantarse);

  switch (mode) {
    case MODO_CARTERA:
      modo_cartera();
      break;
    case MODO_JUGAR:
      modo_jugar();
      break;
    case MODO_JUGANDO:
      modo_jugando();
      break;
    case MODO_PLANTARSE:
      modo_plantarse();
      break;
    case DEALER_JUEGA:
      dealer_juega();
      break;
    case PERDER:
      perder();
      break;
    case COMPARAR:
      comparar();
      break;
    case GANAR:
      ganar();
      break;
    case EMPATE:
      empate();
      break;
    case GAME_OVER:
      game_over();
      break;
  }
}
