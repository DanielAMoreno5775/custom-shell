#ifndef HASHMAP_H_
#define HASHMAP_H_

//include various libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//code for private and public versions of the struct
struct s_hashMap;
typedef struct s_hashMap HashMap;

//function prototypes
HashMap * newHashMap ();
void deleteHashMap (HashMap *);
int getIndexOfKey(HashMap *, char []);
void insertKeyInHashMap(HashMap *, char [], char []);
char * getKeyInHashMap(HashMap *, char []);
int removeKeyInHashMap (HashMap *, char []);
int getNumOfElements (HashMap *);
void duplicateHashMap (HashMap *, HashMap *);
void printHashMap (HashMap *);
void printHashMapToFile (HashMap *, FILE *);
int printHashMapDifference (HashMap *, HashMap *);
void printHashMapKeysToFile (HashMap *, FILE *);

#endif
