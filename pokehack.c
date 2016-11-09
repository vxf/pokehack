#include <stdlib.h>
#include <stdio.h>

/* offsets */
#define SECTION_ID 0xFF4
#define CHECKSUM 0xFF6
#define SAVE_IDX 0xFFC

#define TRAINER_INFO 0
#define GENDER_BOY 0
#define GENDER_GIRL 1

#define CHUNK_SIZE 4096

typedef struct chunk_struct
{
	unsigned char data[CHUNK_SIZE];
} chunk; 

/*
 * pokemon chr to ascii chr
 */
char pokechr(unsigned char c)
{
	if (c >= 0xBB && c < 0xD5)
		return c - 0xBB + 'A';
	else if (c >= 0xD5 && c < 0xEF)
		return c - 0xD5 + 'a';
	else if (c >= 0xA1 && c < 0xAB)
		return c - 0xBB + '0';
	else
		return ' ';
}

/*
 * pokemon strings to ascii strings
 */
void ptoa(char * ascii, unsigned char * poke)
{
	while(*poke != 0xFF)
		*ascii++ = pokechr(*poke++);

	*ascii = 0;
}

/*
 * Read a trainer info block
 */
void readTrainerInfo(FILE * file)
{
	unsigned char namepoke[8];
	char name[8];
	char gender;
	unsigned int trainerid;
	unsigned short hours;
	unsigned char minutes, seconds, frames;
	unsigned int seckey;

	fread(namepoke, sizeof(char), 8, file);
	// namepoke[7] = 0xFF;
	ptoa(name, namepoke);

	fread(&gender, sizeof(char), 1, file);

	fseek(file, 1, SEEK_CUR);
	fread(&trainerid, sizeof(unsigned int), 1, file);

	fread(&hours, sizeof(unsigned short), 1, file);
	fread(&minutes, sizeof(unsigned char), 1, file);
	fread(&seconds, sizeof(unsigned char), 1, file);
	fread(&frames, sizeof(unsigned char), 1, file);

	fseek(file, 3, SEEK_CUR); // options
  fseek(file, 0x8C, SEEK_CUR); // game code
	fread(&seckey, sizeof(unsigned int), 1, file);
  	

  printf("Trainer: %s ", name);
	if (gender && GENDER_GIRL)
		printf("is a girl\n");
	else
		printf("is a boy\n");

	printf("Public ID: %04x Secret ID: %04x\n",
			trainerid & 0xFFFF,
			(trainerid >> 16) & 0xFFFF);

	printf("You have played: %02d:%02d:%02d\n", hours, minutes, seconds);

	printf("Security key: %08X\n", seckey);
}

/*
 * Read the footer of a block
 */
void readFooter(FILE * file, short * secid, short * chksum, int * save_index)
{
		fread(secid, 2, 1, file);
    fread(chksum, 2, 1, file);
		fseek(file, 4, SEEK_CUR);
		fread(save_index, 4, 1, file);
}

/*
 * Read a save offset
 */
void readSave(FILE * file, int saveOffset)
{
	int i;
  short secid;
	short chksum;
	int save_index;

	chunk * chunks = (chunk *)malloc(sizeof(chunk) * 14);

	for (i = 0; i < 14; i++)
	{
		// fseek(file, saveOffset + i * 0x1000, SEEK_SET);
		fseek(file, saveOffset + i * 0x1000 + SECTION_ID, SEEK_SET);
		readFooter(file, &secid, &chksum, &save_index);

		printf("%02d %04hX %08x\n", secid, chksum, save_index);

		switch (secid)
		{
			case TRAINER_INFO:
		  	fseek(file, saveOffset + i * 0x1000, SEEK_SET);
				readTrainerInfo(file);
				break;
		}

		fseek(file, saveOffset + i * 0x1000, SEEK_SET);
		fread(chunks[secid].data, sizeof(unsigned char), CHUNK_SIZE, file);
	}

	// readTrainerInfo(chunk[TRAINER_INFO].data);

	free(chunks);
}

int main(int argc, char * argv[])
{
	FILE * file;
	
	if (argc <= 1)
	{
	  printf("pokehack savegame.sav\n");
	  return -1;
	}

	file = fopen(argv[1], "rb");

	readSave(file, 0);
	readSave(file, 0x00E000);

	fclose(file);

  return 0;
}
