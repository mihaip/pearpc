/*
 *	PearPC
 *	ppc_dec.cc
 *
 *	Copyright (C) 2003, 2004 Sebastian Biallas (sb@biallas.net)
 *	Portions Copyright (C) 2004 Daniel Foesch (dfoesch@cs.nmsu.edu)
 *	Portions Copyright (C) 2004 Apple Computer, Inc.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <algorithm>
#include <cstring>

#include "system/types.h"
#include "cpu/debug.h"
#include "cpu/cpu.h"
#include "debug/tracers.h"
#include "ppc_alu.h"
#include "ppc_cpu.h"
#include "ppc_dec.h"
#include "ppc_exc.h"
#include "ppc_fpu.h"
#include "ppc_vec.h"
#include "ppc_mmu.h"
#include "ppc_opc.h"

#include "io/prom/promosi.h"

static void ppc_opc_invalid(uint32 opc)
{
	if (gCPU.pc == gPromOSIEntry && opc == PROM_MAGIC_OPCODE) {
		call_prom_osi();
		return;
	}
	if (opc == 0x00333301) {
		// memset(r3, r4, r5)
		uint32 dest = gCPU.gpr[3];
		uint32 c = gCPU.gpr[4];
		uint32 size = gCPU.gpr[5];
		if (dest & 0xfff) {
			byte *dst;
			ppc_direct_effective_memory_handle(dest, dst);
			uint32 a = 4096 - (dest & 0xfff);
			memset(dst, c, a);
			size -= a;
			dest += a;
		}
		while (size >= 4096) {
			byte *dst;
			ppc_direct_effective_memory_handle(dest, dst);
			memset(dst, c, 4096);
			dest += 4096;
			size -= 4096;
		}
		if (size) {
			byte *dst;
			ppc_direct_effective_memory_handle(dest, dst);
			memset(dst, c, size);
		}
		gCPU.pc = gCPU.npc;
		return;
	}
	if (opc == 0x00333302) {
		// memcpy
		uint32 dest = gCPU.gpr[3];
		uint32 src = gCPU.gpr[4];
		uint32 size = gCPU.gpr[5];
		byte *d, *s;
		ppc_direct_effective_memory_handle(dest, d);
		ppc_direct_effective_memory_handle(src, s);
		while (size--) {
			if (!(dest & 0xfff)) ppc_direct_effective_memory_handle(dest, d);
			if (!(src & 0xfff)) ppc_direct_effective_memory_handle(src, s);
			*d = *s;
			src++; dest++; d++; s++;
		}
		gCPU.pc = gCPU.npc;
		return;
	}
	if (opc == 0x00005AF0) {
		// End of benchmark stream
		ppc_cpu_stop();
		return;
	}
	fprintf(stderr, "[PPC/DEC] Bad opcode: %08x (%u:%u)\n",
		opc, PPC_OPC_MAIN(opc),
		PPC_OPC_MOD(opc));

	SINGLESTEP("unknown instruction\n");

	// Avoids some benchmarking overhead.
	__builtin_unreachable();
}

static void ppc_opc_no_fpu(uint32 opc)
{
	ppc_exception(PPC_EXC_NO_FPU);
}

ppc_opc_function ppc_opc_table_groupv[965];

static void ppc_opc_init_groupv()
{
	for (uint i=0; i<(sizeof ppc_opc_table_groupv / sizeof ppc_opc_table_groupv[0]);i++) {
		ppc_opc_table_groupv[i] = ppc_opc_invalid;
	}
	ppc_opc_table_groupv[0] = ppc_opc_vaddubm;
	ppc_opc_table_groupv[1] = ppc_opc_vmaxub;
	ppc_opc_table_groupv[2] = ppc_opc_vrlb;
	ppc_opc_table_groupv[4] = ppc_opc_vmuloub;
	ppc_opc_table_groupv[5] = ppc_opc_vaddfp;
	ppc_opc_table_groupv[6] = ppc_opc_vmrghb;
	ppc_opc_table_groupv[7] = ppc_opc_vpkuhum;
	ppc_opc_table_groupv[32] = ppc_opc_vadduhm;
	ppc_opc_table_groupv[33] = ppc_opc_vmaxuh;
	ppc_opc_table_groupv[34] = ppc_opc_vrlh;
	ppc_opc_table_groupv[36] = ppc_opc_vmulouh;
	ppc_opc_table_groupv[37] = ppc_opc_vsubfp;
	ppc_opc_table_groupv[38] = ppc_opc_vmrghh;
	ppc_opc_table_groupv[39] = ppc_opc_vpkuwum;
	ppc_opc_table_groupv[64] = ppc_opc_vadduwm;
	ppc_opc_table_groupv[65] = ppc_opc_vmaxuw;
	ppc_opc_table_groupv[66] = ppc_opc_vrlw;
	ppc_opc_table_groupv[70] = ppc_opc_vmrghw;
	ppc_opc_table_groupv[71] = ppc_opc_vpkuhus;
	ppc_opc_table_groupv[103] = ppc_opc_vpkuwus;
	ppc_opc_table_groupv[129] = ppc_opc_vmaxsb;
	ppc_opc_table_groupv[130] = ppc_opc_vslb;
	ppc_opc_table_groupv[132] = ppc_opc_vmulosb;
	ppc_opc_table_groupv[133] = ppc_opc_vrefp;
	ppc_opc_table_groupv[134] = ppc_opc_vmrglb;
	ppc_opc_table_groupv[135] = ppc_opc_vpkshus;
	ppc_opc_table_groupv[161] = ppc_opc_vmaxsh;
	ppc_opc_table_groupv[162] = ppc_opc_vslh;
	ppc_opc_table_groupv[164] = ppc_opc_vmulosh;
	ppc_opc_table_groupv[165] = ppc_opc_vrsqrtefp;
	ppc_opc_table_groupv[166] = ppc_opc_vmrglh;
	ppc_opc_table_groupv[167] = ppc_opc_vpkswus;
	ppc_opc_table_groupv[192] = ppc_opc_vaddcuw;
	ppc_opc_table_groupv[193] = ppc_opc_vmaxsw;
	ppc_opc_table_groupv[194] = ppc_opc_vslw;
	ppc_opc_table_groupv[197] = ppc_opc_vexptefp;
	ppc_opc_table_groupv[198] = ppc_opc_vmrglw;
	ppc_opc_table_groupv[199] = ppc_opc_vpkshss;
	ppc_opc_table_groupv[226] = ppc_opc_vsl;
	ppc_opc_table_groupv[229] = ppc_opc_vlogefp;
	ppc_opc_table_groupv[231] = ppc_opc_vpkswss;
	ppc_opc_table_groupv[256] = ppc_opc_vaddubs;
	ppc_opc_table_groupv[257] = ppc_opc_vminub;
	ppc_opc_table_groupv[258] = ppc_opc_vsrb;
	ppc_opc_table_groupv[260] = ppc_opc_vmuleub;
	ppc_opc_table_groupv[261] = ppc_opc_vrfin;
	ppc_opc_table_groupv[262] = ppc_opc_vspltb;
	ppc_opc_table_groupv[263] = ppc_opc_vupkhsb;
	ppc_opc_table_groupv[288] = ppc_opc_vadduhs;
	ppc_opc_table_groupv[289] = ppc_opc_vminuh;
	ppc_opc_table_groupv[290] = ppc_opc_vsrh;
	ppc_opc_table_groupv[292] = ppc_opc_vmuleuh;
	ppc_opc_table_groupv[293] = ppc_opc_vrfiz;
	ppc_opc_table_groupv[294] = ppc_opc_vsplth;
	ppc_opc_table_groupv[295] = ppc_opc_vupkhsh;
	ppc_opc_table_groupv[320] = ppc_opc_vadduws;
	ppc_opc_table_groupv[321] = ppc_opc_vminuw;
	ppc_opc_table_groupv[322] = ppc_opc_vsrw;
	ppc_opc_table_groupv[325] = ppc_opc_vrfip;
	ppc_opc_table_groupv[326] = ppc_opc_vspltw;
	ppc_opc_table_groupv[327] = ppc_opc_vupklsb;
	ppc_opc_table_groupv[354] = ppc_opc_vsr;
	ppc_opc_table_groupv[357] = ppc_opc_vrfim;
	ppc_opc_table_groupv[359] = ppc_opc_vupklsh;
	ppc_opc_table_groupv[384] = ppc_opc_vaddsbs;
	ppc_opc_table_groupv[385] = ppc_opc_vminsb;
	ppc_opc_table_groupv[386] = ppc_opc_vsrab;
	ppc_opc_table_groupv[388] = ppc_opc_vmulesb;
	ppc_opc_table_groupv[389] = ppc_opc_vcfux;
	ppc_opc_table_groupv[390] = ppc_opc_vspltisb;
	ppc_opc_table_groupv[391] = ppc_opc_vpkpx;
	ppc_opc_table_groupv[416] = ppc_opc_vaddshs;
	ppc_opc_table_groupv[417] = ppc_opc_vminsh;
	ppc_opc_table_groupv[418] = ppc_opc_vsrah;
	ppc_opc_table_groupv[420] = ppc_opc_vmulesh;
	ppc_opc_table_groupv[421] = ppc_opc_vcfsx;
	ppc_opc_table_groupv[422] = ppc_opc_vspltish;
	ppc_opc_table_groupv[423] = ppc_opc_vupkhpx;
	ppc_opc_table_groupv[448] = ppc_opc_vaddsws;
	ppc_opc_table_groupv[449] = ppc_opc_vminsw;
	ppc_opc_table_groupv[450] = ppc_opc_vsraw;
	ppc_opc_table_groupv[453] = ppc_opc_vctuxs;
	ppc_opc_table_groupv[454] = ppc_opc_vspltisw;
	ppc_opc_table_groupv[485] = ppc_opc_vctsxs;
	ppc_opc_table_groupv[487] = ppc_opc_vupklpx;
	ppc_opc_table_groupv[512] = ppc_opc_vsububm;
	ppc_opc_table_groupv[513] = ppc_opc_vavgub;
	ppc_opc_table_groupv[514] = ppc_opc_vand;
	ppc_opc_table_groupv[517] = ppc_opc_vmaxfp;
	ppc_opc_table_groupv[518] = ppc_opc_vslo;
	ppc_opc_table_groupv[544] = ppc_opc_vsubuhm;
	ppc_opc_table_groupv[545] = ppc_opc_vavguh;
	ppc_opc_table_groupv[546] = ppc_opc_vandc;
	ppc_opc_table_groupv[549] = ppc_opc_vminfp;
	ppc_opc_table_groupv[550] = ppc_opc_vsro;
	ppc_opc_table_groupv[576] = ppc_opc_vsubuwm;
	ppc_opc_table_groupv[577] = ppc_opc_vavguw;
	ppc_opc_table_groupv[578] = ppc_opc_vor;
	ppc_opc_table_groupv[610] = ppc_opc_vxor;
	ppc_opc_table_groupv[641] = ppc_opc_vavgsb;
	ppc_opc_table_groupv[642] = ppc_opc_vnor;
	ppc_opc_table_groupv[673] = ppc_opc_vavgsh;
	ppc_opc_table_groupv[704] = ppc_opc_vsubcuw;
	ppc_opc_table_groupv[705] = ppc_opc_vavgsw;
	ppc_opc_table_groupv[768] = ppc_opc_vsububs;
	ppc_opc_table_groupv[770] = ppc_opc_mfvscr;
	ppc_opc_table_groupv[772] = ppc_opc_vsum4ubs;
	ppc_opc_table_groupv[800] = ppc_opc_vsubuhs;
	ppc_opc_table_groupv[802] = ppc_opc_mtvscr;
	ppc_opc_table_groupv[804] = ppc_opc_vsum4shs;
	ppc_opc_table_groupv[832] = ppc_opc_vsubuws;
	ppc_opc_table_groupv[836] = ppc_opc_vsum2sws;
	ppc_opc_table_groupv[896] = ppc_opc_vsubsbs;
	ppc_opc_table_groupv[900] = ppc_opc_vsum4sbs;
	ppc_opc_table_groupv[928] = ppc_opc_vsubshs;
	ppc_opc_table_groupv[960] = ppc_opc_vsubsws;
	ppc_opc_table_groupv[964] = ppc_opc_vsumsws;
}

// main opcode 04
static void ppc_opc_group_v(uint32 opc)
{
	uint32 ext = PPC_OPC_EXT(opc);
#ifndef  __VEC_EXC_OFF__
	if ((gCPU.msr & MSR_VEC) == 0) {
		ppc_exception(PPC_EXC_NO_VEC);
		return;
	}
#endif
	switch(ext & 0x1f) {
		case 16:
			if (opc & PPC_OPC_Rc)
				return ppc_opc_vmhraddshs(opc);
			else
				return ppc_opc_vmhaddshs(opc);
		case 17:	return ppc_opc_vmladduhm(opc);
		case 18:
			if (opc & PPC_OPC_Rc)
				return ppc_opc_vmsummbm(opc);
			else
				return ppc_opc_vmsumubm(opc);
		case 19:
			if (opc & PPC_OPC_Rc)
				return ppc_opc_vmsumuhs(opc);
			else
				return ppc_opc_vmsumuhm(opc);
		case 20:
			if (opc & PPC_OPC_Rc)
				return ppc_opc_vmsumshs(opc);
			else
				return ppc_opc_vmsumshm(opc);
		case 21:
			if (opc & PPC_OPC_Rc)
				return ppc_opc_vperm(opc);
			else
				return ppc_opc_vsel(opc);
		case 22:	return ppc_opc_vsldoi(opc);
		case 23:
			if (opc & PPC_OPC_Rc)
				return ppc_opc_vnmsubfp(opc);
			else
				return ppc_opc_vmaddfp(opc);
	}
	switch(ext & 0x1ff)
	{
		case 3: return ppc_opc_vcmpequbx(opc);
		case 35: return ppc_opc_vcmpequhx(opc);
		case 67: return ppc_opc_vcmpequwx(opc);
		case 99: return ppc_opc_vcmpeqfpx(opc);
		case 227: return ppc_opc_vcmpgefpx(opc);
		case 259: return ppc_opc_vcmpgtubx(opc);
		case 291: return ppc_opc_vcmpgtuhx(opc);
		case 323: return ppc_opc_vcmpgtuwx(opc);
		case 355: return ppc_opc_vcmpgtfpx(opc);
		case 387: return ppc_opc_vcmpgtsbx(opc);
		case 419: return ppc_opc_vcmpgtshx(opc);
		case 451: return ppc_opc_vcmpgtswx(opc);
		case 483: return ppc_opc_vcmpbfpx(opc);
	}

	if (ext >= (sizeof ppc_opc_table_groupv / sizeof ppc_opc_table_groupv[0])) {
		return ppc_opc_invalid(opc);
	}
	return ppc_opc_table_groupv[ext](opc);
}

// Opcode lookup table, indexed by primary opcode (bits 0...5) and modifier (bits 21...31).
static ppc_opc_function ppc_opc_table[64 * 2048];

// Variant opccode lookup table, where FPU instructions trigger a "NO FPU" exception.
// Used when the FP bit in the MSR is off.
static ppc_opc_function ppc_opc_no_fpu_table[64 * 2048];

// "raw" insertion into the opcode table
#define OPr(opcode, mod, fn) \
do { \
	uint64_t index = ((opcode) << 11) | (mod); \
	if (ppc_opc_table[index] != ppc_opc_invalid) { \
		PPC_DEC_WARN("Opcode %d-%d already had a funtion, will overwrite.\n", opcode, mod); \
	} \
	ppc_opc_table[index] = fn; \
	ppc_opc_no_fpu_table[index] = fn; \
} while (0)

// "raw" insertion into the opcode table (FPU instruction variant)
#define FPr(opcode, mod, fn) \
do { \
	uint64_t index = ((opcode) << 11) | (mod); \
	if (ppc_opc_table[index] != ppc_opc_invalid) { \
		PPC_DEC_WARN("Opcode %d-%d already had a funtion, will overwrite.\n", opcode, mod); \
	} \
	ppc_opc_table[index] = fn; \
	ppc_opc_no_fpu_table[index] = ppc_opc_no_fpu; \
} while (0)

// Inserts a top-level opcode (identified by bits 0...5) into the opcode table
#define OP(opcode, fn) \
do { \
for (uint32_t mod = 0; mod < 2048; mod++) { \
	OPr(opcode, mod, fn); \
} \
} while (0)

// Inserts a top-level opcode (identified by bits 0...5) into the opcode table
// (FPU instruction variant)
#define FP(opcode, fn) \
do { \
for (uint32_t mod = 0; mod < 2048; mod++) { \
	FPr(opcode, mod, fn); \
} \
} while (0)

// Inserts an opcode identified by the top level (bits 0..5) and the final
// two bits (AA and LK).
#define OPla(opcode, subopcode, fn) \
do { \
for (uint32_t mod = 0; mod < 512; mod++) { \
	OPr(opcode, (mod << 2) | (subopcode), fn); \
} \
} while (0)

// Inserts an opcode identified by the the top level (bits 0..5) that supports a
// a final Rc bit (the function is assumed to be templated using it).
#define OPrc(opcode, fn) \
do { \
for (uint32_t mod = 0; mod < 1024; mod++) { \
	OPr(opcode, (mod << 1) | 0x0, fn<Rc0>); \
	OPr(opcode, (mod << 1) | 0x1, fn<Rc1>); \
} \
} while (0)

// Inserts an opcode 31 handler identified by bits 21...30. Bit 31 can be
// anything.
#define OP31(subopcode, fn) \
do { \
	OPr(31, ((subopcode)<<1) | 0x0, fn); \
	OPr(31, ((subopcode)<<1) | 0x1, fn); \
} while (0)

// Inserts an opcode 31 handler identified by bits 21...30. Bit 31 can be
// anything (FPU instruction variant).
#define FP31(subopcode, fn) \
do { \
	FPr(31, ((subopcode)<<1) | 0x0, fn); \
	FPr(31, ((subopcode)<<1) | 0x1, fn); \
} while (0)

// Inserts an opcode 31 handler identified by bits 21...30. Bit 31 is the Rc
// bit (the function is assumed to be templated using it).
#define OP31rc(subopcode, fn) \
do { \
	OPr(31, ((subopcode)<<1) | 0x0, fn<Rc0>); \
	OPr(31, ((subopcode)<<1) | 0x1, fn<Rc1>); \
} while (0)

// Inserts an opcode 59 handler identified by bits 26...30. But 31 can be
// anything.
#define FP59(subopcode, fn) \
do { \
for (uint32_t mod = 0; mod < 32; mod++) { \
	FPr(59, (mod << 6) | ((subopcode)<<1) | 0x0, fn); \
	FPr(59, (mod << 6) | ((subopcode)<<1) | 0x1, fn); \
} \
} while (0)

// Inserts an opcode 63 handler identified by bits 26...30. But 31 can be
// anything.
#define FP63a(subopcode, fn) \
do { \
for (uint32_t mod = 0; mod < 32; mod++) { \
	FPr(63, (mod << 6) | ((subopcode)<<1) | 0x0, fn); \
	FPr(63, (mod << 6) | ((subopcode)<<1) | 0x1, fn); \
} \
} while (0)

// Inserts an opcode 63 handler identified by bits 21...30. But 31 can be
// anything.
#define FP63b(subopcode, fn) \
do { \
	FPr(63, ((subopcode)<<1) | 0x0, fn); \
	FPr(63, ((subopcode)<<1) | 0x1, fn); \
} while (0)

void FASTCALL ppc_exec_opc(ppc_opc_function *opc_table, uint32 opc)
{
	opc_table[PPC_OPC_MAIN(opc) | PPC_OPC_MOD(opc)](opc);
}

ppc_opc_function* ppc_current_opc_table() {
	return gCPU.msr & MSR_FP ? ppc_opc_table : ppc_opc_no_fpu_table;
}

void ppc_dec_init()
{
	auto ppc_opc_table_size = sizeof(ppc_opc_table) / sizeof(ppc_opc_table[0]);
	std::fill_n(ppc_opc_table, ppc_opc_table_size, ppc_opc_invalid);
	std::fill_n(ppc_opc_no_fpu_table, ppc_opc_table_size, ppc_opc_invalid);

	bool is_g4 = (ppc_cpu_get_pvr(0) & 0xffff0000) == 0x000c0000;

	OP(3, ppc_opc_twi);
	OP(7, ppc_opc_mulli);
	OP(8, ppc_opc_subfic);
	OP(10, ppc_opc_cmpli);
	OP(11, ppc_opc_cmpi);
	OP(12, ppc_opc_addic);
	OP(13, ppc_opc_addic_);
	OP(14, ppc_opc_addi);
	OP(15, ppc_opc_addis);
	OP(17, ppc_opc_sc);
	OP(24, ppc_opc_ori);
	OP(25, ppc_opc_oris);
	OP(26, ppc_opc_xori);
	OP(27, ppc_opc_xoris);
	OP(28, ppc_opc_andi_);
	OP(29, ppc_opc_andis_);
	OP(32, ppc_opc_lwz);
	OP(33, ppc_opc_lwzu);
	OP(34, ppc_opc_lbz);
	OP(35, ppc_opc_lbzu);
	OP(36, ppc_opc_stw);
	OP(37, ppc_opc_stwu);
	OP(38, ppc_opc_stb);
	OP(39, ppc_opc_stbu);
	OP(40, ppc_opc_lhz);
	OP(41, ppc_opc_lhzu);
	OP(42, ppc_opc_lha);
	OP(43, ppc_opc_lhau);
	OP(44, ppc_opc_sth);
	OP(45, ppc_opc_sthu);
	OP(46, ppc_opc_lmw);
	OP(47, ppc_opc_stmw);
	FP(48, ppc_opc_lfs);
	FP(49, ppc_opc_lfsu);
	FP(50, ppc_opc_lfd);
	FP(51, ppc_opc_lfdu);
	FP(52, ppc_opc_stfs);
	FP(53, ppc_opc_stfsu);
	FP(54, ppc_opc_stfd);
	FP(55, ppc_opc_stfdu);

	OPla(16, 0x0, (ppc_opc_bcx<LK0, AA0>)); // bc
	OPla(16, 0x1, (ppc_opc_bcx<LK1, AA0>)); // bcl
	OPla(16, 0x2, (ppc_opc_bcx<LK0, AA1>)); // bca
	OPla(16, 0x3, (ppc_opc_bcx<LK1, AA1>)); // bcla

	OPla(18, 0x0, (ppc_opc_bx<LK0, AA0>)); // b
	OPla(18, 0x1, (ppc_opc_bx<LK1, AA0>)); // bl
	OPla(18, 0x2, (ppc_opc_bx<LK0, AA1>)); // ba
	OPla(18, 0x3, (ppc_opc_bx<LK1, AA1>)); // bla

	OPr(19, 0, ppc_opc_mcrf);
	OPr(19, 32, ppc_opc_bclrx<LK0>);
	OPr(19, 33, ppc_opc_bclrx<LK1>);
	OPr(19, 66, ppc_opc_crnor);
	OPr(19, 100, ppc_opc_rfi);
	OPr(19, 258, ppc_opc_crandc);
	OPr(19, 300, ppc_opc_isync);
	OPr(19, 386, ppc_opc_crxor);
	OPr(19, 450, ppc_opc_crnand);
	OPr(19, 514, ppc_opc_crand);
	OPr(19, 578, ppc_opc_creqv);
	OPr(19, 834, ppc_opc_crorc);
	OPr(19, 898, ppc_opc_cror);
	OPr(19, 1056, ppc_opc_bcctrx<LK0>);
	OPr(19, 1057, ppc_opc_bcctrx<LK1>);

	OPrc(20, ppc_opc_rlwimix);
	OPrc(21, ppc_opc_rlwinmx);
	OPrc(23, ppc_opc_rlwnmx);

	OP31  (   0, ppc_opc_cmp);
	OP31  (   4, ppc_opc_tw);
	OP31rc(   8, ppc_opc_subfcx);//+
	OP31rc(  10, ppc_opc_addcx);//+
	OP31rc(  11, ppc_opc_mulhwux);
	OP31  (  19, ppc_opc_mfcr);
	OP31  (  20, ppc_opc_lwarx);
	OP31  (  23, ppc_opc_lwzx);
	OP31rc(  24, ppc_opc_slwx);
	OP31rc(  26, ppc_opc_cntlzwx);
	OP31rc(  28, ppc_opc_andx);
	OP31  (  32, ppc_opc_cmpl);
	OP31rc(  40, ppc_opc_subfx);
	OP31  (  54, ppc_opc_dcbst);
	OP31  (  55, ppc_opc_lwzux);
	OP31rc(  60, ppc_opc_andcx);
	OP31rc(  75, ppc_opc_mulhwx);
	OP31  (  83, ppc_opc_mfmsr);
	OP31  (  86, ppc_opc_dcbf);
	OP31  (  87, ppc_opc_lbzx);
	OP31rc( 104, ppc_opc_negx);
	OP31  ( 119, ppc_opc_lbzux);
	OP31rc( 124, ppc_opc_norx);
	OP31rc( 136, ppc_opc_subfex);//+
	OP31rc( 138, ppc_opc_addex);//+
	OP31  ( 144, ppc_opc_mtcrf);
	OP31  ( 146, ppc_opc_mtmsr);
	OP31  ( 150, ppc_opc_stwcx_);
	OP31  ( 151, ppc_opc_stwx);
	OP31  ( 183, ppc_opc_stwux);
	OP31rc( 200, ppc_opc_subfzex);//+
	OP31rc( 202, ppc_opc_addzex);//+
	OP31  ( 210, ppc_opc_mtsr);
	OP31  ( 215, ppc_opc_stbx);
	OP31rc( 232, ppc_opc_subfmex);//+
	OP31rc( 234, ppc_opc_addmex);
	OP31rc( 235, ppc_opc_mullwx);//+
	OP31  ( 242, ppc_opc_mtsrin);
	OP31  ( 246, ppc_opc_dcbtst);
	OP31  ( 247, ppc_opc_stbux);
	OP31rc( 266, ppc_opc_addx);//+
	OP31  ( 278, ppc_opc_dcbt);
	OP31  ( 279, ppc_opc_lhzx);
	OP31rc( 284, ppc_opc_eqvx);
	OP31  ( 306, ppc_opc_tlbie);
	OP31  ( 310, ppc_opc_eciwx);
	OP31  ( 311, ppc_opc_lhzux);
	OP31rc( 316, ppc_opc_xorx);
	OP31  ( 339, ppc_opc_mfspr);
	OP31  ( 343, ppc_opc_lhax);
	OP31  ( 370, ppc_opc_tlbia);
	OP31  ( 371, ppc_opc_mftb);
	OP31  ( 375, ppc_opc_lhaux);
	OP31  ( 407, ppc_opc_sthx);
	OP31rc( 412, ppc_opc_orcx);
	OP31  ( 438, ppc_opc_ecowx);
	OP31  ( 439, ppc_opc_sthux);
	OP31rc( 444, ppc_opc_orx);
	OP31rc( 459, ppc_opc_divwux);//+
	OP31  ( 467, ppc_opc_mtspr);
	OP31  ( 470, ppc_opc_dcbi);
	OP31rc( 476, ppc_opc_nandx);
	OP31rc( 491, ppc_opc_divwx);//+
	OP31  ( 512, ppc_opc_mcrxr);
	OP31  ( 533, ppc_opc_lswx);
	OP31  ( 534, ppc_opc_lwbrx);
	FP31  ( 535, ppc_opc_lfsx);
	OP31rc( 536, ppc_opc_srwx);
	OP31  ( 566, ppc_opc_tlbsync);
	FP31  ( 567, ppc_opc_lfsux);
	OP31  ( 595, ppc_opc_mfsr);
	OP31  ( 597, ppc_opc_lswi);
	OP31  ( 598, ppc_opc_sync);
	FP31  ( 599, ppc_opc_lfdx);
	FP31  ( 631, ppc_opc_lfdux);
	OP31  ( 659, ppc_opc_mfsrin);
	OP31  ( 661, ppc_opc_stswx);
	OP31  ( 662, ppc_opc_stwbrx);
	FP31  ( 663, ppc_opc_stfsx);
	FP31  ( 695, ppc_opc_stfsux);
	OP31  ( 725, ppc_opc_stswi);
	FP31  ( 727, ppc_opc_stfdx);
	OP31  ( 758, ppc_opc_dcba);
	FP31  ( 759, ppc_opc_stfdux);
	OP31  ( 790, ppc_opc_lhbrx);
	OP31rc( 792, ppc_opc_srawx);
	OP31rc( 824, ppc_opc_srawix);
	OP31  ( 854, ppc_opc_eieio);
	OP31  ( 918, ppc_opc_sthbrx);
	OP31rc( 922, ppc_opc_extshx);
	OP31rc( 954, ppc_opc_extsbx);
	OP31  ( 982, ppc_opc_icbi);
	FP31  ( 983, ppc_opc_stfiwx);
	OP31  (1014, ppc_opc_dcbz);

	if (is_g4) {
		/* Added for Altivec support */
		OP31(  6, ppc_opc_lvsl);
		OP31(  7, ppc_opc_lvebx);
		OP31( 38, ppc_opc_lvsr);
		OP31( 39, ppc_opc_lvehx);
		OP31( 71, ppc_opc_lvewx);
		OP31(103, ppc_opc_lvx);
		OP31(135, ppc_opc_stvebx);
		OP31(167, ppc_opc_stvehx);
		OP31(199, ppc_opc_stvewx);
		OP31(231, ppc_opc_stvx);
		OP31(342, ppc_opc_dst);
		OP31(359, ppc_opc_lvxl);
		OP31(374, ppc_opc_dstst);
		OP31(487, ppc_opc_stvxl);
		OP31(822, ppc_opc_dss);
	}

	FP59(18, ppc_opc_fdivsx);
	FP59(20, ppc_opc_fsubsx);
	FP59(21, ppc_opc_faddsx);
	FP59(22, ppc_opc_fsqrtsx);
	FP59(24, ppc_opc_fresx);
	FP59(25, ppc_opc_fmulsx);
	FP59(28, ppc_opc_fmsubsx);
	FP59(29, ppc_opc_fmaddsx);
	FP59(30, ppc_opc_fnmsubsx);
	FP59(31, ppc_opc_fnmaddsx);

	// Op 63 instructions where bits 21-25 are wildcards.
	FP63a(18, ppc_opc_fdivx);
	FP63a(20, ppc_opc_fsubx);
	FP63a(21, ppc_opc_faddx);
	FP63a(22, ppc_opc_fsqrtx);
	FP63a(23, ppc_opc_fselx);
	FP63a(25, ppc_opc_fmulx);
	FP63a(26, ppc_opc_frsqrtex);
	FP63a(28, ppc_opc_fmsubx);
	FP63a(29, ppc_opc_fmaddx);
	FP63a(30, ppc_opc_fnmsubx);
	FP63a(31, ppc_opc_fnmaddx);

	// Op 63 instructions where bits 21-25 are part of the subopcode
	FP63b(  0, ppc_opc_fcmpu);
	FP63b( 12, ppc_opc_frspx);
	FP63b( 14, ppc_opc_fctiwx);
	FP63b( 15, ppc_opc_fctiwzx);
	FP63b( 32, ppc_opc_fcmpo);
	FP63b( 38, ppc_opc_mtfsb1x);
	FP63b( 40, ppc_opc_fnegx);
	FP63b( 64, ppc_opc_mcrfs);
	FP63b( 70, ppc_opc_mtfsb0x);
	FP63b( 72, ppc_opc_fmrx);
	FP63b(134, ppc_opc_mtfsfix);
	FP63b(136, ppc_opc_fnabsx);
	FP63b(264, ppc_opc_fabsx);
	FP63b(583, ppc_opc_mffsx);
	FP63b(711, ppc_opc_mtfsfx);

	if (is_g4) {
		OP(4, ppc_opc_group_v);
		ppc_opc_init_groupv();
	}

	// 64-bit CPU
	// OP(2, ppc_opc_tdi);
	// OP(30, ppc_opc_group_rld);
	// OP(58, ppc_opc_ld);
}
