## High Level

Este sistema ciberfísico (CPS) mejor la versión media para convertirse en un CPS de nivel alto.
donde la lógica de sus transicciones es más compleja.
amplía las funcionalidades de control de semáforos para automóviles y peatones, 
integrando múltiples sensores infrarrojos y sensores de luz. El sistema ajusta 
dinámicamente los tiempos de las luces según la detección de vehículos, 
la intensidad de la luz ambiente y las solicitudes de cruce de peatones. 
También implementa un modo nocturno.


## Entradas y Salidas
### Entradas:

#### Botón del peatón (P1): 
Indica la solicitud de cruce de peatones.

#### Sensores infrarrojos (CNY1 a CNY6):

Detectan vehículos para ajustar el tiempo de luz verde.
Los sensores están distribuidos en ambas direcciones del tráfico.

#### Sensores de luz (LDR1, LDR2):

Detectan condiciones de poca luz para activar el modo nocturno.

### Salidas:

#### Luces de tráfico (LR1, LY1, LG1, LR2, LY2, LG2):

Controlan el estado del semáforo para dos direcciones de tráfico.

#### Pantalla LCD (LiquidCrystal_I2C):

Muestra información del estado del sistema, como prioridad de sensores y configuraciones.

#### Modo de Operación
Control de tráfico estándar:

Los semáforos cambian entre estados predefinidos en un ciclo continuo.
El tiempo de luz verde se incrementa dinámicamente cuando los sensores detectan vehículos.

#### Cruce peatonal:

Si un peatón presiona el botón, el sistema ajusta los tiempos de las luces para permitir el cruce seguro.

#### Modo nocturno:

Activado cuando los sensores de luz detectan condiciones de poca iluminación.
Cambia los semáforos a una operación intermitente para dar prelación a el semáforo de la vía principal y que tiene un túnel, permitiendo reducir los niveles de CO2 ahorrar energía, reducir el desgaste.

#### Priorización de sensores:

Si varios sensores detectan vehículos simultáneamente, la dirección correspondiente recibe mayor tiempo de luz verde.


### Máquina de estados:

- STATE_LIGHT1_GREEN_ON_START (0):

Semáforo 1 en verde al inicio. El semáforo 1 enciende su luz verde, permitiendo el paso de vehículos en esa dirección y semáforo en rojo en el otro sentido.

- STATE_LIGHT1_GREEN_BLINK (1):

Parpadeo de luz verde en semáforo 1. La luz verde del semáforo 1 comienza a parpadear para avisar que pronto cambiará.
- STATE_LIGHT1_YELLOW_ON (2):

Semáforo 1 en amarillo. El semáforo 1 enciende la luz amarilla, indicando a los conductores que deben prepararse para detenerse.
- STATE_LIGHT_RED1_ON (3):

Semáforo 1 en rojo y semáforo 2 preparándose. El semáforo 1 está en rojo, mientras que el semáforo 2 se prepara para cambiar a verde.
- STATE_LIGHT2_GREEN_ON (4):

Semáforo 2 en verde. El semáforo 2 enciende su luz verde, permitiendo el paso de vehículos en esa dirección.
- STATE_LIGHT2_GREEN_BLINK (5):

Parpadeo de luz verde en semáforo 2. La luz verde del semáforo 2 comienza a parpadear para avisar que pronto cambiará.
- STATE_LIGHT2_YELLOW_ON (6):

Semáforo 2 en amarillo. El semáforo 2 enciende la luz amarilla, indicando a los conductores que deben prepararse para detenerse.
- STATE_LIGHT_NIGHT_MODE (7):

Modo nocturno con prioridad en semáforo 2. Se activa el modo nocturno, dando prioridad al semáforo 2.

#### Transiciones:

- De STATE_LIGHT1_GREEN_ON_START a STATE_LIGHT1_GREEN_BLINK:
Después de greenTime1.
- De STATE_LIGHT1_GREEN_BLINK a STATE_LIGHT1_YELLOW_ON:
Después de totalBlinksInOut parpadeos.
- De STATE_LIGHT1_YELLOW_ON a STATE_LIGHT_RED1_ON:
Después de yellowTime.
- De STATE_LIGHT_RED1_ON a STATE_LIGHT2_GREEN_ON:
Después de yellowTime.
- De STATE_LIGHT2_GREEN_ON a STATE_LIGHT2_GREEN_BLINK o STATE_LIGHT_NIGHT_MODE:
Después de greenTime2.
Dependiendo de si el modo nocturno está activado.
- De STATE_LIGHT2_GREEN_BLINK a STATE_LIGHT2_YELLOW_ON:
Después de totalBlinksInOut parpadeos.
- De STATE_LIGHT2_YELLOW_ON a STATE_LIGHT1_GREEN_ON_START:
Después de yellowTime.
- De STATE_LIGHT_NIGHT_MODE a STATE_LIGHT2_GREEN_ON:
Al desactivarse el modo nocturno.

# Parametrización:

La parametrización de manera remota nos permite configurar cualquier dispositivo por medio de la 
interfaz web, en nuestro caso vamos a usar la entrada Serial al no contar con elementos de red en 
la maqueta.

## Lista de valores configurables.

| ID  | Name                              | Value  | Descripción                                                                                      |
|-----|-----------------------------------|--------|--------------------------------------------------------------------------------------------------|
| 1   | greenTime1                        | 2000   | Tiempo asignado en milisegundos para la primera luz verde.                                       |
| 2   | greenTime2                        | 4000   | Tiempo asignado en milisegundos para la segunda luz verde.                                       |
| 3   | yellowTime                        | 500    | Tiempo asignado en milisegundos para la luz amarilla.                                            |
| 4   | blinkTime                         | 100    | Tiempo de parpadeo en milisegundos para la luz verde intermitente.                               |
| 5   | totalBlinksInOut                  | 6      | Número total de parpadeos durante el ciclo de luz verde intermitente.                           |
| 6   | pedestrianCrossTime               | 10000  | Tiempo asignado en milisegundos para el cruce de peatones.                                       |
| 7   | pedestrianReduceGreenTime1        | 3000   | Reducción del tiempo de la luz verde 1 para permitir el cruce de peatones.                      |
| 8  | co2GreenTime2                      | 20000   | Tiempo asignado para la luz verde 2 variable según el nivel de CO2.                              |
algún sensores está activo.                           |
| 9   | lightGreen2IncreaseWhenSensors    | 2000   | Incremento del tiempo para la luz verde 2 cuando algún sensor detecta actividad.                   |
| 10  | priorityWaitingTimeOnLight2       | 7000   | Tiempo de espera en milisegundos antes de dar prioridad nuevamente a la luz 2.                  |
| 11  | priorityWaitingTimeOnLight1       | 3000   | Tiempo de espera en milisegundos antes de dar prioridad nuevamente a la luz 1.                  |
| 12  | lightGreen1IncreaseWhenSensors    | 2000   | Incremento del tiempo para la luz verde 1 cuando sensores detectan actividad.                   |
| 13  | displayRefreshTimeAfterNotification | 4000 | Tiempo de refresco en milisegundos del display después de mostrar una notificación.             |
| 14  | greenLight1TimeWhenCar            | 3000   | Tiempo asignado para la luz verde 1 cuando se detecta un vehículo.                              |





![maqueta Smart Weather](../resources/smart_weather.png) 

**Figura 1**: Sensores y componentes.