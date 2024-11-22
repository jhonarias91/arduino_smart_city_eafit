from flask import Flask, request, jsonify, render_template
import serial
import websocket
import json
import threading
import time
import mysql.connector

# Configuración del puerto serial RFC2217
#SERIAL_URL = 'rfc2217://localhost:4000'
SERIAL_PORT = "COM5"
SERIAL_BAUDRATE = 115200
id = "id23" #This will be update every mesage
WEBSOCKET_URL = "wss://ws.davinsony.com/"+id
nightmode = 0

# Inicia Flask
app = Flask(__name__)

# Inicializa conexión serial
def get_serial_connection():
    try:
        ser = serial.Serial()
        ser.port = SERIAL_PORT
        ser.baudrate = SERIAL_BAUDRATE
        ser.timeout = 1
        ser.dtr = False  # Set DTR False before opening to avoid reboot
        ser.open()

        ser.reset_input_buffer()  # Clean in buffer
        ser.reset_output_buffer()  # Clean output buffer
        return ser
    except serial.SerialException as e:
        #print(f"Error al conectar al puerto serial: {e}")
        return None

@app.route("/realtimes")
def realtime():
    data = []
    return render_template("real.html", data=data)

# Configura los datos iniciales
@app.route("/")
def index():
   
    data = [
    {"id": 1, "name": "greenTime1", "value": "2000"},
    {"id": 2, "name": "greenTime2", "value": "4000"},
    {"id": 3, "name": "yellowTime", "value": "500"},
    {"id": 4, "name": "blinkTime", "value": "100"},
    {"id": 5, "name": "totalBlinksInOut", "value": "6"},
    {"id": 6, "name": "pedestrianCrossTime", "value": "10000"},
    {"id": 7, "name": "pedestrianReduceGreenTime1", "value": "3000"},
    {"id": 8, "name": "co2GreenTime2", "value": "20000"},
    {"id": 9, "name": "lightGreen2IncreaseWhenSensors", "value": "2000"},    
    {"id": 10, "name": "priorityWaitingTimeOnLight2", "value": "7000"},
    {"id": 11, "name": "priorityWaitingTimeOnLight1", "value": "3000"},
    {"id": 12, "name": "lightGreen1IncreaseWhenSensors", "value": "2000"},    
    {"id": 13, "name": "displayRefreshTimeAfterNotification", "value": "4000"},
    {"id": 14, "name": "greenLight1TimeWhenCar", "value": "3000"},
    {"id": 15, "name": "pedestrian2CrossTime", "value": "4000"},
    {"id": 16, "name": "pedestrianReduceGreenTime1", "value": "3000"},
    {"id": 17, "name": "pedestrianReduceGreenTime2", "value": "5000"},
    
]
    return render_template("index.html", data=data)

@app.route("/update", methods=["POST"])
def update_value():
    data = request.json  # Obtiene los datos del frontend
    name = data.get("name")
    value = data.get("value")

    if not name or not value:
        return jsonify({"status": "error", "message": "Datos incompletos"}), 400

    ser = get_serial_connection()
    if ser:
        try:
            ser.write(f"{name}:{value}\n".encode("utf-8"))
            print(f"Enviado: {name}:{value}")
            return jsonify({"status": "success", "message": f"{name} actualizado a {value}"})
        finally:
            ser.close()
            print("Puerto serial cerrado.")
    else:
        return jsonify({"status": "error", "message": "No se pudo abrir el puerto serial"}), 500

# WebSocket callbacks
def on_message(ws, message):
    data = json.loads(message)
    print(f"Mensaje recibido: {data}")
    ser = get_serial_connection()
    if ser:
        ser.write((data["msg"] + "\n").encode())
        ser.close()

def on_error(ws, error):
    print(f"Error WebSocket: {error}")

def on_close(ws, close_status_code, close_msg):
    print(f"Conexión WebSocket cerrada. Código: {close_status_code}, Mensaje: {close_msg}")

def on_open(ws):
    print("Conexión WebSocket abierta.")
    threading.Thread(target=serial_to_websocket, daemon=True).start()

# Lee datos seriales y los envía al WebSocket
def serial_to_websocket():
    ser = get_serial_connection()
    if ser:
        while True:
            try:
                if ser.in_waiting > 0:  # Hay datos disponibles
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(line);
                        idValue = line.split("_")
                        id = idValue[0] 
                        ## check if line have :
                        if ":" in line:                            
                            handleVariable(idValue[1])
                        else:
                            ws.send(json.dumps({"msg": idValue[1], "to": "metropolitana", "from": id}))
            except serial.SerialException as e:
                print(f"Error leyendo del puerto serial: {e}")
                break
            time.sleep(0.1)


def handleVariable(keyValue):
    print(f"Variable: {keyValue}")
    keyValueArray = keyValue.split(":")
    key = keyValueArray[0]
    value = keyValueArray[1]
    saveToDatabase(id, key, value)
    if key == "nightMode":
        nightmode = int(value)
        # Enviar el estado de nightMode al frontend
        if ws:
            ws.send(json.dumps({"msg": keyValue, "to": "metropolitana", "from": id, "key": "nightMode", "value": nightmode}))

# Hilo para ejecutar WebSocket
def run_websocket():
    global ws
    ws = websocket.WebSocketApp(
        WEBSOCKET_URL,
        on_message=on_message,
        on_error=on_error,
        on_close=on_close,
        on_open=on_open
    )
    ws.run_forever(sslopt={"cert_reqs": 0})

# Hilo para ejecutar Flask
def run_flask():
    app.run(debug=True, use_reloader=False, host="127.0.0.1", port=5000)

# Configuración de la conexión a MySQL
db_config = {
    "host": "database-1.cd0uas88ikvu.us-east-1.rds.amazonaws.com",
    "user": "admin",
    "password": "smartcity",
    "database": "smartcity"
}

def saveToDatabase(clientId, key, value):
    try:
        # Establecer conexión con la base de datos
        connection = mysql.connector.connect(**db_config)
        cursor = connection.cursor()

        # Consulta SQL para insertar datos
        query = "INSERT INTO logs (client_id, key_name, key_value) VALUES (%s, %s, %s)"
        cursor.execute(query, (clientId, key, value))

        # Confirmar la transacción
        connection.commit()

        print(f"Guardado en la base de datos: {key} -> {value}")
    except mysql.connector.Error as err:
        print(f"Error al guardar en la base de datos: {err}")
    finally:
        # Cerrar la conexión
        if connection.is_connected():
            cursor.close()
            connection.close()

if __name__ == "__main__":
    # Ejecuta Flask y WebSocket en hilos separados
    flask_thread = threading.Thread(target=run_flask)
    websocket_thread = threading.Thread(target=run_websocket)

    flask_thread.start()
    websocket_thread.start()

    flask_thread.join()
    websocket_thread.join()