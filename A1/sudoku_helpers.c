#include <stdio.h>

/* Each of the n elements of array elements, is the address of an
 * array of n integers.
 * Return 0 if every integer is between 1 and n^2 and all
 * n^2 integers are unique, otherwise return 1.
 */
int check_group(int **elements, int n) {
    // TODO: replace this return statement with a real function body
    int num = n * n;
    int arr[num];
	int count = 0;	
    int value = 0;
    for (int i = 0; i < n; i++) {
		for (int j = 0 ; j < n; j++) {
			arr[count] = elements[i][j];
			count++; 
			if (elements[i][j] < 1 || elements[i][j] > num){
				value = 1;
			}
		}
    }
    for (int i = 0; i < num; i++){
		for (int j = i + 1; j < num ; j++){
			if (arr[i] == arr[j]){
				value = 1;				
			}
		}	
    }
   return value;
}

/* puzzle is a 9x9 sudoku, represented as a 1D array of 9 pointers
 * each of which points to a 1D array of 9 integers.
 * Return 0 if puzzle is a valid sudoku and 1 otherwise. You must
 * only use check_group to determine this by calling it on
 * each row, each column and each of the 9 inner 3x3 squares
 */
int check_regular_sudoku(int **puzzle) {

    // TODO: replace this return statement with a real function body
    int value = 0;

    // check the row
    for (int i = 0; i < 9; i++){
    	int first[3] = {puzzle[i][0], puzzle[i][1], puzzle[i][2]};
    	int second[3] = {puzzle[i][3], puzzle[i][4], puzzle[i][5]};
    	int third[3] = {puzzle[i][6], puzzle[i][7], puzzle[i][8]};
    	int *row[3] = {first, second, third};
    	int h = check_group(row, 3);
    	if (h == 1) {
    		value = 1;
    	}
    }

    // check the column
    for (int i = 0; i < 9; i++){
    	int first[3] = {puzzle[0][i], puzzle[1][i], puzzle[2][i]};
    	int second[3] = {puzzle[3][i], puzzle[4][i], puzzle[5][i]};
    	int third[3] = {puzzle[6][i], puzzle[7][i], puzzle[8][i]};
    	int *column[3] = {first, second, third};
    	int h = check_group(column, 3);
    	if (h == 1) {
    		value = 1;
    	}
    }

    // check for inner box
    for (int i = 0; i < 9; i += 3){
    	for (int j = 0; j < 9; j += 3){
    		int first[3] = {puzzle[i][j], puzzle[i][j + 1], puzzle[i][j + 2]};
    		int second[3] = {puzzle[i + 1][j], puzzle[i + 1][j + 1], puzzle[i + 1][j + 2]};
    		int third[3] = {puzzle[i + 2][j], puzzle[i + 2][j + 1], puzzle[i + 2][j + 2]};
    		int *inner_box[3] = {first, second, third};
    		int h = check_group(inner_box, 3);
	    	if (h == 1) {
	    		value = 1;
	    	}
    	}
    }
    return value;
}
