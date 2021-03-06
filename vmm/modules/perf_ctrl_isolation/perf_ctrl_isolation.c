/*
 * Copyright (c) 2015-2019 Intel Corporation.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "vmm_base.h"
#include "vmm_arch.h"
#include "guest.h"
#include "gcpu.h"
#include "vmcs.h"
#include "dbg.h"
#include "vmx_cap.h"
#include "event.h"

#include "modules/msr_monitor.h"

/* Monitor IA32_PERF_GLOBAL_CTRL MSR.
 * Save the value to Guest IA32_PERF_GLOBAL_CTRL VMCS field when guest try
 * to write this MSR. The MSR will be loaded on each VM-entry. */
static void msr_perf_ctrl_write_handler(guest_cpu_handle_t gcpu, uint32_t msr_id UNUSED)
{
	uint64_t msr_value;
	D(VMM_ASSERT(msr_id == MSR_PERF_GLOBAL_CTRL));
	msr_value = get_val_for_wrmsr(gcpu);

	print_trace("VMM: gcpu[%d] %s:MSR_ID=0x%x, CUR_VAL=0x%08llx --> TO_VAL=0x%08llx\n",
			gcpu->id, __func__, msr_id, vmcs_read(gcpu->vmcs, VMCS_GUEST_PERF_G_CTRL), msr_value);

	vmcs_write(gcpu->vmcs, VMCS_GUEST_PERF_G_CTRL, msr_value);

	gcpu_skip_instruction(gcpu);
}

static void guest_msr_perf_ctrl_monitor_setup(UNUSED guest_cpu_handle_t gcpu, void *pv)
{
	guest_handle_t guest = (guest_handle_t)pv;
	monitor_msr_write(guest->id, MSR_PERF_GLOBAL_CTRL, msr_perf_ctrl_write_handler);
}

static void perf_ctrl_isolation_gcpu_init(guest_cpu_handle_t gcpu, UNUSED void *pv)
{
	uint32_t ctrl_may1, ctrl_may0;
	uint32_t exit_ctrl, entry_ctrl;

	D(VMM_ASSERT(gcpu));

	vmcs_write(gcpu->vmcs, VMCS_HOST_PERF_G_CTRL, 0);
	vmcs_write(gcpu->vmcs, VMCS_GUEST_PERF_G_CTRL, 0);

	/* Enable "Load IA32_PERF_GLOBAL_CTRL" bit in VM-Exit controls. */
	ctrl_may1 = get_exitctl_cap(&ctrl_may0);
	exit_ctrl = vmcs_read(gcpu->vmcs, VMCS_EXIT_CTRL) | (ctrl_may1 & EXIT_LOAD_IA32_PERF_CTRL);
	vmcs_write(gcpu->vmcs, VMCS_EXIT_CTRL, exit_ctrl);

	/* Enable "Load IA32_PERF_GLOBAL_CTRL" bit in VM-Entry controls. */
	ctrl_may1 = get_entryctl_cap(&ctrl_may0);
	entry_ctrl = vmcs_read(gcpu->vmcs, VMCS_ENTRY_CTRL) | (ctrl_may1 & ENTRY_LOAD_IA32_PERF_CTRL);
	vmcs_write(gcpu->vmcs, VMCS_ENTRY_CTRL, entry_ctrl);

}

void msr_perf_ctrl_isolation_init(void)
{
	if (!(get_entryctl_cap(NULL) & ENTRY_LOAD_IA32_PERF_CTRL)) {
		print_warn("ENTRY_LOAD_IA32_PERF_CTRL not supported! IA32_PERF_CTRL MSR will not isolated!\n");
		return;
	}

	if (!(get_exitctl_cap(NULL) & EXIT_LOAD_IA32_PERF_CTRL)) {
		print_warn("EXIT_LOAD_IA32_PERF_CTRL not supported! IA32_PERF_CTRL MSR will not isolated!\n");
		return;
	}

	event_register(EVENT_GUEST_MODULE_INIT, guest_msr_perf_ctrl_monitor_setup);
	event_register(EVENT_GCPU_MODULE_INIT, perf_ctrl_isolation_gcpu_init);
}
