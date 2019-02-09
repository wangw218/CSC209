#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* Return an array of FreqRecord elements for the word. The freq value
* would be 0 and the filename would be an empty string if it is the last
* record or the word is not found in the head linked list.
*/
FreqRecord *get_word(char *word, Node *head, char **file_names) {
	FreqRecord *first; // the FreqRecord array
	Node *prev = head;
	Node *node_wanted; // the node with the word
	int found = 0; //whether the word in the list
	while (prev != NULL){
		if (strcmp(prev -> word, word) == 0){
			//find the word in the word list array
			found = 1;
			node_wanted = prev;
		}
		prev = prev -> next;
	}

	if (found == 0){// the word not in the list
		if ((first = malloc(sizeof(FreqRecord))) == NULL){
			perror("get_word: FreqRecord");
			exit(1);
		}
		first[0].freq = 0;
		first[0].filename[0] = '\0';
		return  first;
	}

	// the word is in the list
	//detemine the size of the FreqRecord array
	int size = 0;
	while(file_names[size] != NULL){
		// there is a file in the file_names
		size ++;
	}
	
	if ((first = malloc(sizeof(FreqRecord) * (size + 1))) == NULL){
		perror("get_word: FreqRecord");
		exit(1);
	}

	int i = 0; // the index of the FreqRecord array
	int f_index = 0; // the index of the freq and filenames
	while (f_index < size){
		if ((node_wanted -> freq)[f_index] != 0){
			//fileter the freq with 0 filename
			first[i].freq = (node_wanted -> freq)[f_index];
			strcpy(first[i].filename, file_names[f_index]); 
			i ++;
		}
		f_index ++; 
	}
	//the last record
	first[i].freq = 0;
	first[i].filename[0] = '\0'; 

    return first;
}

/* Print to standard output the frequency records for a word.
* Use this for your own testing and also for query.c
*/
void print_freq_records(FreqRecord *frp) {
    int i = 0;

    while (frp != NULL && frp[i].freq != 0) {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

/* Read the words from in until it ends. For each word read,look for the word 
* in the index and filenames of the subdictionary (if it has) of dirname and 
* send the non-zero frequency of the word to out.
*/
void run_worker(char *dirname, int in, int out) {
	Node *head = NULL;
	char **filenames = init_filenames();
	
	char listfile[PATHLENGTH] ;
	strcpy(listfile, dirname);
	strcat(listfile, "/index");  
    
    char namefile[PATHLENGTH];
    strcpy(namefile, dirname);
    strcat(namefile, "/filenames");  

	//load the index into the data structure head and construct a filenames array
	read_list(listfile, namefile, &head, filenames);
	
	//read a word from in
	char word[MAXWORD];
	memset(word, '\0',MAXWORD);
	while (read(in, word, MAXWORD) > 0){
		// look for word in the head
		word[strlen(word) - 1] = '\0';
		FreqRecord *first = get_word(word, head, filenames);

		int i = 0;
		while (first[i].freq != 0){
			if(write(out, &(first[i]), sizeof(FreqRecord)) == -1){
				perror("write in worker");
				exit(1);
			};
			i ++;
			
		}
		//white the end of FreqArray
		if(write(out, &(first[i]), sizeof(FreqRecord)) == -1){
			perror("wirte in worker");
			exit(1);
		}; 
		memset(word, '\0', MAXWORD);
	}
}

// A help function that helps to sort the Master Frequency array
void array_sort(int size, FreqRecord first[]){
	for (int i = 0; i < size; i++){
		for (int j = i + 1; j < size; j++){
			if (first[i].freq < first[j].freq){
				FreqRecord a = first[i];
				first[i] = first[j];
				first[j] = a;
			}
		}
	}  
}

 
