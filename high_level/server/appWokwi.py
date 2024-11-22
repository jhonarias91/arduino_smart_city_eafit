from flask import Flask, request, jsonify, render_template
import serial
import websocket
import json
import threading
import time

# Configuración del puerto serial RFC2217
SERIAL_URL = 'rfc2217://localhost:4000'
SERIAL_BAUDRATE = 9600
WEBSOCKET_URL = "wss://ws.davinsony.com/ccampos"

# Inicia Flask
app = Flask(__name__)

# Verifica que `serial` tiene `serial_for_url`
if not hasattr(serial, 'serial_for_url'):
    raise AttributeError("El módulo 'serial' no tiene el atributo 'serial_for_url'")

# Inicializa conexión serial
def get_serial_connection():
    try:
        ser = serial.serial_for_url(SERIAL_URL, baudrate=SERIAL_BAUDRATE, timeout=1)
        print(f"Conectado en {SERIAL_URL} a {SERIAL_BAUDRATE} baudios.")
        return ser
    except serial.SerialException as e:
        print(f"Error al conectar al puerto serial: {e}")
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
                    line = ser.readline().decode().strip()
                    if line:
                        ws.send(json.dumps({"msg": line, "to": "ccampos", "from": "ccampos"}))
            except serial.SerialException as e:
                print(f"Error leyendo del puerto serial: {e}")
                break
            time.sleep(0.1)

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
    app.run(debug=True, use_reloader=False, host="127.0.0.1", port=5001)

if __name__ == "__main__":
    # Ejecuta Flask y WebSocket en hilos separados
    flask_thread = threading.Thread(target=run_flask)
    websocket_thread = threading.Thread(target=run_websocket)

    flask_thread.start()
    websocket_thread.start()

    flask_thread.join()
    websocket_thread.join()
