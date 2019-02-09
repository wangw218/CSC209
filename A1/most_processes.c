#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
	if (argc > 2) {
		fprintf(stderr, "USAGE: most_processes [ppid]\n");
		return 1;
	} 

	char s[1024];
	char cur_user[32];
	char max_user[32];
	char last_user[32];
	int max_count = 0;
	int cur_count = 0;
	char ppid[1024];
	int i = 1;
	
	while (fgets(s, 1024, stdin) != NULL){
		if (i == 1){
			sscanf(s, "%s", max_user);
	        sscanf(s, "%s", cur_user);
			sscanf (s, "%*s%*s%s", ppid);
			if ((argc == 2 && strcmp(ppid, argv[1]) == 0) || argc == 1){
				max_count ++;
				cur_count ++;
			}
			
		} else {
			sscanf(s, "%s", cur_user);

			if (strcmp(cur_user, max_user) == 0){
				sscanf (s, "%*s%*s%s", ppid);
				if ((argc == 2 && strcmp(ppid, argv[1]) == 0) || argc == 1){
					max_count ++;
					cur_count ++;
				}

			} else if(strcmp(cur_user, last_user) == 0) {
					sscanf (s, "%*s%*s%s", ppid);
					if ((argc == 2 && strcmp(ppid, argv[1]) == 0) || argc == 1){
						cur_count  ++ ;
					}
					if (cur_count  >= max_count){
						max_count = cur_count ;
						strcpy(max_user, cur_user);
					}
			
			} else {
				sscanf (s, "%*s%*s%s", ppid);
				if ((argc == 2 && strcmp(ppid, argv[1]) == 0) || argc == 1){
					cur_count = 1;
				} else {
					cur_count = 0;
				}
			}	
		}
		i++;
		strcpy(last_user,cur_user);
	}

	if (max_count != 0){
		printf("%s %d\n",max_user, max_count);
	}
	return 0;
}
