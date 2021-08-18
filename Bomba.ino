#include <SevSeg.h> //Biblioteca do display
int estado = 0; // Flag da maquina de estado
int porta[5] = {0,12,A3,A0,A1}; //corta fios
int botao = A2;
int rele = 13; //

//check de portas
int portaComProblema = 0;
int somaMinutos = 1;
int somaHoras = 0;

int fioCortado = 0;

//display
SevSeg display7seg;
int contador = 59; //contador regressivo
unsigned long timer;
char tempString[10]; //Used for sprintf no DISPLAY

//botao
int count;    // How long the button was held (secs)
byte previous = HIGH;
unsigned long firstTime;
int current;

//estado configura fios
//int configuracaoFio[5]= {0, 3, 2, 4, 1}; //armazena configuração dos fios - TIRAR RECEBIMENTO DE VALORES EM PROD
int configuracaoFio[5]; //armazena configuração dos fios
int sequenciaDeFios = 1; //1-fio 1;2-fio 2;3-fio 3;4-fio 4
int tipoConfiguracao = 1; //1-explodir;2-desarmar;3-contagem rapida

//estado armada
unsigned long velocidadeContador = 60000;
int jaCortado[5]={0,0,0,0,0};

void setup() {
  //CORTA FIOS
  pinMode(porta[1], INPUT_PULLUP);
  pinMode(porta[2], INPUT_PULLUP);
  pinMode(porta[3], INPUT_PULLUP);
  pinMode(porta[4], INPUT_PULLUP);

  pinMode(botao, INPUT_PULLUP);

  //Display
  int displayType = COMMON_ANODE; 
  //Definicao dos pinos dos digitos
  int digit1 = A4; //Pino Digito1 do display 12
  int digit2 = 11; //Pino Digito2 do display 9
  int digit3 = 2; //Pino Digito3 do display 8
  int digit4 = 4; //Pino Digito4 do display 6
  //Pinos ligados aos segmentos A - H
  int segA = A5; //Pino segmento A  11
  int segB = 3; //Pino segmento B 7
  int segC = 6; //Pino segmento C 4
  int segD = 8; //Pino segmento D 2
  int segE = 9; //Pino segmento E 1
  int segF = 10; //Pino segmento F 10
  int segG = 5; //Pino segmento G 5
  int segDP= 7; //Pino segmento H 3
  //Define o numero de digitos do display
  int numberOfDigits = 4;
  //Inicializa o display
  display7seg.Begin(displayType, numberOfDigits, digit1, digit2, digit3, digit4, segA, segB, segC, segD, segE, segF, segG, segDP);
  //Nivel de brilho do display
  display7seg.SetBrightness(100);
  timer = millis();

//  Serial.begin(9600);

  pinMode(rele,OUTPUT);
  digitalWrite(rele, HIGH);

  //temporario para programação ARMADA
  /*
  somaMinutos = 15;
  somaHoras = 0;
  */
}
void loop() { 
  current = digitalRead(botao); //botao
  switch (estado){
    case 0:  //Configurando tempo
      //Check que fiação
      
        if (fioCortado == 0) {
            for (int contaCheckPorta = 1; contaCheckPorta < 5; contaCheckPorta++){
                if (digitalRead(porta[contaCheckPorta]) == HIGH){
                    portaComProblema = contaCheckPorta + 1;
                    contaCheckPorta = 4;
                    fioCortado = 1;
                }
            }
            //INICIO BOTAO
            if (current == LOW && previous == HIGH && millis() - firstTime > 200) {
                firstTime = millis();
            }
            if (current == LOW && ((millis() - firstTime) % 1000) < 20 && millis() - firstTime > 4000){
                count = 10;
                estado = 1;
                delay(800);
                break;
            }  
            if (current == HIGH && previous == LOW && count >=0 && count < 3) {
                if(somaMinutos == 1){
                    somaMinutos = somaMinutos + 14;
                }else{
                    somaMinutos = somaMinutos + 15;
                }
                if (somaMinutos > 59){
                    somaMinutos = 0;
                    somaHoras = somaHoras + 1;
                }
                if (somaHoras == 5){
                    somaHoras = 0;
                }
            }
            if (current == HIGH) {
                count = 0;
            }
            previous = current;
            //TERMINO BOTAO
        }else{
            if (digitalRead(botao) != HIGH){
                fioCortado = 0;
            }
        }
        mostrarDisplay(0,1000,fioCortado,somaHoras,somaMinutos);
    break;
    case 1: //Configurando fios
    mostrarDisplay(0,1000,2,sequenciaDeFios,tipoConfiguracao);
            //INICIO BOTAO
        if (current == LOW && previous == HIGH && millis() - firstTime > 100) {
            firstTime = millis();
        }
        if (current == LOW && ((millis() - firstTime) % 1000) < 20 && millis() - firstTime > 4000){ //pressionar      
            if (sequenciaDeFios != 4){
                configuracaoFio[sequenciaDeFios]=tipoConfiguracao;
                tipoConfiguracao = 0;
                sequenciaDeFios++;
                delay(800);
                count++;
            }else{
                configuracaoFio[sequenciaDeFios]=tipoConfiguracao;
                sequenciaDeFios++;
                count=10;
                estado = 2;
                break;
            }
        }  
        if (current == HIGH && previous == LOW && count >=0 && count < 3) { //clicar
            if (tipoConfiguracao < 4){
                tipoConfiguracao++;  
            }else{
                tipoConfiguracao = 1;
            }
        }
        if (current == HIGH) {
            count = 0;
        }
        previous = current;
        //TERMINO BOTAO
    break;
    case 2: //ARMADA
        // INICIO CONTADOR REGRESSIVO
        sprintf(tempString, "%02d%02d", somaHoras, somaMinutos);
        display7seg.DisplayString(tempString, 2);
        if (millis() - timer >= velocidadeContador){
            timer = millis();
            somaMinutos--;
        }
        delay(5);
        if (somaMinutos == 0 && somaHoras == 0){
            estado = 3;
            break;
        }
        if (somaMinutos < 0){
            somaHoras = somaHoras-1;
            somaMinutos = 59;
        }
        //TERMINO CONTADOR REGRESSIVO

        //Verificar se tem fio cortado
        if (fioCortado == 0) {
            for (int contaCheckPorta = 1; contaCheckPorta < 5; contaCheckPorta++){
                if (digitalRead(porta[contaCheckPorta]) == HIGH && jaCortado[contaCheckPorta] != 1){
                    portaComProblema = contaCheckPorta ;
                    contaCheckPorta = 4;
                    fioCortado = 1;
                } 
            }
        }else{
            switch (configuracaoFio[portaComProblema]){
                case 1:
                    estado = 3;
                break;
                case 2:
                    estado = 4;
                break;
                case 3:
                    velocidadeContador = velocidadeContador / 2;
                    fioCortado = 0;
                    jaCortado[portaComProblema] = 1;
                break;
                default:
                    fioCortado = 0;
                break;
            }
        }

        
    break;
    case 3:
        digitalWrite(rele, LOW);
    break;
    default:
        tempString[0] = '-';
        tempString[1] = '-';
        tempString[2] = '-';
        tempString[3] = '-';
        display7seg.DisplayString(tempString, 0);
    break; 
  }
}
void mostrarDisplay(int formato, int tempo,int error, int digitos1, int digitos2){
  //formato sem nada = 0, hora = 2
  //tempo milisegundos do contador se tiver que incrementar ou decrementar
  //error 1 mostrar E
  if (error != 1){
    sprintf(tempString, "%02d%02d", digitos1, digitos2);
  }else{
    tempString[0] = 'E';
    tempString[1] = ' ';
    tempString[2] = ' ';
    tempString[3] = portaComProblema-1;
  }  
  display7seg.DisplayString(tempString, formato);
}


