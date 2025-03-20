import { faker } from "@faker-js/faker";
import express from "express";
import expressWs from "express-ws";
import { OPEN } from "ws";

const app = express();
expressWs(app);

const wsRouter = express.Router();
const controllerRouter = express.Router();

wsRouter.ws("/", (ws, req) => {
  console.log("Client connected");
  const intervalMs = 20;
  var counter: number = 0;
  const interval = setInterval(() => {
    if (ws.readyState !== OPEN) {
      return;
    }
    const message = JSON.stringify({
      angle: faker.number.float({ min: 70, max: 110 }),
      input: faker.number.float({ min: -5, max: 5 }),
      target: 90,
      time: counter++ * intervalMs,
    });
    ws.send(message);
  }, intervalMs);

  ws.on("close", () => {
    console.log("Client disconnected");
    clearInterval(interval);
  });
});

controllerRouter.get("/parameters", (req, res) => {
  res.send({
    kp: faker.number.float({ min: 0, max: 1 }),
    ki: faker.number.float({ min: 0, max: 1 }),
    kd: faker.number.float({ min: 0, max: 1 }),
  });
});

controllerRouter.post("/parameters", (req, res) => {
  res.status(204).send();
});

controllerRouter.get("/target-angle", (req, res) => {
  res.send({
    angle: faker.number.float({ min: 60, max: 120 }),
  });
});

controllerRouter.post("/target-angle", (req, res) => {
  res.status(204).send();
});

app.use("/ws", wsRouter);
app.use("/controller", controllerRouter);
app.listen(3001);
