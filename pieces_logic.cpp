// We take input -> A .txt file, and encode it into pieces of size 16KB each.

// Workflow, 
// ScanFile -> Generate Pieces (treat file as a stream) -> Generate Hashes
// Concatenate Hashes -> Build Info Dict -> Bencode Info -> SHA1 (info bytes)

// Whats bencoder? Well since we're dealing with json, it can have different sequence for
// different attributes, thus same values ordered differently would produce different hash
// thus bencoding is like a deterministic json file manager, that makes sure that cross platform
// the files are deterministic.

#include <sstream>   // for std::ostringstream
#include <iomanip>   // for std::setw, std::setfill
#include <iostream>  // for std::cout, std::cerr
#include <fstream>   // for std::ifstream
#include <vector>    // for std::vector
#include <openssl/evp.h> // for SHA256 functions
#include <chrono>   // for noting time

// Initialize the hash utilities, hash some data and return a vector
// of the hash
class SHA1Hasher {
    public : 
        std::vector<unsigned char> hash(const char* data, size_t len) {
            EVP_MD_CTX* ctx = EVP_MD_CTX_new();
            if (!ctx)
                    throw std::runtime_error("Failed to create EVP_MD_CTX");

            if (EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr) != 1)
                throw std::runtime_error("DigestInit Failed");

            if (EVP_DigestUpdate(ctx, data, len) != 1)
                throw std::runtime_error("DigestUpdate failed");
            
            unsigned char hash[EVP_MAX_MD_SIZE];
            unsigned int hash_len = 0;

            if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1)
                throw std::runtime_error("DigestFinal failed");
            
            EVP_MD_CTX_free(ctx);
            
            return std::vector<unsigned char>(hash, hash + hash_len);
        }
};


// Scan a file and return its contents in a vector buffer, w.r.t the buffer size
class FileScanner {
    public: 
        explicit FileScanner(const std::string& path) : file_(path, std::ios::binary) {

            if (!file_) throw std::runtime_error("Failed to load file");

            file_.seekg(0, std::ios::end);
            file_size_  = file_.tellg();
            file_.seekg(0, std::ios::beg);

        }

        bool read_chunk(std::vector<char>& buffer, std::streamsize& bytesRead) {

            if (!file_) return false;
            file_.read(buffer.data(), buffer.size());
            bytesRead = file_.gcount();

            return bytesRead > 0;
        }

        uint64_t file_size() const { return file_size_; }
    private:
        std::ifstream file_;
        uint64_t file_size_;
};

// Piece manager, orchestrates the piece creation, and generates the pieces blob
class PieceManager {
    public: 
        PieceManager(size_t piece_size, SHA1Hasher& hasher) : piece_size_(piece_size),
            hasher_(hasher) {}

        void process(FileScanner& scanner) {
            std::vector<char> buffer(piece_size_);
            std::streamsize bytesRead;

            while (scanner.read_chunk(buffer, bytesRead)) {
                std::vector<unsigned char> hash = hasher_.hash(buffer.data(), bytesRead);

                pieces_blob_.insert(
                    pieces_blob_.end(),
                    hash.begin(),
                    hash.end()
                );

                piece_count_++; 
            }
        }

        const std::vector<unsigned char>& pieces_blob() const{
            return pieces_blob_;
        }

        size_t piece_count() const {
            return piece_count_;
        }

    private:
        size_t piece_size_;
        SHA1Hasher hasher_;
        std::vector<unsigned char> pieces_blob_;
        size_t piece_count_;
    };

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Execution time: " << duration.count() << " ms\n";
    return 0;
    

}

