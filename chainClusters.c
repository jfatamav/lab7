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

void printClusterChain(unsigned int FATidx, unsigned int FAT[]){
    printf("Cluster chain: ");
    while (FATidx != CLUSTER_EOF && FATidx != 0)
    {
        printf("%d ",FATidx);
        FATidx = FAT[FATidx];
    }
    printf("\n");
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

    size_t FATOffset = boot.FATOffset * pow(2.0, boot.BytePerSector);

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

    // LECTURA DEL FAT

    unsigned int FAT[boot.ClusterCount + 2];

    if (lseek(fd, FATOffset, SEEK_SET) < 0)
        perror("Error en seek");

    if (read(fd, FAT, sizeof(FAT)) < 0)
        perror("Error al leer el FAT");

    // EJERCICIO

    // De la salida del ejercicio 1 puedo saber el offset en bloques de 32 bytes de cada archivo (el enunciado no me prohibe hacer eso)
    unsigned int offsetPrimerArchivo = 25; // Win98.png (en el enunciado pone .c pero consultando con un JP me dijo que era un typo porque no existe Win98.c)
    unsigned int offsetSegundoArchivo = 40; // leeClustersFAT32.c.jpg

    size_t offsetFileDirectoryEntry1 = rootDirOffset + 32 * offsetPrimerArchivo;
    size_t offsetFileDirectoryEntry2 = rootDirOffset + 32 * offsetSegundoArchivo;

    // Para el primer archivo
    unsigned char fileDirEntry1[32];
    unsigned char streamExtensionEntry1[32];

    if (lseek(fd, offsetFileDirectoryEntry1, SEEK_SET) < 0)
        perror("Error al hacer seek");

    if (read(fd, fileDirEntry1, sizeof(fileDirEntry1)) < 0)
        perror("Error al leer file dir entry");

    if (read(fd, streamExtensionEntry1, sizeof(streamExtensionEntry1)) < 0)
        perror("Error al leer file dir entry");

    unsigned int firstCluster1 = streamExtensionEntry1[20];

    printf("Primer cluster para Win98.png: %d\n", firstCluster1);
    printClusterChain(firstCluster1, FAT);

    printf("\n");

    // Para el segundo archivo
    unsigned char fileDirEntry2[32];
    unsigned char streamExtensionEntry2[32];

    if (lseek(fd, offsetFileDirectoryEntry2, SEEK_SET) < 0)
        perror("Error al hacer seek");

    if (read(fd, fileDirEntry1, sizeof(fileDirEntry2)) < 0)
        perror("Error al leer file dir entry");

    if (read(fd, streamExtensionEntry2, sizeof(streamExtensionEntry2)) < 0)
        perror("Error al leer file dir entry");

    unsigned int firstCluster2 = streamExtensionEntry2[20];

    printf("Primer cluster para Win98.png: %d\n", firstCluster2);
    printClusterChain(firstCluster2, FAT);

    // Me parecio raro que ninguno tenga cadena de clusters, entonces corrobore haciendo un hexdump en el archivo "binary.hex"
    // Encontre que la FAT estaba llena de ceros, se puede ver a partir de la linea 0004000 (buscar con /0004000 con vim)
    // 0004000 es el offset de la FAT, no deberia haber nada mal a pesar de que la cadena de ambos archivos es de un solo cluster

    exit(0);
}