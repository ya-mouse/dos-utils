#define PCI_CONFIG_H
#define PCI_ARCH_I386
#define PCI_OS_WATCOM
#define PCI_HAVE_PM_INTEL_CONF
#define PCI_HAVE_PM_DUMP
#define PCI_COMPRESSED_IDS
#define PCI_IDS "pci.idz"
#define PCI_PATH_IDS_DIR "."
#define PCILIB_VERSION "3.1.7"

typedef unsigned char u_int8_t;
typedef unsigned short int u_int16_t;
typedef unsigned long u_int32_t;

#include <stdio.h>
