What is the purpose of NODUS?
What functionalities I want?
--TorrentFiles: .torrent which will have hashes of the pieces, metadata of the file
--TorrentTracker : No torrent tracker in our case, we'll use DHT and KADEMLIA
--Peers, Seeds and Leachers

--------------------------------------------------
TERMINAL-UI
--------------------------------------------------

A) What the menu will ask? We'll have an ASCII animation in the terminal, a 5-second animation, then after that a static window with dropdowns and other options.
B) We'll show graphs**, very important, network performance graphs, when a file is being downloaded, etc. (complex as well)
C) You'll have options to choose files to upload, or search a particular file by uploading the .torrent file.

--------------------------------------------------
ARCHITECTURAL WORKFLOW
--------------------------------------------------

a) You can declare/host a file, say : "I Have file xu12ed91e, my IP is : 198.162.10.1, etc.."
b) When we're declaring that, "I have a file!, we also want a few close-by peers, to hold that file as well, just in case our node goes missing/disconnects"
    b.1) We'll also be implementing a compression algorithm, just like .zip files etc, a table that maps the compression for sucessfull re-cryption or de-compression.

c) You can ask : "Does anyone have file 1ex1ecjd?"
d) You'll get a response, IP:198.... has the file
e) Then a conversation begins here, we'll check our status, choked or unchoked, this is the networking part.
    e.1) Choked, basically is for free-riders, if there's someone who simply downloads, without uploading anything, then they are choked, meaning they cannot download further, but will keep on uploading
    e.2) So there will be a proper choking/unchoking algorithm as well, we can implement a simpler version as well, unchoke the top 3 peers + one random peer or an optimistic peer. (Optimistic Unchoke)

// There's connectivity problem, we can have two tiers, first one ofcourse will be a local network system, where the nodes are connected on a local network.
// Second ofcourse that we're sending messages anywhere, that is difficult and we'll avoid this for now

f) You can then download pieces from different files asynchronously.Different threads/cores will be at work here.
    f.1) When you completely download a file, you again perform operation b)
    f.2) Rarest First principle when downloading a piece of a file
    f.3) EndGame mode, as download nears completion, clients request remaining pieces from all available peers

g) Quick Bandwidth Estimation : Estimate peer upload bandwidth to intelligently unchoke faster peers. A tit-for-tat policy, upload for those who upload for you (and fast)

h) Parwise Block-Level : Enforce strict fairness, upload blocks only if uploaded are less than or equal to downloaded blocks.

i) Also, based on the number of cores present in the, we can ask the user a core/based or thread based workflow? or hybrid? and also, ask the user to set aside certain cores/threads for simply sharing the files, so that whenever the torrent client is active, it has transparency with the user, about everything.






# CID
Input bytes
   ↓
SHA-256
   ↓
Multihash
   ↓
Add version + codec
   ↓
Binary CID
   ↓
Base32 encode
   ↓
Final CID string

Codecs : dag - pb , dab - cbor, raw
Multihases : sha256, sha512, sha128
Proper decoding + validation
+ 
DAG PB Serialization

Chunk file
Create protobuf nodes
Hash node
Create Merkle DAG
Return root CID
