#include <stdio.h>
#include <time.h>
#include "list.h"
#include "hash.h"
#include "test.h"

#define COUNT 100000
#define HOT_RATIO 10  // 10% hot

/* Helper function to output the MAC address */
void print_mac(uint8_t mac[6]) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/* viper mac generator */
void viper_gen_mac(uint8_t *mac, int idx) {
    /* The first 3~24 bits are vendor identifier */
    mac[0] =
        0x02; /* Locally administered address, unicast (binary: 00000010) */
    mac[1] = 0xab;
    mac[2] = 0xcd;
    /* Assign uniqueness MAC address by idx */
    mac[3] = (idx >> 16) & 0xFF;
    mac[4] = (idx >> 8) & 0xFF;
    mac[5] = idx & 0xFF;
}

void print_memory_usage() {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0 || strncmp(line, "VmPeak:", 7) == 0) {
            printf("%s", line);
        }
    }
    fclose(f);
}

void get_memory_usage_kb(int *vmrss_kb, int *vmpeak_kb) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) {
        *vmrss_kb = *vmpeak_kb = -1;
        return;
    }
    char line[256];
    *vmrss_kb = *vmpeak_kb = -1;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%d", vmrss_kb);
        }
        if (strncmp(line, "VmPeak:", 7) == 0) {
            sscanf(line + 7, "%d", vmpeak_kb);
        }
    }
    fclose(f);
}

/* Benchmark function */
void benchmark(const char* name,
                 void (*add_func)(uint8_t*, int),
                 int (*lookup_func)(uint8_t*)) {
    uint8_t mac[6];
    clock_t start, end;
    int vmrss_kb = 0, vmpeak_kb = 0;

    printf("=== %s ===\n", name);

    // Insert
    start = clock();
    for (int i = 0; i < COUNT; ++i) {
        viper_gen_mac(mac, i);
        add_func(mac, i % 8);
    }
    end = clock();
    double insert_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Insert Time: %.4fs\n", insert_time);

    // Lookup
    int miss = 0;
    start = clock();
    for (int i = 0; i < COUNT; ++i) {
        viper_gen_mac(mac, i);
        if(lookup_func(mac) == -1)
            ++miss;
    }
    end = clock();
    
    double lookup_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Lookup Time: %.4fs\n", lookup_time);
    printf("\n");
    printf("MISS: %d\n", miss);
    
    double miss_rate = (double)miss / (double)COUNT;
    printf("Miss Rate: %.4f\n", miss_rate);
    printf("\n");

    // Get memory usage
    get_memory_usage_kb(&vmrss_kb, &vmpeak_kb);
    printf("VmRSS: %d (KB)\n", vmrss_kb);
    printf("VmPeak: %d (KB)\n", vmpeak_kb);

    FILE *csv = fopen("benchmark.csv", "a");
    fprintf(csv, "%s,%.4f,%.4f,%d,%d\n", name, insert_time, lookup_time, vmpeak_kb, vmrss_kb);
    fclose(csv);

}

/* Hot/Cold Benchmark function */
void benchmark_hot_cold(const char* name,
    void (*add_func)(uint8_t*, int),
    int (*lookup_func)(uint8_t*)) {
    uint8_t mac[6];
    clock_t start, end;
    int vmrss_kb = 0, vmpeak_kb = 0;

    
    const int HOT_COUNT = COUNT * HOT_RATIO / 100;
    const int COLD_COUNT = COUNT - HOT_COUNT;

    printf("=== %s (Hot/Cold Traffic) ===\n", name);

    // Insert Phase
    printf("Insert Phase (cold first, then hot)...\n");
    start = clock();
    for (int i = HOT_COUNT; i < COUNT; ++i) {
        viper_gen_mac(mac, i);
        add_func(mac, i % 8);
    }
    for (int i = 0; i < HOT_COUNT; ++i) {
        viper_gen_mac(mac, i);
        add_func(mac, i % 8);
    }
    end = clock();
    double insert_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Insert Time: %.4fs\n", insert_time);

    // Lookup Phase
    printf("Lookup Phase (biased to hot MACs)...\n");
    int miss = 0;
    const int LOOKUP_TIMES = 5; // 5x lookups compared to inserts
    start = clock();
    for (int i = 0; i < LOOKUP_TIMES * COUNT; ++i) {
        int id;
        if (rand() % 100 < 80) {
            // 80% chance: lookup hot MAC
            id = rand() % HOT_COUNT;
        } else {
            // 20% chance: lookup cold MAC
            id = HOT_COUNT + (rand() % COLD_COUNT);
        }

        viper_gen_mac(mac, id);
        if (lookup_func(mac) == -1)
        ++miss;
    }
    end = clock();

    double lookup_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Lookup Time: %.4fs\n", lookup_time);
    printf("\n");
    printf("Total Lookups: %d\n", LOOKUP_TIMES * COUNT);
    printf("Total Misses: %d\n", miss);

    double miss_rate = (double)miss / (double)(LOOKUP_TIMES * COUNT);
    printf("Miss Rate: %.4f\n", miss_rate);
    printf("\n");

    // Get memory usage
    get_memory_usage_kb(&vmrss_kb, &vmpeak_kb);
    printf("VmRSS: %d (KB)\n", vmrss_kb);
    printf("VmPeak: %d (KB)\n", vmpeak_kb);

    FILE *csv = fopen("benchmark.csv", "a");
    fprintf(csv, "%s (hot/cold),%.4f,%.4f,%d,%d\n", name, insert_time, lookup_time, vmpeak_kb, vmrss_kb);
    fclose(csv);
}

int main() {
    // Clear benchmark.csv when starting
   /* FILE *csv = fopen("benchmark.csv", "w");
    if (csv) {
        fprintf(csv, "Name, Insert Time, Lookup Time, VmPeak(kB), VmRSS(kB)\n");  // (Optional) Write header line
        fclose(csv);
    }*/

    //benchmark("Linked List", list_add, list_lookup);
    //benchmark("uthash Hash Table", hash_add, hash_lookup);
    benchmark("uthash Hash Table with LRU", lru_hash_add, lru_hash_lookup);
    benchmark_hot_cold("uthash Hash Table with LRU", lru_hash_add, lru_hash_lookup);
    return 0;
}
