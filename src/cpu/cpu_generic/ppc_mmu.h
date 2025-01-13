/*
 *	PearPC
 *	ppc_mmu.h
 *
 *	Copyright (C) 2003, 2004 Sebastian Biallas (sb@biallas.net)
 *	Copyright (C) 2004 Daniel Foesch (dfoesch@cs.nmsu.edu)
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

#ifndef __PPC_MMU_H__
#define __PPC_MMU_H__

#include "system/types.h"

extern byte *gMemory;
extern uint32 gMemorySize;

#define PPC_MMU_READ  1
#define PPC_MMU_WRITE 2
#define PPC_MMU_CODE  4
#define PPC_MMU_SV    8
#define PPC_MMU_NO_EXC 16

#define PPC_MMU_OK 0
#define PPC_MMU_EXC 1
#define PPC_MMU_FATAL 2

int FASTCALL ppc_effective_to_physical(uint32 addr, int flags, uint32 &result);
bool FASTCALL ppc_mmu_set_sdr1(uint32 newval, bool quiesce);
void ppc_mmu_tlb_invalidate();

int FASTCALL ppc_read_physical_dword(uint32 addr, uint64 &result);
int FASTCALL ppc_read_physical_word(uint32 addr, uint32 &result);
int FASTCALL ppc_read_physical_half(uint32 addr, uint16 &result);
int FASTCALL ppc_read_physical_byte(uint32 addr, uint8 &result);
 
int FASTCALL ppc_read_effective_code(uint32 addr, uint32 &result);
int FASTCALL ppc_read_effective_dword(uint32 addr, uint64 &result);
int FASTCALL ppc_read_effective_word(uint32 addr, uint32 &result);
int FASTCALL ppc_read_effective_half(uint32 addr, uint16 &result);
int FASTCALL ppc_read_effective_byte(uint32 addr, uint8 &result);

int FASTCALL ppc_write_physical_dword(uint32 addr, uint64 data);
int FASTCALL ppc_write_physical_word(uint32 addr, uint32 data);
int FASTCALL ppc_write_physical_half(uint32 addr, uint16 data);
int FASTCALL ppc_write_physical_byte(uint32 addr, uint8 data);

int FASTCALL ppc_write_effective_dword(uint32 addr, uint64 data);
int FASTCALL ppc_write_effective_word(uint32 addr, uint32 data);
int FASTCALL ppc_write_effective_half(uint32 addr, uint16 data);
int FASTCALL ppc_write_effective_byte(uint32 addr, uint8 data);

int FASTCALL ppc_direct_physical_memory_handle(uint32 addr, byte *&ptr);
int FASTCALL ppc_direct_effective_memory_handle(uint32 addr, byte *&ptr);
int FASTCALL ppc_direct_effective_memory_handle_code(uint32 addr, byte *&ptr);
bool FASTCALL ppc_mmu_page_create(uint32 ea, uint32 pa);
bool FASTCALL ppc_mmu_page_free(uint32 ea);

/*
pte: (page table entry)
1st word:
0     V    Valid
1-24  VSID Virtual Segment ID
25    H    Hash function
26-31 API  Abbreviated page index
2nd word:
0-19  RPN  Physical page number
20-22 res
23    R    Referenced bit
24    C    Changed bit
25-28 WIMG Memory/cache control bits
29    res
30-31 PP   Page protection bits
*/

/*
 *	MMU Opcodes
 */
void ppc_opc_dcbz(uint32 opc);

void ppc_opc_lbz(uint32 opc);
void ppc_opc_lbzu(uint32 opc);
void ppc_opc_lbzux(uint32 opc);
void ppc_opc_lbzx(uint32 opc);
void ppc_opc_lfd(uint32 opc);
void ppc_opc_lfdu(uint32 opc);
void ppc_opc_lfdux(uint32 opc);
void ppc_opc_lfdx(uint32 opc);
void ppc_opc_lfs(uint32 opc);
void ppc_opc_lfsu(uint32 opc);
void ppc_opc_lfsux(uint32 opc);
void ppc_opc_lfsx(uint32 opc);
void ppc_opc_lha(uint32 opc);
void ppc_opc_lhau(uint32 opc);
void ppc_opc_lhaux(uint32 opc);
void ppc_opc_lhax(uint32 opc);
void ppc_opc_lhbrx(uint32 opc);
void ppc_opc_lhz(uint32 opc);
void ppc_opc_lhzu(uint32 opc);
void ppc_opc_lhzux(uint32 opc);
void ppc_opc_lhzx(uint32 opc);
void ppc_opc_lmw(uint32 opc);
void ppc_opc_lswi(uint32 opc);
void ppc_opc_lswx(uint32 opc);
void ppc_opc_lwarx(uint32 opc);
void ppc_opc_lwbrx(uint32 opc);
void ppc_opc_lwz(uint32 opc);
void ppc_opc_lwzu(uint32 opc);
void ppc_opc_lwzux(uint32 opc);
void ppc_opc_lwzx(uint32 opc);
void ppc_opc_lvx(uint32 opc);             /* for altivec support */
void ppc_opc_lvxl(uint32 opc);
void ppc_opc_lvebx(uint32 opc);
void ppc_opc_lvehx(uint32 opc);
void ppc_opc_lvewx(uint32 opc);
void ppc_opc_lvsl(uint32 opc);
void ppc_opc_lvsr(uint32 opc);
void ppc_opc_dst(uint32 opc);

void ppc_opc_stb(uint32 opc);
void ppc_opc_stbu(uint32 opc);
void ppc_opc_stbux(uint32 opc);
void ppc_opc_stbx(uint32 opc);
void ppc_opc_stfd(uint32 opc);
void ppc_opc_stfdu(uint32 opc);
void ppc_opc_stfdux(uint32 opc);
void ppc_opc_stfdx(uint32 opc);
void ppc_opc_stfiwx(uint32 opc);
void ppc_opc_stfs(uint32 opc);
void ppc_opc_stfsu(uint32 opc);
void ppc_opc_stfsux(uint32 opc);
void ppc_opc_stfsx(uint32 opc);
void ppc_opc_sth(uint32 opc);
void ppc_opc_sthbrx(uint32 opc);
void ppc_opc_sthu(uint32 opc);
void ppc_opc_sthux(uint32 opc);
void ppc_opc_sthx(uint32 opc);
void ppc_opc_stmw(uint32 opc);
void ppc_opc_stswi(uint32 opc);
void ppc_opc_stswx(uint32 opc);
void ppc_opc_stw(uint32 opc);
void ppc_opc_stwbrx(uint32 opc);
void ppc_opc_stwcx_(uint32 opc);
void ppc_opc_stwu(uint32 opc);
void ppc_opc_stwux(uint32 opc);
void ppc_opc_stwx(uint32 opc);
void ppc_opc_stvx(uint32 opc);            /* for altivec support */
void ppc_opc_stvxl(uint32 opc);
void ppc_opc_stvebx(uint32 opc);
void ppc_opc_stvehx(uint32 opc);
void ppc_opc_stvewx(uint32 opc);
void ppc_opc_dstst(uint32 opc);
void ppc_opc_dss(uint32 opc);

#endif

