/*
 *	PearPC
 *	ppc_alu.h
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

#ifndef __PPC_ALU_H__
#define __PPC_ALU_H__

#include "ppc_dec.h"

template <RcBit rc> void ppc_opc_addx(uint32 opc);
template <RcBit rc> void ppc_opc_addcx(uint32 opc);
template <RcBit rc> void ppc_opc_addex(uint32 opc);
void ppc_opc_addi(uint32 opc);
void ppc_opc_addic(uint32 opc);
void ppc_opc_addic_(uint32 opc);
void ppc_opc_addis(uint32 opc);
template <RcBit rc> void ppc_opc_addmex(uint32 opc);
template <RcBit rc> void ppc_opc_addzex(uint32 opc);

template <RcBit rc> void ppc_opc_andx(uint32 opc);
template <RcBit rc> void ppc_opc_andcx(uint32 opc);
void ppc_opc_andi_(uint32 opc);
void ppc_opc_andis_(uint32 opc);

void ppc_opc_cmp(uint32 opc);
void ppc_opc_cmpi(uint32 opc);
void ppc_opc_cmpl(uint32 opc);
void ppc_opc_cmpli(uint32 opc);

template <RcBit rc> void ppc_opc_cntlzwx(uint32 opc);

void ppc_opc_crand(uint32 opc);
void ppc_opc_crandc(uint32 opc);
void ppc_opc_creqv(uint32 opc);
void ppc_opc_crnand(uint32 opc);
void ppc_opc_crnor(uint32 opc);
void ppc_opc_cror(uint32 opc);
void ppc_opc_crorc(uint32 opc);
void ppc_opc_crxor(uint32 opc);

template <RcBit rc> void ppc_opc_divwx(uint32 opc);
template <RcBit rc> void ppc_opc_divwux(uint32 opc);

template <RcBit rc> void ppc_opc_eqvx(uint32 opc);

template <RcBit rc> void ppc_opc_extsbx(uint32 opc);
template <RcBit rc> void ppc_opc_extshx(uint32 opc);

template <RcBit rc> void ppc_opc_mulhwx(uint32 opc);
template <RcBit rc> void ppc_opc_mulhwux(uint32 opc);
void ppc_opc_mulli(uint32 opc);
template <RcBit rc> void ppc_opc_mullwx(uint32 opc);

template <RcBit rc> void ppc_opc_nandx(uint32 opc);

template <RcBit rc> void ppc_opc_negx(uint32 opc);
template <RcBit rc> void ppc_opc_norx(uint32 opc);

template <RcBit rc> void ppc_opc_orx(uint32 opc);
template <RcBit rc> void ppc_opc_orcx(uint32 opc);
void ppc_opc_ori(uint32 opc);
void ppc_opc_oris(uint32 opc);

template <RcBit rc> void ppc_opc_rlwimix(uint32 opc);
template <RcBit rc> void ppc_opc_rlwinmx(uint32 opc);
template <RcBit rc> void ppc_opc_rlwnmx(uint32 opc);

template <RcBit rc> void ppc_opc_slwx(uint32 opc);
template <RcBit rc> void ppc_opc_srawx(uint32 opc);
template <RcBit rc> void ppc_opc_srawix(uint32 opc);
template <RcBit rc> void ppc_opc_srwx(uint32 opc);

template <RcBit rc> void ppc_opc_subfx(uint32 opc);
template <RcBit rc> void ppc_opc_subfcx(uint32 opc);
template <RcBit rc> void ppc_opc_subfex(uint32 opc);
void ppc_opc_subfic(uint32 opc);
template <RcBit rc> void ppc_opc_subfmex(uint32 opc);
template <RcBit rc> void ppc_opc_subfzex(uint32 opc);

template <RcBit rc> void ppc_opc_xorx(uint32 opc);
void ppc_opc_xori(uint32 opc);
void ppc_opc_xoris(uint32 opc);

#endif

