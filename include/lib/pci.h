/*
 * Copyright (c) 2015-2019 Intel Corporation.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _PCI_H_
#define _PCI_H_

#ifndef LIB_PCI
#error "LIB_PCI is not defined"
#endif

#include "vmm_base.h"

typedef union {
	struct {
		uint16_t fun:3;
		uint16_t dev:5;
		uint16_t bus:8;
	} bits;
	uint16_t u16;
} pci_dev_t;

typedef union {
	struct {
		uint32_t reg:8;
		uint32_t pci_dev:16;
		uint32_t rsvd:7;
		uint32_t enable:1;
	} bits;
	uint32_t u32;
} pci_addr_t;

typedef union {
	struct {
		uint32_t reg:12;
		uint32_t pci_dev:16;
		uint32_t pcie_base:4;
	} bits;
	uint32_t u32;
} pcie_addr_t;

#define PCI_CFG_ADDR 0xCF8
#define PCI_CFG_DATA 0xCFC
#define PCI_ADDR_ENABLE 0x80000000

#define PCI_CMD_REG     0x04
#define PCI_CLASS_REG   0x0A
#define PCI_BAR_REG(n) (0x10 + ((n) << 2)) // Base Register starts from 0x10

#define PCI_BAR_IS_IO_SPACE(bar) ((bar) & BIT(0)) // Bit 0 of BAR indicates the type: 0-->MMIO, 1-->IO
#define PCI_BAR_MMIO_MASK 0xFFFFFFF0
#define PCI_BAR_IO_MASK   0xFFFFFFFC

#define PCI_CMD_IO_SPACE      BIT(0)
#define PCI_CMD_MEMORY_SPACE  BIT(1)

#define PCI_CLASS_COM_SERIAL 0x0700
#define PCI_CLASS_COM_MODEM  0x0703

#define PCI_DEV(bus, dev, fun) (((uint16_t)(bus) << 8) | \
				((uint16_t)(dev) << 3)| \
				(uint16_t)(fun))

#define PCI_ADDR(pci_dev, reg)	(PCI_ADDR_ENABLE | \
				((uint32_t)(pci_dev) << 8) | \
				(uint32_t)(reg))

#define PCIE_ADDR_OFFSET(pcie_dev, reg) (((uint32_t)(pcie_dev) << 12) | \
					(uint32_t)(reg))

#define PCIE_BASE_OFFSET(pci_dev) PCIE_ADDR_OFFSET(pci_dev, 0)

uint8_t pci_read8(uint16_t pci_dev, uint8_t reg);

void pci_write8(uint16_t pci_dev, uint8_t reg, uint8_t val);

uint16_t pci_read16(uint16_t pci_dev, uint8_t reg);

void pci_write16(uint16_t pci_dev, uint8_t reg, uint16_t val);

uint32_t pci_read32(uint16_t pci_dev, uint8_t reg);

void pci_write32(uint16_t pci_dev, uint8_t reg, uint32_t val);

#endif
