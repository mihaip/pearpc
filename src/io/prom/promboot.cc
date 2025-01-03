/*
 *	PearPC
 *	promboot.cc
 *
 *	Copyright (C) 2004 Stefan Weyergraf
 *	Copyright (C) 2004 Sebastian Biallas (sb@biallas.net)
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
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug/tracers.h"
#include "tools/debug.h"
#include "tools/endianess.h"
#include "cpu/cpu.h"
#include "cpu/mem.h"
#include "io/prom/fs/part.h"
#include "io/ide/ide.h"
#include "io/cuda/cuda.h"
#include "system/keyboard.h"
#include "system/display.h"
#include "system/sys.h"
#include "system/ui/gui.h"
#include "tools/debug.h"
#include "tools/except.h"
#include "tools/strtools.h"
#include "configparser.h"
#include "prom.h"
#include "promboot.h"
#include "promdt.h"
#include "prommem.h"

#define MSR_SF		(1<<31)
#define MSR_UNKNOWN	(1<<30)
#define MSR_UNKNOWN2	(1<<27)
#define MSR_VEC		(1<<25)
#define MSR_KEY		(1<<19)		// 603e
#define MSR_POW		(1<<18)
#define MSR_TGPR	(1<<15)		// 603(e)
#define MSR_ILE		(1<<16)
#define MSR_EE		(1<<15)
#define MSR_PR		(1<<14)
#define MSR_FP		(1<<13)
#define MSR_ME		(1<<12)
#define MSR_FE0		(1<<11)
#define MSR_SE		(1<<10)
#define MSR_BE		(1<<9)
#define MSR_FE1		(1<<8)
#define MSR_IP		(1<<6)
#define MSR_IR		(1<<5)
#define MSR_DR		(1<<4)
#define MSR_PM		(1<<2)
#define MSR_RI		(1<<1)
#define MSR_LE		(1)


byte ELF_PROGRAM_HEADER32_struct[]= {
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	0,
};

byte MACHO_HEADER_struct[]= {
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	0,
};

byte MACHO_COMMAND_struct[]= {
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	0,
};

byte MACHO_SEGMENT_COMMAND_struct[]= {
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_8 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	0,
};

byte MACHO_THREAD_COMMAND_struct[] = {
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	0,
};


struct MachOHeader {
	byte	magic[4];
	uint32	cputype;
	uint32	cpusubtype;
	uint32	filetype;
	uint32	ncmds;
	uint32	sizeofcmds;
	uint32	flags;
};

struct MachOCommand {
	uint32 cmd;			/* type of load command */
	uint32 cmdsize;			/* total size of command in bytes */
};


struct MachOSegmentCommand {
	byte	segname[16];	/* segment name */
	uint32	vmaddr;		/* memory address of this segment */
	uint32	vmsize;		/* memory size of this segment */
	uint32	fileoff;	/* file offset of this segment */
	uint32	filesize;	/* amount to map from the file */
	uint32	maxprot;	/* maximum VM protection */
	uint32	initprot;	/* initial VM protection */
	uint32	nsects;		/* number of sections in segment */
	uint32	flags;		/* flags */
};

struct MachOPPCThreadState {
	uint32	flavor;		/* flavor of thread state */
	uint32	count;		/* count of longs in thread state */

	uint32 srr0;		/* Instruction address register (PC) */
	uint32 srr1;		/* Machine state register (supervisor) */
	uint32 r[32];
	uint32 cr;		/* Condition register */
	uint32 xer;		/* User's integer exception register */
	uint32 lr;		/* Link register */
	uint32 ctr;		/* Count register */
	uint32 mq;		/* MQ register (601 only) */

	uint32 vrsave;		/* Vector Save Register */
};

struct COFF_HEADER {
	uint16 machine PACKED;
	uint16 section_count PACKED;
	uint32 timestamp PACKED;
	uint32 symbol_table_offset PACKED;
	uint32 symbol_count PACKED;
	uint16 optional_header_size PACKED;
	uint16 characteristics PACKED;
};

byte COFF_HEADER_struct[] = {
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,
	0
};

#define COFF_SIZEOF_SHORT_NAME			8

struct COFF_SECTION_HEADER {
	byte name[COFF_SIZEOF_SHORT_NAME];
	uint32 data_vsize;	// or data_phys_address !
	uint32 data_address;
	uint32 data_size;
	uint32 data_offset;
	uint32 relocation_offset;
	uint32 linenumber_offset;
	uint16 relocation_count;
	uint16 linenumber_count;
	uint32 characteristics;
} PACKED;

byte COFF_SECTION_HEADER_struct[] = {
	COFF_SIZEOF_SHORT_NAME,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,
	0
};

#define ELF_SIZEOF_IDENT			16	

static byte ELF_HEADER_struct[] = {
	ELF_SIZEOF_IDENT,
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,	// e_type
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,	// e_machine
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// e_version
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// e_entry
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// e_phoff
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// e_shoff
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// e_flags
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,	// e_ehsize
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,	// e_phentsize
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,	// e_phnum
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,	// e_shentsize
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,	// e_shnum
	STRUCT_ENDIAN_16 | STRUCT_ENDIAN_HOST,	// e_shstrndx
	0
};

static byte ELF_PHEADER_struct[] = {
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// p_type
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// p_offset
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// p_vaddr
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// p_paddr
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// p_filesz
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// p_memsz
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// p_flags
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// p_align
	0
};

static byte ELF_NHEADER_struct[] = {
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// n_namesz
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// n_descsz
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// n_type
	0
};

struct ELF_HEADER32 {
	uint8  e_ident[ELF_SIZEOF_IDENT];
	uint16 e_type;
	uint16 e_machine;
	uint32 e_version;
	uint32 e_entry;
	uint32 e_phoff;
	uint32 e_shoff;
	uint32 e_flags;
	uint16 e_ehsize;
	uint16 e_phentsize;
	uint16 e_phnum;
	uint16 e_shentsize;
	uint16 e_shnum;
	uint16 e_shstrndx;
};

struct ELF_PROGRAM_HEADER32 {
	uint32 p_type;
	uint32 p_offset;
	uint32 p_vaddr;
	uint32 p_paddr;
	uint32 p_filesz;
	uint32 p_memsz;
	uint32 p_flags;
	uint32 p_align;
};

struct ELF_NHEADER32 {
	uint32 n_descsz;
	uint32 n_namesz;
	uint32 n_type;
};

struct CHRP_desc {
	uint32 real_mode;
	uint32 real_base;
	uint32 real_size;
	uint32 virt_base;
	uint32 virt_size;
	uint32 load_base;
};

static byte ELF_DESC_struct[] = {
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// real_mode
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// real_base
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// real_size
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// virt_base
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// virt_size
	STRUCT_ENDIAN_32 | STRUCT_ENDIAN_HOST,	// load_base
	0
};

static bool real_mode = false;

/*
 *	helpers
 */
static bool init_page_create(uint32 ea, uint32 pa)
{
	return prom_claim_page(pa) && ppc_prom_page_create(ea, pa);
}

static bool chrp_init_page_create(uint32 ea, uint32 pa, int no_claim)
{
	bool ret = true;

	if (!no_claim)
		ret = prom_claim_page(pa);

	if (real_mode)
		return ret;
	else
		return ret && ppc_prom_page_create(ea, pa);
}

/*
 *	ELF
 */
//#define ELF_LOAD_ADDRESS	0x20000
#define ELF_LOAD_ADDRESS	0x800000
#define ROM_BASE		0x40800000
#define MAX_ELF_HEADERS		32
#define PPC_PAGE_SIZE		4096
#define PPC_PAGE_MASK		0x00000fff

#define PROMBOOT_OUTPUT(...)	do { \
		gDisplay->printf(__VA_ARGS__);	\
		ht_printf(__VA_ARGS__); \
	} while(0)

#define PLOAD_NOCOPY	(1 << 0)
#define PLOAD_PADDR_MAP	(1 << 1)

bool promboot_copy_file_to_memory(File &f, uint32 &paddr, uint32 &vaddr, sint32 loader_size, sint32 zero_size, int flags)
{
	byte page[PPC_PAGE_SIZE];

	bool no_copy = flags & PLOAD_NOCOPY;
	bool paddr_map = flags & PLOAD_PADDR_MAP;

	bool copy = !no_copy;
	if (paddr_map) {
		copy = true;
	}
	printf("promboot_copy_file_to_memory(%08x, %08x, %08x, %08x)\n", paddr, vaddr, loader_size, zero_size);
	while (loader_size > 0) {
		sint32 size = loader_size;
		if (size > PPC_PAGE_SIZE)
			size = PPC_PAGE_SIZE;

/*		if (paddr_map && !chrp_init_page_create(vaddr, paddr, false)) {
			PROMBOOT_OUTPUT("PROM ERROR: Unable to create load page!\n");
			return false;
		}*/

		if (!chrp_init_page_create(vaddr, paddr, no_copy)) {
			PROMBOOT_OUTPUT("LOADER WARNING: data overwrite!\n");
			return false;
		}

		if (copy) {
			f.readx(page, size);

			if (size < PPC_PAGE_SIZE && zero_size) {
				sint32 offset = size;

				size += zero_size;
				if (size > PPC_PAGE_SIZE) {
					size = PPC_PAGE_SIZE;
				}

				memset(page+offset, 0, size-offset);
			}

			if (!ppc_dma_write(paddr, page, size)) {
				PROMBOOT_OUTPUT("LOADER ERROR: Unable to write page!\n");
				return false;
			}
		}

		paddr += PPC_PAGE_SIZE;
		vaddr += PPC_PAGE_SIZE;
		loader_size -= size;
	}

	if (zero_size > 0) {
		loader_size += zero_size;

		if (copy) memset(page, 0, PPC_PAGE_SIZE);

		while(loader_size > 0) {
			sint32 size = loader_size;
			if (size > PPC_PAGE_SIZE)
				size = PPC_PAGE_SIZE;

			if (!chrp_init_page_create(vaddr, paddr, no_copy)) {
				PROMBOOT_OUTPUT("LOADER WARNING: zero-page overwrite!\n");
				return false;
			}
			if (copy) {
				if (!ppc_dma_write(paddr, page, size)) {
					PROMBOOT_OUTPUT("LOADER ERROR: Unable to write page!\n");
					return false;
				}
			}
			paddr += PPC_PAGE_SIZE;
			vaddr += PPC_PAGE_SIZE;
			loader_size -= size;
		}
	}
}

extern uint32 gMemorySize;

/* Based on those standards from:
 * PowerPC Microprocessor Common Hardware Reference Platform (CHRP)
 *   System binding to: IEEE Std 1275-1994
 *   Standard for Boot (Initialization, Configuration)
 *   Firmware
 *   Revision: 1.8 [Approved Version]
 *   Date: Feburary 2, 1998
 */
bool mapped_load_elf_from_chrp(File &f, uint disp_off)
{
	String fn;
	PROMBOOT_OUTPUT("ELF: trying to load '%y'\n", &f.getDesc(fn));
	try {
		ELF_HEADER32 hdr;
		ELF_PROGRAM_HEADER32 *phdr;
		ELF_NHEADER32 nhdr;
		uint32 stack = 0;
		uint32 stackp = 0;

//		for (int i=0; i < gMemorySize - PROM_MEM_SIZE; i+=4096)
//			ppc_prom_page_create(i, i);

		f.seek(disp_off);
		f.readx(&hdr, sizeof hdr);
		createHostStructx(&hdr, sizeof hdr, ELF_HEADER_struct, big_endian);

		if (hdr.e_ident[0] != 0x7f
		 || hdr.e_ident[1] != 'E'
		 || hdr.e_ident[2] != 'L'
		 || hdr.e_ident[3] != 'F') {
			PROMBOOT_OUTPUT("ELF ERROR: bad magic number.\n");
			return false;
		}

		if (hdr.e_ident[4] != 1) {
			PROMBOOT_OUTPUT("ELF ERROR: non-32-bit ELF.\n");
			return false;
		}

		if (hdr.e_ident[5] != 2) {
			PROMBOOT_OUTPUT("ELF ERROR: non-big-endian ELF.\n");
			return false;
		}

		if (hdr.e_type != 2) {
			PROMBOOT_OUTPUT("ELF ERROR: non-executable ELF.\n");
			return false;
		}
		if (hdr.e_machine != 20) {
			PROMBOOT_OUTPUT("ELF ERROR: non-PowerPC ELF.\n");
			return false;
		}
		if (hdr.e_version != 1) {
			PROMBOOT_OUTPUT("ELF ERROR: bad ELF version.\n");
			return false;
		}
		if (hdr.e_flags != 0) {
			PROMBOOT_OUTPUT("ELF ERROR: non-empty flag field.\n");
			return false;
		}

		/* Non-specification limit */
		if (hdr.e_phnum > MAX_ELF_HEADERS) {
			PROMBOOT_OUTPUT("ELF ERROR: too many program headers.\n");
			return false;
		}

		phdr = new ELF_PROGRAM_HEADER32[hdr.e_phnum];
		f.seek(disp_off + hdr.e_phoff);
		f.readx(phdr, sizeof(ELF_PROGRAM_HEADER32) * hdr.e_phnum);
		for (int i=0; i < hdr.e_phnum; i++) {
			createHostStructx(&(phdr[i]), sizeof(phdr[i]), ELF_PHEADER_struct, big_endian);
		}

		uint32 load_base = ELF_LOAD_ADDRESS;
		uint32 real_base = 0x00000000;
		uint32 real_size = 0xffffffff;
		uint32 virt_base = 0x00000000;
		uint32 virt_size = 0xffffffff;

		for (int i=0; i < hdr.e_phnum; i++) {
			if (phdr[i].p_type == 4) { // PT_NOTE
				f.seek(disp_off + phdr[i].p_offset);
				f.readx(&nhdr, sizeof nhdr);
				
				createHostStructx(&nhdr, sizeof nhdr, ELF_NHEADER_struct, big_endian);

				if (nhdr.n_type != 0x1275) {
					PROMBOOT_OUTPUT("ELF WARNING: unusual PT_NOTE section: %04x\n", nhdr.n_type);
					continue;
				}

				char *name;
				name = (char *)malloc(nhdr.n_namesz);
				f.readx(name, nhdr.n_namesz);
				PROMBOOT_OUTPUT("ELF PT_NOTE: Name '%s'\n", name);
				free(name);

				CHRP_desc *desc;
				desc = (CHRP_desc *)malloc(nhdr.n_descsz);
				f.readx(desc, nhdr.n_descsz);
				createHostStructx(desc, sizeof(CHRP_desc), ELF_DESC_struct, big_endian);

				real_mode = desc->real_mode;

				if (desc->real_base != -1)
					real_base = desc->real_base;

				if (desc->real_size != -1)
					real_size = desc->real_size;

				if (desc->virt_base != -1)
					virt_base = desc->virt_base;

				if (desc->virt_size != -1)
					virt_size = desc->virt_size;

				if (desc->load_base != -1)
					load_base = desc->load_base;

				free(desc);
				break;
			}
		}

		PROMBOOT_OUTPUT("ELF: real-mode: %s\n", real_mode ? "true" : "false");
		PROMBOOT_OUTPUT("ELF: real-base: %08x\n", real_base);
		PROMBOOT_OUTPUT("ELF: real-size: %08x\n", real_size);
		PROMBOOT_OUTPUT("ELF: virt-base: %08x\n", virt_base);
		PROMBOOT_OUTPUT("ELF: virt-size: %08x\n", virt_size);
		PROMBOOT_OUTPUT("ELF: load-base: %08x\n", load_base);

		if (real_mode) {
			PROMBOOT_OUTPUT("ELF ERROR: real-mode bootup not supported.\n");
			return false;
		}

		sint32 client_size = f.getSize() - disp_off;

		uint32 paddr = load_base;
		byte page[PPC_PAGE_SIZE];

		f.seek(disp_off);
//		promboot_copy_file_to_memory(f, paddr, vaddr, client_size, 0, PLOAD_PADDR_MAP);

		for (int i=0; i < hdr.e_phnum; i++) {
			if (phdr[i].p_type != 1) // PT_LOAD
				continue;

			uint32 vaddr = phdr[i].p_vaddr;

			sint32 loader_size = phdr[i].p_filesz;
			if (loader_size > phdr[i].p_memsz) {
				loader_size = phdr[i].p_memsz;
			}

			sint32 zero_size = 0;
			if (phdr[i].p_memsz > phdr[i].p_filesz)
				zero_size = phdr[i].p_memsz - phdr[i].p_filesz;

			f.seek(disp_off+phdr[i].p_offset);

			promboot_copy_file_to_memory(f, paddr, vaddr, loader_size, zero_size, 0);
		}

		stack = gMemorySize - 21*4096 - PROM_MEM_SIZE;
		stackp = gMemorySize - 21*4096 - PROM_MEM_SIZE;
		//stack = (stack + PPC_PAGE_MASK) & ~PPC_PAGE_MASK;
		//stackp = (stackp + PPC_PAGE_MASK) & ~PPC_PAGE_MASK;

		for (int i=0; i<20; i++) {
			if (!init_page_create(stack, stackp)) {
				PROMBOOT_OUTPUT("ELF ERROR: Unable to create stack page %i!\n", i);
				return false;
			}

			stack += PPC_PAGE_SIZE;
			stackp += PPC_PAGE_SIZE;
		}

		delete[] phdr;

		ppc_cpu_set_pc(0, hdr.e_entry);

		if (real_mode) {
			ppc_cpu_set_msr(0, MSR_FP);
			ppc_cpu_set_gpr(0, 1, stackp-(4096+32));
		} else {	// turn on address translation
			ppc_cpu_set_msr(0, MSR_IR | MSR_DR | MSR_FP);
			ppc_cpu_set_gpr(0, 1, stack-(4096+32));
		}

		ppc_cpu_set_gpr(0, 2, 0);
		ppc_cpu_set_gpr(0, 3, 0);
		ppc_cpu_set_gpr(0, 4, 0);
		ppc_cpu_set_gpr(0, 5, gPromOSIEntry);

		PROMBOOT_OUTPUT("Booting ELF...\n");

		ppc_prom_page_create(0, 0);
		return true;
	} catch (...) {
		PROMBOOT_OUTPUT("ELF ERROR: caught exception.\n");
		return false;
	}
}

bool mapped_load_elf(File &f)
{
	String fn;
	gDisplay->printf("ELF: trying to load '%y'\n", &f.getDesc(fn));
	IO_PROM_TRACE("ELF: trying to load '%y'\n", &fn);
	try {
		// FIXME: better code
		byte magic[4];
		f.seek(0);
		f.readx(magic, 4);
		if ((magic[0] != 0x7f) || (magic[1] != 'E') 
		|| (magic[2] != 'L') ||(magic[3] != 'F')) {
			IO_PROM_TRACE("no ELF\n");
			return false;
		}

		uint32 la = ELF_LOAD_ADDRESS;
		f.seek(0x2c);
		uint16 program_hdrs;
		f.readx(&program_hdrs, 2);
		program_hdrs = createHostInt(&program_hdrs, 2, big_endian);
		uint32 stack=0;
		uint32 stackp=0;
		for (int i=0; i<program_hdrs; i++) {
//			IO_PROM_TRACE("program header %d:\n", i);
			ELF_PROGRAM_HEADER32 program_hdr;
			f.seek(0x34+i*sizeof program_hdr);
			f.readx(&program_hdr, sizeof program_hdr);
			createHostStructx(&program_hdr, sizeof program_hdr, ELF_PROGRAM_HEADER32_struct, big_endian);
			uint32 pages_from_file = program_hdr.p_filesz / 4096;
			uint32 ea = program_hdr.p_vaddr;
			sint32 vs = program_hdr.p_memsz;
			f.seek(program_hdr.p_offset);
			uint32 fo = program_hdr.p_offset;
			byte page[4096];
			while (pages_from_file) {
//				IO_PROM_TRACE("loading from %08x to ea:%08x (pa:%08x)\n", fo, ea, la);
				f.readx(page, sizeof page);
		    		if (!init_page_create(ea, la)) return false;
				if (!ppc_dma_write(la, page, 4096)) {
					return false;
				}
				pages_from_file--;
				la += 4096;
	        		ea += 4096;
				fo += 4096;
				vs -= 4096;
			}
			if (program_hdr.p_filesz % 4096) {
//		  		ht_printf("loading remaining from %08x to ea:%08x (pa:%08x)\n", fo, ea, la);
				f.read(page, program_hdr.p_filesz % 4096);
				if (!init_page_create(ea, la)) return false;
				if (!ppc_dma_write(la+(ea&0xfff), page, program_hdr.p_filesz % 4096)) {
					return false;
				}
				la += 4096;
				ea += 4096;
				fo += 4096;
				vs -= 4096;
			}
			while (vs > 0) {
//  				ht_printf("creating for ea:%08x (pa:%08x)\n", ea, la);
				if (!init_page_create(ea, la)) return false;
				la += 4096;
				ea += 4096;
				vs -= 4096;
			}
			stack = ea;
			stackp = la;
		}

		stack = gMemorySize - 65536 - (4096*20) - PROM_MEM_SIZE;
		stackp = gMemorySize - 65536 - (4096*20) - PROM_MEM_SIZE;
		// allocate stack
		for (int i=0; i<20; i++) {
			if (!init_page_create(stack, stackp)) return false;
			stack += 4096;
			stackp += 4096;
		}
	
	
		f.seek(0x18);
		uint32 entry;
		f.readx(&entry, 4);
		entry = createHostInt(&entry, 4, big_endian);	

		// turn on address translation
		ppc_cpu_set_msr(0, MSR_IR | MSR_DR | MSR_FP);
		ppc_cpu_set_pc(0, entry);
		
		ppc_cpu_set_gpr(0, 1, gMemorySize - 65536 - 32 - PROM_MEM_SIZE);
		ppc_cpu_set_gpr(0, 2, 0);
		ppc_cpu_set_gpr(0, 3, 0);
		ppc_cpu_set_gpr(0, 4, 0);
		ppc_cpu_set_gpr(0, 5, gPromOSIEntry);
	
		// wtf?!
		ppc_prom_page_create(0, 0);
		return true;
	} catch (...) {
		return false;
	}
}

/*
 *	XCOFF
 */
bool mapped_load_xcoff(File &f, uint disp_ofs)
{
	String fn;
	gDisplay->printf("XCOFF trying to load '%y'\n", &f.getDesc(fn));
	IO_PROM_TRACE("XCOFF trying to load '%y'\n", &fn);

	f.seek(disp_ofs);
	try {
		COFF_HEADER hdr;
		f.readx(&hdr, sizeof hdr);
		createHostStructx(&hdr, sizeof hdr, COFF_HEADER_struct, big_endian);
		if (hdr.machine != 0x1df) {
			IO_PROM_TRACE("invalid machine %04x (expecting 01df)\n", hdr.machine);
			return false;
		}
		if (hdr.optional_header_size != 0x48) {
			IO_PROM_TRACE("invalid optional header size %04x (expecting 0048)\n", hdr.optional_header_size);
			return false;
		}
		uint32 stack=0;
		uint32 stackp=0;
		uint32 entrypoint;
		uint32 entrypoint_ofs = 0;
		f.seek(disp_ofs+0x24);
		f.readx(&entrypoint, 4);
		entrypoint = createHostInt(&entrypoint, 4, big_endian);

		for (int i=0; i<hdr.section_count; i++) {
//			ht_printf("program header %d:\n", i);
			COFF_SECTION_HEADER shdr;
			f.seek(disp_ofs+0x5c+i*sizeof shdr);
			f.readx(&shdr, sizeof shdr);
			createHostStructx(&shdr, sizeof shdr, COFF_SECTION_HEADER_struct, big_endian);
			if ((entrypoint >= shdr.data_address) && (entrypoint < shdr.data_address+shdr.data_size)) {
				IO_PROM_TRACE("found entrypoint-structure in section %d\n", i);
				entrypoint_ofs = entrypoint - shdr.data_address + shdr.data_offset;
			}
			uint32 in_file_size = (shdr.data_offset) ? shdr.data_size : 0;
			uint32 pages_from_file = in_file_size / 4096;
			uint32 pa = shdr.data_vsize; /* really: physical addr */
			uint32 ea = shdr.data_address;
			sint32 vs = shdr.data_size;
			f.seek(disp_ofs+shdr.data_offset);
			uint32 fo = shdr.data_offset;
			byte page[4096];
			while (pages_from_file) {
//				ht_printf("loading from %08x to ea:%08x (pa:%08x)\n", fo, ea, pa);
				f.readx(page, sizeof page);
		    		if (!init_page_create(ea, pa)) return false;
				if (!ppc_dma_write(pa, page, 4096)) {
					return false;
				}
				pages_from_file--;
				pa += 4096;
	        		ea += 4096;
				fo += 4096;
				vs -= 4096;
			}
			if (in_file_size % 4096) {
//				ht_printf("loading remaining from %08x to ea:%08x (pa:%08x)\n", fo, ea, pa);
				f.readx(page, in_file_size % 4096);
				if (!init_page_create(ea, pa)) return false;
				if (!ppc_dma_write(pa+(ea&0xfff), page, in_file_size % 4096)) {
					return false;
				}
				pa += 4096;
				ea += 4096;
				fo += 4096;
				vs -= 4096;
			}
			while (vs > 0) {
//				ht_printf("creating for ea:%08x (pa:%08x)\n", ea, pa);
				if (!init_page_create(ea, pa)) return false;
				pa += 4096;
				ea += 4096;
				vs -= 4096;
			}
			stack = ea;
			stackp = pa;
		}
		if (!entrypoint_ofs) {
			IO_PROM_TRACE("couldn't find entrypoint offset\n");
			return false;
		}
		// allocate stack
		for (int i=0; i<20; i++) {
			if (!init_page_create(stack, stackp)) return false;
			stack += 4096;
			stackp += 4096;
		}

		uint32 real_entrypoint = 0;
		f.seek(disp_ofs+entrypoint_ofs);
		f.read(&real_entrypoint, 4);
		real_entrypoint = createHostInt(&real_entrypoint, 4, big_endian);
		IO_PROM_TRACE("real_entrypoint = %08x\n", real_entrypoint);
		// turn on address translation
		ppc_cpu_set_msr(0, MSR_IR | MSR_DR | MSR_FP);
		ppc_cpu_set_pc(0, real_entrypoint);
		
		ppc_cpu_set_gpr(0, 1, stack-(4096+32));
		ppc_cpu_set_gpr(0, 2, 0);
		ppc_cpu_set_gpr(0, 3, 0);
		ppc_cpu_set_gpr(0, 4, 0);
		ppc_cpu_set_gpr(0, 5, gPromOSIEntry);

		return true;
	} catch (...) {
		return false;
	}
}

static void chrpReadWaitForChar(File &f, char *buf, uint buflen, char waitFor)
{
	while (buflen) {
		f.readx(buf, 1);

		if (*buf == 0x0d) *buf = '\n';

		if (!*buf) throw new MsgException("Binary files not allowed here");
		if (*buf == waitFor) {
			*buf = 0;
			return;
		}
		buf++;
		buflen--;
	}
	throw new MsgException("wait for char failed");
}

static void chrpReadWaitForString(File &f, char *buf, uint buflen, char *waitFor)
{
	*buf = 0;
	uint zlen = strlen(waitFor);
	if (!zlen) return;
	char *z = strdup(waitFor);
	*z = 0;
	FileOfs o = f.tell();
	while (buflen) {
		f.seek(o);
		f.readx(z, zlen);
		if (strcmp(z, waitFor) == 0) {
			*buf = 0;
			free(z);
			return;
		}
		if (buf != NULL) {
			*buf = (*z == 0xd) ? '\n' : *z;
			buf++;
			buflen--;
		}
		o++;
	}
	free(z);
	throw new MsgException("wait for string failed");
}

#if 0
static void readCHRPIcon(byte *icon, char *&t, uint width, uint height)
{
	memset(icon, 0, width*height);
	uint y = 0;
	uint x = 0;
	while (t[0] && t[1]) {
		if (t[0] == '\n') {
			while (*t == '\n') t++;
//			fprintf(stderr, "parse icon: line %d length %d\n", y, x);
			x = 0;
			y++;
			if (y >= height) break;
			continue;
		}
		if (x>=width) break;
		hexb_ex(icon[x+y*width], t);
		t += 2;
		x++;
	}
}
#endif

static bool chrpBoot(const char *bootpath, const char *bootargs)
{
	IO_PROM_TRACE("CHRP boot file (bootpath = %s, bootargs = %s).\n", bootpath, bootargs);
	// set bootpath in device tree
	PromNode *chosen = findDevice("/chosen", FIND_DEVICE_FIND, NULL);
	if (chosen) {
		PromProp *bp = chosen->findProp("bootpath");
		if (bp) {
			PromPropString *s = dynamic_cast<PromPropString*>(bp);
			if (s) {
				free(s->value);
				s->value = strdup(bootpath);
			}
		} else {
			chosen->addProp(new PromPropString("bootpath", bootpath));
		}
		PromProp *ba = chosen->findProp("bootargs");
		if (ba) {
			PromPropString *s = dynamic_cast<PromPropString*>(ba);
			if (s) {
				free(s->value);
				s->value = strdup(bootargs);
			}
		} else {
			chosen->addProp(new PromPropString("bootargs", bootargs));
		}
	}

	PromInstanceHandle ih;
	if (!findDevice(bootpath, FIND_DEVICE_OPEN, &ih)) return false;

	PromInstanceDiskFile *ix = dynamic_cast<PromInstanceDiskFile*>(handleToInstance(ih));
	if (!ix || !ix->mFile) {
		IO_PROM_TRACE("couldn't load CHRP boot file (1) (bootpath = %s).\n", bootpath);
		return false;
	}

	PromNode *pn = ix->getType();
	File *f = ix->mFile;

	if (!mapped_load_elf(*f)
	&&  !mapped_load_xcoff(*f, 0)
	&&  !mapped_load_chrp(*f)) {
		IO_PROM_TRACE("couldn't load CHRP boot file (2) (bootpath = %s).\n", bootpath);
		pn->close(ih);
		return false;
	}
	pn->close(ih);
	return true;
}

bool mapped_load_chrp(File &f)
{
	String fn;
	gDisplay->printf("CHRP: trying to load '%y'\n", &f.getDesc(fn));
	IO_PROM_TRACE("CHRP: trying to load '%y'\n", &fn);
	try {
		char hdr[13];
		f.seek(0);
		if (f.read(hdr, 12) != 12) return false;
		hdr[12] = 0;
		IO_PROM_TRACE("header: %s\n", hdr);
		if (strncmp(hdr, "<CHRP-BOOT>", 11)) return false;
		IO_PROM_TRACE("CHRP-BOOT OK\n");
		char buf[32*1024];
		uint buflen;
		char tag[32*1024];
		char expect[4*1024+1];	// sizeof buf+1
		while (1) {
			chrpReadWaitForChar(f, buf, sizeof buf, '\n');
			buflen = strlen(buf);
			IO_PROM_TRACE("read %d bytes: %s\n", buflen, buf);
			if (buflen < 1)
				continue;
			if ((buf[0] != '<') || (buflen<3) || (buf[buflen-1] != '>')) return false;
			if (buf[1] == '/') {
				if (memcmp(buf, "</CHRP-BOOT>", buflen-1) != 0)
					return false;
				byte b;
				FileOfs from = f.tell();
		    		while (1) {
				    	if (!f.read(&b, 1)) return false;
		    			if (b == 0x01) break;
		    		}
				if (b != 0x01) return false;
				if (!f.read(&b, 1)) return false;
				if (b == 0xdf) {
					PROMBOOT_OUTPUT("Loading XCOFF...\n");
					return mapped_load_xcoff(f, f.tell()-2);
				} else if (b == 0x02) {
					f.seek(from);
					while (1) {
						if (!f.read(&b, 1))
							return false;
						if (b == 0x7f) break;
					}

					PROMBOOT_OUTPUT("Loading ELF...\n");
					return mapped_load_elf_from_chrp(f, f.tell()-1);
				} else
					return false;
			}
			expect[0] = '<';
			expect[1] = '/';
			int index;
			for (index=0; index<buflen-1; index++) {
				if (buf[index+1] == ' ') break;
				if (buf[index+1] == '>') break;
				expect[index+2] = buf[index+1];
			}
			//memcpy(expect+2, buf+1, buflen-1);
			//expect[buflen+1] = '\n';
			expect[index+2] = '>';
			expect[index+3] = 0;
			strcpy(tag, buf);
			IO_PROM_TRACE("waitforstring: %s\n", expect);
			chrpReadWaitForString(f, buf, sizeof buf, expect);
			IO_PROM_TRACE("found: %s\n", buf);
			if (strcmp(tag, "<BOOT-SCRIPT>") == 0) {
				char *bootpath = strstr(buf, "boot ");
				if (bootpath) {
					bootpath += 5;
					char *bootpathend = strchr(bootpath, '\n');
					if (!bootpathend) bootpathend = bootpath + strlen(bootpath);
					char mybootpath[1024];
					char *mybootargs = NULL;
					strncpy(mybootpath, bootpath, sizeof mybootpath-1);
					mybootpath[bootpathend-bootpath] = 0;
					mybootpath[sizeof mybootpath-1] = 0;
					mybootargs = mybootpath+ sizeof mybootpath-1;
					int l = strlen(mybootpath);
					for (int i=0; i<l; i++) {
						if (mybootpath[i] == '\n') {
							mybootpath[i] = 0;
							break;
						} else 	if (mybootpath[i] == ' ') {
							mybootpath[i] = 0;
							mybootargs = mybootpath+i+1;
							break;
						}
					}
					return chrpBoot(mybootpath, mybootargs);
				}
			}
#if 0
			if (strcmp(tag, "<OS-BADGE-ICONS>") == 0) {
				char *t = buf;
				uint width = 1000, height = 1000;
				char fmt[64];
				while (*t && *t != '\n') t++;
				if (*t == '\n') t++;
				if (t-buf < (int)sizeof buf) {
					strncpy(fmt, buf, t-buf);
					fmt[t-buf] = 0;
				} else {
					fmt[0] = 0;
				}
//				fprintf(stderr, "format string: %s\n", fmt);
				if (strlen(fmt) == 4) {
					uint8 x;
					if (hexb_ex(x, fmt)) width = x;
					if (hexb_ex(x, fmt+2)) height = x;
				} else if (strlen(fmt) == 8) {
					uint16 x;
					if (hexw_ex(x, fmt)) width = x;
					if (hexw_ex(x, fmt+4)) height = x;
				}
				if ((width > 128) || (height > 128)) {
					// safety
					width = 16;
					height = 16;
				}
				byte icon0[width*height];
				byte icon1[width*height];
				byte icon2[width*height];
				readCHRPIcon(icon0, t, width, height);
				readCHRPIcon(icon1, t, width, height);
				readCHRPIcon(icon2, t, width, height);
				uint8 colors2_r[] = {0xff, 0xaa, 0x55, 0x00};
				uint8 colors3_r[] = {0xff, 0xdb, 0xb7, 0x92, 0x6e, 0x49, 0x25, 0x00};
				uint8 colors2[] = {0x00, 0x55, 0xaa, 0xff};
				uint8 colors3[] = {0x00, 0x25, 0x49, 0x6e, 0x92, 0xb7, 0xdb, 0xff};
				for (uint y=0; y < height; y++) {
					for (uint x=0; x < height; x++) {
						uint8 r = colors3_r[(icon0[x+y*width]>>5) & 0x7];
						uint8 g = colors3_r[(icon0[x+y*width]>>2) & 0x7];
						uint8 b = colors2_r[(icon0[x+y*width]>>0) & 0x3];
						uint src = icon0[x+y*width];
						r = (src>>5) & 0x7;
						g = (src>>2) & 0x7;
						b = (src>>0) & 0x3;
						r = colors3_r[r];
						g = colors3_r[g];
						b = colors2_r[b];
						RGBA rgba = MK_RGBA(r, g, b, icon2[x+y*width]);
						gDisplay->putPixelRGBA(20+2*x+0, 20+2*y+0, rgba);
						gDisplay->putPixelRGBA(20+2*x+0, 20+2*y+1, rgba);
						gDisplay->putPixelRGBA(20+2*x+1, 20+2*y+0, rgba);
						gDisplay->putPixelRGBA(20+2*x+1, 20+2*y+1, rgba);
						r = ~icon1[x+y*width];
						g = ~icon1[x+y*width];
						b = ~icon1[x+y*width];
						rgba = MK_RGBA(r, g, b, icon2[x+y*width]);
						gDisplay->putPixelRGBA(60+x, 20+y, rgba);
					}
				}
//				while (1) ;
			}
#endif
		}
		return false;
	} catch (Exception *x) {
		String s;
		IO_PROM_TRACE("mapped_load_chrp: exception: %y\n", &x->reason(s));
		return false;
	} catch (...) {
		return false;
	}
}

/*
 *	brute force loading
 */
bool mapped_load_flat(const char *filename, uint fileofs, uint filesize, uint vaddr, uint pc)
{
	gDisplay->printf("FLAT loading: %s\n", filename);

	uint32 pa;
	uint32 ea;
	// allocate image
	pa = vaddr;
	ea = vaddr;
	
	uint32 loadaddr = pa;
	
	for (uint p = 0; p<(filesize+4095) / 4096; p++) {
		if (!init_page_create(ea, pa)) return false;
		ea += 4096;
		pa += 4096;
	}

 	// allocate stack
	int stackea = 44*1024*1024;
	int stacksize = 40*4096;
	ea = stackea;
	pa = stackea;
	for (int i=0; i<(stacksize+4095) / 4096; i++) {
		if (!init_page_create(ea, pa)) return false;
		ea += 4096;
		pa += 4096;
	}

	// allocate bss
	int bssea = 0x1c25000;
	int bsssize = 0x9c000;
	pa = bssea;
	ea = bssea;
	for (int i=0; i<(bsssize+4095) / 4096; i++) {
		if (!init_page_create(ea, pa)) return false;
		ea += 4096;
		pa += 4096;
	}

	FILE *f;
	if (!(f = fopen(filename, "rb"))) {
		return false;
	}

	fseek(f, fileofs, SEEK_SET);

	byte *p = (byte *)malloc(filesize);
	if (!p) return false;
	
	if (fread(p, filesize, 1, f) != 1) {
		free(p);
		return false;
	}

	ppc_dma_write(loadaddr, p, filesize);
	
	free(p);

	ppc_cpu_set_msr(0, MSR_IR | MSR_DR | MSR_FP);
	ppc_cpu_set_pc(0, pc);
		
	ppc_cpu_set_gpr(0, 1, stackea+stacksize/2);
	ppc_cpu_set_gpr(0, 2, 0);
	ppc_cpu_set_gpr(0, 3, 0x47110815);
	ppc_cpu_set_gpr(0, 4, 0);
	ppc_cpu_set_gpr(0, 5, gPromOSIEntry);
	return true;
}

bool mapped_load_direct(File &f, uint vaddr, uint pc)
{
	String fn;
	gDisplay->printf("direct: trying to load '%y'\n", &f.getDesc(fn));
	IO_PROM_TRACE("direct: trying to load '%y'\n", &fn);

	uint32 pa;
	uint32 ea;
	// allocate image
	pa = vaddr;
	ea = vaddr;

	uint32 loadaddr = pa;
	
	uint size = f.getSize();
	for (uint p = 0; p<(size+4095) / 4096; p++) {
		if (!init_page_create(ea, pa)) return false;
		ea += 4096;
		pa += 4096;
	}

 	// allocate stack
	int stackea = (vaddr+size + 4096) & 0xfffff000;
	int stacksize = 40*4096;
	ea = stackea;
	pa = stackea;
	for (int i=0; i<(stacksize+4095) / 4096; i++) {
		if (!init_page_create(ea, pa)) return false;
		ea += 4096;
		pa += 4096;
	}

	// allocate bss
/*	int bssea = 0x1c25000;
	int bsssize = 0x9c000;
	pa = bssea;
	ea = bssea;
	for (int i=0; i<(bsssize+4095) / 4096; i++) {
		if (!init_page_create(ea, pa)) return false;
		ea += 4096;
		pa += 4096;
	}*/

//	memcpy(pt, mem, size);
	f.seek(0);

	byte *p = (byte *)malloc(f.getSize());
	if (!p) return false;
	
	if (f.read(p, f.getSize()) != f.getSize()) {
		free(p);
		return false;
	}

	ppc_dma_write(loadaddr, p, f.getSize());
	
	free(p);

	ppc_cpu_set_msr(0, MSR_IR | MSR_DR | MSR_FP);
	ppc_cpu_set_pc(0, pc);
		
	ppc_cpu_set_gpr(0, 1, stackea+stacksize/2);
	ppc_cpu_set_gpr(0, 2, 0);
	ppc_cpu_set_gpr(0, 3, 0x47110815);
	ppc_cpu_set_gpr(0, 4, 0);
	ppc_cpu_set_gpr(0, 5, gPromOSIEntry);
	return true;
}

/*
 *	Mach-O
 */
#if 0
bool mapped_load_mach_o(const char *filename)
{
	gDisplay->printf("MACH-O loading: %s\n", filename);
	byte *pt;
	if (ppc_direct_physical_memory_handle(0x0, pt)) {
		return false;
	}
	
	FILE *f;
	f = fopen(filename, "rb");
	MachOHeader hdr;
	if (fread(&hdr, sizeof hdr, 1, f) != 1) return false;
	createHostStructx(&hdr, sizeof hdr, MACHO_HEADER_struct, big_endian);
	for (uint i=0; i < hdr.ncmds; i++) {
//		ht_printf("cmd %d/%d\n", i+1, hdr.ncmds);
		long fpos = ftell(f);
		MachOCommand cmd;
		if (fread(&cmd, sizeof cmd, 1, f) != 1) return false;
		createHostStructx(&cmd, sizeof cmd, MACHO_COMMAND_struct, big_endian);
		if (cmd.cmd == 1) {
			MachOSegmentCommand seg;
			if (fread(&seg, sizeof seg, 1, f) != 1) return false;
			createHostStructx(&seg, sizeof seg, MACHO_SEGMENT_COMMAND_struct, big_endian);
//			ht_printf("mapping %08x->%08x (%d bytes)\n", seg.fileoff, seg.vmaddr, seg.filesize);
			fseek(f, seg.fileoff, SEEK_SET);
			memset(pt+seg.vmaddr, 0, seg.vmsize);
			if (fread(pt+seg.vmaddr, seg.filesize, 1, f) != 1) return false;
		} else if (cmd.cmd == 5) {
			MachOPPCThreadState ts;
			if (fread(&ts, sizeof ts, 1, f) != 1) return false;
			createHostStructx(&ts, sizeof ts, MACHO_THREAD_COMMAND_struct, big_endian);
			
			gCPU.pc = ts.srr0;
			gCPU.srr[0] = ts.srr0;
			gCPU.srr[1] = ts.srr1;
			
			gCPU.gpr[1] = 8*1024*1024;
			gCPU.gpr[3] = 0x47110815;
			gCPU.gpr[4] = 0x4d4f5358;   // MOSX
			gCPU.gpr[5] = gPromOSIEntry;   // prom entry
//			gSinglestep = true;
		}
		fseek(f, fpos, SEEK_SET);
		fseek(f, cmd.cmdsize, SEEK_CUR);
	}
	fclose(f);
	return true;
}
#endif

/*
 *
 */
class BootRec: public Object {
public:
	PromNodeDisk	*d;
	PartitionEntry	*pe;
	int		partnum;
	String		*devname;

	BootRec(PromNodeDisk *aD, PartitionEntry *aPe, int aPartnum, const String &aDevname)
	{
		d = aD;
		pe = aPe;
		partnum = aPartnum;
		devname = new String(aDevname);
	}
	virtual ~BootRec()
	{
		delete devname;
	}
};

static void read_partitions(Container &brs, bool only_bootable)
{
	brs.delAll();
	const char *boot_devices[] = {"cdrom0", "cdrom1", "disk0", "disk1", NULL};
	const char **boot_device = boot_devices;
	while (*boot_device) {
		PromNode *node = findDevice(*boot_device, FIND_DEVICE_FIND, NULL);
		PromNodeDisk *d = dynamic_cast<PromNodeDisk*>(node);
		if (d) {
			Enumerator *e = d->pm->getPartitions();
			int partnum=0;
			foreach(PartitionEntry, pe, *e, 
				if (!only_bootable || (pe->mBootMethod != BM_none)) {
					brs.insert(new BootRec(d, pe, partnum, *boot_device));
				}
				partnum++;
			);
		}
		boot_device++;
	}
}

bool prom_user_boot_partition(File *&ret_file, uint32 &size, bool &direct, uint32 &loadAddr, uint32 &entryAddr)
{
	gDisplay->setAnsiColor(VCP(VC_LIGHT(VC_YELLOW), VC_TRANSPARENT));
	gDisplay->printf("\n        PROM boot-loader\n");
	gDisplay->printf("       ==================\n\n");
	Array brs(true);
	read_partitions(brs, true);
	char key2digit[256];
	for (int i=0; i<256; i++) key2digit[i] = -1;
	key2digit[KEY_0] = 0;
	key2digit[KEY_1] = 1;
	key2digit[KEY_2] = 2;
	key2digit[KEY_3] = 3;
	key2digit[KEY_4] = 4;
	key2digit[KEY_5] = 5;
	key2digit[KEY_6] = 6;
	key2digit[KEY_7] = 7;
	key2digit[KEY_8] = 8;
	key2digit[KEY_9] = 9;

	bool only_bootable = true;

	while (1) {
		if (gPromBootMethod == prombmSelect) gDisplay->printf("Which partition do you want to boot?\n");
		if (only_bootable) {
			gDisplay->printf("\n%d bootable partition(s) found:\n", brs.count());
			if (gPromBootMethod == prombmSelect) {
				gDisplay->printf("     0. Show all (even unbootable)\n");
			}
		} else {
			gDisplay->printf("\n%d partition(s) found:\n", brs.count());
			gDisplay->printf("     0. Show only bootable\n");
		}
		for (uint i=0; i < brs.count(); i++) {
			BootRec *bootrec = dynamic_cast<BootRec *>(brs[i]);
			gDisplay->printf("    %2d. partition %d of '%y' (%s/%s)\n", i+1, bootrec->partnum, bootrec->devname, bootrec->pe->mName, bootrec->pe->mType);
		}
		uint choice = 0;
		if (gPromBootMethod == prombmSelect) {
			gDisplay->printf("\nYour choice (ESC abort):");
			while (1) {
				gDisplay->printf("\r\e[0K\rYour choice (ESC abort): %d", choice);
				uint32 keycode;
				do {
					while (!cuda_prom_get_key(keycode)) {
						sys_gui_event();
						sys_suspend();
					}
				} while (keycode & 0x80);

				if (keycode == KEY_DELETE) choice = 0; else
				if (keycode == KEY_RETURN) break; else
				if (keycode == KEY_ESCAPE) return false;

				if ((keycode<256) && (key2digit[keycode]>=0)) {
					choice *= 10;
					choice += key2digit[keycode];
				}
			}
		} else {
			choice = 1;
		}
		if (choice == 0) {
			gDisplay->printf("\n\n");
			only_bootable = !only_bootable;
			read_partitions(brs, only_bootable);			
			continue;
		}
		gPromBootMethod = prombmSelect;
		if ((choice > 0) && (choice <= brs.count())) {
			choice--;
			BootRec *bootrec = dynamic_cast<BootRec *>(brs[choice]);
			if (bootrec->pe->mBootMethod == BM_none) {
				gDisplay->printf("\nThis partition is not bootable!\n");
				continue;
			}
			gDisplay->printf("\nBooting %d: '%y:%d'...\n", choice+1, bootrec->devname, bootrec->partnum);
			// FIXME: ic hack
			IDEConfig *ic = ide_get_config(bootrec->d->mNumber);
			File *rawFile = ic->device->promGetRawFile();
			File *bootFile = bootrec->pe->mInstantiateBootFile(rawFile,
				bootrec->pe->mInstantiateBootFilePrivData);
			if (!bootFile) {
				IO_PROM_ERR("can't open boot file\n");
				return false;
			}
			uint bootFileSize = bootFile->getSize();
			if (bootFileSize > 64*1024*1024) {
				gDisplay->printf("Boot file too large. size=%d (>64MB)\n", bootFileSize);
				continue;
			}
			// set bootpath
			char bootpath[1024];
			char devicebootpath[1024];
			bootrec->d->toPath(devicebootpath, sizeof devicebootpath);
			ht_snprintf(bootpath, sizeof bootpath, "%s:%d,BootX", devicebootpath, bootrec->partnum);
//			ht_snprintf(bootpath, sizeof bootpath, "/pci/pci-bridge/pci-ata/ata-4/disk1@1:9,BootX");
			PromNode *chosen = findDevice("/chosen", FIND_DEVICE_FIND, NULL);
			if (chosen) {
				chosen->addProp(new PromPropString("bootpath", bootpath));
			}

			ret_file = bootFile;
			// FIXME: HACK!!!
			gBootPartNum = bootrec->partnum;
			gBootNodeID = bootrec->d->getPHandle();

			gDisplay->setAnsiColor(VCP(VC_LIGHT(VC_BLUE), VC_TRANSPARENT));
			size = bootFileSize;
			switch (bootrec->pe->mBootMethod) {
			case BM_chrp: {
				direct = false;
				return true;
			}
			case BM_direct: {
				direct = true;
				loadAddr = bootrec->pe->mBootImageLoadAddr;
				entryAddr = bootrec->pe->mBootImageEntrypoint;
				return true;
			}
			default:
				ASSERT(0);
			}
			return false;
		} else {
			gDisplay->printf("\nInvalid choice, try again.\n");
		}
	}
	return false;
}

bool prom_load_boot_file()
{
	if (gPromBootMethod == prombmForce) {
		if (gConfig->haveKey("prom_loadfile")) {
			String loadfile;
			gConfig->getConfigString("prom_loadfile", loadfile);
			PromNode *pn = findDevice(gPromBootPath.contentChar(), FIND_DEVICE_FIND, NULL);
			if (pn) {
				gBootNodeID = pn->getPHandle();
				String dev, rest;
				gPromBootPath.leftSplit(':', dev, rest);
				String partNum, rest2;
				rest.leftSplit(',', partNum, rest2);
				uint32 p;
				if (partNum.toInt32(p)) gBootPartNum = p;
			}
			
			try {
				LocalFile f(loadfile);
//				if (!mapped_load_elf_from_chrp(f, 0)
				if (!mapped_load_elf(f)
				 && !mapped_load_xcoff(f, 0)
				 && !mapped_load_chrp(f)) {
					IO_PROM_WARN("couldn't load '%y'.\n", &loadfile);
					return false;
				}
			} catch (IOException &e) {
				String res;
				e.reason(res);
				IO_PROM_ERR("couldn't load '%y': %y\n", &loadfile, &res);
			}
			
		} else {
			IO_PROM_ERR("bootmethod is 'force', but no prom_loadfile defined\n");
			return false;
		}
	} else {
		uint32 loadAddr;
		uint32 entryAddr;
		File *f;
		uint32 msize;
		bool direct;
		if (!prom_user_boot_partition(f, msize, direct, loadAddr, entryAddr)) {
			IO_PROM_WARN("Can't boot a partition.\nTry bootmethod 'force' and specify a 'prom_loadfile' in your config-file...\n");
			return false;
		}
		try {
			LocalFile lf("bootfile.dump", IOAM_WRITE, FOM_CREATE);
			f->seek(0);
			f->copyAllTo(&lf);
		} catch (...) {
			printf("error dumping bootfile");
		}
		if (direct) {
			if (!mapped_load_direct(*f, loadAddr, entryAddr)) {
				IO_PROM_TRACE("couldn't load.\n");
				return false;
			}
		} else {
			if (!mapped_load_chrp(*f)) {
				IO_PROM_TRACE("couldn't load.\n");
				return false;
			}
		}
		delete f;
	}
	return true;
}
