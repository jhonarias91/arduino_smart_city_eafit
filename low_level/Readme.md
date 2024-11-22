## Low Level

Este CPS gestiona un semáforo para automóviles y peatones, con un sensor infrarrojo para detectar vehículos. 
Controla el flujo del tráfico y la seguridad de los peatones ajustando los tiempos de luz verde y roja 
en función de la presencia de vehículos o solicitudes de paso de peatones.

![maqueta Smart Weather](../resources/smart_weather.png) 

**Figura 1**: Sensores low level.

## Entradas y Salidas
### Entradas:

- Botón del peatón (P1): Indica la solicitud de cruce de peatones.
- Botón del peatón (P2): Indica solicitud de cruce peatonal del semáforo 2.
- Sensores infrarrojo (CNY1, CNY2, CNY3): Detecta vehículos para ajustar el tiempo de luz verde en el semáforo 1.
- Sensores infrarrojo (CNY4, CNY5, CNY6): Detecta vehículos para ajustar el tiempo de luz verde en el semáforo 2.
- Monitor de CO2: Detecta los niveles de CO2 en el túnel, si hay vehículos en alguno de los sensores (CNY4, CNY5, CNY6)
le incrementa el tiempo a la luz verde porcentualmente.
- Sensores LDR1 y LDR2: Si el valor está por debajo de los 60 en ambos se activa el modo nocturno. El cual deja siempre 
en verde el semáforo 2 debido a que esta vía es arterial, tiene un tunel y por los niveles de CO2 es mejor dar prelacción a este.
Si hay un vehículo en el otro semáforo, se activa el semáforo 1 y luego se regresa al modo nocturno.

### Salidas:

Luces de tráfico (LR1, LY1, LG1, LR2, LY2, LG2): Controlan el estado del semáforo para dos direcciones de tráfico.

