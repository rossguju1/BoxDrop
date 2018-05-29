### CS60
### README

##TRACKER:

       Trackers fist creates a monitor thread to handle/disconnect all peers that havent sent heartbeat for a while.
       Then the tracker listens and accepts connections from peers, creating a new handshake thread for each peer.
       The handshake thread calls new_peer() that adds the peer to the table if table is not full.
       Then the thread keeps on receiving messages from the peer until it disconnects/timeouts. If the client sends "Close" message then this thread disconnects the client. If the client timeouts, then monitor thread disconnects the client.

##PEER:
       First it prompts to enter server name, which can for example be flume.cs.dartmouth.edu
       It connects to the tracker and sends a "REGISTER" signal and keeps on receiving data fro the tracker.
       Meanwhile another thread "keepAlive" periodically sends heartbeat to the tracker.
