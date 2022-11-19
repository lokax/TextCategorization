#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <deque>
#include <exception>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cstdint>
#include <filesystem>

#include <cstring>

class FileReader {
public:
    FileReader() = default;

    explicit FileReader(const std::string& file_name, size_t index)
    {
        file_stream.open(file_name, std::ios::binary | std::ios::in);
        if (!file_stream.is_open()) {
            std::cout << "DEBUG: name = " << file_name << "\n";
            std::cout << "DEBUG: index = " << index << "\n";
            throw std::invalid_argument("Open file error" + file_name);
        }
    }

    ~FileReader()
    {
        if (file_stream.is_open()) {
            file_stream.close();
        }
    }

    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;

    void ReadLine(std::string &buffer)
    {
        std::getline(file_stream, buffer);
    }

    int ReadChar()
    {
        return file_stream.get();
    }

private:
    std::fstream file_stream;
};

class FileWriter {
public:
    explicit FileWriter(const std::string& file_name, size_t index)
    {
        file_stream.open(file_name, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file_stream.is_open()) {
            std::cout << "DEBUG index = " << index << "\n";
            throw std::invalid_argument("Open file error" + file_name);
        }
    }

    ~FileWriter()
    {
        if (file_stream.is_open()) {
            file_stream.close();
        }
    }

    FileWriter(const FileWriter&) = delete;
    FileWriter& operator=(const FileWriter&) = delete;

    void WriteLine(const std::string& str)
    {
        file_stream.write(str.data(), str.size());
        file_stream.put('\n');
    }

private:
    std::fstream file_stream;
};

struct NGram {
    std::string str;
    size_t count;
    size_t pos;

    NGram(std::string s, size_t c)
        : str(s)
        , count(c)
    {
    }

    NGram(std::string s, size_t c, size_t p)
        : str(std::move(s))
        , count(c)
        , pos(p)
    {
    }
};

struct Arena {
    char* Allocate(uint64_t size)
    {
        if (!chunk || remaining_size < size) {
            auto memory = std::unique_ptr<char[]>(new char[4096]);
            chunk = memory.get();
            chunks.push_back(std::move(memory));
            remaining_size = 4096;
        }
        assert(remaining_size >= size);
        auto res = chunk;
        chunk += size;
        remaining_size -= size;
        return res;
    }

    char* chunk = nullptr;
    size_t remaining_size = 0;
    std::vector<std::unique_ptr<char[]>> chunks;
};

template <size_t N>
class CircularQueue {
public:
    int size_ { 0 };
    int l { 0 };
    int r { 0 };
    char que_[N];

    CircularQueue()
    {
    }

    bool enQueue(char value)
    {
        if (isFull())
            return false;
        que_[r] = value;
        r = (r + 1) % N;
        size_++;
        return true;
    }

    bool deQueue()
    {
        if (isEmpty())
            return false;
        l = (l + 1) % N;
        size_--;
        return true;
    }

    char Front()
    {
        if (isEmpty())
            return -1;
        int value = que_[l];
        return value;
    }

    char Rear()
    {
        if (isEmpty())
            return -1;
        int index = ((r - 1) + N) % N;
        return que_[index];
    }

    bool isEmpty()
    {
        return size_ == 0;
    }

    bool isFull()
    {
        return size_ == N;
    }

    void Traverse(char* to)
    {
        char* que_raw_ptr = que_;
        for (size_t index = 0; index < size_; index++) {
            size_t arr_index = (l + index) % N;
            to[index] = que_raw_ptr[arr_index];
        }
    }
};

struct RadixTree {
    struct RadixNode {
        bool has_child = false;
        bool has_word = false;
        size_t count = 0;
        RadixNode* children[256] = { nullptr };

        ~RadixNode()
        {
            
        }
    };

    RadixTree()
    {
        root = std::make_unique<RadixNode>();
    }

    ~RadixTree()
    {
        
    }

    void AddWord(const char* data, size_t size)
    {
        RadixNode* p = root.get();
        for (size_t index = 0; index < size; ++index) {
            uint8_t c = (uint8_t)data[index];
            if (!p->children[c]) {
                p->has_child = true;
                auto* node = arena.Allocate(sizeof(RadixNode));
                new (node) RadixNode();
                p->children[c] = (RadixNode*)node;
            }
            p = p->children[c];
        }

        unique_word_size += !p->has_word;
        p->has_word = p->has_word || true;
        p->count += 1;
        total_count++;
    }

    void AddWord(const std::string& word)
    {
        RadixNode* p = root.get();
        for (uint8_t c : word) {
            if (!p->children[c]) {
                p->has_child = true;
                auto* node = arena.Allocate(sizeof(RadixNode));
                new (node) RadixNode();
                p->children[c] = (RadixNode*)node;
            }
            p = p->children[c];
        }

        unique_word_size += !p->has_word;
        p->has_word = p->has_word || true;
        p->count += 1;
        total_count++;
    }

    std::vector<NGram> GatterWord()
    {
        std::vector<NGram> ngrams;
        // std::cout << "unique_word_size = " << unique_word_size << "\n";
        ngrams.reserve(unique_word_size);
        RadixNode* p = root.get();
        std::string path;
        path.reserve(5);
        Traverse(p, path, ngrams);
        return ngrams;
    }

    void Traverse(RadixNode* root, std::string& path, std::vector<NGram>& ngrams)
    {
        if (root->has_word) {
            ngrams.emplace_back(path, root->count, ngrams.size());
        }

        if (!root->has_child) {
            return;
        }

        for (size_t i = 0; i < 256; ++i) {
            if (root->children[i]) {
                path.push_back((char)i);
                Traverse(root->children[i], path, ngrams);
                path.pop_back();
            }
        }
    }

    std::unique_ptr<RadixNode> root;
    size_t unique_word_size = 0;
    size_t total_count = 0;
    Arena arena;
};

template <size_t NGRAM>
class NGramParser {
public:
    NGramParser(const std::string& input_file_name, const std::string& output_file_name, size_t index)
        : reader_(input_file_name, index)
        , output_file_name_(output_file_name)
        , debug_index_(index)
    {
    }

    /*
    NGramParser(std::string&& text, size_t index)
        : text_(std::move(text))
        , debug_index_(index)
    {
    }
    */

    NGramParser(size_t index)
        : debug_index_(index)
    {
    }

    std::vector<NGram> ParseText(const std::string& text)
    {
        
        // Clear hash table
        ngram_map_.clear();
        int c = 0;
        bool seen = false;
        for (size_t index = 0; index < text.size(); ++index) {
            c = text[index];
            bool need = NeedToReplace(c);
            if (need && !seen) {
                seen = true;
                c = '_';
                Process<true>(c);
            } else if (!need) {
                seen = false;
                Process<true>(c);
            }
        }

        // auto ngrams = tree_.GatterWord();

        std::vector<NGram> ngrams;
        ngrams.reserve(ngram_map_.size());
        for (const auto& ngram : ngram_map_) {
            ngrams.push_back(NGram(ngram.first, ngram.second));
        }
        

        std::sort(ngrams.begin(), ngrams.end(), [&](const NGram& lhs, const NGram& rhs) {
            if (lhs.count == rhs.count) {
                return lhs.str < rhs.str;
            }
            return lhs.count < rhs.count;
        });

        return std::move(ngrams);
    }

    void Parse()
    {
        auto start = std::chrono::system_clock::now();
        int c = 0;
        bool seen = false;
        while ((c = reader_.ReadChar()) != EOF) {
            bool need = NeedToReplace(c);
            if (need && !seen) {
                seen = true;
                c = '_';
                Process(c);
            } else if (!need) {
                seen = false;
                Process(c);
            }
        }
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "parse time : " << (double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒"
                  << "\n";
    }

    template <bool HASH_TABLE = false>
    void Process(int c)
    {
        // char_cacher_.push_back((char)c);
        // size_t size = char_cacher_.size();
       //  std::cout << "char : " << (int)c << "\n";
        cque_.enQueue((char)c);
        size_t size = cque_.size_;

        // deque_vec_.push_back((char)c);
        // size_t loop_size = std::min(deque_vec_.size(), NGRAM);
        // size_t size = deque_vec_.size();

        // std::string str;
        // str.reserve(5);

        static char chars[NGRAM];
        cque_.Traverse(chars);

        /*
        for(size_t i = 0; i < NGRAM; ++i) {
            std::cout << (int)chars[i] << ", ";
        }
        std::cout << "\n";
        */

        // Fast restrive
        /*
        deque_vec_.clear();
        for (size_t index = 0; index < size; ++index) {
            deque_vec_.push_back(char_cacher_[index]);
        }
        */

        for (size_t index = size; index > 0; --index) {
            // str.clear();
            // size_t c_idx = 0;
            if constexpr (!HASH_TABLE) {
                // std::cout << "std::string = " << std::string(chars + (size - index), index) << "\n";
                tree_.AddWord(chars + (size - index), index);
            } else {
                // std::cout << "std::string = " << std::string(chars + (size - index), index) << "\n";
                ngram_map_[std::string(chars + (size - index), index)]++;
            }

            /*
            for (size_t vec_index = size - index; vec_index < size; vec_index++) {
                str.push_back(deque_vec_[vec_index]);
            }
            */

            /*
            for (size_t vec_index = size - index; vec_index < size; vec_index++) {
                str.push_back(deque_vec_[vec_index]);
            }
            */

            /*
            auto iter = char_cacher_.begin() + (size - index);
            while(iter != char_cacher_.end()) {
                str.push_back(*iter);
                ++iter;
            }
            */
            /*
            for (size_t vec_index = 0; vec_index < 5; ++vec_index) {
                str.push_back(deque_vec_[vec_index]);
            }
            */
            /*
            for (size_t deq_index = size - index; deq_index < size; deq_index++) {
                str.push_back(char_cacher_[deq_index]);
                // chars[c_idx++] = char_cacher_[deq_index];
            }
            */
            // string_t str(chars, c_idx);
            // ngram_map_vec_[0][str]++;

            // radix tree
            // tree_.AddWord(str);
        }

        if (cque_.isFull()) {
            cque_.deQueue();
        }

        /*
        if (size == NGRAM) {
            char_cacher_.pop_front();
        }
        */

        /*
        if (size == 1024) {
            for (size_t index = 0; index < NGRAM; ++index) {
                deque_vec_[index] = deque_vec_[size - NGRAM + index];
            }
            deque_vec_.resize(NGRAM);
            std::cout << "DEBUG: " << deque_vec_.capacity() << "\n";
        }
        */
    }

    void Generate()
    {
#if 1
        std::cout << "AddWord call nums : " << tree_.total_count << "\n";
        auto start = std::chrono::system_clock::now();
        auto ngrams = tree_.GatterWord();
        std::sort(ngrams.begin(), ngrams.end(), [&](const NGram& lhs, const NGram& rhs) {
            if (lhs.count == rhs.count) {
                return lhs.pos < rhs.pos;
            }
            return lhs.count > rhs.count;
        });

        FileWriter writer(output_file_name_, debug_index_);

        auto min_size = std::min<size_t>(ngrams.size(), 400);
        for (size_t index = 0; index < min_size; ++index) {
            auto encode_str = std::move(ngrams[index].str);
            encode_str += " ";
            encode_str += std::to_string(ngrams[index].count);
            writer.WriteLine(encode_str);
        }

#endif

#if 0
        auto start = std::chrono::system_clock::now();
        std::vector<std::vector<NGram>> ngram_vecs(NGRAM);
        for (size_t index = 0; index < NGRAM; ++index) {
            auto& ngram_map = ngram_map_vec_[index];
            auto& ngram_vec = ngram_vecs[index];
            ngram_vec.reserve(ngram_map.size());
            auto iter = ngram_map.begin();
            while (iter != ngram_map.end()) {
                ngram_vec.push_back(NGram(iter->first, iter->second));
                iter++;
            }
        }
        std::vector<std::future<void>> futs;
        for (size_t index = 0; index < NGRAM; ++index) {
            auto& ngram_vec = ngram_vecs[index];

            auto fut = std::async(std::launch::async, [&] { std::sort(ngram_vec.begin(), ngram_vec.end(), [&](const NGram& lhs, const NGram& rhs) {
                                                                if (lhs.count == rhs.count) {
                                                                    return lhs.str < rhs.str;
                                                                }
                                                                return lhs.count > rhs.count;
                                                            }); });
            futs.push_back(std::move(fut));

            /*
            std::sort(ngram_vec.begin(), ngram_vec.end(), [&](const NGram& lhs, const NGram& rhs) {
                if (lhs.count == rhs.count) {
                    return lhs.str < rhs.str;
                }
                return lhs.count > rhs.count;
            });
            */
        }

        for (size_t index = 0; index < NGRAM; ++index) {
            futs[index].get();
        }

#endif
#if 0
        auto start = std::chrono::system_clock::now();

        size_t size = 0;
        for (size_t index = 0; index < NGRAM; ++index) {
            size += ngram_map_vec_[index].size();
        }
        std::cout << "size = " << size << "\n";
        std::vector<NGram> ngram_vec;
        ngram_vec.reserve(size);
        for (size_t index = 0; index < NGRAM; ++index) {
            auto& ngram_map = ngram_map_vec_[index];
            auto iter = ngram_map.begin();
            while (iter != ngram_map.end()) {
                ngram_vec.push_back(NGram(iter->first, iter->second));
                iter++;
            }
        }

        std::sort(ngram_vec.begin(), ngram_vec.end(), [&](const NGram& lhs, const NGram& rhs) {
            if (lhs.count == rhs.count) {
                return lhs.str < rhs.str;
            }
            return lhs.count > rhs.count;
        });
#endif

        auto end
            = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "generate time : " << (double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒"
                  << "\n";

        /**
        for (const auto& ngram : ngram_vec) {
            std::cout << "value : " << ngram.str << ", count : " << ngram.count << "\n";
        }
        */

        /*
        for (size_t index = 0; index < NGRAM; ++index) {
            std::cout <<        "############# " << index + 1 << "gram"
                      << "\n";
            for (const auto& ngram : ngram_vecs[index]) {
                std::cout << "value : " << ngram.str << ", count : " << ngram.count << "\n";
            }
        }
        */
    }

private:
    FileReader reader_;
    std::string output_file_name_;
    // std::unordered_map<string_t, int> ngram_map_vec_[NGRAM];
    // std::unordered_map<string_t, int, NGramHashFunction, NGramEquality> ngram_map_vec_[NGRAM];
    // std::unordered_map<std::string, int> ngram_map_vec_[NGRAM];
    RadixTree tree_;
    std::unordered_map<std::string, int> ngram_map_;
    // std::deque<char> char_cacher_;
    // std::vector<char> deque_vec_;
    // size_t deque_vec_index_ = 0;
    CircularQueue<NGRAM> cque_;
    size_t debug_index_;
    // std::string text_;

private:
    bool NeedToReplace(int c) const
    {
        return std::isspace(c) || std::isdigit(c);
    }
};