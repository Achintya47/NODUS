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

class PieceGenerator {
    public: 



        std::vector<unsigned char> sha256_raw(const std::vector<char>& buffer,
            std::streamsize length) {
                // Context Object
                EVP_MD_CTX* ctx = EVP_MD_CTX_new();
                if (!ctx)
                    throw std::runtime_error("Failed to create EVP_MD_CTX");
                
                // Initialize sha256 instance
                const EVP_MD* md = EVP_sha256();

                if (EVP_DigestInit_ex(ctx, md, nullptr) != 1)
                    throw std::runtime_error("EVP_DigestInit_ex failed");

                if (EVP_DigestUpdate(ctx, buffer.data(), length) != 1)
                    throw std::runtime_error("EVP_DigestUpdate failed");

                unsigned char hash[EVP_MAX_MD_SIZE];
                unsigned int hash_len = 0;

                if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1)
                    throw std::runtime_error("EVP_DigestFinal_ex failed");

                EVP_MD_CTX_free(ctx);

                return std::vector<unsigned char>(hash, hash + hash_len);
        }

        // The performance gain is not significant, thus we'll initially stick with the og
        // std::vector<unsigned char> sha256_raw_2(EVP_MD_CTX* ctx,
        //     const std::vector<char>& buffer, std::streamsize length) {

        //         const EVP_MD* md = EVP_sha256();

        //         if (EVP_DigestInit_ex(ctx, md, nullptr) != 1)
        //             throw std::runtime_error("EVP_DigestInit_ex failed");

        //         if (EVP_DigestUpdate(ctx, buffer.data(), length) != 1)
        //             throw std::runtime_error("EVP_DigestUpdate failed");

        //         unsigned char hash[EVP_MAX_MD_SIZE];
        //         unsigned int hash_len = 0;

        //         if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1)
        //             throw std::runtime_error("EVP_DigestFinal_ex failed");

        //         return std::vector<unsigned char>(hash, hash + hash_len);
        // }

        std::vector<unsigned char> piece_generator(const std::string& file_path,
            const size_t buffer_size) {
                std::ifstream file(file_path, std::ios::binary);
                if (!file)
                    throw std::runtime_error("Failed to open file");

                std::vector<char> buffer(buffer_size);
                std::vector<unsigned char> pieces_blob;

                size_t piece_count = 0;

                // Context initialization overhead is very small, thus no issues
                // EVP_MD_CTX* ctx = EVP_MD_CTX_new();
                // if (!ctx) throw std::runtime_error("Failed to create EVP_MD_CTX");


                while (file.read(buffer.data(), buffer_size) || file.gcount() > 0) {
                        std::streamsize bytesRead = file.gcount();

                        std::vector<unsigned char> hash = sha256_raw(buffer, bytesRead);

                        // append raw 32-byte hash
                        pieces_blob.insert(pieces_blob.end(), hash.begin(), hash.end());

                        piece_count++;

                        std::cout << "Piece: " << piece_count
                                << " Size: " << bytesRead
                                << " bytes\n";
                }
                std::cout << "Total Pieces: " << piece_count << "\n";

                return pieces_blob;
            }

};

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    PieceGenerator piece_gen;
    std::vector<unsigned char> hash_blob = piece_gen.piece_generator("In search of the castaways.txt", 1024);
    std::cout << '\n' << hash_blob.size() << '\n';

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Execution time: " << duration.count() << " ms\n";
    return 0;

}

class FileScanner {};
class SHA1Hasher {}; 
class Bencoder {};
class TorrentFileBuilder {};
