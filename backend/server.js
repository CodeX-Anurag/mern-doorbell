import express from "express";
import multer from "multer";
import { WebSocketServer } from "ws";

const app = express();
const port = 5000;

app.use(express.static("uploads"));

const storage = multer.diskStorage({
  destination: (req, file, cb) => cb(null, "uploads/"),
  filename: (req, file, cb) => {
    const timestamp = new Date().toISOString().replace(/:/g, "-");
    cb(null, `doorbell_${timestamp}.jpg`);
  },
});
const upload = multer({ storage });

const wss = new WebSocketServer({ port: 8080 });
const clients = new Set();

wss.on("connection", (ws) => {
  console.log("Client connected");
  clients.add(ws);
  ws.on("close", () => clients.delete(ws));
});

function broadcastNotification(message) {
  clients.forEach((client) => {
    if (client.readyState === 1) client.send(message);
  });
}

app.post("/upload", upload.single("image"), (req, res) => {
  console.log("Image received:", req.file.filename);
  broadcastNotification(JSON.stringify({ type: "alert", image: req.file.filename }));
  res.json({ success: true, filename: req.file.filename });
});

app.listen(port, () => console.log(`Backend running at http://localhost:${port}`));
