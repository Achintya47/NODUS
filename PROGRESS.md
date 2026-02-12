<pre align = "center">
███╗   ██╗ ██████╗ ██████╗ ██╗   ██╗███████╗
████╗  ██║██╔═══██╗██╔══██╗██║   ██║██╔════╝
██╔██╗ ██║██║   ██║██║  ██║██║   ██║███████╗
██║╚██╗██║██║   ██║██║  ██║██║   ██║╚════██║
██║ ╚████║╚██████╔╝██████╔╝╚██████╔╝███████║
╚═╝  ╚═══╝ ╚═════╝ ╚═════╝  ╚═════╝ ╚══════╝
</pre>

### 13-02-2026
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
