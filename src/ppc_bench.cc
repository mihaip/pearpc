#include <chrono>

#include "configparser.h"
#include "cpu/cpu.h"
#include "cpu/mem.h"
#include "system/types.h"
#include "tools/snprintf.h"
#include "debug/ppcdis.h"
#include "cpu/cpu_generic/ppc_mmu.h"
#include "cpu/cpu_generic/ppc_cpu.h"
#include "system/arch/sysendian.h"

uint32 cs_code[] = {
    0x3863FFFC, 0x7C861671, 0x41820090, 0x70600002, 0x41E2001C, 0xA0030004,
    0x3884FFFE, 0x38630002, 0x5486F0BF, 0x7CA50114, 0x41820070, 0x70C60003,
    0x41820014, 0x7CC903A6, 0x84030004, 0x7CA50114, 0x4200FFF8, 0x5486E13F,
    0x41820050, 0x80030004, 0x7CC903A6, 0x80C30008, 0x7CA50114, 0x80E3000C,
    0x7CA53114, 0x85030010, 0x7CA53914, 0x42400028, 0x80030004, 0x7CA54114,
    0x80C30008, 0x7CA50114, 0x80E3000C, 0x7CA53114, 0x85030010, 0x7CA53914,
    0x4200FFE0, 0x7CA54114, 0x70800002, 0x41E20010, 0xA0030004, 0x38630002,
    0x7CA50114, 0x70800001, 0x41E20010, 0x88030004, 0x5400402E, 0x7CA50114,
    0x7C650194, /* 0x4E800020 */ 0x00005AF0
};

constexpr uint32 test_size = 0x8000; // 0x7FFFFFFC is the max
constexpr uint32 test_samples = 2000;
constexpr uint32 test_iterations = 5;

int ppc_bench() {
    gConfig = new ConfigParser();
    ppc_cpu_init_config();

    if (!ppc_init_physical_memory(64 * 1024 * 1024)) { // 64MB is the minimum
        ht_printf("cannot initialize memory.\n");
        return 1;
    }
    if (!ppc_cpu_init()) {
        ht_printf("cpu_init failed! Out of memory?\n");
        return 1;
    }

    size_t code_size = sizeof(cs_code) / sizeof(cs_code[0]);
    ht_printf("Loading %llu instructions:\n", code_size);

    /* load executable code into RAM at address 0 */
    for (int i = 0; i < code_size; i++) {
        uint32 instr_code = cs_code[i];
        uint32 instr_code_be = ppc_word_to_BE(instr_code);
        uint32 instr_addr = i * 4;
        uint32 instr_physical_addr;
	    int r = ppc_effective_to_physical<PPC_MMU_WRITE>(instr_addr, instr_physical_addr);
        if (r != PPC_MMU_OK) {
            ht_printf("MMU error when mapping instruction address: %d\n", r);
            return 1;
        }

        // ppc_write_physical_word gets optimized out, do its equivalent.
        *((uint32*)(gMemory+instr_physical_addr)) = instr_code_be;

        PPCDisassembler dis(PPC_MODE_32);
        CPU_ADDR addr;
        addr.addr32.offset = instr_addr;
        const char *instr_disasm = dis.str(dis.decode((byte*)&instr_code_be, 4, addr), 0);
        ht_printf("Instruction %02llu: 0x%08x %s\n", i, instr_code, instr_disasm);
    }

    srand(0xCAFEBABE);

    ht_printf("Test size: 0x%X\n", test_size);
    ht_printf("First few bytes:\n");
    bool did_lf = false;
    for (int i = 0; i < test_size; i++) {
        uint8 val = rand() % 256;
        if (i < 64) {
            ht_printf("%02x", val);
            did_lf = false;
            if (i % 32 == 31) {
                ht_printf("\n");
                did_lf = true;
            }
        }

        uint32 addr = 0x1000+i;
        uint32 physical_addr;
	    int r = ppc_effective_to_physical<PPC_MMU_WRITE>(addr, physical_addr);
        if (r != PPC_MMU_OK) {
            ht_printf("MMU error when mapping data address: %d\n", r);
            return 1;
        }

        gMemory[physical_addr] = val;
    }
    if (!did_lf)
        ht_printf("\n");

    // Run once to warm up the cache cache and to get the expected checksum value.
    ppc_cpu_set_pc(0, 0);
    ppc_cpu_set_gpr(0, 3, 0x1000);    // buf
    ppc_cpu_set_gpr(0, 4, test_size); // len
    ppc_cpu_set_gpr(0, 5, 0);         // sum
    ppc_cpu_run();
    uint32 checksum = ppc_cpu_get_gpr(0, 3);
    ht_printf("Checksum: 0x%08X\n", checksum);

    // Also warm up the clock
    uint64_t overhead = -1;
    for (int i = 0; i < test_samples; i++) {
        auto start_time   = std::chrono::steady_clock::now();
        auto end_time     = std::chrono::steady_clock::now();
        auto time_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
        if (time_elapsed.count() < overhead) {
            overhead = time_elapsed.count();
        }
    }
    ht_printf("Overhead Time: %lld ns\n", overhead);

    ht_printf("Running benchmark...\n");
    for (int i = 0; i < test_iterations; i++) {
        uint64_t best_sample = -1;
        for (int j = 0; j < test_samples; j++) {
            ppc_cpu_set_pc(0, 0);
            ppc_cpu_set_gpr(0, 3, 0x1000);    // buf
            ppc_cpu_set_gpr(0, 4, test_size); // len
            ppc_cpu_set_gpr(0, 5, 0);         // sum

            auto start_time   = std::chrono::steady_clock::now();
            ppc_cpu_run();
            auto end_time     = std::chrono::steady_clock::now();
            auto time_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
            if (time_elapsed.count() < best_sample)
                best_sample = time_elapsed.count();
        }
        uint32 iter_checksum = ppc_cpu_get_gpr(0, 3);
        if (iter_checksum != checksum)
        ht_printf("Checksum changed in iteration %d: 0x%08X\n", i, iter_checksum);
        best_sample -= overhead;
        ht_printf("(%d) %lld ns, %.4lf MiB/s\n", i+1, best_sample, 1E9 * test_size / (best_sample * 1024 * 1024));
    }

    return 0;
}
