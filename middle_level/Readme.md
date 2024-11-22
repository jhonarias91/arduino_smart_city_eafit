## Middle Level

Incluye todo lo del low level, adicionalmente implementa un display para mostrar los datos cada segundo, como los valores de algunos sensores
y el tiempo en verde pendiente de cada semáforo, además soporta comunicación por serial para parametrizar y enviar valores a la nube y mostrar
los distintos mensajes que se impriman. 

Adicionalmente, tiene una lógica más robusta que el de low level en cuanto a:
- Si hay prioridad sobre el semáforo 1, se incrementa el tiempo de redución del verde cuando el peatón solicita vía en función del tiempo
por incremento por cuando se tiene prelacion sobre este.
- Si hay prioridad sobre semáforo 1, el tiempo en rojo del peatón solo se incrementa en la mitad establecida.
- Si hay prioridad sobre el semáforo 2, se incrementa el tiempo de redución del verde cuando el peatón solicita vía en función 
del tiempo por incremento por cuando se tiene prelacion sobre este y los niveles de CO2.
- Si hay prioridad sobre semáforo 2, el tiempo en rojo del peatón no se incrementa, debido a que es vía arterial y tenemos un tunel.

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

# Run:

Run the middle_level/wokwi/diagram.json

`python server/appWokwi.py`

Go to  http://127.0.0.1:5001

![maqueta Smart Weather](../resources/smart_weather.png) 

**Figura 1**: Sensores y componentes.