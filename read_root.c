#include <stdio.h>
#include <stdlib.h>


typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char starting_cluster[4];
    char file_size[4];
} __attribute((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;

    unsigned char sectorsPerCluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short numberOfSectorsFS;
    unsigned char mediaType;
    unsigned short fat_size_sectors;
    unsigned short sectorsPerTrack;
    unsigned short numbersOfHead;
    unsigned int numberOfSectorsBeforeStartPartition;
    unsigned int numberOfSectorsFSExtended;
    unsigned char bios13h;
    char notUsed;
    char bootSignature;

    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

//Estructura de 32 bytes
typedef struct {
    unsigned char filename[1];
    unsigned char name[7];
    unsigned char extension[3];
    unsigned char attributes[1];
    unsigned char reserved;
    unsigned char created_time_seconds;
    unsigned char created_time_hours_minutes_seconds[2];
    unsigned char created_day[2];
    unsigned char accessed_day[2];
    unsigned char cluster_highbytes_address[2];
    unsigned char written_time[2];
    unsigned char written_day[2];
    unsigned short cluster_lowbytes_address;
    unsigned int size_of_file;
} __attribute((packed)) Fat12Entry;

void print_file_info(Fat12Entry *entry) {

    switch(entry->filename[0]) {
        case 0x00:
            return; // unused entry
        case 0x05:
            //Muestro nombre y extension de archivo borrado
            printf("Archivo borrado: [?%.8s.%.3s]\n", entry->filename, entry->extension);
            return;
        case 0xE5:
            //Muestro nombre y extension
            printf("Archivo que comienza con 0xE5: [%c%.8s.%.3s]\n", 0xE5, entry->filename, entry->extension);
            break;
        default:
            switch (entry->attributes[0])
            {
            case 0x20:
                //Es archivo
                printf("Archivo: [%.8s.%.3s]\n", entry->filename, entry->extension);
            break;
            case 0x10:
                //Es directorio
                printf("Directorio: [%.1s%.7s]\n",  entry->filename, entry->name);
            default:
                break;
            }
    }
   
}

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;
    int partitionStart = 446; //Inicio de particion
   
    fseek(in, partitionStart, SEEK_SET);
    fread(pt, sizeof(PartitionTable), 4, in);  
   
    for(i=0; i<4; i++) {        
        if(pt[i].partition_type == 1) {
            printf("Encontrada particion FAT12 %d\n", i);
            break;
        }
    }
   
    if(i == 4) {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }
   
    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);
   
    //printf("En  0x%X, sector size %d, FAT size %d sectors, %d FATs\n\n",
    //       ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);
    printf("sector size %d, FAT size %d sectors, %d FATs\n\n",
           bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);
                  
    fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
   
    printf("Root dir_entries %d \n", bs.root_dir_entries);

    for(i=0; i<bs.root_dir_entries; i++) {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry);
    }
   
    //printf("\nLeido Root directory, ahora en 0x%X\n", ftell(in));
    fclose(in);
    return 0;
}
