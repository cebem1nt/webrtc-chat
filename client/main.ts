const SERVER_URL = "localhost:8080"
const RTC_SETTINGS = {
    "iceServers": [
        {"urls": "stun:stun.l.google.com:19302"}
    ]
}

function toServer(message: object | string) {
    if (typeof message == "object") {
        message = JSON.stringify(message)
    }

    server.send(message)
}

function handleChannelMessage(event: MessageEvent) {
    console.log("received:", event.data)
}

const roomID = prompt("Enter room name:", "room_123")
const server = new WebSocket("ws://" + SERVER_URL)
const peer = new RTCPeerConnection(RTC_SETTINGS)

let dataChanel: RTCDataChannel | null = null

server.addEventListener("open", () => {
    toServer("roomID:" + roomID)
})

server.addEventListener("message", async (event) => {
    const msg = JSON.parse(event.data)
    console.log(msg)

    if (msg.answer) {
        await peer.setRemoteDescription(new RTCSessionDescription(msg.answer))
    } 
    else if (msg.offer) {
        await peer.setRemoteDescription(new RTCSessionDescription(msg.offer))
        const answer = await peer.createAnswer()
        await peer.setLocalDescription(answer)
        toServer({answer})
    }
    else if (msg.candidate) {
        try {
            await peer.addIceCandidate(msg.candidate);
        } catch (e) {
            console.error("Error adding received ice candidate", e)
        }
    }
});

peer.onicecandidate = (ev) => {
    if (ev.candidate) {
        toServer({"candidate": ev.candidate})
    }
}

peer.onconnectionstatechange = () => {
    if (peer.connectionState === "connected") {
        console.log("Peers connected!")
    }
};

peer.ondatachannel = (event) => {
    dataChanel = event.channel
    dataChanel.onmessage = handleChannelMessage
};

async function start() {
    dataChanel = peer.createDataChannel("chat")
    const offer = await peer.createOffer()

    await peer.setLocalDescription(offer)
    toServer({offer})
}