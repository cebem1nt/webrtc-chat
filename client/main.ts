class SignalServer {
    private socket
    
    constructor (url: string) {
        this.socket = new WebSocket(url)
        console.log(this.socket)
    }

    public sendMessage(msg: object) {
        this.socket.send(JSON.stringify(msg))
    }
}

async function start() {
    const sc = new SignalServer("ws://localhost:8080")
    sc.sendMessage({"message": "Hello world!!"});
}