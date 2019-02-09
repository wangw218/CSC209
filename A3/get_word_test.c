#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"


/*A program that tests whether the get_word function in worker.c works.
*Test the given index.txt and filenames.txt file (simpletest/d1)
*/
int main(int argc, char **argv) {
    Node *head = NULL;
    char **filenames = init_filenames();
    char *listfile = "index.txt";
    char *namefile = "filenames.txt";

    read_list(listfile, namefile, &head, filenames);
    Node *front = head;
    while (front != NULL){
    	printf("%s\n", front -> word);
    	FreqRecord *record = get_word(front -> word, head, filenames);
    	print_freq_records(record);
    	free(record); 
    	front = front -> next;
    }
    return 0;
}
