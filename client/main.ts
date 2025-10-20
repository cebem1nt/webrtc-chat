const INPUT_EL = document.getElementById("local-input") as HTMLInputElement

const SERVER_URL = "localhost:8080"
const RTC_SETTINGS = {
    "iceServers": [
        {"urls": "stun:stun.l.google.com:19302"}
    ]
}

const roomID = prompt("Enter room name:", "room_123")
const server = new WebSocket("ws://" + SERVER_URL)
let peer = new RTCPeerConnection(RTC_SETTINGS)

let dataChanel: RTCDataChannel | null = null

function toServer(message: object | string) {
    if (typeof message == "object") {
        message = JSON.stringify(message)
    }

    server.send(message)
}

function handleChannelMessage(event: MessageEvent) {
    const chat = document.getElementById("chat") as HTMLElement

    console.log("Handlinn")
    const message_el = document.createElement("p")
    message_el.textContent = event.data
    message_el.className = "remote-message"
    chat.appendChild(message_el)
}

function sendChannelMessage(msg: string) {
    if (msg.length == 0) {
        return
    }

    const chat = document.getElementById("chat") as HTMLElement

    console.log("Sendinn")
    dataChanel?.send(msg)
    const message_el = document.createElement("p")
    message_el.textContent = msg
    message_el.className = "local-message"
    chat.appendChild(message_el)
}

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
            await peer.addIceCandidate(msg.candidate)
        } catch (e) {
            console.error("Error adding received ice candidate", e)
        }
    }
})

peer.onicecandidate = (ev) => {
    if (ev.candidate) {
        toServer({"candidate": ev.candidate})
    }
}

peer.onconnectionstatechange = () => {
    if (peer.connectionState == "connected") {
        console.log("Peers connected!")
        document.getElementById("send")?.addEventListener("click", () => {
            sendChannelMessage(INPUT_EL.value)
            INPUT_EL.value = ""
        })
    } else if (peer.connectionState == "disconnected") {
        
    }
}

peer.ondatachannel = (event) => {
    dataChanel = event.channel
    dataChanel.onmessage = handleChannelMessage
}

async function start() {
    if (peer.connectionState != "connected") {
        dataChanel = peer.createDataChannel("chat")
        dataChanel.onmessage = handleChannelMessage
    
        const offer = await peer.createOffer()
        await peer.setLocalDescription(offer)
        toServer({offer})
    }
}