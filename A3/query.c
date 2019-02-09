#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"


/* A program to model calling run_worker and to test it. Notice that run_worker
 * produces binary output, so the output from this program to STDOUT will 
 * not be human readable.  You will need to work out how to save it and view 
 * it (or process it) so that you can confirm that your run_worker 
 * is working properly.
 */
int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";
    struct dirent *dp;
    
    int fd1[MAXWORKERS][2];  //for transfer words
    int fd2[MAXWORKERS][2];  //for transfer FreqRecord
    
    
    int i = -1; // index of children
    
    int master_or_not = 1; // whether or not is master, 1 means true
    FreqRecord mfa[MAXRECORDS];  //Master Frequency Array
    FreqRecord print[MAXRECORDS + 1];  //array we want to print

    /* this models using getopt to process command-line flags and arguments */
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }

    // Open the directory provided by the user (or current working directory)
    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    }

    /* For each entry in the directory, eliminate . and .., and check
     * to make sure that the entry is a directory, then call run_worker
     * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
     * sequentially through the directories, and will expect to read
     * a word from standard input for each index it checks.
     */
   
    
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0 ||
            strcmp(dp->d_name, ".git") == 0) {
                continue;
        }

        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path));
        strncat(path, dp->d_name, PATHLENGTH - strlen(path));
        path[PATHLENGTH - 1] = '\0';

        struct stat sbuf;
        if (stat(path, &sbuf) == -1) {
            // This should only fail if we got the path wrong
            // or we don't have permissions on this entry.
            perror("stat");
            exit(1);
        }

        // Only call run_worker if it is a directory
        // Otherwise ignore it.
        if (S_ISDIR(sbuf.st_mode)) {
            if (master_or_not > 0){ // parent process
                i = i + 1;
                if (pipe(fd1[i]) == -1) {  //create pipe
                    perror("pipe");
                }
                if (pipe(fd2[i]) == -1) {  //create pipe
                    perror("pipe");
                }
                master_or_not = fork();  //one worker(only parent create worker)
            }
            if (master_or_not < 0) {
                perror("fork");
                exit(1);
            } 
            if (master_or_not == 0) {// child process
            	//close useless pipe
            	if (close(fd1[i][1]) == -1) {
      				perror("close: child words write");
      				exit(1);
    			} 
    			if (close(fd2[i][0]) == -1) {
      				perror("close: child FreqRecord read");
      				exit(1);
    			} 
               	//read a words from master and write FreqRecord to master
                run_worker(path, fd1[i][0], fd2[i][1]);
                
				//the child finish working, close pipe		
				if (close(fd1[i][0]) == -1) {
      				perror("close: child words read");
      				exit(1);
    			} 
    			if (close(fd2[i][1]) == -1) {
      				perror("close: child FreqRecord write");
      				exit(1);
    			} 
      		          
                exit(0); //child exit      
            } 
        }          
    }

    if (closedir(dirp) < 0){
        perror("closedir");
        exit(1);
    }
	
	
	//master process
	
	// close useless pipe
    for (int h = 0; h <= i; h++){
       	if (close(fd2[h][1]) == -1) {//close pipe for FreqRecord write
      		perror("close: master FreqRecord write");
      		exit(1);
    	} 
		if (close(fd1[h][0]) == -1) {//close pipe for words read
  			perror("close: master words read");
  			exit(1);
		}  
    }
    
	//read words form stdin
    char word[MAXWORD];
    memset(word, '\0', MAXWORD);
    
    while ((fgets(word, MAXWORD, stdin)) != NULL) {
     
     	for (int m = 0; m <= i; m ++){//write the word to each worker process
           
            if (write(fd1[m][1], word, MAXWORD) == -1) {
                perror("write to pipe");
                exit(1);           
            }
        }
        
        FreqRecord record;
        int num = 0;
        // read one FreqRecord form a worker and add to master frequency array
	    for (int h = 0; h <= i; h++){	  
	    	while ((read(fd2[h][0], &record, sizeof(FreqRecord)) > 0) && (record.freq != 0) ){
				
	    		if (num < MAXRECORDS ){
	    			mfa[num] = record;
	    			num ++;
	    			array_sort(num,mfa);   //sort array			
	    		} else if (num == MAXRECORDS){//Master Frequency Array full
	    			if (record.freq > mfa[num - 1].freq){
	    				mfa[num - 1] = record;
	    			}
	    			array_sort(num, mfa); //sort array
	    		}
	    		
	    	}
	    }
	    
	    // Constuct the array to print according to the Master Frequency Array
	    for (int h = 0; h < num; h ++){
	    	print[h] = mfa[h];
	    }
	    
	    print[num].freq = 0;
	    print[num].filename[0] = '\0'; 
	    
	    //print the array
        print_freq_records(print);
        
        
        // a new word begin, make sure the element is new
        //memset(word, '\0',MAXWORD);
        for (int h = 0; h < MAXWORD; h ++){
	    	mfa[h].freq = 0;
	    	mfa[h].filename[0] = '\0'; 
	    }        
    }
    
    //stdin has been closed
    //close the opening pipe connected with master
    for (int m = 0; m <= i; m++){
    	if(close(fd1[m][1]) == -1){// close pipe for words write
		 	perror("close:master words write");
		 	exit(1);
     	}
     	if (close(fd2[m][0]) == -1){//close pipe for FreqRecord read
     		perror("close:master FreqRecord read");
     		exit(1);
     }
    }
    
    
     // wait for childen to terminate
     for (int m = 0; m <= i; m++) {
		 wait(NULL);
	 }
	 
    return 0;

}



















