const { faker } = require("@faker-js/faker");
const WebSocket = require("ws");
const server = new WebSocket.Server({ port: 3001 });

// interface ControlData {
//   target: number;
//   angle: number;
//   timestamp: number;
// }

server.on("connection", (ws) => {
  console.log("Client connected");

  const interval = setInterval(() => {
    if (ws.readyState === WebSocket.OPEN) {
      const message = {
        target: 1,
        angle: faker.number.float({ min: 0, max: 1 }),
        timestamp: Date.now(),
      };
      ws.send(JSON.stringify(message));
    }
  }, 20);

  ws.on("close", () => {
    console.log("Client disconnected");
    clearInterval(interval);
  });
});
