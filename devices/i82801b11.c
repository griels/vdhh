/*
 * Copyright (c) 2006 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/*
 * QEMU i82801b11 dmi-to-pci Bridge Emulation
 *
 *  Copyright (c) 2009, 2010, 2011
 *                Isaku Yamahata <yamahata at valinux co jp>
 *                VA Linux Systems Japan K.K.
 *  Copyright (C) 2012 Jason Baron <jbaron@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */

#include "pci.h"
#include "iich9.h"


/*****************************************************************************/
/* ICH9 DMI-to-PCI bridge */
#define I82801ba_SSVID_OFFSET   0x50
#define I82801ba_SSVID_SVID     0
#define I82801ba_SSVID_SSID     0

typedef struct I82801b11Bridge {
    /*< private >*/
    PCIBridge parent;
    /*< public >*/
} I82801b11Bridge;

static int i82801b11_bridge_initfn(PCIDevice *d)
{
    int rc;

    rc = pci_bridge_initfn(d, TYPE_PCI_BUS);
    if (rc < 0) {
        return rc;
    }

    rc = pci_bridge_ssvid_init(d, I82801ba_SSVID_OFFSET,
                               I82801ba_SSVID_SVID, I82801ba_SSVID_SSID);
    if (rc < 0) {
        goto err_bridge;
    }
    pci_config_set_prog_interface(d->config, PCI_CLASS_BRIDGE_PCI_INF_SUB);
    return 0;

err_bridge:
    pci_bridge_exitfn(d);

    return rc;
}

void pci_bridge_write_config(PCIDevice *device, uint32_t addr, uint32_t val, int len);

static void i82801b11_bridge_class_init(VeertuTypeClassHold *klass, void *data)
{
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);
    DeviceClass *dc = DEVICE_CLASS(klass);

    k->is_bridge = 1;
    k->vendor_id = PCI_VENDOR_ID_INTEL;
    k->device_id = PCI_DEVICE_ID_INTEL_82801BA_11;
    k->revision = ICH9_D2P_A2_REVISION;
    k->init = i82801b11_bridge_initfn;
    k->config_write = pci_bridge_write_config;
    set_bit(DEVICE_CATEGORY_BRIDGE, dc->categories);
}

static const VeertuTypeInfo i82801b11_bridge_info = {
    .name          = "i82801b11-bridge",
    .parent        = TYPE_PCI_BRIDGE,
    .instance_size = sizeof(I82801b11Bridge),
    .class_init    = i82801b11_bridge_class_init,
};

PCIBus *ich9_d2pbr_init(PCIBus *bus, int devfn, int sec_bus)
{
    PCIDevice *d;
    PCIBridge *br;
    char buf[16];
    DeviceState *qdev;

    d = pci_create_multifunction(bus, devfn, true, "i82801b11-bridge");
    if (!d) {
        return NULL;
    }
    br = PCI_BRIDGE(d);
    qdev = DEVICE(d);

    snprintf(buf, sizeof(buf), "pci.%d", sec_bus);
    pci_bridge_map_irq(br, buf, pci_swizzle_map_irq_fn);
    qdev_init_nofail(qdev);

    return pci_bridge_get_sec_bus(br);
}

void d2pbr_register(void)
{
    register_type_internal(&i82801b11_bridge_info);
}
