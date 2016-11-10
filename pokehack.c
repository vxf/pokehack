#include <stdlib.h>
#include <stdio.h>

/* offsets */
#define SECTION_ID 0xFF4
#define CHECKSUM 0xFF6
#define SAVE_IDX 0xFFC

#define TRAINER_NAME    0x0000
#define TRAINER_GENDER  0x0008
#define TRAINER_ID      0x000A
#define TRAINER_TIME    0x000E
#define TRAINER_OPTS    0x0013
#define TRAINER_SKEY    0x00AC

#define TEAM_MONEY 0x0490

#define SECTION_INFO 0
#define SECTION_TEAM 1

#define GENDER_BOY 0
#define GENDER_GIRL 1

typedef struct trainer_struct
{
  char name[8];
  char gender;
	unsigned int pub_id, prv_id;
	unsigned short hours;
	unsigned char minutes, seconds, frames;
	unsigned int seckey;
}
trainer;

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
void readTrainerInfo(char * data, trainer * t)
{
  unsigned int id;
  
	ptoa(t->name, data + TRAINER_NAME);
	t->gender = *(data + TRAINER_GENDER);
	
  id = *(unsigned int *)(data + TRAINER_ID);
	t->pub_id = id & 0xFFFF,
	t->prv_id = (id >> 16) & 0xFFFF;
	
	t->hours   = *(unsigned short *)(data + TRAINER_TIME);
	t->minutes = *(unsigned char *)(data + TRAINER_TIME + 2);
	t->seconds = *(unsigned char *)(data + TRAINER_TIME + 3);
	t->frames  = *(unsigned char *)(data + TRAINER_TIME + 4);
  
	t->seckey = *(unsigned int *)(data + TRAINER_SKEY);
}

/*
 * Print trainer info.
 */
void printTrainerInfo(trainer * t)
{
  printf("Trainer: %s ", t->name);
	if (t->gender && GENDER_GIRL)
		printf("is a girl\n");
	else
		printf("is a boy\n");
	printf("Public ID: %04x Secret ID: %04x\n", t->pub_id, t->prv_id);
	printf("You have played: %02d:%02d:%02d\n", t->hours, t->minutes, t->seconds);
	printf("Security key: %08X\n", t->seckey);
}

void readTeam(char * data, trainer * t)
{

	unsigned int money = *(unsigned int *)(data + TEAM_MONEY);
	money = money ^ t->seckey;
	printf("Moneys: %d\n", money);
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
	trainer t;

	chunk * chunks = (chunk *)malloc(sizeof(chunk) * 14);

	for (i = 0; i < 14; i++)
	{
		fseek(file, saveOffset + i * 0x1000 + SECTION_ID, SEEK_SET);
		readFooter(file, &secid, &chksum, &save_index);
		
		fseek(file, saveOffset + i * 0x1000, SEEK_SET);
		fread(chunks[secid].data, sizeof(unsigned char), CHUNK_SIZE, file);
	}

	readTrainerInfo(chunks[SECTION_INFO].data, &t);
	printTrainerInfo(&t);
	
	readTeam(chunks[SECTION_TEAM].data, &t);

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
