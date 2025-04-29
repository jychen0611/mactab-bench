#include <stdio.h>
#include <time.h>
#include "list.h"
#include "hash.h"
#include "lru.h"

#define COUNT 100000

/* Helper function to output the MAC address */
void print_mac(uint8_t mac[6]) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/* Generate pseudo-random MAC */
void gen_mac(uint8_t *mac, int i) {
    for (int j = 0; j < 6; ++j)
        mac[j] = (i >> (j * 2)) & 0xFF;
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
        gen_mac(mac, i);
        add_func(mac, i % 8);
    }
    end = clock();
    double insert_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Insert Time: %.4fs\n", insert_time);

    // Lookup
    int miss = 0;
    start = clock();
    for (int i = 0; i < COUNT; ++i) {
        gen_mac(mac, i);
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

int main() {
    // Clear benchmark.csv when starting
    FILE *csv = fopen("benchmark.csv", "w");
    if (csv) {
        fprintf(csv, "Name, Insert Time, Lookup Time, VmPeak(kB), VmRSS(kB)\n");  // (Optional) Write header line
        fclose(csv);
    }

    benchmark("Linked List", list_add, list_lookup);
    benchmark("uthash Hash Table", hash_add, hash_lookup);
    benchmark("uthash Hash Table with LRU and Aging", lru_hash_add, lru_hash_lookup);
    return 0;
}
