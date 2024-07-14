#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CLUSTER_EOF 0xFFFFFFFF
#define CLUSTER_MAX 0xFFFFFFF6
#define CLUSTER_MIN 0x00000003

typedef struct
{
    char jump[3];
    char FSName[8];
    char Chunk[53];
    long offsetPart;
    long sizeVol;
    int FATOffset;
    int FATlen;
    int ClusterHeapOffset;
    int ClusterCount;
    int RootDirFirstCluster;
    int VSN;
    short FSR;
    short FlagVol;
    char BytePerSector;
    char SectorPerCluster;
    char NumberFats;
    char DriveSelect;
    char PercentUse;
    char reserved[7];
    char BootCode[390];
    short BootSignature;
} __attribute((packed)) exFatBootSector;

unsigned int getBit(unsigned char byte, int pos)
{
    unsigned char mask = 1 << pos;
    return (byte & mask) > 0;
}

int main(int argc, char *argv[])
{
    int fd, exp;
    exFatBootSector boot;

    if (argc != 3)
    {
        printf("Uso: %s <Image File System> <Nro Cluster>\n", argv[0]);
        exit(1);
    }

    if ((fd = open(argv[1], O_RDONLY)) < 0)
        perror("No se pudo abrir la imagen del disco\n");

    // LECTURA DEL BOOT

    if (read(fd, &boot, sizeof(boot)) < 0)
        perror("No se pudo leer la imagen del disco\n");

    // CALCULO DE LOS OFFSETS 

    size_t rootDirOffset =
        boot.ClusterHeapOffset * pow(2.0, boot.BytePerSector) +
        (boot.RootDirFirstCluster - 2) *
            pow(2.0, boot.SectorPerCluster + boot.BytePerSector);

        // PARA ENCONTRAR DONDE ESTA EL ALLOCATIONBITMAP
        if (lseek(fd,rootDirOffset + 32,SEEK_SET)<0)
            perror("Error en seek");

        unsigned char entry[32];
        if (read(fd,entry,sizeof(entry) )<0)
            perror("Error leyendo");

        unsigned char entryType = entry[0];

    unsigned int allocationBitMapCluster = entry[20];
    size_t allocationBitMapOffset =
        boot.ClusterHeapOffset * pow(2.0, boot.BytePerSector) +
        (allocationBitMapCluster - 2) *
            pow(2.0, boot.SectorPerCluster + boot.BytePerSector);

    // LECTURA DEL ALLOCATION BIT MAP

    size_t bytesInAllocationBitMap = (boot.ClusterCount + 8) / 8;
    unsigned char allocationBitMap[bytesInAllocationBitMap];

    if (lseek(fd, allocationBitMapOffset, SEEK_SET) < 0)
        perror("Error en seek");

    if (read(fd, allocationBitMap, sizeof(allocationBitMap)) < 0)
        perror("Error al leer el allocationBitMap");

    // EJERCICIO

    unsigned int cluster = atoi(argv[2]);
    unsigned int byteOffset = (cluster - 2) / 8;
    unsigned int bitOffset = (cluster - 2) % 8;

    if (getBit(allocationBitMap[byteOffset],bitOffset)) {
        printf("Esta alocado\n");
    } else {
        printf("Esta libre\n");
    }

    exit(0);
}