#ifndef _EXTEND_H_
#define _EXTEND_H_


typedef struct animalData
{
	int color;
	int name[20];
}animalData;

typedef struct animal
{
	animalData data;
	void (*print)(void *arg);
	void (*show)(void *arg);
}animal;

typedef struct dogData
{
	int color;
	int name[20];
}dogData;

typedef struct dog
{
	dogData dog;
	animal dogExtendsAnimal;
	void* (*eat)(void *arg);
}dog;







#endif
