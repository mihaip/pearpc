/*
 *	PearPC
 *	ppc_opc.h
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
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

#ifndef __PPC_OPC_H__
#define __PPC_OPC_H__

#include "system/types.h"
#include "ppc_dec.h"

static inline void ppc_update_cr0(uint32 r)
{
	gCPU.cr &= 0x0fffffff;
	if (!r) {
		gCPU.cr |= CR_CR0_EQ;
	} else if (r & 0x80000000) {
		gCPU.cr |= CR_CR0_LT;
	} else {
		gCPU.cr |= CR_CR0_GT;
	}
	if (gCPU.xer & XER_SO) gCPU.cr |= CR_CR0_SO;
}

template <LKBit lk, AABit aa> void ppc_opc_bx(uint32 opc);
template <LKBit lk, AABit aa> void ppc_opc_bcx(uint32 opc);
template <LKBit lk> void ppc_opc_bcctrx(uint32 opc);
template <LKBit lk> void ppc_opc_bclrx(uint32 opc);

void ppc_opc_dcba(uint32 opc);
void ppc_opc_dcbf(uint32 opc);
void ppc_opc_dcbi(uint32 opc);
void ppc_opc_dcbst(uint32 opc);
void ppc_opc_dcbt(uint32 opc);
void ppc_opc_dcbtst(uint32 opc);

void ppc_opc_eciwx(uint32 opc);
void ppc_opc_ecowx(uint32 opc);
void ppc_opc_eieio(uint32 opc);

void ppc_opc_icbi(uint32 opc);
void ppc_opc_isync(uint32 opc);

void ppc_opc_mcrf(uint32 opc);
void ppc_opc_mcrfs(uint32 opc);
void ppc_opc_mcrxr(uint32 opc);
void ppc_opc_mfcr(uint32 opc);
void ppc_opc_mffsx(uint32 opc);
void ppc_opc_mfmsr(uint32 opc);
void ppc_opc_mfspr(uint32 opc);
void ppc_opc_mfsr(uint32 opc);
void ppc_opc_mfsrin(uint32 opc);
void ppc_opc_mftb(uint32 opc);
void ppc_opc_mtcrf(uint32 opc);
void ppc_opc_mtfsb0x(uint32 opc);
void ppc_opc_mtfsb1x(uint32 opc);
void ppc_opc_mtfsfx(uint32 opc);
void ppc_opc_mtfsfix(uint32 opc);
void ppc_opc_mtmsr(uint32 opc);
void ppc_opc_mtspr(uint32 opc);
void ppc_opc_mtsr(uint32 opc);
void ppc_opc_mtsrin(uint32 opc);

void ppc_opc_rfi(uint32 opc);
void ppc_opc_sc(uint32 opc);
void ppc_opc_sync(uint32 opc);
void ppc_opc_tlbia(uint32 opc);
void ppc_opc_tlbie(uint32 opc);
void ppc_opc_tlbsync(uint32 opc);
void ppc_opc_tw(uint32 opc);
void ppc_opc_twi(uint32 opc);


#endif

