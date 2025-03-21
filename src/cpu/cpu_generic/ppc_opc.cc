/*
 *	PearPC
 *	ppc_opc.cc
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
 *	Copyright (C) 2004 Dainel Foesch (dfoesch@cs.nmsu.edu)
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

#include "debug/tracers.h"
#include "cpu/debug.h"
#include "io/pic/pic.h"
#include "info.h"
#include "ppc_cpu.h"
#include "ppc_exc.h"
#include "ppc_mmu.h"
#include "ppc_opc.h"
#include "ppc_dec.h"


void ppc_set_msr(uint32 newmsr)
{
/*	if ((newmsr & MSR_EE) && !(gCPU.msr & MSR_EE)) {
		if (pic_check_interrupt()) {
			gCPU.exception_pending = true;
			gCPU.ext_exception = true;
		}
	}*/
	ppc_mmu_tlb_invalidate();
#ifndef PPC_CPU_ENABLE_SINGLESTEP
	if (newmsr & MSR_SE) {
		SINGLESTEP("");
		PPC_CPU_WARN("MSR[SE] (singlestep enable) set, but compiled w/o SE support.\n");
	}
#else 
	gCPU.singlestep_ignore = true;
#endif
	if (newmsr & PPC_CPU_UNSUPPORTED_MSR_BITS) {
		PPC_CPU_ERR("unsupported bits in MSR set: %08x @%08x\n", newmsr & PPC_CPU_UNSUPPORTED_MSR_BITS, gCPU.pc);
	}
	if (newmsr & MSR_POW) {
		// doze();
		newmsr &= ~MSR_POW;
	}
	gCPU.msr = newmsr;
	
}

/*
 *	bx		Branch
 *	.435
 */
template <LKBit lk, AABit aa>
void ppc_opc_bx(uint32 opc)
{
	uint32 li;
	PPC_OPC_TEMPL_I(opc, li);
	if (aa == AA0) {
		li += gCPU.pc;
	}
	if (lk == LK1) {
		gCPU.lr = gCPU.pc + 4;
	}
	gCPU.npc = li;
}

template void ppc_opc_bx<LK0, AA0>(uint32 opc);
template void ppc_opc_bx<LK0, AA1>(uint32 opc);
template void ppc_opc_bx<LK1, AA0>(uint32 opc);
template void ppc_opc_bx<LK1, AA1>(uint32 opc);

/*
 *	bcx		Branch Conditional
 *	.436
 */
template <LKBit lk, AABit aa>
void ppc_opc_bcx(uint32 opc)
{
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_B(opc, BO, BI, BD);
	if (!(BO & 4)) {
		gCPU.ctr--;
	}
	bool bo2 = (BO & 2);
	bool bo8 = (BO & 8); // branch condition true
	bool cr = (gCPU.cr & (1<<(31-BI)));
	if (((BO & 4) || ((gCPU.ctr!=0) ^ bo2))
	&& ((BO & 16) || (!(cr ^ bo8)))) {
		if (aa == AA0) {
			BD += gCPU.pc;
		}
		 if (lk == LK1) {
			gCPU.lr = gCPU.pc + 4;
		}
		gCPU.npc = BD;
	}
}

template void ppc_opc_bcx<LK0, AA0>(uint32 opc);
template void ppc_opc_bcx<LK0, AA1>(uint32 opc);
template void ppc_opc_bcx<LK1, AA0>(uint32 opc);
template void ppc_opc_bcx<LK1, AA1>(uint32 opc);

/*
 *	bcctrx		Branch Conditional to Count Register
 *	.438
 */
template <LKBit lk>
void ppc_opc_bcctrx(uint32 opc)
{
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_XL(opc, BO, BI, BD);
	PPC_OPC_ASSERT(BD==0);
	PPC_OPC_ASSERT(!(BO & 2));
	bool bo8 = (BO & 8);
	bool cr = (gCPU.cr & (1<<(31-BI)));
	if ((BO & 16) || (!(cr ^ bo8))) {
		if (lk == LK1) {
			gCPU.lr = gCPU.pc + 4;
		}
		gCPU.npc = gCPU.ctr & 0xfffffffc;
	}
}

template void ppc_opc_bcctrx<LK0>(uint32 opc);
template void ppc_opc_bcctrx<LK1>(uint32 opc);


/*
 *	bclrx		Branch Conditional to Link Register
 *	.440
 */
template <LKBit lk>
void ppc_opc_bclrx(uint32 opc)
{
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_XL(opc, BO, BI, BD);
	PPC_OPC_ASSERT(BD==0);
	if (!(BO & 4)) {
		gCPU.ctr--;
	}
	bool bo2 = (BO & 2);
	bool bo8 = (BO & 8);
	bool cr = (gCPU.cr & (1<<(31-BI)));
	if (((BO & 4) || ((gCPU.ctr!=0) ^ bo2))
	&& ((BO & 16) || (!(cr ^ bo8)))) {
		BD = gCPU.lr & 0xfffffffc;
		if (lk == LK1) {
			gCPU.lr = gCPU.pc + 4;
		}
		gCPU.npc = BD;
	}
}

template void ppc_opc_bclrx<LK0>(uint32 opc);
template void ppc_opc_bclrx<LK1>(uint32 opc);

/*
 *	dcbf		Data Cache Block Flush
 *	.458
 */
void ppc_opc_dcbf(uint32 opc)
{
	// NO-OP
}
/*
 *	dcbi		Data Cache Block Invalidate
 *	.460
 */
void ppc_opc_dcbi(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	// FIXME: check addr
}
/*
 *	dcbst		Data Cache Block Store
 *	.461
 */
void ppc_opc_dcbst(uint32 opc)
{
	// NO-OP
}
/*
 *	dcbt		Data Cache Block Touch
 *	.462
 */
void ppc_opc_dcbt(uint32 opc)
{
	// NO-OP
}
/*
 *	dcbtst		Data Cache Block Touch for Store
 *	.463
 */
void ppc_opc_dcbtst(uint32 opc)
{
	// NO-OP
}
/*
 *	eciwx		External Control In Word Indexed
 *	.474
 */
void ppc_opc_eciwx(uint32 opc)
{
	PPC_OPC_ERR("eciwx unimplemented.\n");
}
/*
 *	ecowx		External Control Out Word Indexed
 *	.476
 */
void ppc_opc_ecowx(uint32 opc)
{
	PPC_OPC_ERR("ecowx unimplemented.\n");
}
/*
 *	eieio		Enforce In-Order Execution of I/O
 *	.478
 */
void ppc_opc_eieio(uint32 opc)
{
	// NO-OP
}

/*
 *	icbi		Instruction Cache Block Invalidate
 *	.519
 */
void ppc_opc_icbi(uint32 opc)
{
	// NO-OP
}

/*
 *	isync		Instruction Synchronize
 *	.520
 */
void ppc_opc_isync(uint32 opc)
{
	// NO-OP
}

static uint32 ppc_cmp_and_mask[8] = {
	0xfffffff0,
	0xffffff0f,
	0xfffff0ff,
	0xffff0fff,
	0xfff0ffff,
	0xff0fffff,
	0xf0ffffff,
	0x0fffffff,
};
/*
 *	mcrf		Move Condition Register Field
 *	.561
 */
void ppc_opc_mcrf(uint32 opc)
{
	uint32 crD, crS, bla;
	PPC_OPC_TEMPL_X(opc, crD, crS, bla);
	// FIXME: bla == 0
	crD>>=2;
	crS>>=2;
	crD = 7-crD;
	crS = 7-crS;
	uint32 c = (gCPU.cr>>(crS*4)) & 0xf;
	gCPU.cr &= ppc_cmp_and_mask[crD];
	gCPU.cr |= c<<(crD*4);
}
/*
 *	mcrfs		Move to Condition Register from FPSCR
 *	.562
 */
void ppc_opc_mcrfs(uint32 opc)
{
	PPC_OPC_ERR("mcrfs unimplemented.\n");
}
/*
 *	mcrxr		Move to Condition Register from XER
 *	.563
 */
void ppc_opc_mcrxr(uint32 opc)
{
	PPC_OPC_ERR("mcrxr unimplemented.\n");
}
/*
 *	mfcr		Move from Condition Register
 *	.564
 */
void ppc_opc_mfcr(uint32 opc)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(opc, rD, rA, rB);
	PPC_OPC_ASSERT(rA==0 && rB==0);
	gCPU.gpr[rD] = gCPU.cr;
}
/*
 *	mffs		Move from FPSCR
 *	.565
 */
template <RcBit rc> 
void ppc_opc_mffsx(uint32 opc)
{
	int frD, rA, rB;
	PPC_OPC_TEMPL_X(opc, frD, rA, rB);
	PPC_OPC_ASSERT(rA==0 && rB==0);
	gCPU.fpr[frD] = gCPU.fpscr;
	if (rc == Rc1) {
		// update cr1 flags
		PPC_OPC_ERR("mffs. unimplemented.\n");
	}
}

template void ppc_opc_mffsx<Rc0>(uint32 opc);
template void ppc_opc_mffsx<Rc1>(uint32 opc);

/*
 *	mfmsr		Move from Machine State Register
 *	.566
 */
void ppc_opc_mfmsr(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(opc, rD, rA, rB);
	PPC_OPC_ASSERT((rA == 0) && (rB == 0));
	gCPU.gpr[rD] = gCPU.msr;
}
/*
 *	mfspr		Move from Special-Purpose Register
 *	.567
 */
void ppc_opc_mfspr(uint32 opc)
{
	int rD, spr1, spr2;
	PPC_OPC_TEMPL_XO(opc, rD, spr1, spr2);
	switch (spr2) {
	case 0:
		switch (spr1) {
		case 1: gCPU.gpr[rD] = gCPU.xer; return;
		case 8: gCPU.gpr[rD] = gCPU.lr; return;
		case 9: gCPU.gpr[rD] = gCPU.ctr; return;
		}
	case 8: // altivec made this spr unpriviledged
		if (spr1 == 0) {
			gCPU.gpr[rD] = gCPU.vrsave; 
			return;
		}
	}
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	switch (spr2) {
	case 0:
		switch (spr1) {
		case 18: gCPU.gpr[rD] = gCPU.dsisr; return;
		case 19: gCPU.gpr[rD] = gCPU.dar; return;
		case 22: {
			gCPU.dec = gCPU.pdec / TB_TO_PTB_FACTOR;
			gCPU.gpr[rD] = gCPU.dec;
			return;
		}
		case 25: gCPU.gpr[rD] = gCPU.sdr1; return;
		case 26: gCPU.gpr[rD] = gCPU.srr[0]; return;
		case 27: gCPU.gpr[rD] = gCPU.srr[1]; return;
		}
		break;
	case 8:
		switch (spr1) {
		case 12: {
			gCPU.tb = gCPU.ptb / TB_TO_PTB_FACTOR;
			gCPU.gpr[rD] = gCPU.tb;
			return;
		}
		case 13: {
			gCPU.tb = gCPU.ptb / TB_TO_PTB_FACTOR;
			gCPU.gpr[rD] = gCPU.tb >> 32;
			return;
		}
		case 16: gCPU.gpr[rD] = gCPU.sprg[0]; return;
		case 17: gCPU.gpr[rD] = gCPU.sprg[1]; return;
		case 18: gCPU.gpr[rD] = gCPU.sprg[2]; return;
		case 19: gCPU.gpr[rD] = gCPU.sprg[3]; return;
		case 26: gCPU.gpr[rD] = gCPU.ear; return;
		case 31: gCPU.gpr[rD] = gCPU.pvr; return;
		}
		break;
	case 16:
		switch (spr1) {
		case 16: gCPU.gpr[rD] = gCPU.ibatu[0]; return;
		case 17: gCPU.gpr[rD] = gCPU.ibatl[0]; return;
		case 18: gCPU.gpr[rD] = gCPU.ibatu[1]; return;
		case 19: gCPU.gpr[rD] = gCPU.ibatl[1]; return;
		case 20: gCPU.gpr[rD] = gCPU.ibatu[2]; return;
		case 21: gCPU.gpr[rD] = gCPU.ibatl[2]; return;
		case 22: gCPU.gpr[rD] = gCPU.ibatu[3]; return;
		case 23: gCPU.gpr[rD] = gCPU.ibatl[3]; return;
		case 24: gCPU.gpr[rD] = gCPU.dbatu[0]; return;
		case 25: gCPU.gpr[rD] = gCPU.dbatl[0]; return;
		case 26: gCPU.gpr[rD] = gCPU.dbatu[1]; return;
		case 27: gCPU.gpr[rD] = gCPU.dbatl[1]; return;
		case 28: gCPU.gpr[rD] = gCPU.dbatu[2]; return;
		case 29: gCPU.gpr[rD] = gCPU.dbatl[2]; return;
		case 30: gCPU.gpr[rD] = gCPU.dbatu[3]; return;
		case 31: gCPU.gpr[rD] = gCPU.dbatl[3]; return;
		}
		break;
	case 29:
		switch (spr1) {
		case 16:
			gCPU.gpr[rD] = 0;
			return;
		case 17:
			gCPU.gpr[rD] = 0;
			return;
		case 18:
			gCPU.gpr[rD] = 0;
			return;
		case 24:
			gCPU.gpr[rD] = 0;
			return;
		case 25:
			gCPU.gpr[rD] = 0;
			return;
		case 26:
			gCPU.gpr[rD] = 0;
			return;
		case 28:
			gCPU.gpr[rD] = 0;
			return;
		case 29:
			gCPU.gpr[rD] = 0;
			return;
		case 30:
			gCPU.gpr[rD] = 0;
			return;
		}
	case 31:
		switch (spr1) {
		case 16:
//			PPC_OPC_WARN("read from spr %d:%d (HID0) not supported!\n", spr1, spr2);
			gCPU.gpr[rD] = gCPU.hid[0];
			return;
		case 17:
			PPC_OPC_WARN("read from spr %d:%d (HID1) not supported!\n", spr1, spr2);
			gCPU.gpr[rD] = gCPU.hid[1];
			return;
		case 18:
			gCPU.gpr[rD] = 0;
			return;
		case 21:
			gCPU.gpr[rD] = 0;
			return;
		case 22:
			gCPU.gpr[rD] = 0;
			return;
		case 23:
			gCPU.gpr[rD] = 0;
			return;
		case 25:
			PPC_OPC_WARN("read from spr %d:%d (L2CR) not supported! (from %08x)\n", spr1, spr2, gCPU.pc);
			gCPU.gpr[rD] = 0;
			return;
		case 27:
			PPC_OPC_WARN("read from spr %d:%d (ICTC) not supported!\n", spr1, spr2);
			gCPU.gpr[rD] = 0;
			return;
		case 28:
//			PPC_OPC_WARN("read from spr %d:%d (THRM1) not supported!\n", spr1, spr2);
			gCPU.gpr[rD] = 0;
			return;
		case 29:
//			PPC_OPC_WARN("read from spr %d:%d (THRM2) not supported!\n", spr1, spr2);
			gCPU.gpr[rD] = 0;
			return;
		case 30:
//			PPC_OPC_WARN("read from spr %d:%d (THRM3) not supported!\n", spr1, spr2);
			gCPU.gpr[rD] = 0;
			return;
		case 31:
//			PPC_OPC_WARN("read from spr %d:%d (???) not supported!\n", spr1, spr2);
			gCPU.gpr[rD] = 0;
			return;
		}
	}
	fprintf(stderr, "unknown mfspr: %i:%i\n", spr1, spr2);
	SINGLESTEP("invalid mfspr\n");
}
/*
 *	mfsr		Move from Segment Register
 *	.570
 */
void ppc_opc_mfsr(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rD, SR, rB;
	PPC_OPC_TEMPL_X(opc, rD, SR, rB);
	// FIXME: check insn
	gCPU.gpr[rD] = gCPU.sr[SR & 0xf];
}
/*
 *	mfsrin		Move from Segment Register Indirect
 *	.572
 */
void ppc_opc_mfsrin(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(opc, rD, rA, rB);
	// FIXME: check insn
	gCPU.gpr[rD] = gCPU.sr[gCPU.gpr[rB] >> 28];
}
/*
 *	mftb		Move from Time Base
 *	.574
 */
void ppc_opc_mftb(uint32 opc)
{
	int rD, spr1, spr2;
	PPC_OPC_TEMPL_X(opc, rD, spr1, spr2);
	switch (spr2) {
	case 8:
		switch (spr1) {
		case 12: {
			gCPU.tb = gCPU.ptb / TB_TO_PTB_FACTOR;
			gCPU.gpr[rD] = gCPU.tb;
			return;
		}
		case 13: {
			gCPU.tb = gCPU.ptb / TB_TO_PTB_FACTOR;
			gCPU.gpr[rD] = gCPU.tb >> 32;
			return;
		}
		}
		break;
	}
	SINGLESTEP("unknown mftb\n");
}
/*
 *	mtcrf		Move to Condition Register Fields
 *	.576
 */
void ppc_opc_mtcrf(uint32 opc)
{
	
	int rS;
	uint32 crm;
	uint32 CRM;
	PPC_OPC_TEMPL_XFX(opc, rS, crm);
	CRM = ((crm&0x80)?0xf0000000:0)|((crm&0x40)?0x0f000000:0)|((crm&0x20)?0x00f00000:0)|((crm&0x10)?0x000f0000:0)|
	      ((crm&0x08)?0x0000f000:0)|((crm&0x04)?0x00000f00:0)|((crm&0x02)?0x000000f0:0)|((crm&0x01)?0x0000000f:0);
	gCPU.cr = (gCPU.gpr[rS] & CRM) | (gCPU.cr & ~CRM);
}
/*
 *	mtfsb0x		Move to FPSCR Bit 0
 *	.577
 */
template <RcBit rc>
void ppc_opc_mtfsb0x(uint32 opc)
{
	int crbD, n1, n2;
	PPC_OPC_TEMPL_X(opc, crbD, n1, n2);
	if (crbD != 1 && crbD != 2) {
		gCPU.fpscr &= ~(1<<(31-crbD));
	}
	if (rc == Rc1) {
		// update cr1 flags
		PPC_OPC_ERR("mtfsb0. unimplemented.\n");
	}
}

template void ppc_opc_mtfsb0x<Rc0>(uint32 opc);
template void ppc_opc_mtfsb0x<Rc1>(uint32 opc);

/*
 *	mtfsb1x		Move to FPSCR Bit 1
 *	.578
 */
template <RcBit rc>
void ppc_opc_mtfsb1x(uint32 opc)
{
	int crbD, n1, n2;
	PPC_OPC_TEMPL_X(opc, crbD, n1, n2);
	if (crbD != 1 && crbD != 2) {
		gCPU.fpscr |= 1<<(31-crbD);
	}
	if (rc == Rc1) {
		// update cr1 flags
		PPC_OPC_ERR("mtfsb1. unimplemented.\n");
	}
}

template void ppc_opc_mtfsb1x<Rc0>(uint32 opc);
template void ppc_opc_mtfsb1x<Rc1>(uint32 opc);

/*
 *	mtfsfx		Move to FPSCR Fields
 *	.579
 */
template <RcBit rc>
void ppc_opc_mtfsfx(uint32 opc)
{
	int frB;
	uint32 fm, FM;
	PPC_OPC_TEMPL_XFL(opc, frB, fm);
	FM = ((fm&0x80)?0xf0000000:0)|((fm&0x40)?0x0f000000:0)|((fm&0x20)?0x00f00000:0)|((fm&0x10)?0x000f0000:0)|
	     ((fm&0x08)?0x0000f000:0)|((fm&0x04)?0x00000f00:0)|((fm&0x02)?0x000000f0:0)|((fm&0x01)?0x0000000f:0);
	gCPU.fpscr = (gCPU.fpr[frB] & FM) | (gCPU.fpscr & ~FM);
	if (rc == Rc1) {
		// update cr1 flags
		PPC_OPC_ERR("mtfsf. unimplemented.\n");
	}
}

template void ppc_opc_mtfsfx<Rc0>(uint32 opc);
template void ppc_opc_mtfsfx<Rc1>(uint32 opc);

/*
 *	mtfsfix		Move to FPSCR Field Immediate
 *	.580
 */
template <RcBit rc>
void ppc_opc_mtfsfix(uint32 opc)
{
	int crfD, n1;
	uint32 imm;
	PPC_OPC_TEMPL_X(opc, crfD, n1, imm);
	crfD >>= 2;
	imm >>= 1;
	crfD = 7-crfD;
	gCPU.fpscr &= ppc_cmp_and_mask[crfD];
	gCPU.fpscr |= imm<<(crfD*4);
	if (rc == Rc1) {
		// update cr1 flags
		PPC_OPC_ERR("mtfsfi. unimplemented.\n");
	}
}

template void ppc_opc_mtfsfix<Rc0>(uint32 opc);
template void ppc_opc_mtfsfix<Rc1>(uint32 opc);

/*
 *	mtmsr		Move to Machine State Register
 *	.581
 */
void ppc_opc_mtmsr(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(opc, rS, rA, rB);
	PPC_OPC_ASSERT((rA == 0) && (rB == 0));
	ppc_set_msr(gCPU.gpr[rS]);
}
/*
 *	mtspr		Move to Special-Purpose Register
 *	.584
 */
void ppc_opc_mtspr(uint32 opc)
{
	int rS, spr1, spr2;
	PPC_OPC_TEMPL_X(opc, rS, spr1, spr2);
	switch (spr2) {
	case 0:
		switch (spr1) {
		case 1: gCPU.xer = gCPU.gpr[rS]; return;
		case 8:	gCPU.lr = gCPU.gpr[rS]; return;
		case 9:	gCPU.ctr = gCPU.gpr[rS]; return;
		}
	case 8:	//altivec makes this register unpriviledged
		if (spr1 == 0) {
			gCPU.vrsave = gCPU.gpr[rS]; 
			return;
		}
	}
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	switch (spr2) {
	case 0:
		switch (spr1) {
/*		case 18: gCPU.gpr[rD] = gCPU.dsisr; return;
		case 19: gCPU.gpr[rD] = gCPU.dar; return;*/
		case 22: {
			gCPU.dec = gCPU.gpr[rS];
			gCPU.pdec = gCPU.dec;
			gCPU.pdec *= TB_TO_PTB_FACTOR;
			return;
		}
		case 25: 
			if (!ppc_mmu_set_sdr1(gCPU.gpr[rS], true)) {
				PPC_OPC_ERR("cannot set sdr1\n");
			}
			return;
		case 26: gCPU.srr[0] = gCPU.gpr[rS]; return;
		case 27: gCPU.srr[1] = gCPU.gpr[rS]; return;
		}
		break;
	case 8:
		switch (spr1) {
		case 16: gCPU.sprg[0] = gCPU.gpr[rS]; return;
		case 17: gCPU.sprg[1] = gCPU.gpr[rS]; return;
		case 18: gCPU.sprg[2] = gCPU.gpr[rS]; return;
		case 19: gCPU.sprg[3] = gCPU.gpr[rS]; return;
/*		case 26: gCPU.gpr[rD] = gCPU.ear; return;
		case 31: gCPU.gpr[rD] = gCPU.pvr; return;*/
		}
		break;
	case 16:
		switch (spr1) {
		case 16:
			gCPU.ibatu[0] = gCPU.gpr[rS];
			gCPU.ibat_bl17[0] = ~(BATU_BL(gCPU.ibatu[0])<<17);
			return;
		case 17:
			gCPU.ibatl[0] = gCPU.gpr[rS];
			return;
		case 18:
			gCPU.ibatu[1] = gCPU.gpr[rS];
			gCPU.ibat_bl17[1] = ~(BATU_BL(gCPU.ibatu[1])<<17);
			return;
		case 19:
			gCPU.ibatl[1] = gCPU.gpr[rS];
			return;
		case 20:
			gCPU.ibatu[2] = gCPU.gpr[rS];
			gCPU.ibat_bl17[2] = ~(BATU_BL(gCPU.ibatu[2])<<17);
			return;
		case 21:
			gCPU.ibatl[2] = gCPU.gpr[rS];
			return;
		case 22:
			gCPU.ibatu[3] = gCPU.gpr[rS];
			gCPU.ibat_bl17[3] = ~(BATU_BL(gCPU.ibatu[3])<<17);
			return;
		case 23:
			gCPU.ibatl[3] = gCPU.gpr[rS];
			return;
		case 24:
			gCPU.dbatu[0] = gCPU.gpr[rS];
			gCPU.dbat_bl17[0] = ~(BATU_BL(gCPU.dbatu[0])<<17);
			return;
		case 25:
			gCPU.dbatl[0] = gCPU.gpr[rS];
			return;
		case 26:
			gCPU.dbatu[1] = gCPU.gpr[rS];
			gCPU.dbat_bl17[1] = ~(BATU_BL(gCPU.dbatu[1])<<17);
			return;
		case 27:
			gCPU.dbatl[1] = gCPU.gpr[rS];
			return;
		case 28:
			gCPU.dbatu[2] = gCPU.gpr[rS];
			gCPU.dbat_bl17[2] = ~(BATU_BL(gCPU.dbatu[2])<<17);
			return;
		case 29:
			gCPU.dbatl[2] = gCPU.gpr[rS];
			return;
		case 30:
			gCPU.dbatu[3] = gCPU.gpr[rS];
			gCPU.dbat_bl17[3] = ~(BATU_BL(gCPU.dbatu[3])<<17);
			return;
		case 31:
			gCPU.dbatl[3] = gCPU.gpr[rS];
			return;
		}
		break;
	case 29:
		switch(spr1) {
		case 17: return;
		case 24: return;
		case 25: return;
		case 26: return;
		}
	case 31:
		switch (spr1) {
		case 16:
//			PPC_OPC_WARN("write(%08x) to spr %d:%d (HID0) not supported! @%08x\n", gCPU.gpr[rS], spr1, spr2, gCPU.pc);
			gCPU.hid[0] = gCPU.gpr[rS];
			return;
		case 17: return;
		case 18:
			PPC_OPC_ERR("write(%08x) to spr %d:%d (IABR) not supported!\n", gCPU.gpr[rS], spr1, spr2);
			return;
		case 21:
			PPC_OPC_ERR("write(%08x) to spr %d:%d (DABR) not supported!\n", gCPU.gpr[rS], spr1, spr2);
			return;
		case 24:
			PPC_OPC_WARN("write(%08x) to spr %d:%d (?) not supported!\n", gCPU.gpr[rS], spr1, spr2);
			return;
		case 27:
			PPC_OPC_WARN("write(%08x) to spr %d:%d (ICTC) not supported!\n", gCPU.gpr[rS], spr1, spr2);
			return;
		case 28:
//			PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM1) not supported!\n", gCPU.gpr[rS], spr1, spr2);
			return;
		case 29:
//			PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM2) not supported!\n", gCPU.gpr[rS], spr1, spr2);
			return;
		case 30:
//			PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM3) not supported!\n", gCPU.gpr[rS], spr1, spr2);
			return;
		case 31: return;
		}
	}
	fprintf(stderr, "unknown mtspr: %i:%i\n", spr1, spr2);
	SINGLESTEP("unknown mtspr\n");
}
/*
 *	mtsr		Move to Segment Register
 *	.587
 */
void ppc_opc_mtsr(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rS, SR, rB;
	PPC_OPC_TEMPL_X(opc, rS, SR, rB);
	// FIXME: check insn
	gCPU.sr[SR & 0xf] = gCPU.gpr[rS];
}
/*
 *	mtsrin		Move to Segment Register Indirect
 *	.591
 */
void ppc_opc_mtsrin(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(opc, rS, rA, rB);
	// FIXME: check insn
	gCPU.sr[gCPU.gpr[rB] >> 28] = gCPU.gpr[rS];
}

/*
 *	rfi		Return from Interrupt
 *	.607
 */
void ppc_opc_rfi(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	ppc_set_msr(gCPU.srr[1] & MSR_RFI_SAVE_MASK);
	gCPU.npc = gCPU.srr[0] & 0xfffffffc;
}

/*
 *	sc		System Call
 *	.621
 */
#include "io/graphic/gcard.h"
void ppc_opc_sc(uint32 opc)
{
	if (gCPU.gpr[3] == 0x113724fa && gCPU.gpr[4] == 0x77810f9b) {
		gcard_osi(0);
		return;
	}
	ppc_exception(PPC_EXC_SC);
}

/*
 *	sync		Synchronize
 *	.672
 */
void ppc_opc_sync(uint32 opc)
{
	// NO-OP
}

/*
 *	tlbie		Translation Lookaside Buffer Invalidate Entry
 *	.676
 */
void ppc_opc_tlbia(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(opc, rS, rA, rB);
	// FIXME: check rS.. for 0
	ppc_mmu_tlb_invalidate();
}

/*
 *	tlbie		Translation Lookaside Buffer Invalidate All
 *	.676
 */
void ppc_opc_tlbie(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(opc, rS, rA, rB);
	// FIXME: check rS.. for 0     
	ppc_mmu_tlb_invalidate();
}

/*
 *	tlbsync		Translation Lookaside Buffer Syncronize
 *	.677
 */
void ppc_opc_tlbsync(uint32 opc)
{
	if (gCPU.msr & MSR_PR) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(opc, rS, rA, rB);
	// FIXME: check rS.. for 0     
	ppc_mmu_tlb_invalidate();
}

/*
 *	tw		Trap Word
 *	.678
 */
void ppc_opc_tw(uint32 opc)
{
	int TO, rA, rB;
	PPC_OPC_TEMPL_X(opc, TO, rA, rB);
	uint32 a = gCPU.gpr[rA];
	uint32 b = gCPU.gpr[rB];
	if (((TO & 16) && ((sint32)a < (sint32)b)) 
	|| ((TO & 8) && ((sint32)a > (sint32)b)) 
	|| ((TO & 4) && (a == b)) 
	|| ((TO & 2) && (a < b)) 
	|| ((TO & 1) && (a > b))) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_TRAP);
	}
}

/*
 *	twi		Trap Word Immediate
 *	.679
 */
void ppc_opc_twi(uint32 opc)
{
	int TO, rA;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(opc, TO, rA, imm);
	uint32 a = gCPU.gpr[rA];
	if (((TO & 16) && ((sint32)a < (sint32)imm)) 
	|| ((TO & 8) && ((sint32)a > (sint32)imm)) 
	|| ((TO & 4) && (a == imm)) 
	|| ((TO & 2) && (a < imm)) 
	|| ((TO & 1) && (a > imm))) {
		ppc_exception(PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_TRAP);
	}
}

/*	dcba		Data Cache Block Allocate
 *	.???
 */
void ppc_opc_dcba(uint32 opc)
{
	/* NO-OP */
}
