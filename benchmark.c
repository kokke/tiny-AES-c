#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

#define CTR 1
#define ECB 1
#include "aes.h"
#include "aes_openmp.h"

// CSV output file handle
static FILE* csv_file = NULL;

// Helper function to get wall-clock time in seconds
static double get_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Helper function to print throughput
static void print_throughput(const char* name, size_t bytes, double time_sec)
{
    double mb = bytes / (1024.0 * 1024.0);
    double throughput = mb / time_sec;
    printf("  %-30s: %10.3f MB/s  (%.3f seconds for %.2f MB)\n",
           name, throughput, time_sec, mb);
}

// Test correctness by comparing sequential and parallel results
static int test_correctness()
{
    printf("\n=== Correctness Test ===\n");

    const size_t test_size = 1024 * 1024; // 1 MB
    uint8_t* data_seq = (uint8_t*)malloc(test_size);
    uint8_t* data_par = (uint8_t*)malloc(test_size);
    uint8_t* data_par_nocopy = (uint8_t*)malloc(test_size);

    // Initialize with random data
    for (size_t i = 0; i < test_size; ++i)
    {
        data_seq[i] = data_par[i] = data_par_nocopy[i] = rand() & 0xFF;
    }

    // Setup AES context
    uint8_t key[AES_KEYLEN] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };

    uint8_t iv[AES_BLOCKLEN] = {
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    };

    struct AES_ctx ctx_seq, ctx_par, ctx_par_nocopy;
    AES_init_ctx_iv(&ctx_seq, key, iv);
    AES_init_ctx_iv(&ctx_par, key, iv);
    AES_init_ctx_iv(&ctx_par_nocopy, key, iv);

    // Encrypt with sequential version
    AES_CTR_xcrypt_buffer(&ctx_seq, data_seq, test_size);

    // Encrypt with parallel version (with copy)
    AES_CTR_xcrypt_buffer_openmp(&ctx_par, data_par, test_size);

    // Encrypt with parallel version (no copy)
    AES_CTR_xcrypt_buffer_openmp_no_copy(&ctx_par_nocopy, data_par_nocopy, test_size);

    // Compare sequential vs parallel (with copy)
    int errors_par = 0;
    for (size_t i = 0; i < test_size; ++i)
    {
        if (data_seq[i] != data_par[i])
        {
            if (errors_par < 10)
            {
                printf("Error (PAR vs SEQ) at byte %zu: seq=0x%02x, par=0x%02x\n",
                       i, data_seq[i], data_par[i]);
            }
            errors_par++;
        }
    }

    // Compare sequential vs parallel (no copy)
    int errors_par_nocopy = 0;
    for (size_t i = 0; i < test_size; ++i)
    {
        if (data_seq[i] != data_par_nocopy[i])
        {
            if (errors_par_nocopy < 10)
            {
                printf("Error (NOCOPY vs SEQ) at byte %zu: seq=0x%02x, nocopy=0x%02x\n",
                       i, data_seq[i], data_par_nocopy[i]);
            }
            errors_par_nocopy++;
        }
    }

    // Compare parallel (with copy) vs parallel (no copy)
    int errors_both_par = 0;
    for (size_t i = 0; i < test_size; ++i)
    {
        if (data_par[i] != data_par_nocopy[i])
        {
            if (errors_both_par < 10)
            {
                printf("Error (NOCOPY vs COPY) at byte %zu: copy=0x%02x, nocopy=0x%02x\n",
                       i, data_par[i], data_par_nocopy[i]);
            }
            errors_both_par++;
        }
    }

    free(data_seq);
    free(data_par);
    free(data_par_nocopy);

    int all_passed = 1;

    if (errors_par == 0)
    {
        printf("✓ OpenMP (with copy) vs Sequential:  PASSED\n");
    }
    else
    {
        printf("✗ OpenMP (with copy) vs Sequential:  FAILED - Found %d mismatches\n", errors_par);
        all_passed = 0;
    }

    if (errors_par_nocopy == 0)
    {
        printf("✓ OpenMP (no copy) vs Sequential:    PASSED\n");
    }
    else
    {
        printf("✗ OpenMP (no copy) vs Sequential:    FAILED - Found %d mismatches\n", errors_par_nocopy);
        all_passed = 0;
    }

    if (errors_both_par == 0)
    {
        printf("✓ OpenMP (with copy) vs (no copy):   PASSED\n");
    }
    else
    {
        printf("✗ OpenMP (with copy) vs (no copy):   FAILED - Found %d mismatches\n", errors_both_par);
        all_passed = 0;
    }

    return all_passed ? 0 : 1;
}

// Benchmark function
static void benchmark_size(size_t size_mb)
{
    const size_t size = size_mb * 1024 * 1024;
    const int iterations = (size_mb >= 64) ? 3 : 5; // Fewer iterations for larger sizes

    printf("\n=== Benchmark: %zu MB data ===\n", size_mb);

    uint8_t* data = (uint8_t*)malloc(size);
    if (!data)
    {
        printf("Error: Failed to allocate %zu MB\n", size_mb);
        return;
    }

    // Initialize with random data
    for (size_t i = 0; i < size; ++i)
    {
        data[i] = rand() & 0xFF;
    }

    // Setup AES context
    uint8_t key[AES_KEYLEN] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };

    uint8_t iv[AES_BLOCKLEN] = {
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    };

    // Benchmark sequential version
    double total_time_seq = 0.0;
    for (int i = 0; i < iterations; ++i)
    {
        struct AES_ctx ctx;
        AES_init_ctx_iv(&ctx, key, iv);

        double start = get_time();
        AES_CTR_xcrypt_buffer(&ctx, data, size);
        double end = get_time();

        total_time_seq += (end - start);
    }
    double avg_time_seq = total_time_seq / iterations;
    double throughput_seq = (size / (1024.0 * 1024.0)) / avg_time_seq;

    // Write sequential result to CSV
    if (csv_file)
    {
        fprintf(csv_file, "%zu,Sequential,1,%lf,%lf\n", size_mb, throughput_seq, avg_time_seq);
    }

    // Benchmark parallel version with different thread counts
    int max_threads = omp_get_max_threads();
    printf("\nMaximum available threads: %d\n\n", max_threads);

    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);

    double time_1thread = 0.0;

    for (int t = 0; t < num_tests; ++t)
    {
        int num_threads = thread_counts[t];
        if (num_threads > max_threads)
            break;

        omp_set_num_threads(num_threads);

        double total_time_par = 0.0;
        for (int i = 0; i < iterations; ++i)
        {
            struct AES_ctx ctx;
            AES_init_ctx_iv(&ctx, key, iv);

            double start = get_time();
            AES_CTR_xcrypt_buffer_openmp(&ctx, data, size);
            double end = get_time();

            total_time_par += (end - start);
        }
        double avg_time_par = total_time_par / iterations;
        double throughput_par = (size / (1024.0 * 1024.0)) / avg_time_par;

        char thread_label[64];
        snprintf(thread_label, sizeof(thread_label), "OpenMP (%d thread%s)",
                 num_threads, num_threads > 1 ? "s" : "");
        print_throughput(thread_label, size, avg_time_par);

        // Write parallel result to CSV
        if (csv_file)
        {
            fprintf(csv_file, "%zu,OpenMP,%d,%lf,%lf\n", size_mb, num_threads, throughput_par, avg_time_par);
        }

        if (num_threads == 1)
        {
            time_1thread = avg_time_par;
            printf("  Sequential version");
            printf("            : %10.3f MB/s  (%.3f seconds for %.2f MB)\n",
                   throughput_seq, avg_time_seq,
                   size / (1024.0 * 1024.0));
            printf("  Speedup vs sequential      : %.2fx\n", avg_time_seq / avg_time_par);
        }
        else
        {
            printf("  Speedup vs 1 thread        : %.2fx\n", time_1thread / avg_time_par);
        }
    }

    free(data);
}

// Benchmark function comparing copy vs no-copy implementations
static void benchmark_copy_vs_nocopy(size_t size_mb)
{
    const size_t size = size_mb * 1024 * 1024;
    const int iterations = (size_mb >= 64) ? 3 : 5;

    printf("\n=== Benchmark: Copy vs No-Copy Comparison (%zu MB) ===\n", size_mb);

    uint8_t* data = (uint8_t*)malloc(size);
    if (!data)
    {
        printf("Error: Failed to allocate %zu MB\n", size_mb);
        return;
    }

    // Initialize with random data
    for (size_t i = 0; i < size; ++i)
    {
        data[i] = rand() & 0xFF;
    }

    // Setup AES context
    uint8_t key[AES_KEYLEN] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };

    uint8_t iv[AES_BLOCKLEN] = {
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    };

    int max_threads = omp_get_max_threads();
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);

    printf("\nThread Count | WITH Copy          | WITHOUT Copy        | Difference\n");
    printf("-------------|--------------------|--------------------|----------------\n");

    for (int t = 0; t < num_tests; ++t)
    {
        int num_threads = thread_counts[t];
        if (num_threads > max_threads)
            break;

        omp_set_num_threads(num_threads);

        // Benchmark WITH copy version
        double total_time_copy = 0.0;
        for (int i = 0; i < iterations; ++i)
        {
            struct AES_ctx ctx;
            AES_init_ctx_iv(&ctx, key, iv);

            double start = get_time();
            AES_CTR_xcrypt_buffer_openmp(&ctx, data, size);
            double end = get_time();

            total_time_copy += (end - start);
        }
        double avg_time_copy = total_time_copy / iterations;
        double throughput_copy = (size / (1024.0 * 1024.0)) / avg_time_copy;

        // Benchmark WITHOUT copy version
        double total_time_nocopy = 0.0;
        for (int i = 0; i < iterations; ++i)
        {
            struct AES_ctx ctx;
            AES_init_ctx_iv(&ctx, key, iv);

            double start = get_time();
            AES_CTR_xcrypt_buffer_openmp_no_copy(&ctx, data, size);
            double end = get_time();

            total_time_nocopy += (end - start);
        }
        double avg_time_nocopy = total_time_nocopy / iterations;
        double throughput_nocopy = (size / (1024.0 * 1024.0)) / avg_time_nocopy;

        // Calculate difference
        double diff_percent = ((avg_time_nocopy - avg_time_copy) / avg_time_copy) * 100.0;
        double slowdown = avg_time_nocopy / avg_time_copy;

        printf("%12d | %6.3f MB/s (%.3fs) | %6.3f MB/s (%.3fs) | %.2f%% slower (%.2fx)\n",
               num_threads,
               throughput_copy, avg_time_copy,
               throughput_nocopy, avg_time_nocopy,
               diff_percent, slowdown);

        // Write copy vs no-copy comparison to CSV
        if (csv_file)
        {
            fprintf(csv_file, "%zu,OpenMP-Copy,%d,%lf,%lf\n", size_mb, num_threads, throughput_copy, avg_time_copy);
            fprintf(csv_file, "%zu,OpenMP-NoCopy,%d,%lf,%lf\n", size_mb, num_threads, throughput_nocopy, avg_time_nocopy);
        }
    }

    free(data);
}

int main(int argc, char* argv[])
{
    printf("=======================================================\n");
    printf("  AES-CTR OpenMP Benchmark\n");
    printf("=======================================================\n");

#if defined(AES256)
    printf("AES Key Size: 256-bit\n");
#elif defined(AES192)
    printf("AES Key Size: 192-bit\n");
#else
    printf("AES Key Size: 128-bit\n");
#endif

    printf("Block Size: %d bytes\n", AES_BLOCKLEN);
    printf("=======================================================\n");

    srand(time(NULL));

    // Open CSV file for output
    csv_file = fopen("benchmark_results.csv", "w");
    if (csv_file)
    {
        fprintf(csv_file, "DataSize_MB,Type,Threads,Throughput_MB_s,Time_Seconds\n");
        printf("CSV output will be written to: benchmark_results.csv\n");
    }
    else
    {
        printf("Warning: Could not open CSV file for writing\n");
    }

    // Run correctness test first
    if (test_correctness() != 0)
    {
        printf("\nAborting benchmarks due to correctness test failure.\n");
        if (csv_file)
            fclose(csv_file);
        return 1;
    }

    // Run benchmarks with different data sizes
    benchmark_size(1);      // 1 MB
    benchmark_size(10);     // 10 MB
    benchmark_size(100);    // 100 MB

    // Run copy vs no-copy comparison
    printf("\n\n");
    printf("=======================================================\n");
    printf("  RoundKey Copy vs No-Copy Performance Comparison\n");
    printf("=======================================================\n");
    benchmark_copy_vs_nocopy(10);    // 10 MB for comparison

    // Close CSV file
    if (csv_file)
    {
        fclose(csv_file);
        printf("\nBenchmark results saved to: benchmark_results.csv\n");
    }

    printf("\n=======================================================\n");
    printf("  Benchmark Complete\n");
    printf("=======================================================\n");

    return 0;
}
