// class SignalServer {
//     private socket
    
//     constructor (url: string) {
//         this.socket = new WebSocket(url)

//         this.socket.addEventListener("open", (event) => {
//             // The hardest hello world so far. Im proud of it.
//             console.log("Hello world!!")
//         });
//     }

//     public sendMessage(msg: object) {
//         this.socket.send(JSON.stringify(msg))
//     }
// }

async function start() {
    // Create WebSocket connection.
    const socket = new WebSocket("ws://localhost:8080");

    // Connection opened
    socket.addEventListener("open", (event) => {
        socket.send("Hello Server!");
    });

    // Listen for messages
    socket.addEventListener("message", (event) => {
        console.log(event.data);
    });
}