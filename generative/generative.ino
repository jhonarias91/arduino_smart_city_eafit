// Pines de los sensores de presencia de carros
const int sensorPC1 = 2; // Sensor para carro 1
const int sensorPC2 = 3; // Sensor para carro 2
const int sensorPC3 = 4; // Sensor para carro 3
const int sensorPC4 = 5; // Sensor para carro 4
const int sensorPC5 = 6; // Sensor para carro 5
const int sensorPC6 = 7; // Sensor para carro 6

// Pines de salida para controlar los semáforos
const int semaforo1 = 8; // LED semáforo 1
const int semaforo2 = 9; // LED semáforo 2

// Tiempos iniciales
int tiempo1 = 10; // Tiempo del semáforo 1 en verde (segundos)
int tiempo2 = 10; // Tiempo del semáforo 2 en verde (segundos)

void setup() {
  // Configurar sensores como entradas
  pinMode(sensorPC1, INPUT);
  pinMode(sensorPC2, INPUT);
  pinMode(sensorPC3, INPUT);
  pinMode(sensorPC4, INPUT);
  pinMode(sensorPC5, INPUT);
  pinMode(sensorPC6, INPUT);

  // Configurar semáforos como salidas
  pinMode(semaforo1, OUTPUT);
  pinMode(semaforo2, OUTPUT);

  // Inicializar comunicación serial
  Serial.begin(9600);
}

void loop() {
  // Leer sensores de presencia
  bool pc1 = digitalRead(sensorPC1);
  bool pc2 = digitalRead(sensorPC2);
  bool pc3 = digitalRead(sensorPC3);
  bool pc4 = digitalRead(sensorPC4);
  bool pc5 = digitalRead(sensorPC5);
  bool pc6 = digitalRead(sensorPC6);

  // Evaluar flujo actual
  int flujo_actual = evaluarFlujo(pc1, pc2, pc3, pc4, pc5, pc6, tiempo1, tiempo2);

  // Ajustar tiempos de los semáforos
  ajustarTiempos(flujo_actual, pc1, pc2, pc3, pc4, pc5, pc6);

  // Controlar semáforos
  digitalWrite(semaforo1, HIGH); // Semáforo 1 en verde
  digitalWrite(semaforo2, LOW);  // Semáforo 2 en rojo
  delay(tiempo1 * 1000);         // Esperar el tiempo del semáforo 1

  digitalWrite(semaforo1, LOW);  // Semáforo 1 en rojo
  digitalWrite(semaforo2, HIGH); // Semáforo 2 en verde
  delay(tiempo2 * 1000);         // Esperar el tiempo del semáforo 2
}

int evaluarFlujo(bool pc1, bool pc2, bool pc3, bool pc4, bool pc5, bool pc6, int tiempo1, int tiempo2) {
  // Contar carros en cada semáforo
  int carrosSemaforo1 = pc1 + pc2 + pc3;
  int carrosSemaforo2 = pc4 + pc5 + pc6;

  // Calcular flujo
  int flujo1 = carrosSemaforo1 * tiempo1;
  int flujo2 = carrosSemaforo2 * tiempo2;

  return flujo1 + flujo2;
}

void ajustarTiempos(int flujo_actual, bool pc1, bool pc2, bool pc3, bool pc4, bool pc5, bool pc6) {
  // Probar ajustes pequeños en los tiempos
  for (int i = 0; i < 10; i++) {
    int variacionTiempo1 = tiempo1 + random(-1, 2); // Variación -1 a 1
    int variacionTiempo2 = tiempo2 + random(-1, 2);

    // Limitar los tiempos para evitar valores negativos o demasiado cortos
    variacionTiempo1 = max(5, variacionTiempo1);
    variacionTiempo2 = max(5, variacionTiempo2);

    // Evaluar flujo con nuevos tiempos
    int nuevo_flujo = evaluarFlujo(pc1, pc2, pc3, pc4, pc5, pc6, variacionTiempo1, variacionTiempo2);

    // Si mejora el flujo, actualizar los tiempos
    if (nuevo_flujo > flujo_actual) {
      tiempo1 = variacionTiempo1;
      tiempo2 = variacionTiempo2;
      flujo_actual = nuevo_flujo;

      // Mostrar cambios por Serial
      Serial.print("Nuevo tiempo 1: ");
      Serial.println(tiempo1);
      Serial.print("Nuevo tiempo 2: ");
      Serial.println(tiempo2);
    }
  }
}
