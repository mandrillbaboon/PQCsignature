#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <string>

constexpr size_t BITS_PER_SET = 100000;
constexpr size_t BYTES_PER_SET = BITS_PER_SET / 8;
constexpr int NUM_SETS = 1;

std::atomic<bool> active;
std::atomic<bool> coin;
std::atomic<uint64_t> x64{0};
std::atomic<uint64_t> y64{0};
std::vector<uint64_t> sampleX_dump;




void threadA() {
    active = true;
    for (int n = 0; n < 1000000; ++n) {
        coin.store(n % 2, std::memory_order_relaxed);
    }
    active = false;
}


void threadB() {
    for (int n = 1000000; n > 0; --n) {
        coin.store(n % 2, std::memory_order_relaxed);
    }
    active = false;
}


void samplerX() {
    while (active) {
        uint8_t loc = coin.load(std::memory_order_relaxed);
        uint64_t current = x64.load(std::memory_order_relaxed);
        current = (current & ~1ULL) | loc;
        current <<= 1;
        x64.store(current, std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

void samplerY() {
    while (active) {
        uint8_t loc = coin.load(std::memory_order_relaxed);
        uint64_t current = y64.load(std::memory_order_relaxed);
        current = (current & ~1ULL) | loc;
        current <<= 1;
        y64.store(current, std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}


// Fold 64-bitów do 1 bitu
uint8_t fold_bits(uint64_t bits) {
    uint8_t result = 0;
    for (int i = 0; i < 64; ++i)
        result ^= (bits >> i) & 1;
    return result;
}

// von Neumann unbiasing
bool fair_coin(uint8_t a, uint8_t b, bool &bit) {
    if (a == b) return false;
    bit = (a == 1);
    return true;
}

std::vector<uint8_t> generate_random_bits() {
    sampleX_dump.clear();
    std::vector<uint8_t> buffer(BYTES_PER_SET, 0);

    size_t bit_index = 0;

    

    

    while (bit_index < BITS_PER_SET) {
        active = true;
        x64 = 0;
        y64 = 0;
        std::thread ta(threadA);
        std::thread tb(threadB);
        std::thread sx(samplerX);
        std::thread sy(samplerY);
        ta.join();
        tb.join();
        sx.join();
        sy.join();

        uint64_t sampleX = x64.load();
        uint64_t sampleY = y64.load();
        sampleX_dump.push_back(sampleX);

        uint8_t bitA = fold_bits(sampleX);
        uint8_t bitB = fold_bits(sampleY);
        bool unbiased_bit;
        if (fair_coin(bitA, bitB, unbiased_bit)) {
            size_t byte_pos = bit_index / 8;
            size_t bit_pos = 7 - (bit_index % 8);
            buffer[byte_pos] |= (unbiased_bit << bit_pos);
            ++bit_index;
            //std::cout << std::hex << "y64: " << sampleX << "\nx64: " << sampleY  <<std::endl;
            std::cout << std::dec << bit_index << std::endl; 
            
        }
    }

    active = false;

    return buffer;
}

void save_to_file(const std::string &filename, const std::vector<uint8_t> &data) {
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char *>(data.data()), data.size());
    out.close();
}
void save_sampleX_dump(const std::string &filename, const std::vector<uint64_t> &data) {
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char *>(data.data()), data.size() * sizeof(uint64_t));
    out.close();
}


int main() {


    for (int i = 1; i <= NUM_SETS; ++i) {
        std::cout << "\n🔄 Generowanie zestawu " << i << "...\n";

        std::vector<uint8_t> bits;
        int attempt = 0;
            bits = generate_random_bits();
            if (!bits.empty()) break;



        std::string base_name = "output" + std::to_string(i);
        save_to_file(base_name + ".bin", bits);
        save_sampleX_dump(base_name + "_sampleX_dump.bin", sampleX_dump);
    }
    return 0;
}