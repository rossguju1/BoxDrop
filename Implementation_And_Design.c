
// Implementation of structures, protocols, peer node, tracker, ect


//	Link for more detail:  http://www.cs.dartmouth.edu/~xia/cs60/project/localsync.pdf


/***** Provided structures *******/


typedef struct segment_peer {

	int protocol_len;

	char protocol_name[PROTOCOL_LEN + 1];

	int type;

	char reserved[RESERVED_LEN];

	char peer_ip[IP_LEN]

	int port;

	int file_table_size;
} ptp_peer_t;


typedef struct  segment_tracker {

	int interval;

	int piece_len;

	int file_table_size;


	file_t file_table;
} ptp_tracker_t;

typedef struct node {

	int size;

	char* name;

	unsigned long int timestamp;

	struct node *pNext;

	char* newpeerip;

} Node, *pNode;




typedef struct _peer_side_peer_t {

	char ip[IP_LEN];

	char file_name[FILE_NAME_LEN];

	unsigned long file_time_stamp;

	int socket_fd;

	struct _peer_side_peer_t *next;
} peer_peer_t;


typedef struct _tracker_side_peer_t {
	//Remote peer IP address, 16 bytes.
	char ip[IP_LEN];

	//Last alive timestamp of this peer.
	unsigned long last_time_stamp;

	//TCP connection to this remote peer.
	int sockfd;

//Pointer to the next peer, linked list.
	struct _tracker_side_peer_t *next;

} tracker_peer_t;


typedef struct {

	filepath // Path of the file

	size // Size of the file

	lastModifyTime // time stamp of last modification
} FileInfo;






// _______________________________________
//
// Tracker Role
// _______________________________________
//
//
// * Collect file info from peers
//
// * Compare track-side with peer-side files
//
// * Keep record of all files
//
// * Informs peers on file updates
//
// NOTE: tracker does not contain the actual file
//
// * Monitors online/alive peers
//
// 		~> Must recieve alive message every 10 min
//
// 		~> Tracker shakes hand with peers, so we need trackerTable
//
// 			 	-> trackerTable:	typedef struct trackerTable {
// 										int PeerIP;
// 										int timeStamp;
// 										} trackerTable_t
//
// _______________________________________
//
// ~~> Tracker Threads <~~
//
// 		~> Main Thread:
//
// 			-> Listen on handshake port
//
// 			-> pthread_create(HandShake thread) when new peer joins
//
// 		~> Handshake thread:
//
// 			 -> Receive messages from a specific peer
//
// 			 -> If needed respond using peer-tracker handshake protocol (PTP)
//
//		~> Monitoring thread:
//
//			-> monitor and accept alive message from peers
//
//			-> Remove dead peers if timeout occurs
//
// _______________________________________







// _______________________________________
//
// Peer Node Role
// _______________________________________
//
//
//
// * Monitors a local file director
//
// * Communicates with the tracker
//
// * Updates files
//
// _______________________________________
//
// ~~> Monitors a local file director <~~
//
// Monitoring local direct:
//
// 		~> define local directory for synchrization
//
// 		~> node monitors LOCAL folder
//
// 		~> if any updates, sends hand shake messages to tracker
//
// ________________________________________
//
// ~~> Updates files <~~
//
// Downloading and Uploading files:
//
// 		~> upload/download newest files w/ P2P
//
// 		~> THREADS:
// 			-> Peer has thread that listens on messages from others
//
// 			-> Peer thread to create P2P connection to upload/download files.
//
// 		~> Peer side table:
//
// 			-> tracks all its existing P2P download threads
//
// _________________________________________
//
// 	~~> Peer Threads <~~
//
// 		~> Main Thread:
//
// 			-> Make connection to Tracker
//
// 			-> If needed, pthread_create( P2PDownload_Threads )
//
// 		~> P2P listening thread:
//
// 			-> Listen on the P2P port
//
// 			-> when receiving data request from other peer node, pthread_create( P2PUpload Thread )
//
// 		~> P2P download thread:
//
// 			-> download data from the remote peer
//
// 		~> P2P Upload thread:
//
// 			-> upload data to the remote peer
//
// 		~> File monitoring thread:
//
// 			-> monitor a local file directory
//
// 			-> when file changes in the local file directory, send out updated file table to the tracker
//
// 		~> Alive thread:
//
// 			-> send out HeartBeat ( means node is alive ) message to tracker
//
// 	 _______________________________________
//





//  _______________________________________
//
//  Peer-Tracker Protocol (PTP) Implementation:
//  _______________________________________
//
// * PTP = Structure of packets sent between peers and the tracker
//
//  	PeerNode(device) <-> Tracker <-> PeerNode(device)
//
// * Peer to Tracker
//
//  	~> when new Peer, Peer tells tracker
//
//  	~> Send keep-alive messages
//
//  	~> send local table
//
//
// * Tracker to Peer
//
// 		~> if new Peer, tracker provides info for set-up
//
// 		~> Broad Cast updated fileTable
//
//
//  _______________________________________
//
// ~~> Implementation/Design Specs of PTP <~~
//
//  	-> when new peer, send( packet.type = REGISTER )
//
//  			-> if tracker recieved( packet.type = REGISTER ) && recieved( packet.data =  NULL) then tracker_Table[NewPeerNode] = NewPeerNode
//
//  	-> if tracker recieved( packet.type = REGISTER ), then send( NewPeerNode , packet.interval = Interval , packet.piece_len = Piece Length )
//
//  	-> Now peer must send( packet.type = Keep_Alive ) after sleep(SOME_TIME_INTERVAL)
//
//  		-> if tracker recieved( packet.type = Keep_Alive ), then send( sleep( SOME_TIME_INTERVAL ) )
//
//  	-> if change/update in peer local directory, then peer updates packet.Local_File_Table = Updated_Local_File_Table
//
//  			-> send( packet.type = FILE_UPDATE , packet.Local_File_Table = Updated_Local_File_Table )
//
//  	-> if tracker recieved( packet.type = FILE_UPDATE , packet.Local_File_Table), then
//
//  			tracker compares tracker_Table[PeerNodeID] (which has the past file information for peer node) with packet.Local_File_Table
//
//  			   -> if tracker_Table[PeerNodeID] != packet.Local_File_Table, then
//
//  					tracker updates tracker_Table[PeerNodeID] = Updated_info;
//
//  					tracker send_To_ALL_NODES( tracker_Table[PeerNodeID] = Updated_info );
//  					(tracker broadcasts the updated file table to all alive peers)
//
//
//  _______________________________________
//
//
//
// ~~> Peer to Tracker : Table Variables <~~
//
//
// |Protocol Length |Protocol Name |Request Type |Reserved |Peer IP |Listening Port |Files |
//
//
//
//		-> Protocol Length : protocol length
//
//		-> Protocol Name : protocol name
//
//		-> Reserved : reserved space for the further extensions
//
//		-> Request Type : type of the packet: 0 –REGISTER 1 –KEEP ALIVE 2 –FILE UPDATE
//
//		-> Peer IP : IP address of the peer
//
//		-> Listening Port : p2p listening port number
//
//		-> Files : file table of the peer
//
// _________________________________________
//
//
//
// ~~> Tracker to Peer : Table Variables<~~
//
//
// |Interval |Piece Length |Files |
//
//
//		-> Interval : interval between two consecutive keep-alive messages
//
// 		-> Piece Length : length of a piece of the file
//
// 		-> Files : file table of the server.
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//






