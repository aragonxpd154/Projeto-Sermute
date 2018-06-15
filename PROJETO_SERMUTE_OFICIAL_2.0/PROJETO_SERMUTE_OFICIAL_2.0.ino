/* Biblioteca criada exclusivamente para tratamento do transistor LM35, disponivel no dominio publico
    http://aragones.esy.es
    Por Marcos S. (aragonxpd154)

    __Version___: 2.0ALPHA
    __License___: Public Domain
    __date______: 15/07/2017
    __title_____: Projeto SERMUTE

    NOTA: Inserindo leitura e gravacao na EEPROM caso um resert prematuro ou um desligamento desnecessario.

    OBS: Atenta-se que o cliclo para esse equipamento ARDUINO UNO sobre um ciclo loop contador da funcao EEPROM sao de 100.000 ciclos de leitura e escrita
    foi definido um intervalo de 6*0.5 dias para a gravacaoo na memoria EEPROM que resulta em 50.000 ciclo de leitura sobre o endereco 0 e 50.000 ciclo de
    gravacao, / por 2 ciclo a cada 1,5 possuimos cerca de 5.555,55556 ciclos de leitura + 5.555,55556 ciclo de gravacao que resulta numa escala de tempo
    aproximadamente ~2 anos e 6 dias 10 horas dentro de um intervalo de tempo do contador em funcao do intervalo de tempo de comutacao do mesmo. Nao foram
    levadas a probabilidade de funcionamento entre os periodos de manuntecao ou desligamento forcado, falta de energia etc...

*/
/* EEPROM CODE EM FASE DE TESTE E ADAPTACAO */

/* Desenvolvendo metodo para EEPROM gravacao do estado de comutacao pelo contador do MEC

*/

#include <LM35.h>
#include <limits.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

// Variavel tipoo constante definido
const int sensorTemp = A0;

// Variaveis dos LED informativos
/* Esses LED estarao no circuito/ placa, ele serao para informar que ocorreu a comutacao das
    chaves, ora, pois, LED VERDE indicarar chave 1 ligada constantemente e LED AMARELO ou outro
    LED VERDE, informando que chave 2 esta ligada constantemente

*/

// Variaveis da EEPROM determinantes para consulta

int endereco = 0; // Variavel de endereco de localizacao da memoria EEPROM
int dados = 0; // Variavel de dados a serem amarzenada na EEPROM
int VarTempEEPROM = 0; // Variavel usado para armazenar a leitura da EEPROM


// Variaveis de infomativo do circuito
//const int ledPin1 = 13;
//const int ledPin2 = 12;

// Variavel CHAVE MANUAL DETERMINISTICO
int BotaoManualAutomatico = 2;  // Pino de leitura da chave 1 e 0;
//int chaveAutManual = 0; // 0 Para automatico e 1 manual

long SetaLed = 7200000; // Valores de tempo de execucao da funcaoo exponencial.
long previsaoMillSegundos = 0; // Variavel de previsaoo milisegundos, que realizarar o inicio da contagem da funcao millis().
long intervaloBlack = 7200000; // Intervalo de comutacaoo descrita em (milisegundos) em funcao de millis().
int semana = 0; // Variavel tipo semana, determina o periodo em que vai comutar as chaves

// Variaveis controladora do tempo ligado dos equipamentos (CHAVE ??)

long acender2Min = 120000; // Tempo em ms de 15min de chave 1/chave 2 on
long intervaloInterrompido = 119999; // Intervalo de 15 min antes de entra novamente, para evitar alternanca de chave desnecessaria e aleatoria por variacao de tensao, caso ocorra uma harmonica no circuito ou qualquer coisa do genero
long previsaoMillSegundos15Min = 120000;
int temporizadorLogico = 0; // Variavel que determinara o atraso do contador de 30 min em outra funcao Millis() de 30 min para o delay das chaves comutadas

// Variaveis tipo temporaria
/* Essas variaveis sao para uso temporario, e seus valores iniciais nao sao reais, e soferam
    modificacoes ao longo do compilamento/depuramento do codigo fonte
*/

int valorSensorTemp = 0; // Variavel temporario do Sensor de temperatura #LM35
int menorValorTemp = INT_MAX; // Variavel para enxergar o maior valor MAXIMO
int valorTensao = 0; // Capturar o valor da tensao em A1 e exibir via Serial/ou qualquer entrada de parametros externos.

/* Variaveis tipo pulso ou PWMPin sao variaveis estacionaria e direcionada para seus pino correspondente
    que realiza a comutacao e contagem independente se o pulso do DIGI MEC estiver indisponivel dentro da
    escala de tempo abaixo.
*/
int pwmPin = 13; // Localizacao da chave (CHAVE 1)
int pwm2Pin = 12; // Localizacao da chave (CHAVE 2)

int valorCheckUp = 0; // Variavel temporaria para chegar se possui tensao de saida de 3.3V no DIGIMEC - DTWM-1

byte a[8] = {B00110, B01001, B00110, B00000, B00000, B00000, B00000, B00000,};

LiquidCrystal lcd(53, 51, 49, 47, 43, 33); //10, 5 ,4 ,3 ,11 ,2

void controle() {
  digitalWrite(pwmPin, HIGH);
  digitalWrite(pwm2Pin, HIGH);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  digitalWrite(pwmPin, LOW);
  digitalWrite(pwm2Pin, LOW);
}

void setup() {
  
  Serial.begin(9600); // Inicializa comunicação serial
  pinMode(pwmPin, OUTPUT);
  pinMode(pwm2Pin, OUTPUT);
  lcd.begin(16, 2); //Inicializa
  lcd.print("INICIALIZANDO");
  lcd.begin(16, 2);

  //Limpa a tela
  lcd.clear();
  lcd.print("Temperatura: ");
  //Atribui a "1" o valor do array "A", que desenha o simbolo de grau
  lcd.createChar(1, a);
  lcd.setCursor(7, 1); //Coloca o cursor na coluna 7, linha 1
  lcd.write(1); //Escreve o simbolo de grau
  lcd.setCursor(15, 0);
  lcd.write(1);
  lcd.setCursor(15, 1);
  lcd.write(1);
  Serial.begin(9600);
  pinMode (pwmPin, OUTPUT);
  pinMode (pwm2Pin, OUTPUT);
  pinMode (BotaoManualAutomatico, INPUT);
  //pushPwm = digitalRead(pushPwm); // ***EM TESTE
  Serial.print("LENDO MEMORIA EEPROM\n\n");
  VarTempEEPROM = (EEPROM.read(endereco));
  if (semana == VarTempEEPROM) {
    Serial.print("DADOS PREMATUROS\n\n");
  }
  else {
    semana = VarTempEEPROM;
  }

  if (semana == 0) {
    Serial.print("CHAVE 1 SEMPRE LIGADO ATE QUE COMUTE PWM NA VARIAVEL COMUTATIVA\n\n\n");
  }
  else {
    Serial.print("CHAVE 2 SEMPRE LIGADO ATE QUE COMUTE PWM NA VARIAVEL COMUTATIVA\n\n\n");
  }

  Serial.print("LIGANDO EQUIPAMENTOS\n");
  //controle();

  
}


void loop() {

  /* COndicao que vai determinar e alto afirmar a que o tempoOcorrido = a previsao calculada dentro do loop
  */


  unsigned long tempoOcorrido = millis(); // Declarando a funcao millis() diante da variavel tempoOcorrido
  unsigned long tempoOcorrido15Min = millis(); // Declarando a funcao contadora millis() diante da variavel tempoOcorrido15Min

  if (tempoOcorrido - previsaoMillSegundos > intervaloBlack) {
    // Salva o tempo anterior para referencial do tempo do LED sob o intervalo
    previsaoMillSegundos = tempoOcorrido;

    if (SetaLed == LOW) {
      SetaLed = HIGH;
      semana = 1;
      dados = 1; // Armazenando dado na variavel para enderecamento de memoria na EEPROM diante da condicao acima
      Serial.print("\nGRAVANDO DADOS ALTERADO DO PULSO SEMANAL NA MEMORIA EEPROM, AGUARDE");
      EEPROM.write(endereco, semana);
      delay(100);
      Serial.print("\n");
      Serial.print("ARMAZENADO COM SUCESSO");
    }
    else {
      SetaLed = LOW;
      semana = 0;
      dados = 0; // Armazenando dado na variavel para enderecamento de memoria na EEPROM diante da condicao acima
      Serial.print("\nGRAVANDO DADOS ALTERADO DO PULSO SEMANAL NA MEMORIA EEPROM, AGUARDE");
      EEPROM.write(endereco, semana);
      delay(100);
      Serial.print("\n");
      Serial.print("ARMAZENADO COM SUCESSO");
    }

    //digitalWrite(ledPin1, SetaLed);

    //Serial.print(semana); //**E TESTE

  }


  // Lendo o valor do sensor de temperatura
  // Definindo 18 leitura e o menor valor lido prevalece
  for (int i = 1; i <= 20; i++) {
    menorValorTemp = INT_MAX;
    valorSensorTemp = analogRead(sensorTemp);
    // Transformando valor lido no sensor de temperatura
    valorSensorTemp *= 0.54;
    if (valorSensorTemp < menorValorTemp) {
      menorValorTemp = valorSensorTemp;

      Serial.print("\nTEMPERATURA DE:  ");
      Serial.print(menorValorTemp);
      Serial.print(" C");
      Serial.print("\n");
      Serial.print("\n");
      Serial.print("\n");
      Serial.print("TEMPERATURA BAIXA\n");
      Serial.print("COMUTACAO: ");
      Serial.println(semana);
      delay(500);

      if (menorValorTemp <= 25) {
        if (semana == 1) {
          pwmPin = 13;
          pwm2Pin = 12;
        }
        else {
          pwmPin = 12;
          pwm2Pin = 13;
        }

        digitalWrite(pwmPin, LOW);
        digitalWrite(pwm2Pin, HIGH);


      }

      if (menorValorTemp >= 26) {

        /* Essa condicao definida para quando chegar certo C(graus) ligar todas as chaves para alto
           independente se estiverem comutadas.

        */
        if (semana == 1) {
          pwmPin = 13;
          pwm2Pin = 12;
        }
        else {
          pwmPin = 12;
          pwm2Pin = 13;
        }

        Serial.print("\n\nTEMPERATURA ALTA\nCHAVE 1 LIGADO\n");

        if (semana == 1, semana == 0) { // Independente que esteja a valor alto no pusante as chaves ainda estaao todas as 2 ligadas
          Serial.print("CHAVE 2 LIGADO\n");
        }
        else {
          //Serial.print("CHAVE 2 LIGADO\n");
        }
        // Serial.print(menorValorTemp);
        Serial.print("\n");

        if (tempoOcorrido15Min - previsaoMillSegundos15Min >= intervaloInterrompido) {
          previsaoMillSegundos15Min = tempoOcorrido15Min;
          temporizadorLogico = 1;
          acender2Min = LOW;

          if (acender2Min == LOW) {
            acender2Min = HIGH;
          }

          digitalWrite(pwmPin, acender2Min);
          digitalWrite(pwm2Pin, acender2Min);
        }
        //delay(150);
      }

      else {
        if (semana == 1) {
          pwmPin = 13;
          pwm2Pin = 12;
        }
        else {
          pwmPin = 12;
          pwm2Pin = 13;
        }

        // Serial.print(menorValorTemp); //* EM analise

        if (semana == 1) { // Comutando as chaves pelas condicoes do PWM ou variavel pulsante
          Serial.print("\nCHAVE 1 DESLIGADO E CHAVE 2 LIGADO");
        }
        else {
          Serial.print("\nCHAVE 1 LIGADO E CHAVE 2 DESLIGADO");
        }
        if (temporizadorLogico == 1) {
          if (tempoOcorrido15Min - previsaoMillSegundos15Min >= intervaloInterrompido) {
            previsaoMillSegundos15Min = tempoOcorrido15Min;
            temporizadorLogico = 0;
            acender2Min = LOW;

            if (acender2Min == LOW) {
              acender2Min = HIGH;
            }

            digitalWrite(pwmPin, acender2Min);
            digitalWrite(pwm2Pin, acender2Min);
          }
          digitalWrite(pwmPin, HIGH);
          digitalWrite(pwm2Pin, HIGH);
        }

        else  {
          if (menorValorTemp <= 27) {
            digitalWrite(pwmPin, LOW);
          }
          digitalWrite(pwm2Pin, HIGH);
        }
        //delay(9600); // repetir todo o ciclo de medicoes seguindo o banco de atraso anterior
        delayMicroseconds(10000);

      }


      if (menorValorTemp >= 100)
        /* Essa condicao vai determinar que acima de 100 C (graus), deduzindo que a leitura esteja
           divergente do real, e deduz que o componente LM35 em seu coletor, esteja saturado
           e levando em conta o atraso, por precaucoes as CHAVE'S 1 e 2, permanecerao ativas, ate que
           se resolva trocar o transistor LM35 ou averiguar, para a seguranca dos equipamentos que
           estarao ligado a esses reles DC.
        */
      {
        if (semana == 1) {
          pwmPin = 13;
          pwm2Pin = 12;
        }
        else {
          pwmPin = 12;
          pwm2Pin = 13;
        }

        valorSensorTemp = analogRead(sensorTemp);
        valorSensorTemp *= 0.54;
        menorValorTemp = valorSensorTemp;
        Serial.print(menorValorTemp);
        lcd.clear();
        //Posiciona o cursor na coluna 3, linha 0;
        lcd.setCursor(3, 0);

        //Envia o texto entre aspas para o LCD
        lcd.print("Temp: ");
        lcd.print(menorValorTemp);
        lcd.setCursor(3, 1);
        Serial.print("\nVERIFIQUE O TRANSISTOR LM35, LEITURA INCOERENTE COM O AMBIENTE\n\n\n\n\n");

        //digitalWrite(ledPin2, SetaLed);

        if (semana == 1, semana == 0) {
          /*Indepedente que esteja comutado ou com baixa temperatura
            estao setado em alto para todas as chaves, para nao haver erros por corrente parasita. */
          Serial.print("\nCHAVE 1 e 2 LIGADO POR QUESTAO DE SEGURANCA\n");

        }

        else {
          //digitalWrite(ledPin2, SetaLed);
          // Sem reacao caso != difente da ultima condicao acima
        }
      }
    }
  }
}
/*else { // LIGAR MANUALMENTE AS DUAS CHAVES
  Serial.print("CHAVE MANUAL ACIONADA\n");
  Serial.print("CHAVE 1 E CHAVE 2, ACIONADOS\n\n\n");
  digitalWrite(pwmPin, HIGH);
  digitalWrite(pwm2Pin, HIGH);
  // Definir sque sempre
  valorTensao = analogRead(A1);
  float tensaoOutA1 = valorTensao * (5.0 / 1023.0);
  valorCheckUp = analogRead(A4);
  float tensaoOutA2 = valorCheckUp * (5.0 / 1023.0);
  Serial.print("COMUTACAO: N/A \n");
  //Serial.println(semana);
  Serial.print("Tensao de:  ");
  Serial.print(tensaoOutA1);
  Serial.print("V\n");
  Serial.print("SINAL PARASITA : ");
  Serial.print(tensaoOutA2);
  Serial.print("V");
  menorValorTemp = INT_MAX;
  // Lendo o valor do sensor de temperatura
  // Definindo 18 leitura e o menor valor lido prevalece
  for (int i = 1; i <= 18; i++); {
    menorValorTemp = INT_MAX;
    valorSensorTemp = analogRead(sensorTemp);
    // Transformando valor lido no sensor de temperatura
    valorSensorTemp *= 0.54;
    if (valorSensorTemp < menorValorTemp) {
      menorValorTemp = valorSensorTemp;
      Serial.print("\nTEMPERATURA DE:  ");
      Serial.print(menorValorTemp);
      Serial.print(" C");
      Serial.print("\n");
      Serial.print("\n");
      Serial.print("\n");
      delay(1500);
    }
  }
  }
  }
  }
*/



