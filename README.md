# P2P Chat and File Sharing Application

![image](https://github.com/user-attachments/assets/761b6606-55bf-44b7-a798-409de8d3e431)



## Overview

This project is a peer-to-peer (P2P) messaging and file sharing application written in C for Windows using Winsock2. It enables multiple peers to communicate by sending text messages and transferring files by splitting them into chunks. The application uses ephemeral ports for outgoing connections.

## Features

- **Text Messaging:** Send and receive text messages between peers.
![image](https://github.com/user-attachments/assets/1e0e041b-cc0c-4aa5-802b-29e682e8733e)

- **Peer Discovery:** Automatically maintains a list of active peers.
- **File Transfer:** Transfers files in chunks to support large file sizes.
- **Ephemeral Port Usage:** Uses ephemeral ports to handle outgoing connections.
- **Broadcast Messaging:** Connects to all active peers for broadcasting messages.
- **Graceful Exit:** Sends exit notifications to all connected peers before quitting.

## Prerequisites

- **Operating System:** Windows (due to the use of Winsock2 and Windows threading APIs).
- **Compiler:** A C compiler that supports Windows development (e.g., Visual Studio or MinGW).
- **Libraries:** Winsock2 library (included with the Windows SDK).


