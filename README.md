# P2P Chat and File Sharing Application 🚀

![Project Banner](https://github.com/user-attachments/assets/761b6606-55bf-44b7-a798-409de8d3e431)

## Overview

This project is a peer-to-peer (P2P) messaging and file sharing application written in C for Windows using Winsock2. It enables multiple peers to communicate by sending text messages and transferring files by splitting them into chunks. The application uses ephemeral ports for outgoing connections.

## Features

### Text Messaging:
- Send and receive text messages between peers.

![image](https://github.com/user-attachments/assets/1e0e041b-cc0c-4aa5-802b-29e682e8733e)
### Query Active Peers
- It retrives the list of all the peers who have send message to this port.
![Screenshot 2025-02-22 221957](https://github.com/user-attachments/assets/e0562915-b666-4102-a44f-24602ff4d847)


### Exit From Network
- On exiting, all connected peers are sent a message and this port is removed from their active peers list.
![Screenshot 2025-02-22 223032](https://github.com/user-attachments/assets/8f5db589-a89a-4798-9723-3a4e1c647eb9)



## Bonus Task (Connect to active peers)
-  A broadcast message is sent to all peers who have sent message to this peer. Further those peers will have this peer added to their locak peer list.  
  ![Screenshot 2025-02-22 224004](https://github.com/user-attachments/assets/2a61b573-2ff5-43fd-bbf6-a936c266ec42)

## FIle Transfer (Our Bonus Feature) 
- We have added the additional functionality of File Transfer between peers.
- Files are transfered in chunks to enable sharing large file sizes.
  
![Screenshot 2025-02-22 224427](https://github.com/user-attachments/assets/a9f3a00b-d7f7-44c7-a0d9-b42d494035df)


