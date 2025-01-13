/*
 *	PearPC
 *	ppc_vec.h
 *
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
 
#ifndef __PPC_VEC_H__
#define __PPC_VEC_H__

#include "system/types.h"

#define PPC_OPC_VRc	(1<<10)

/* Rather than write each function to be endianless, we're writing these
 *   defines to do an endianless access to elements of the vector.
 *
 * These are for ADDRESSED vector elements.  Usually, most vector operations
 *   can be performed in either direction without care, so most of the
 *   for-loops should not use these, as it will introduce unneeded code
 *   for little-endian systems.
 */
#if HOST_ENDIANESS == HOST_ENDIANESS_LE

#define VECT_B(reg, index)	((reg).b[15 - (index)])
#define VECT_SB(reg, index)	((reg).sb[15 - (index)])
#define VECT_H(reg, index)	((reg).h[7 - (index)])
#define VECT_SH(reg, index)	((reg).sh[7 - (index)])
#define VECT_W(reg, index)	((reg).w[3 - (index)])
#define VECT_SW(reg, index)	((reg).sw[3 - (index)])
#define VECT_D(reg, index)	((reg).d[1 - (index)])
#define VECT_SD(reg, index)	((reg).sd[1 - (index)])

#define VECT_EVEN(index)	(((index) << 1) + 1)
#define VECT_ODD(index)		(((index) << 1) + 0)

#elif HOST_ENDIANESS == HOST_ENDIANESS_BE

#define VECT_B(reg, index)	((reg).b[(index)])
#define VECT_SB(reg, index)	((reg).sb[(index)])
#define VECT_H(reg, index)	((reg).h[(index)])
#define VECT_SH(reg, index)	((reg).sh[(index)])
#define VECT_W(reg, index)	((reg).w[(index)])
#define VECT_SW(reg, index)	((reg).sw[(index)])
#define VECT_D(reg, index)	((reg).d[(index)])
#define VECT_SD(reg, index)	((reg).sd[(index)])

#define VECT_EVEN(index)	(((index) << 1) + 0)
#define VECT_ODD(index)		(((index) << 1) + 1)

#else
#error Endianess not supported!
#endif

//#define VECTOR_DEBUG	fprintf(stderr, "[PPC/VEC] %s\n", __FUNCTION__)
#define VECTOR_DEBUG

//#define VECTOR_DEBUG_COMMON	fprintf(stderr, "[PPC/VEC] %s\n", __FUNCTION__)
#define VECTOR_DEBUG_COMMON

/* Undefine this to turn of the MSR_VEC check for vector instructions. */
//#define __VEC_EXC_OFF__

#include "system/types.h"

#include "tools/snprintf.h"

void ppc_opc_vperm(uint32 opc);
void ppc_opc_vsel(uint32 opc);
void ppc_opc_vsrb(uint32 opc);
void ppc_opc_vsrh(uint32 opc);
void ppc_opc_vsrw(uint32 opc);
void ppc_opc_vsrab(uint32 opc);
void ppc_opc_vsrah(uint32 opc);
void ppc_opc_vsraw(uint32 opc);
void ppc_opc_vsr(uint32 opc);
void ppc_opc_vsro(uint32 opc);
void ppc_opc_vslb(uint32 opc);
void ppc_opc_vslh(uint32 opc);
void ppc_opc_vslw(uint32 opc);
void ppc_opc_vsl(uint32 opc);
void ppc_opc_vslo(uint32 opc);
void ppc_opc_vsldoi(uint32 opc);
void ppc_opc_vrlb(uint32 opc);
void ppc_opc_vrlh(uint32 opc);
void ppc_opc_vrlw(uint32 opc);
void ppc_opc_vmrghb(uint32 opc);
void ppc_opc_vmrghh(uint32 opc);
void ppc_opc_vmrghw(uint32 opc);
void ppc_opc_vmrglb(uint32 opc);
void ppc_opc_vmrglh(uint32 opc);
void ppc_opc_vmrglw(uint32 opc);
void ppc_opc_vspltb(uint32 opc);
void ppc_opc_vsplth(uint32 opc);
void ppc_opc_vspltw(uint32 opc);
void ppc_opc_vspltisb(uint32 opc);
void ppc_opc_vspltish(uint32 opc);
void ppc_opc_vspltisw(uint32 opc);
void ppc_opc_mfvscr(uint32 opc);
void ppc_opc_mtvscr(uint32 opc);
void ppc_opc_vpkuhum(uint32 opc);
void ppc_opc_vpkuwum(uint32 opc);
void ppc_opc_vpkpx(uint32 opc);
void ppc_opc_vpkuhus(uint32 opc);
void ppc_opc_vpkshss(uint32 opc);
void ppc_opc_vpkuwus(uint32 opc);
void ppc_opc_vpkswss(uint32 opc);
void ppc_opc_vpkuhus(uint32 opc);
void ppc_opc_vpkshus(uint32 opc);
void ppc_opc_vpkuwus(uint32 opc);
void ppc_opc_vpkswus(uint32 opc);
void ppc_opc_vupkhsb(uint32 opc);
void ppc_opc_vupkhpx(uint32 opc);
void ppc_opc_vupkhsh(uint32 opc);
void ppc_opc_vupklsb(uint32 opc);
void ppc_opc_vupklpx(uint32 opc);
void ppc_opc_vupklsh(uint32 opc);
void ppc_opc_vaddubm(uint32 opc);
void ppc_opc_vadduhm(uint32 opc);
void ppc_opc_vadduwm(uint32 opc);
void ppc_opc_vaddfp(uint32 opc);
void ppc_opc_vaddcuw(uint32 opc);
void ppc_opc_vaddubs(uint32 opc);
void ppc_opc_vaddsbs(uint32 opc);
void ppc_opc_vadduhs(uint32 opc);
void ppc_opc_vaddshs(uint32 opc);
void ppc_opc_vadduws(uint32 opc);
void ppc_opc_vaddsws(uint32 opc);
void ppc_opc_vsububm(uint32 opc);
void ppc_opc_vsubuhm(uint32 opc);
void ppc_opc_vsubuwm(uint32 opc);
void ppc_opc_vsubfp(uint32 opc);
void ppc_opc_vsubcuw(uint32 opc);
void ppc_opc_vsububs(uint32 opc);
void ppc_opc_vsubsbs(uint32 opc);
void ppc_opc_vsubuhs(uint32 opc);
void ppc_opc_vsubshs(uint32 opc);
void ppc_opc_vsubuws(uint32 opc);
void ppc_opc_vsubsws(uint32 opc);
void ppc_opc_vmuleub(uint32 opc);
void ppc_opc_vmulesb(uint32 opc);
void ppc_opc_vmuleuh(uint32 opc);
void ppc_opc_vmulesh(uint32 opc);
void ppc_opc_vmuloub(uint32 opc);
void ppc_opc_vmulosb(uint32 opc);
void ppc_opc_vmulouh(uint32 opc);
void ppc_opc_vmulosh(uint32 opc);
void ppc_opc_vmaddfp(uint32 opc);
void ppc_opc_vmhaddshs(uint32 opc);
void ppc_opc_vmladduhm(uint32 opc);
void ppc_opc_vmhraddshs(uint32 opc);
void ppc_opc_vmsumubm(uint32 opc);
void ppc_opc_vmsumuhm(uint32 opc);
void ppc_opc_vmsummbm(uint32 opc);
void ppc_opc_vmsumshm(uint32 opc);
void ppc_opc_vmsumuhs(uint32 opc);
void ppc_opc_vmsumshs(uint32 opc);
void ppc_opc_vsum4ubs(uint32 opc);
void ppc_opc_vsum4sbs(uint32 opc);
void ppc_opc_vsum4shs(uint32 opc);
void ppc_opc_vsum2sws(uint32 opc);
void ppc_opc_vsumsws(uint32 opc);
void ppc_opc_vnmsubfp(uint32 opc);
void ppc_opc_vavgub(uint32 opc);
void ppc_opc_vavgsb(uint32 opc);
void ppc_opc_vavguh(uint32 opc);
void ppc_opc_vavgsh(uint32 opc);
void ppc_opc_vavguw(uint32 opc);
void ppc_opc_vavgsw(uint32 opc);
void ppc_opc_vmaxub(uint32 opc);
void ppc_opc_vmaxsb(uint32 opc);
void ppc_opc_vmaxuh(uint32 opc);
void ppc_opc_vmaxsh(uint32 opc);
void ppc_opc_vmaxuw(uint32 opc);
void ppc_opc_vmaxsw(uint32 opc);
void ppc_opc_vmaxfp(uint32 opc);
void ppc_opc_vminub(uint32 opc);
void ppc_opc_vminsb(uint32 opc);
void ppc_opc_vminuh(uint32 opc);
void ppc_opc_vminsh(uint32 opc);
void ppc_opc_vminuw(uint32 opc);
void ppc_opc_vminsw(uint32 opc);
void ppc_opc_vminfp(uint32 opc);
void ppc_opc_vrfin(uint32 opc);
void ppc_opc_vrfip(uint32 opc);
void ppc_opc_vrfim(uint32 opc);
void ppc_opc_vrfiz(uint32 opc);
void ppc_opc_vrefp(uint32 opc);
void ppc_opc_vrsqrtefp(uint32 opc);
void ppc_opc_vlogefp(uint32 opc);
void ppc_opc_vexptefp(uint32 opc);
void ppc_opc_vcfux(uint32 opc);
void ppc_opc_vcfsx(uint32 opc);
void ppc_opc_vctsxs(uint32 opc);
void ppc_opc_vctuxs(uint32 opc);
void ppc_opc_vand(uint32 opc);
void ppc_opc_vandc(uint32 opc);
void ppc_opc_vor(uint32 opc);
void ppc_opc_vnor(uint32 opc);
void ppc_opc_vxor(uint32 opc);
void ppc_opc_vcmpequbx(uint32 opc);
void ppc_opc_vcmpequhx(uint32 opc);
void ppc_opc_vcmpequwx(uint32 opc);
void ppc_opc_vcmpeqfpx(uint32 opc);
void ppc_opc_vcmpgtubx(uint32 opc);
void ppc_opc_vcmpgtsbx(uint32 opc);
void ppc_opc_vcmpgtuhx(uint32 opc);
void ppc_opc_vcmpgtshx(uint32 opc);
void ppc_opc_vcmpgtuwx(uint32 opc);
void ppc_opc_vcmpgtswx(uint32 opc);
void ppc_opc_vcmpgtfpx(uint32 opc);
void ppc_opc_vcmpgefpx(uint32 opc);
void ppc_opc_vcmpbfpx(uint32 opc);

#endif
