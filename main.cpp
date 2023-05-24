#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// Cache block struct
struct CacheBlock {
    bool valid = false;         // Valid bit
    bool dirty = false;         // Dirty bit (modified or not)
    int tag = 0;                // Tag bits
    int accessTime = 0;         // Last access time (for LRU eviction)
    vector<int> data;           // Data stored in the block
};

// Cache class
class Cache {
private:
    int blockSize;              // Block size in bytes
    int cacheSize;              // Cache size in bytes
    int associativity;          // Number of blocks per set
    int numSets;                // Number of sets in the cache
    int numBlocks;              // Total number of blocks in the cache
    int reads;                  // Number of reads
    int writes;                 // Number of writes
    int hits;                   // Number of hits
    int misses;  
    int writebacks;               // Number of misses
    int recent;         // Next access time (for LRU eviction)
    map<int, CacheBlock> cache; // Cache blocks stored as a map from tag to block

public:
    Cache(int blockSize, int cacheSize, int associativity) {
        this->blockSize = blockSize;
        this->cacheSize = cacheSize;
        this->associativity = associativity;
        this->numSets = cacheSize / (blockSize * associativity);
        this->numBlocks = numSets * associativity;
        this->reads = 0;
        this->writes = 0;
        this->hits = 0;
        this->misses = 0;
        this->recent = 0;
        this->writebacks = 0;
    }

    // Read data from cache or DRAM
    vector<int> read(int address) {
        // Compute set and tag bits from address

        reads++;

        int set = (address / blockSize) % numSets;
        int tag = address / (blockSize * numSets);

        // Search for block in cache
        for (int i = 0; i < associativity; i++) {
            int index = set * associativity + i;
            CacheBlock& block = cache[index];

            // If block is valid and tag matches, it's a hit
            if (block.valid && block.tag == tag) {
                hits++;
                block.accessTime = recent++;
                return block.data;
            }
        }

        // If block is not in cache, it's a miss
        misses++;

        // Read block from DRAM
        vector<int> data(blockSize);
        for (int i = 0; i < blockSize; i++) {
            data[i] = i + address - (address % blockSize);
        }

        int ind = set * associativity;
        CacheBlock& blk = cache[ind];
        for (int i = 0; i < associativity; i++) {
            int index = set * associativity + i;
            CacheBlock& block = cache[index];
            if (!block.valid) {
                block.valid = true;
                block.tag = tag;
                block.accessTime = recent++;
                block.data = data;
                return data;
            }
        }

        // Find LRU block to replace
        int lru = set * associativity;
        CacheBlock& lrublk = cache[lru];
        for (int i = 1; i < associativity; i++) {
            int index = set * associativity + i;
            CacheBlock& block = cache[index];
            if (block.accessTime < lrublk.accessTime) {
                lru = index;
                lrublk = block;
            }
        }

        // If LRU block is dirty, write it back to DRAM
        if (lrublk.valid && lrublk.dirty) {
            int evictedAddress = lrublk.tag * (blockSize * numSets) + set * blockSize;
            write(evictedAddress, lrublk.data);
            writebacks++;
        }

        // // Read block from DRAM
        // vector<int> data(blockSize);
        // for (int i = 0; i < blockSize; i++) {
        //     data[i] = i + address - (address % blockSize);
        // }

        // Update block in cache and return data
        lrublk.valid = true;
        lrublk.dirty = false;
        lrublk.tag = tag;
        lrublk.accessTime = recent++;
        lrublk.data = data;

        return data;
    }

    // Write data to cache and/or DRAM
    void write(int address, vector<int> data) {
        // Compute set and tag bits from address
        writes++;

        int set = (address / blockSize) % numSets;
        int tag = address / (blockSize * numSets);

        // Search for block in cache
        for (int i = 0; i < associativity; i++) {
            int index = set * associativity + i;
            CacheBlock& block = cache[index];

            // If block is valid and tag matches, it's a hit
            if (block.valid && block.tag == tag) {
                hits++;
                block.accessTime = recent++;
                block.dirty = true;
                block.data = data;
                return;
            }
        }

        // If block is not in cache, it's a miss
        misses++;

        
        int ind = set * associativity;
        CacheBlock& blk = cache[ind];
        for (int i = 0; i < associativity; i++) {
            int index = set * associativity + i;
            CacheBlock& block = cache[index];
            if (!block.valid) {
                block.valid = true;
                block.dirty = true;
                block.tag = tag;
                block.accessTime = recent++;
                block.data = data;
                return;
            }
        }

        // Find LRU block to replace
        int lru = set * associativity;
        CacheBlock& lrublk = cache[lru];
        for (int i = 1; i < associativity; i++) {
            int index = set * associativity + i;
            CacheBlock& block = cache[index];
            if (block.accessTime < lrublk.accessTime) {
                lru = index;
                lrublk = block;
            }
        }

        // If LRU block is dirty, write it back to DRAM
        if (lrublk.valid && lrublk.dirty) {
            int evictedAddress = lrublk.tag * (blockSize * numSets) + set * blockSize;
            write(evictedAddress, lrublk.data);
            writebacks++;
        }

        // Update block in cache and mark as dirty
        lrublk.valid = true;
        lrublk.dirty = true;
        lrublk.tag = tag;
        lrublk.accessTime = recent++;
        lrublk.data = data;
    }

    // Print cache statistics
    void printStats() {
        cout << "Reads: " << reads << endl;
        cout << "Writes: " << writes << endl;
        cout << "Hits: " << hits << endl;
        cout << "Misses: " << misses << endl;
        double hitRatio = (double) hits / (hits + misses);
        double missRatio = (double) misses / (hits + misses);
        cout << "Hit ratio: " << hitRatio << endl;
        cout << "Miss ratio: " << missRatio << endl;
    }
};

pair<int, uint32_t> parse(string line) {
    
    istringstream streaming(line);
    string rw, address;

    getline(streaming, rw,'\t');
    getline(streaming, address, '\t');
    streaming >> rw >> address;
    int p1 = (rw == "w") ? 1 : 0;
    uint32_t p2;
    stringstream ss;
    ss << hex << address;
    ss >> p2;
    return make_pair(p1, p2);
}

int main(int argc, char* argv[]) {

    // Parse the arguments
    int blksize = stoi(argv[1]);
    int size_l1 = stoi(argv[2]);
    int assoc_l1 = stoi(argv[3]);
    int size_l2 = stoi(argv[4]);
    int assoc_l2 = stoi(argv[5]);
    string trace_file = argv[6];

    // Open the trace file
    ifstream infile(trace_file);
    if (!infile) {
        cout << "Unable to open file: " << trace_file << endl;
        return 1;
    }

    // Read the contents of the trace file

    Cache L1(blksize, size_l1, assoc_l1);
    // Cache L2(blksize, size_l2, assoc_l2);

    string line;
    while (getline(infile, line)) {
        // Process each line of the trace file

        pair<int, int> p = parse(line);
        int rw = p.first; // read = 0, write = 1;
        int address =  p.second;

        vector<int> data;

        if(rw == 1){
            L1.write(address, data);
        }
        else if(rw == 0){
            L1.read(address);
        }

    }


    // int blockSize = 64;     // Block size in bytes
    // int cacheSize = 4096;   // Cache size in bytes
    // int associativity = 4;  // Number of blocks per set

    // Cache cache(blockSize, cacheSize, associativity);

    // // Read and write to cache
    // vector<int> data1 = {1, 2, 3, 4};
    // vector<int> data2 = {5, 6, 7, 8};
    // cache.write(0, data1);
    // cache.read(0);
    // cache.write(64, data2);
    // cache.read(0);
    // cache.read(64);

   
    // Print cache statistics
    L1.printStats();

    return 0;
}
