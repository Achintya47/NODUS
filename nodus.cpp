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
#include <map> // for bencode dicts
#include <variant> // for bencode variant dtypes

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
            hasher_(hasher), piece_count_(0) {}

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


class BencodeValue {
    public: 
        using Integer = int64_t;
        using String = std::vector<unsigned char>;
        using List = std::vector<BencodeValue>;
        using Dict = std::map<std::string, BencodeValue>;

        BencodeValue(Integer val) : value_(val) {}
        BencodeValue(const String& val) : value_(val) {}
        BencodeValue(const List& val) : value_(val) {}
        BencodeValue(const Dict& val) : value_(val) {}

        const auto& value() const { return value_; }

    private:
        std::variant<Integer, String, List, Dict> value_;

        friend class Bencoder;
};


class Bencoder {
    public:
        static std::vector<unsigned char> encode(const BencodeValue& val);

    private:
        static void encode_value(const BencodeValue& val,
                                std::vector<unsigned char>& out);

        static void encode_integer(int64_t value,
                                std::vector<unsigned char>& out);

        static void encode_string(const std::vector<unsigned char>& str,
                                std::vector<unsigned char>& out);

        static void encode_list(const BencodeValue::List& list,
                                std::vector<unsigned char>& out);

        static void encode_dict(const BencodeValue::Dict& dict,
                            std::vector<unsigned char>& out);
};

std::vector<unsigned char> Bencoder::encode(const BencodeValue& val) {
    std::vector<unsigned char> output;
    encode_value(val, output);
    return output;
}

void Bencoder::encode_value(const BencodeValue& val,
                            std::vector<unsigned char>& out) {
    const auto& v = val.value();

    if (std::holds_alternative<BencodeValue::Integer>(v)) {
        encode_integer(std::get<BencodeValue::Integer>(v), out);
    }
    else if (std::holds_alternative<BencodeValue::String>(v)) {
        encode_string(std::get<BencodeValue::String>(v), out);
    }
    else if (std::holds_alternative<BencodeValue::List>(v)) {
        encode_list(std::get<BencodeValue::List>(v), out);
    }
    else if (std::holds_alternative<BencodeValue::Dict>(v)) {
        encode_dict(std::get<BencodeValue::Dict>(v), out);
    }
}

void Bencoder::encode_integer(int64_t value,
                              std::vector<unsigned char>& out)
{
    out.push_back('i');
    auto str = std::to_string(value);
    out.insert(out.end(), str.begin(), str.end());
    out.push_back('e');
}

void Bencoder::encode_string(const std::vector<unsigned char>& str,
                             std::vector<unsigned char>& out)
{
    auto len_str = std::to_string(str.size());
    out.insert(out.end(), len_str.begin(), len_str.end());
    out.push_back(':');
    out.insert(out.end(), str.begin(), str.end());
}

void Bencoder::encode_list(const BencodeValue::List& list,
                           std::vector<unsigned char>& out)
{
    out.push_back('l');
    for (const auto& item : list) {
        encode_value(item, out);
    }
    out.push_back('e');
}

void Bencoder::encode_dict(const BencodeValue::Dict& dict,
                           std::vector<unsigned char>& out)
{
    out.push_back('d');

    // std::map guarantees lexicographic key ordering
    for (const auto& [key, value] : dict) {

        std::vector<unsigned char> key_bytes(key.begin(), key.end());
        encode_string(key_bytes, out);

        encode_value(value, out);
    }

    out.push_back('e');
}

BencodeValue build_info_dict (
    const std::string& name,
    uint64_t length,
    size_t piece_length,
    const std::vector<unsigned char> & pieces_blob) {

        BencodeValue::Dict dict;
        dict["length"] = BencodeValue(static_cast<int64_t>(length));
        dict["name"] = BencodeValue(std::vector<unsigned char>(name.begin(), name.end()));
        dict["piece length"] = BencodeValue(static_cast<int64_t>(piece_length));
        dict["pieces"] = BencodeValue(pieces_blob);
    
        return BencodeValue(dict);
}


int main() {
    SHA1Hasher hasher;

    FileScanner scanner("file.txt");
    size_t PIECE_SIZE = 1024;

    PieceManager manager(PIECE_SIZE, hasher);

    manager.process(scanner);

    auto info_dict = build_info_dict(
        "file.txt", 
        scanner.file_size(),
        PIECE_SIZE,
        manager.pieces_blob()
    );

    auto bencoded_info = Bencoder::encode(info_dict);

    auto info_hash = hasher.hash(
        reinterpret_cast<const char*>(bencoded_info.data()),
        bencoded_info.size());
    
}