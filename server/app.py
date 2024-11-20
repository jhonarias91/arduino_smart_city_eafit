from flask import Flask, request, jsonify, render_template
import serial
import time

app = Flask(__name__)

# Inicializa la conexión serial con el Arduino
def get_serial_connection():
    try:
        ser = serial.Serial()
        ser.port = "COM5"
        ser.baudrate = 9600
        ser.timeout = 1
        ser.dtr = False  # Set DTR False before opening to avoid reboot
        ser.open()
        print("Conectado al puerto COM5 a 9600 baudios.")
        return ser
    except serial.SerialException as e:
        print(f"Error al conectar al puerto serial: {e}")
        return None

@app.route("/")
def index():
    # Initial data, TODO: Read from Arduino
    data = [
        {"id": 1, "name": "greenTime1", "value": "500"},
        {"id": 2, "name": "greenTime2", "value": "200"},
        {"id": 3, "name": "yellowTime", "value": "30"},
    ]
    return render_template("index.html", data=data)

@app.route("/update", methods=["POST"])
def update_value():
    data = request.json  # Get the data from front
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

if __name__ == "__main__":
    app.run(debug=True)
