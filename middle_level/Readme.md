## Middle Level

Este sistema ciberfísico (CPS) amplía las funcionalidades de control de semáforos para automóviles y peatones, 
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


![maqueta Smart Weather](../resources/smart_weather.png) 

**Figura 1**: Sensores y componentes.