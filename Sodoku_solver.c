/*
SODOKU SOLVER! 
Author: Rohan Mehra
Project Start: 7:57pm 14/04/2020 - Karratha
Running Develpment Time: 
D1 - 2.5 hours 
*/ 

#include <stdio.h> 
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h> 


#define FALSE 0
#define TRUE 1 
#define MAX_ITERS 100

typedef struct puzzle_state{
    int solve_state; 
    int puzzle[81]; 
    int poss_length[81]; 
    int possibles[81][9];
} Puzzle; 

int puzzle_reader(Puzzle *p_puz, char *str_puzzle);
int sod_file_parser(char *filename); 
int build_puzzle(char *str_puzzle, Puzzle *p_puz);
int get_block(int *puzzle, int n, int *vector); 
int get_row(int *puzzle, int n, int *vector);  
int get_col(int *puzzle, int n, int *vector); 
int valid_vector(int *vector); 
int print_puzzle(Puzzle *p_puz); 
int print_vector(int *vector);
int solve_puzzle(Puzzle *p_puz); 
int solver_eng1(Puzzle *p_puz);
int poss(int *puzzle, char mode, int elem, int *poss); 
int merge_int_lists(int *list1, int *list2, int *merged_list, int length1, int length2);
int solve1(Puzzle *p_puz);
int hidden_singles(Puzzle *p_puz, int row, char mode);
static FILE* xfopen(char *filename, char *option);


int main(int argc, char **argv){
    char opt;
    Puzzle puz = { FALSE, {0}, {-1}, {{0}} };

    while( (opt = getopt(argc,argv,"p:,f:"))  !=-1 ) {			//Use getopt to loop through arguments
		if(opt == 'p') puzzle_reader(&puz, optarg); //puzzle
		else if(opt == 'f') sod_file_parser(optarg); //file 
    }

    solve_puzzle(&puz);

    return 0; 
}

int puzzle_reader(Puzzle *p_puz, char *str_puzzle){
    build_puzzle(str_puzzle, p_puz);
    print_puzzle(p_puz);

    return 0; 
}

int solve_puzzle(Puzzle *p_puz){
    
    int iters = 0; 

    while(p_puz->solve_state != 1 && iters < MAX_ITERS){
        solver_eng1(p_puz);
        if( p_puz->solve_state == -1 ){
            fprintf(stderr, "Guess required. Iteration:%d\n", iters);
            print_puzzle(p_puz);
            for(int pi = 0; pi < 81; pi++){
                fprintf(stderr, "%d ", p_puz->poss_length[pi]);
            }
            exit(EXIT_FAILURE); 
        }
    }

    return 0; 
}

int build_puzzle(char *str_puzzle, Puzzle *p_puz){
    int vector[9] = {0};
    int digit_counter = 0; 
    
    //Check string input validity 
    if(strlen(str_puzzle) != 81){
        fprintf(stderr,"Incorrect puzzle length\n."); //Check for too long puzzle
        exit(EXIT_FAILURE); 
    } 
    for(int i = 0; i<81; i++){
        if( !isdigit(str_puzzle[i]) && str_puzzle[i] != '.'){ //BUG? SHOULD THIS BE AN OR - WHY DOES THIS WORK ? 
            fprintf(stderr,"Puzzle contains illegal character. Only digits and '.' permitted.\n.Digit:%d",i); //Check for illegal characters
            exit(EXIT_FAILURE);
        }
        if( isdigit(str_puzzle[i]) ){
            digit_counter++; 
            p_puz->puzzle[i] = str_puzzle[i] - '0'; //Build numeric puzzle from string using ASCII offset
        }
        else if(str_puzzle[i] == '.'){
            p_puz->puzzle[i] = 0; 
        }
        if(p_puz->puzzle[i] <0 || p_puz->puzzle[i]>9){
            fprintf(stderr, "Invalid input - value not in range [0,9]\n"); 
            exit(EXIT_FAILURE); 
        }        
    }
    if( digit_counter <17 ){
        fprintf(stderr,"Puzzle does not contain enough starting values for unique solution. Minimum 17\n."); //Check for illegal characters
        exit(EXIT_FAILURE);
    }
    
    //Check for duplicates
    for(int i=0; i<9; i++){ 
        get_block(p_puz->puzzle, i, vector);
        if( !valid_vector(vector) ){
            fprintf(stderr, "Invalid Block: %d (duplicates)", i); 
            exit(EXIT_FAILURE); 
        }
        get_row(p_puz->puzzle, i, vector); 
        if( !valid_vector(vector) ){
            fprintf(stderr, "Invalid Row: %d (duplicates)", i); 
            exit(EXIT_FAILURE); 
        }
        get_col(p_puz->puzzle, i, vector);
        if( !valid_vector(vector) ){
            fprintf(stderr, "Invalid Col:%d (duplicates)", i); 
            exit(EXIT_FAILURE); 
        }
    }

     return 0; 
}

int solver_eng1(Puzzle *p_puz){ //basic engine which solves it like pleb brute force
    int row_poss[9] = {0}; 
    int col_poss[9] = {0}; 
    int block_poss[9]= {0}; 
    int iterations = 0;
    int row_poss_len = 0; 
    int col_poss_len = 0; 
    int block_poss_len = 0;
    int temp_merge[9] = {0};
    int merge_len = 0;  
    
    while( !p_puz->solve_state && iterations < MAX_ITERS ){
        for(int i=0; i <81; i++){
            if(p_puz->puzzle[i] == 0){
                row_poss_len = poss(p_puz->puzzle, 'r', i, row_poss);    //Allowable in the row 
                col_poss_len = poss(p_puz->puzzle, 'c', i, col_poss);    //Allowable in the row 
                block_poss_len = poss(p_puz->puzzle, 'b', i, block_poss);  //Allowable in the row
                if(row_poss_len == 0 || col_poss_len == 0 || block_poss_len == 0){
                    return FALSE; // Puzzle failed to solve
                }
                memset(temp_merge, 0, sizeof temp_merge);
                merge_len = merge_int_lists(row_poss, col_poss, temp_merge, row_poss_len, col_poss_len);
                p_puz->poss_length[i] = merge_int_lists(temp_merge, block_poss, &p_puz->possibles[i][0], merge_len, block_poss_len);
                //I now have a list of valid possibilities for each empty 
            }
        }
        solve1(p_puz);  //Solve what you can uniquely. If you can't guess and try and solve uniquely. Will have to keep track of guesses some how? Maybe save puzzle states as 'backups'
        if(p_puz->solve_state == -1){
            return p_puz->solve_state; //Generate branch. 
        }
        memset(p_puz->possibles, 0, 81*9*sizeof(int)); //Reset the possibilities after each pass.
        memset(p_puz->poss_length, -1, 81*sizeof(int));
        iterations++;
    }
    return p_puz->solve_state;
}

int solve1(Puzzle *p_puz){
    int change_made = FALSE;
    int solved = FALSE;
    int num_zeros  = 0; 

    for(int i = 0; i < 81; i++){
        if(p_puz->poss_length[i] == 1){ // Strictly only one possibility for a particular cell
            p_puz->puzzle[i] = p_puz->possibles[i][0];
            change_made = TRUE; 
        }
        if(p_puz->puzzle[i] == 0){
            num_zeros++; 
        }
    }

    for(int group = 0; group < 9; group++){
        if( hidden_singles(p_puz, group, 'r') )  change_made++;
        else if( hidden_singles(p_puz, group, 'c') ) change_made++;
        else if( hidden_singles(p_puz, group, 'b') ) change_made++; 
    }

//Check if solved. 
    if(num_zeros == 0){
        printf("\n Puzzle Solved:\n"); 
        print_puzzle(p_puz); 
        p_puz->solve_state = TRUE;
        exit(EXIT_SUCCESS); 
    }

    if(change_made == FALSE && solved == FALSE){
        p_puz->solve_state = -1;
        return -1; //Branch needed  
    }
    else{
        p_puz->solve_state = FALSE; 
        return 0; 
    } 
}

int poss(int *puzzle, char mode, int elem, int *poss){
    int vector[9] = {0};
    int contains = FALSE;
    int poss_index = 0; 

    if(mode == 'r') get_row(puzzle, elem/9, vector); 
    if(mode == 'c') get_col(puzzle, elem%9, vector);
    if(mode == 'b') get_block(puzzle, (((elem/9) / 3) * 3 + ((elem%9) / 3)),vector); //Hack math from Math SE

    for(int i=1; i<10; i++){
        contains = FALSE;
        for(int k=0; k<9; k++){
            if(vector[k] == i)  contains = TRUE; 
        }
        if(contains == FALSE){
            poss[poss_index] = i;
            poss_index++;
        }
    }

    return poss_index; //How many elemnts were filled 
}

int get_block(int *puzzle, int n, int *vector){
    int puz_inds[9] = {0,3,6,27,30,33,54,57,60}; 
    int puz_start = puz_inds[n];
    
    if(n > 8){
        fprintf(stderr, "Requesting block >8\n");
        exit(EXIT_FAILURE);
    }
    for(int i=0; i<9; i++){
        if (i < 3) vector[i] = puzzle[puz_start+i];
        if (i>=3 && i < 6) vector[i] = puzzle[puz_start+9+i%3];
        if (i>=6 && i < 9) vector[i] = puzzle[puz_start+18+i%3];
    }
    return 0; 
}

int get_row(int *puzzle, int n, int *vector){
    int k = 0; 
    if(n > 8){
        fprintf(stderr, "Requesting row >8\n");
        exit(EXIT_FAILURE);
    }
    for(int i=n*9; i<9*(n+1); i++){
        vector[k] = puzzle[i];
        k++; 
    }
    return 0; 
}

int get_col(int *puzzle, int n, int *vector){
    int k = 0; 
    if(n > 8){
        fprintf(stderr, "Requesting col >8\n");
        exit(EXIT_FAILURE);
    }
    for(int i=n; i<81; i=i+9){
        vector[k] = puzzle[i];
        k++;
    }
    return 0; 
}

//Return FALSE if INVALID, Returns TRUE if VALID, Returns 2 if Complete. 
int valid_vector(int *vector){
    int freq = 0;
    int zero_frq = 0; 
    for(int i=1; i<10; i++){ //Don't care how many 0's we have, need to check for 9s too!  
        freq = 0; 
        for(int k=0; k<9; k++){ 
            if( vector[k] == i) freq++; 
            if ( i==9 && vector[k] == 0 ) zero_frq++; //Only sum zeros on the last iteration 
        }
        if( freq > 1 ) return FALSE; // We have duplicates 
    }
    if( zero_frq == 0 ) return 2; //We have no 0s and no duplicates. A complete row 
    
    return 1; // Valid, incomplete row. 
}

int print_puzzle(Puzzle *p_puz){
    int vector[9] = {0};
    printf("\n  ===================\n");
    for(int i=0; i<9; i++){ 
        if(i == 3 || i == 6) printf("||-------------------||\n");
        printf("||");
        get_row(p_puz->puzzle, i, vector);
        for(int k=0; k<9; k++){
            if(k == 3 || k == 6) printf("|");
            if(k != 8) printf("%d ", vector[k]);
            else printf("%d", vector[k]);
        }
        printf("||\n");
    }
    printf("  ===================\n"); 
    return 0; 
}

int print_vector(int *vector){
    printf("\n Vector: "); 
    for(int i=0; i<9; i++){
        printf("%d ", vector[i]);
    }
    return 0; 
}

int sod_file_parser(char *filename){
    FILE *sdk_file = xfopen(filename, "r");
    fclose(sdk_file);
    return 0; 
}

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

int merge_int_lists(int *list1, int *list2, int *merged_list, int length1, int length2){
    //assumes no repeats and that the lists are sorted
    int i = 0;
    int j = 0;
    int k = 0; 

    while (j < length2){
        for(i = 0; i < length1; i++){
            if(list1[i]==list2[j]){
                merged_list[k] = list1[i];
                k++;
                break; 
            }
        }
        j++;
    }
    return k; //length of merged list 
}

int hidden_singles(Puzzle *p_puz, int group_indx, char mode){
    int j = 0;
    int poss_counter[9] = {0}; 
    int poss_first_indx[9] = {-1};
    int change_made = FALSE;
    int vector_index[9] = {0};

    if(mode == 'r'){
        for(int i = 0; i < 9; i++){
            vector_index[i] = group_indx*9+i;
        }
    }

    if(mode == 'c'){
        int l = 0; 
        for(int i = group_indx; i<81; i=i+9){
            vector_index[l] = i;
            l++;
        }
    }

    if(mode == 'b'){
        int puz_inds[9] = {0,3,6,27,30,33,54,57,60}; 
        int puz_start = puz_inds[group_indx];
        for(int i=0; i<9; i++){
            if (i < 3) vector_index[i] = puz_start+i;
            if (i>=3 && i < 6) vector_index[i] = puz_start+9+i%3;
            if (i>=6 && i < 9) vector_index[i] = puz_start+18+i%3;
        }
    }

    for(int i = 0; i < 9; i++){
        j=0;
        while( p_puz->possibles[vector_index[i]][j] != 0 ){
            for(int k = 1; k < 10; k++){
                if(p_puz->possibles[vector_index[i]][j] == k){
                    poss_counter[k-1]++;
                    poss_first_indx[k-1] = vector_index[i];
                    break;
                }
            }
            j++;
        }         
    }

    for(int x = 0; x < 9; x++){
        if(poss_counter[x] == 1){
            p_puz->puzzle[poss_first_indx[x]] = x+1; //Array indexed 0, but sodoku starts 1 
            change_made = TRUE; 
        }
    }

    return change_made; 
}

static FILE* xfopen(char *filename, char *option){ 
	FILE *retp = fopen(filename, option); 
	if( retp == NULL ){ 
		fprintf(stderr,"File %s failed to open. \n",filename); 
		exit(EXIT_FAILURE); 
	}
	return retp;
}