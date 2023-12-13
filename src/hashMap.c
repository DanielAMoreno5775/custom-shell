/**Primary Author: Daniel Moreno
 * CSCE 3600.003
 * Group 1
 * LAST EDITED: OCTOBER 31
 * 
 * Contains functions newHashMap, deleteHashMap, getIndexOfKey, insert, get
 * Will create functionality that resembles a Hash Map class without some of the protections inherent to a true class
*/

#include "hashMap.h"

//create literal value of 512 called MAX_HASH_MAP_SIZE which functions like a constant
#define MAX_HASH_MAP_SIZE 512

//private struct to store private variables
struct s_hashMap {
	//create variable for current size;
  	int currentSize;
  	//array to store the keys
	char keys[MAX_HASH_MAP_SIZE][MAX_HASH_MAP_SIZE]; 
	//array to store the values
	char values[MAX_HASH_MAP_SIZE][MAX_HASH_MAP_SIZE];
};

//public function
//creates and returns a new HashMap object
HashMap * newHashMap () {
	HashMap * object = (HashMap*)malloc(sizeof(struct s_hashMap));
	if (!object) {
		fprintf(stderr, "Memory allocation error\n");
		return NULL;
	}
	object->currentSize = 0;
	return object;
}

//public function
//frees up the space allocated for a HashMap object
void deleteHashMap (HashMap *object) {
	object->currentSize = 0;
	free(object);
}

//public function
//gets the index of a key if it is present or -1 if not
int getIndexOfKey(HashMap *object, char key[]) {
    //iterate through the list of keys looking for the passed key
    for (int i = 0; i < object->currentSize; i++) { 
    	//compare current element and passed key
    	//if equivalent, return the current index
        if (strcmp(object->keys[i], key) == 0) { 
            return i; 
        } 
    } 
    //to reach this return, the other return never triggered which means that a match was not found
    //as such, return an index of -1
    return -1;
}

//public function
//will insert the new value of the key was not found or update a pre-existing key's value
void insertKeyInHashMap(HashMap *object, char key[], char value[])  { 
    //search for the key
    int index = getIndexOfKey(object, key); 
    //if key wasn't found
    if (index == -1) {
    	//copy over the key
        strcpy(object->keys[object->currentSize], key); 
        //copy over the value
        strcpy(object->values[object->currentSize], value); 
        //increment the current size
        object->currentSize++; 
    } 
    //if the key was found
    else { 
    	//update the associated value
        strcpy(object->values[index], value); 
    } 
} 

//public function
//will return the value associated with the passed key
char * getKeyInHashMap(HashMap *object, char key[]) { 
    //search for the key
    int index = getIndexOfKey(object, key); 
    //if the key was not found, return a -1
    if (index == -1) {
        return NULL; 
    } 
    //otherwise, return the associated value
    else {
    	char * value = malloc(512);
    	if (!value) {
		fprintf(stderr, "Memory allocation error\n");
		return NULL;
	}
    	memcpy(value, object->values[index], MAX_HASH_MAP_SIZE);
        return value; 
    } 
} 

//public function
//removes the passed key and its value from the hash map
int removeKeyInHashMap (HashMap *object, char key[]) {
	//search for the key
    	int index = getIndexOfKey(object, key); 
    	//if the key was not found, return
    	if (index == -1) {
       		return -1; 
    	} else {
    		//iterate through the arrays starting at the found index
    		for (int i = index; i < (object->currentSize - 1); i++) {
    			//copy the next element to the current element
    			strcpy(object->keys[i], object->keys[i + 1]); 
    			strcpy(object->values[i], object->values[i + 1]); 
    		}
    		//decremenet the currentSize
    		object->currentSize -= 1;
    		return 0;
    	}    	
}

//public function
//gets the hash map's current size
int getNumOfElements (HashMap *object) {
	return (object->currentSize);
}

//public function
//copy all elements from one hash map to the other
void duplicateHashMap (HashMap *dest, HashMap *src) {
	dest->currentSize = src->currentSize;
	for (int i = 0; i < src->currentSize; i++) {
		strcpy(dest->keys[i], src->keys[i]); 
    		strcpy(dest->values[i], src->values[i]); 
	}
}

//public function
//prints all elements in the hash map
void printHashMap (HashMap *object) {
	for (int i = 0; i < object->currentSize; i++) {
		printf("%s='%s'\n", object->keys[i], object->values[i]);
	}
}

//public function
//prints all elements in the hash map to the file
void printHashMapToFile (HashMap *object, FILE *file) {
	for (int i = 0; i < object->currentSize; i++) {
		fprintf(file, "%s='%s'\n", object->keys[i], object->values[i]);
	}
}

//public function
//prints all keys in the hash map to the file
void printHashMapKeysToFile (HashMap *object, FILE *file) {
	for (int i = 0; i < object->currentSize; i++) {
		fprintf(file, "%s\n", object->keys[i]);
	}
}

//public function
//prints all elements in the hash map that are not the second hash map
int printHashMapDifference (HashMap *object, HashMap *comparisonObject) {
	int numOfPrintedEntries = 0;
	for (int i = 0; i < object->currentSize; i++) {
		if (getIndexOfKey(comparisonObject, object->keys[i]) == -1) {
			printf("%s='%s'\n", object->keys[i], object->values[i]);
			numOfPrintedEntries += 1;
		}
	}
	return numOfPrintedEntries;
}
