# P2P Chat and File Sharing Application 🚀

![Project Banner](https://github.com/user-attachments/assets/761b6606-55bf-44b7-a798-409de8d3e431)

## Overview

This project is a peer-to-peer (P2P) messaging and file sharing application written in C for Windows using Winsock2. It enables multiple peers to communicate by sending text messages and transferring files by splitting them into chunks. The application uses ephemeral ports for outgoing connections.

## Features

### 💬 Text Messaging
- **Send & Receive Messages:** Communicate effortlessly with peers by sending text messages.

![image](https://github.com/user-attachments/assets/1e0e041b-cc0c-4aa5-802b-29e682e8733e)

### 🔍 Query Active Peers
- **Active Peer List:** Retrieve a list of all peers that have interacted with your port.
![Screenshot 2025-02-22 221957](https://github.com/user-attachments/assets/e0562915-b666-4102-a44f-24602ff4d847)


### ❌ Exit from Network
- **Graceful Disconnect:** On exit, notify all connected peers and remove your port from their active peers list for a clean shutdown.
![Screenshot 2025-02-22 223032](https://github.com/user-attachments/assets/8f5db589-a89a-4798-9723-3a4e1c647eb9)

## Bonus Features

### 🌟 Connect to Active Peers (Broadcast)
- **Broadcast Messaging:** Send a broadcast message to all active peers. Once received, these peers add your connection to their local peer list, expanding your network.

  ![Screenshot 2025-02-22 224004](https://github.com/user-attachments/assets/2a61b573-2ff5-43fd-bbf6-a936c266ec42)

### 📁 File Transfer
- **Large File Sharing:** Transfer files between peers by splitting them into chunks, enabling support for large file sizes without overwhelming the network.

  
![Screenshot 2025-02-22 224427](https://github.com/user-attachments/assets/a9f3a00b-d7f7-44c7-a0d9-b42d494035df)


