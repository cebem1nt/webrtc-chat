const SERVER_URL = "localhost:8080"
const RTC_SETTINGS = {
    "iceServers": [
        {"urls": "stun:stun.l.google.com:19302"}
    ]
}

async function start() {
    const server = new WebSocket("ws://" + SERVER_URL)
    const pc = new RTCPeerConnection(RTC_SETTINGS)

    server.addEventListener("open", () => {
        server.send("roomID: " + "RoomID")
    })

    server.addEventListener("message", async (event) => {
        console.log(event.data)
        const msg = JSON.parse(event.data);
        if (msg.answer) {
            const remoteDesc = new RTCSessionDescription(msg.answer)
            await pc.setRemoteDescription(remoteDesc)
        }
    });

    const offer = await pc.createOffer();
    await pc.setLocalDescription(offer);
    server.send(JSON.stringify({"offer": offer}));
}