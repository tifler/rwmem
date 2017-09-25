#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "debug.h"

/*****************************************************************************/

#define MEMORY_DEVICE                           "/dev/mem"

/*****************************************************************************/

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct Memory {
    int fd;
    u32 phyBase;
    size_t size;
    void *base;
};

/*****************************************************************************/

static inline long getPageSize()
{
    return sysconf(_SC_PAGESIZE);
}

static struct Memory *openMemory(u32 phyAddr, size_t size)
{
    struct Memory *mem;
    long pageSize;
    long pageMask;

    pageSize = getPageSize();
    pageMask = ~(pageSize - 1);

    mem = calloc(1, sizeof(*mem));
    ASSERT(mem);

    mem->fd = open(MEMORY_DEVICE, O_RDWR | O_SYNC);
    if (mem->fd < 0) {
        perror(MEMORY_DEVICE);
        ASSERT(mem->fd > 0);
    }

    mem->phyBase = phyAddr & pageMask;
    mem->size = (phyAddr + size - 
            mem->phyBase + pageSize - 1) / pageSize * pageSize;
#if 1
    DBG("Will map 0x%08x to 0x%08x (len = %d)",
            mem->phyBase, mem->phyBase + mem->size - 1, mem->size);
#endif  /*0*/

    mem->base = mmap(NULL, mem->size,
            PROT_READ | PROT_WRITE, MAP_SHARED, mem->fd, mem->phyBase);
    ASSERT(mem->base != MAP_FAILED);
#if 1
    DBG("Mapped address = %p\n", mem->base);
#endif  /*0*/

    return mem;
}

static void closeMemory(struct Memory *mem)
{
    munmap(mem->base, mem->size);
    close(mem->fd);
    free(mem);
}

/*****************************************************************************/

static void getReg(
        struct Memory *mem, unsigned long addr, void *value, int unitSize)
{
    ASSERT(mem);

    switch (unitSize) {
        case 1:
            *(u8 *)value = *(volatile u8 *)(((u8 *)mem->base) + addr - mem->phyBase);
            break;
        case 2:
            ASSERT((addr & 1) == 0);
            *(u16 *)value = *(volatile u16 *)(((u8 *)mem->base) + addr - mem->phyBase);
            break;
        case 4:
            ASSERT((addr & 3) == 0);
            *(volatile u32 *)value = *(volatile u32 *)(((u8 *)mem->base) + addr - mem->phyBase);
            break;
        default:
            ASSERT(! "Never reached");
            break;
    }
}

static void setReg(
        struct Memory *mem, unsigned long addr, void *value, int unitSize)
{
    ASSERT(mem);

    switch (unitSize) {
        case 1:
            *(volatile u8 *)(((u8 *)mem->base) + addr - mem->phyBase) = *(u8 *)value;
            break;
        case 2:
            ASSERT((addr & 1) == 0);
            *(volatile u16 *)(((u8 *)mem->base) + addr - mem->phyBase) = *(u16 *)value;
            break;
        case 4:
            ASSERT((addr & 3) == 0);
            *(volatile u32 *)(((u8 *)mem->base) + addr - mem->phyBase) = *(u32 *)value;
            break;
        default:
            ASSERT(! "Never reached");
            break;
    }
}

/*****************************************************************************/

static void usage(int argc, char **argv)
{
    fprintf(stderr,
            "Usage: %s <r|w> <address> ...\n"
            " r <address> [count=1]\n"
            " w <address> <data>\n", basename(argv[0]));
}

enum {
    READ_MODE,
    WRITE_MODE,
};

int main(int argc, char **argv)
{
    int mode = READ_MODE;
    u32 addr;
    struct Memory *mem;

    if (argc < 3) {
        usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    if (argv[1][0] == 'r') {
        mode = READ_MODE;
    }
    else if (argv[1][0] == 'w') {
        mode = WRITE_MODE;
    }
    else {
        usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    addr = strtoul(argv[2], NULL, 0);

    if (mode == READ_MODE) {
        u32 i;
        u32 data;
        u32 count = 1;

        if (argc == 4)
            count = strtoul(argv[3], NULL, 0);

        mem = openMemory(addr, count * 4);
        ASSERT(mem);

        for (i = 0; i < count; i++) {
            getReg(mem, addr + i * 4, &data, sizeof(data));
            printf("0x%08x: 0x%08x\n", addr + i * 4, data);
        }
        closeMemory(mem);
    }
    else {
        u32 i;
        u32 odata, ndata;

        if (argc < 4) {
            usage(argc, argv);
            exit(EXIT_FAILURE);
        }

        mem = openMemory(addr, (argc - 3) * 4);
        ASSERT(mem);

        for (i = 3; i < argc; i++) {
            ndata = strtoul(argv[i], NULL, 0);
            getReg(mem, addr, &odata, sizeof(odata));
            setReg(mem, addr, &ndata, sizeof(ndata));
            getReg(mem, addr, &ndata, sizeof(ndata));
            printf("0x%08x: 0x%08x -> 0x%08x\n", addr, odata, ndata);
            addr += sizeof(odata);
        }
        closeMemory(mem);
    }

    return 0;
}
