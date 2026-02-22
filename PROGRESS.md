<pre align = "center">
███╗   ██╗ ██████╗ ██████╗ ██╗   ██╗███████╗
████╗  ██║██╔═══██╗██╔══██╗██║   ██║██╔════╝
██╔██╗ ██║██║   ██║██║  ██║██║   ██║███████╗
██║╚██╗██║██║   ██║██║  ██║██║   ██║╚════██║
██║ ╚████║╚██████╔╝██████╔╝╚██████╔╝███████║
╚═╝  ╚═══╝ ╚═════╝ ╚═════╝  ╚═════╝ ╚══════╝
</pre>

## 13-02-2026
Implemented piece_generator() function, which takes in a filestream (binary), based on the buffer (1KB) generates raw-hash content and concatenates that into a hash-blob.
    *What's Next* :
        Since this as of now works fine, we'll move with implementation of : 
            1. Bencoded info dict and a final hash of that
            2. The pieces created also need to be stored, we'll implement a compression algorithm as well for the file.
            
    *What's to be improved?* : 
        1. Well, piece_generator() currently does : 
                1. Open the file
                2. Hash the file
                3. Manages the buffer and all
            We need to modulate this and seperate logics
        2. Precompute piece count and reserve memory, no dynamic growth
            piece_blob.reserve(piece_count * 32)
        3. Piece size etc.. must be part of NODUS's metadata


## 23-02-2026
Had a long break, IIT Roorkee + VaishnoDevi + Brother's marriage, anyways getting back at the architecture.

1. Implement Proper Bencoding
2. Build the info dictionary correctly
3. Generate info-hash (SHA1 of bencoded info dict)
4. Build .torrent file
5. Implement magnet link generator
6. Implement minimal DHT node (Kademlia)
7. Implement peer wire protocol

## METADATA LAYER 
step 1 : split file into pieces, each piece is raw bytes from the file
step 2 : hash each piece (sha1)
step 3 : concatenate the hashes into a piece blob
step 4 : build info dictionary
    {
        "name": "file.txt",
        "length": 5242880,
        "piece length": 16384,
        "pieces": <binary concatenated SHA1 hashes>
    }
step 5 : bencode the info dict, we need proper sequence of these information, bencoding enforces deterministic format
step 6 : generate info hash (sha1(bencoded info)), this is where DHT comes into play
step 7 : build the .torrent file, containing the file metadata, and piece hashes, not actual piece data

## SHARING LAYER
step 1 : A peer has the info hash, they ask on the network for peers with pieces of the info hash
step 2 : pieces respond, the user recieves data and compares piece hashes

## 23-02-2026 0153 hours
File → Pieces → Piece Hashes → Info Dictionary → Bencoded Bytes → Info Hash

Implemented recursive encoding, using Bencoder::encode(info_dict)
Then finally we generate the info hash, by hashing the bencode_info, this will be our DHT Lookup key, which will be queried against the ip-addresses
    *What's Next* :
        1. We need to get ideas about the connection, and protocols
        2. Setup bootstraps like dht.transmissionbt.com or router.bittorrent.com for a peer to join the dht network.
            2.1. After joining, logic is simple, you talk to peers directly following the torrent protocol


