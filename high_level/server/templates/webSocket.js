const socketUrl = "wss://ws.davinsony.com/metropolitana";

const socket = new WebSocket(socketUrl);

const eventsContainer = document.getElementById("events");

// Función para renderizar mensajes en el HTML
function renderMessage(message) {
    // Crear un nuevo elemento div
    const eventDiv = document.createElement("div");
    eventDiv.className = "event";
    eventDiv.textContent = message; // Agregar el contenido del mensaje
    eventsContainer.appendChild(eventDiv); // Agregar al contenedor

    // Desplazar el contenedor hacia abajo automáticamente
    eventsContainer.scrollTop = eventsContainer.scrollHeight;
}

// Evento: conexión establecida
socket.addEventListener("open", () => {
    console.log("Conexión establecida con el servidor WebSocket");
    renderMessage("Conexión establecida con el servidor WebSocket");

    // Enviar un mensaje de suscripción (opcional)
    socket.send(JSON.stringify({ action: "subscribe", channel: "mi_canal" }));
});

// Evento: recibir mensaje del servidor
socket.addEventListener("message", (event) => {
    console.log("Mensaje recibido:", event.data);

    // Renderizar el mensaje en el HTML
    renderMessage(`Mensaje recibido: ${event.data}`);
});

// Evento: error en la conexión
socket.addEventListener("error", (error) => {
    console.error("Error en la conexión WebSocket:", error);
    renderMessage("Error en la conexión WebSocket");
});

// Evento: conexión cerrada
socket.addEventListener("close", (event) => {
    console.log("Conexión cerrada:", event.reason);
    renderMessage("Conexión cerrada");
});