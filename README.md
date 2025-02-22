# P2P Chat and File Sharing Application 🚀

![Screenshot 2025-02-22 225307](https://github.com/user-attachments/assets/2766cb7d-9b7e-4d48-9ff5-b627065ba038)


## Overview

This project is a peer-to-peer (P2P) messaging and file sharing application written in C for Windows using Winsock2. It enables multiple peers to communicate by sending text messages and transferring files by splitting them into chunks. The application uses ephemeral ports for outgoing connections.

## Features

### 💬 Text Messaging
- **Send & Receive Messages:** Communicate effortlessly with peers by sending text messages.

![image](https://github.com/user-attachments/assets/1e0e041b-cc0c-4aa5-802b-29e682e8733e)

### 🔍 Query Active Peers
- **Active Peer List:** Retrieve a list of all peers that have interacted with your port.
![Screenshot 2025-02-22 225151](https://github.com/user-attachments/assets/ded0b5ce-71d2-42f2-8867-1d9ab58768e5)


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

## How to Use 🛠️

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/Arihant779/P2P_Chature_Bhature.git
2. **Compile the Code**
   ```bash
   gcc -o p2p_chat main.c -lws2_32
3. **Run the Application:**
    ```bash
    .\p2p_chat.exe

## 🔗 Authors

| **Name**         | **Roll Number** |
|------------------|-----------------|
| Arihant Jain     | 230001009       |
| Tanishq Godha    | 230001074       |
| Vedant Jain      | 230008038       |
| Bhuva Vasudha    | 230002017       |



