import React, { useState, useEffect } from "react";

function App() {
  const [alert, setAlert] = useState(false);
  const [image, setImage] = useState(null);

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:8080");
    ws.onmessage = (msg) => {
      const data = JSON.parse(msg.data);
      if (data.type === "alert") {
        setAlert(true);
        setImage(`http://localhost:5000/${data.image}`);
      }
    };
    return () => ws.close();
  }, []);

  return (
    <div className="container">
      <h1>Smart Doorbell</h1>
      {alert && <div className="alert">🚪 Someone is at the door!</div>}
      {image && <img src={image} alt="Visitor" />}
      {!alert && <p>Waiting for doorbell...</p>}
    </div>
  );
}

export default App;
