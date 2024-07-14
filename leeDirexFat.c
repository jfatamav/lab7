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

    if (argc != 2)
    {
        printf("Uso: %s <Image File System>\n", argv[0]);
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
        // printf("Primer entry type: %X\n",entryType);

    unsigned int allocationBitMapCluster = entry[20];
    // printf("Cluster del allocation bitmap: %d\n",allocationBitMapCluster);
    size_t allocationBitMapOffset =
        boot.ClusterHeapOffset * pow(2.0, boot.BytePerSector) +
        (allocationBitMapCluster - 2) *
            pow(2.0, boot.SectorPerCluster + boot.BytePerSector);

    // EJERCICIO

    unsigned int entriesAcumulator = 0;

    while (1)
    {
        unsigned int currOffsetIn32BytesBlocks = entriesAcumulator;
        unsigned char fileEntry[32];
        if(read(fd,fileEntry,sizeof(fileEntry))<0)
            perror("Error leyendo dir entry");
        entriesAcumulator++;

        unsigned char entryType = fileEntry[0];

        if (entryType == 0x00) break;
        if (entryType != 0x85) {
            entriesAcumulator++;
            continue;
        };

        // Esto servira para la pregunta 3
        // printf("Offset en bloques de 32 bytes respecto al inicio del rootDir: %d\n",currOffsetIn32BytesBlocks);

        unsigned char streamExtensionEntry[32];
        if(read(fd,streamExtensionEntry,sizeof(streamExtensionEntry))<0)
            perror("Error leyendo stream extension entry");
        entriesAcumulator++;

        unsigned char nameLen = streamExtensionEntry[3];

        printf("Nombre: ");
        while (nameLen > 0)
        {
            char filenameEntry[32];
            if (read(fd, filenameEntry, sizeof(filenameEntry)) < 0)
                perror("Error leyendo filenameEntry");
            entriesAcumulator++;

            char *nameSection = &filenameEntry[2];
            for(int i=0; i<30 && i<nameLen * 2; i++)
                printf("%c",nameSection[i]);

            if(nameLen < 15) nameLen = 0;
            else nameLen -= 15;
        }
        printf("\n");
        
        printf("\n");
    }
    

    exit(0);
}