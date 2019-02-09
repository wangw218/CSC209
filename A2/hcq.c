#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hcq.h"
#define INPUT_BUFFER_SIZE 256

/*
 * Return a pointer to the struct student with name stu_name
 * or NULL if no student with this name exists in the stu_list
 */
Student *find_student(Student *stu_list, char *student_name) {
    Student *front = stu_list;
    while (front != NULL){
        if (strcmp(front -> name, student_name) == 0){
            return front;
        }
        front = front ->next_overall;
    }
    return NULL;
}



/*   Return a pointer to the ta with name ta_name or NULL
 *   if no such TA exists in ta_list. 
 */
Ta *find_ta(Ta *ta_list, char *ta_name) {
    Ta *front = ta_list;
    while (front != NULL){
        if (strcmp(front -> name, ta_name) == 0){
            return front;
        }
        front = front ->next;
    }
    return NULL;

}


/*  Return a pointer to the course with this code in the course list
 *  or NULL if there is no course in the list with this code.
 */
Course *find_course(Course *courses, int num_courses, char *course_code) {
    Course *course;
    for (int i = 0; i < num_courses; i ++){
        if (strcmp(courses[i].code, course_code) == 0){
            course = &(courses[i]);
            return course;
        }
    }
    return NULL;
}
    

/* Add a student to the queue with student_name and a question about course_code.
 * if a student with this name already has a question in the queue (for any
   course), return 1 and do not create the student.
 * If course_code does not exist in the list, return 2 and do not create
 * the student struct.
 * For the purposes of this assignment, don't check anything about the 
 * uniqueness of the name. 
 */
int add_student(Student **stu_list_ptr, char *student_name, char *course_code,
    Course *course_array, int num_courses) {

    
    if (find_student(*stu_list_ptr,student_name) != NULL){
    	// student with this name in the queue
        return 1;
    } else if (find_course(course_array,num_courses, course_code) == NULL){
    	//the course_code does not exist in the list
        return 2;
    } else {
        // construct the student
        Student *new_stu = malloc(sizeof(Student));
        if (new_stu == NULL) {
            perror("malloc for Student");
            exit(1);
        }
        new_stu -> name = malloc(strlen(student_name) + 1);
        if (new_stu->name  == NULL) {
            perror("malloc for Student name");
            exit(1);
        }
        strcpy(new_stu -> name, student_name); 
        Course *course  = find_course(course_array,num_courses, course_code);
        new_stu -> course = course;
        // arrive time
        new_stu -> arrival_time = malloc(sizeof(time_t));
        time(new_stu -> arrival_time);
        
        // add the student to the queue
        if (*stu_list_ptr == NULL){
        	// no element is the list
            *stu_list_ptr = new_stu;
        } else { //already has element
            Student *before;
            Student *front = *stu_list_ptr;
            while (front != NULL){
                before = front;
                front = front -> next_overall;
            }
            before -> next_overall = new_stu;
        
        }  
        // add student to course and modity the next_course
        if (course -> head == NULL){
        	// no student in the course
            course -> head = new_stu;
            course -> tail = new_stu;
        } else {
            Student *student = course -> head;
            Student *back;
            while (student != NULL){
                back = student;
                student = student -> next_course;
            }
            back -> next_course = new_stu;
            course -> tail = new_stu;
        }
    }
    return 0;
}

//Return the number of sudents in stu_list
int student_num(Student *stu_list){
    int student_num = 0;
    if (stu_list == NULL){
        return 0;
    } else {
        Student *front = stu_list;
        while (front != NULL){
            front =  front -> next_overall;
            student_num ++;
        }
        return student_num;
    }    
}

//Remove the stduent from the stu_list assume the student must in the stu_list
void remove_student(Student **stu_list_ptr, Student *student){
    if (strcmp((*stu_list_ptr) -> name, student -> name) == 0){
         //the student is the first one in the stu_list
        *stu_list_ptr = (*stu_list_ptr) -> next_overall;
    } else {
        Student *back; // record the former student of the student we want to remove
        Student *front = *stu_list_ptr;
        while (front != NULL){
            if (front -> next_overall != NULL && strcmp(front -> next_overall -> name, student -> name) == 0){
                back = front;
            }
            front = front ->next_overall;          
        }
        back -> next_overall = back -> next_overall -> next_overall;  //remove the student          
    }
    student -> next_overall = NULL;
    student -> next_course = NULL;
    // record the waiting time
    student -> course -> wait_time = student -> course -> wait_time + difftime(time(NULL), *(student-> arrival_time));
}
    
//Free the location of the student.
void free_student(Student *stu){
    // the student is done. No course.
    stu -> course = NULL;

    // free the memory
    free(stu -> name);
    free(stu -> arrival_time);
    free(stu);
}

/* Student student_name has given up waiting and left the help centre
 * before being called by a Ta. Record the appropriate statistics, remove
 * the student from the queues and clean up any no-longer-needed memory.
 *
 * If there is no student by this name in the stu_list, return 1.
 */
int give_up_waiting(Student **stu_list_ptr, char *student_name) {
    if (find_student(*stu_list_ptr, student_name) == NULL){
        return 1;
    } else {
        Student *stu = find_student(*stu_list_ptr, student_name);
        Course *course = stu -> course;
        // record the number of students who gave up
        course -> bailed += 1;
         
         //remove the student from the stu_list
        remove_student(stu_list_ptr, stu);
        
        // free the memory
        free_student(stu);

    }
    return 0;
}

/* Create and prepend Ta with ta_name to the head of ta_list. 
 * For the purposes of this assignment, assume that ta_name is unique
 * to the help centre and don't check it.
 */
void add_ta(Ta **ta_list_ptr, char *ta_name) {
    // first create the new Ta struct and populate
    Ta *new_ta = malloc(sizeof(Ta));
    if (new_ta == NULL) {
       perror("malloc for TA");
       exit(1);
    }
    new_ta->name = malloc(strlen(ta_name)+1);
    if (new_ta->name  == NULL) {
       perror("malloc for TA name");
       exit(1);
    }
    strcpy(new_ta->name, ta_name);
    new_ta->current_student = NULL;


    // insert into front of list
    new_ta->next = *ta_list_ptr;
    *ta_list_ptr = new_ta;
}


/* The TA ta is done with their current student. 
 * Calculate the stats (the times etc.) and then 
 * free the memory for the student. 
 * If the TA has no current student, do nothing.
 */
void release_current_student(Ta *ta) {
    if (ta -> current_student != NULL){
    
        Student *stu = ta -> current_student;
        // record the stats
        Course *course = stu -> course;
        course -> help_time = course -> help_time + difftime(time(NULL), *(stu -> arrival_time));
        (course -> helped) += 1;

        ta -> current_student = NULL;
        //free the student
        free_student(stu);
    }

}

/* Remove this Ta from the ta_list and free the associated memory with
 * both the Ta we are removing and the current student (if any).
 * Return 0 on success or 1 if this ta_name is not found in the list
 */
int remove_ta(Ta **ta_list_ptr, char *ta_name) {
    Ta *head = *ta_list_ptr;
    if (head == NULL) {
        return 1;
    } else if (strcmp(head->name, ta_name) == 0) {
        // TA is at the head so special case
        *ta_list_ptr = head->next;
        release_current_student(head);
        // memory for the student has been freed. Now free memory for the TA.
        free(head->name);
        free(head);
        return 0;
    }
    while (head->next != NULL) {
        if (strcmp(head->next->name, ta_name) == 0) {
            Ta *ta_tofree = head->next;
            //  We have found the ta to remove, but before we do that 
            //  we need to finish with the student and free the student.
            //  You need to complete this helper function
            release_current_student(ta_tofree);

            head->next = head->next->next;
            // memory for the student has been freed. Now free memory for the TA.
            free(ta_tofree->name);
            free(ta_tofree);
            return 0;
        }
        head = head->next;
    }
    // if we reach here, the ta_name was not in the list
    return 1;
}


//One student is removed in the course, need to change the  course head and tail 
void change_course(Course *course){
    if (course -> head != NULL){
        // there is a student in the course
        if (strcmp(course -> head -> name, course -> tail -> name) == 0) {
            //only one student in the course, special case
            course -> head = NULL;
            course -> tail = NULL; 
        } else {
            course -> head = course -> head -> next_course;
        }
    }    
}



/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the full queue. 
 * If the queue is empty, then TA ta_name simply finishes with the student 
 * they are currently helping, records appropriate statistics, 
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 */
int take_next_overall(char *ta_name, Ta *ta_list, Student **stu_list_ptr) {
    if (find_ta(ta_list,ta_name) == NULL){
        return 1;
    }
    
    // Ta in the list
    Ta *ta = find_ta(ta_list, ta_name);
    if (ta ->current_student != NULL){
        //remove the current_student currently being helped by the Ta
        release_current_student(ta); //record the helped stats

    } 
    
    // change the queue and assign next_student to the ta if there is a student
    if (*stu_list_ptr != NULL ){
        // change next_student course's head and tail
        Course *course = (*stu_list_ptr) -> course;
        change_course(course);
        
        // remove the next_student from the stu_list
        Student *next_student = *stu_list_ptr;
        remove_student(stu_list_ptr, next_student);
        
        //change the next_student's arrival_time since it is not in the queue
        time(next_student -> arrival_time);

        // assign next_student to the ta
        ta -> current_student = next_student;
       
    } 
    return 0;
}



/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the course with this course_code. 
 * If no student is waiting for this course, then TA ta_name simply finishes 
 * with the student they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 * If course is invalid return 2, but finish with any current student. 
 */
int take_next_course(char *ta_name, Ta *ta_list, Student **stu_list_ptr, char *course_code, Course *courses, int num_courses) {
    if (find_ta(ta_list,ta_name) == NULL){
        return 1;
    } 
    // Ta in the list
    Ta *ta = find_ta(ta_list, ta_name);
    if (ta ->current_student != NULL){
        //remove the current_student currently being helped by the Ta
        release_current_student(ta); //record the helped stats
    } 
    if (find_course(courses,num_courses, course_code) == NULL){
        return 2;
    }
    // The course
    Course *course = find_course(courses,num_courses, course_code);
    // there is student in the course
    if (course -> head != NULL){
        // the next student
        Student *next_student = course -> head;
        
        // change the course's head and tail
        change_course(course);
        
        // remove the next_student from the stu_list
        remove_student(stu_list_ptr, next_student);
        
        //change the next_student's arrival_time since it is not in the queue
        time(next_student -> arrival_time);

        // assign next_student to the ta
        ta -> current_student = next_student;


    }
    return 0;
}

// Return the nubmer of students waiting the course_code in the queue
int waiting(Student *stu_list, char *course_code){
    int students_waiting  = 0; 
    Student *head = stu_list;
     while (head != NULL){ 
        if (strcmp(head ->course -> code, course_code) == 0) {
            students_waiting ++;
        } 
        head = head -> next_overall;        
    }
    return students_waiting;
}

/* For each course (in the same order as in the config file), print
 * the <course code>: <number of students waiting> "in queue\n" followed by
 * one line per student waiting with the format "\t%s\n" (tab name newline)
 * Uncomment and use the printf statements below. Only change the variable
 * names.
 */
void print_all_queues(Student *stu_list, Course *courses, int num_courses) {
    // loop over each course
	for (int i = 0; i < num_courses; i ++){
		Course course = courses[i];  
        printf("%s: %d in queue\n", course.code, waiting(stu_list, course.code));
		
		//loop over to find the student in this course
		Student *head = stu_list;
    	while (head != NULL){
        	if (strcmp(head -> course -> code, course.code) == 0){
            	printf("\t%s\n",head -> name); 
        	}
    	head = head -> next_overall;
     	}
    }                   
}


/*
 * Print to stdout, a list of each TA, who they are serving at from what course
 * Uncomment and use the printf statements 
 */
void print_currently_serving(Ta *ta_list) {
    if (ta_list == NULL){
        printf("No TAs are in the help centre.\n");
    } else {
        Ta *ta = ta_list;
        while (ta != NULL){//loop the ta list
            if (ta -> current_student == NULL){
                printf("TA: %s has no student\n", ta -> name);
            } else {
                printf("TA: %s is serving %s from %s\n",
                ta -> name, ta -> current_student -> name, ta -> current_student -> course ->code);
            }
            ta = ta -> next;
        }
    }  
}


/*  list all students in queue (for testing and debugging)
 *   maybe suggest it is useful for debugging but not included in marking? 
 */ 
void print_full_queue(Student *stu_list) {
     Student *head = stu_list;
     while (head != NULL){    
        printf("\t%s: %s\n",head -> name, head ->course ->code); 
        head = head -> next_overall;        
    }
}



// Return the number of students being helped
int being_helped(Ta *ta_list){
    int being_helped = 0;
    if (ta_list != NULL){
        Ta *current = ta_list;
        while (current != NULL){
            if (current -> current_student != NULL){
                being_helped ++;
            }
            current = current -> next;
        }
    } 
    return being_helped; 
}


/* Prints statistics to stdout for course with this course_code
 * See example output from assignment handout for formatting.
 *
 */
int stats_by_course(Student *stu_list, char *course_code, Course *courses, int num_courses, Ta *ta_list) {

	
    Course *found =  find_course(courses, num_courses, course_code);
	//no a course with such couse_code in the queue
    if (found == NULL){
        return 1;
    }
   	
    int students_waiting = waiting(stu_list, course_code);
    int students_being_helped = being_helped(ta_list); 

    // You MUST not change the following statements or your code 
    //  will fail the testing.   
    printf("%s:%s \n", found->code, found->description);
    printf("\t%d: waiting\n", students_waiting);
    printf("\t%d: being helped currently\n", students_being_helped);
    printf("\t%d: already helped\n", found->helped);
    printf("\t%d: gave_up\n", found->bailed);
    printf("\t%f: total time waiting\n", found->wait_time);
    printf("\t%f: total time helping\n", found->help_time);
    return 0;
}


/* Dynamically allocate space for the array course list and populate it
 * according to information in the configuration file config_filename
 * Return the number of courses in the array.
 * If the configuration file can not be opened, call perror() and exit.
 */
int config_course_list(Course **courselist_ptr, char *config_filename) {
    FILE *config;
    config = fopen(config_filename, "r");
    if (config == NULL) {
        perror("Error open input file");
        exit(1);
    }
    
    int num;  // the number of courses
    fscanf(config, "%d", &num);

    *courselist_ptr = malloc(sizeof(Course) * num);

    // initialize the courses according to the input file
    for (int i = 0; i < num; i ++){
        char code[7];
        char *description;
        description = malloc(sizeof(char)* INPUT_BUFFER_SIZE) ;
		
		//read from file
        fscanf(config, "%6s %[^\n]", code, description);
        
        // initialize the courses
        strcpy((*courselist_ptr)[i].code, code);
        (*courselist_ptr)[i].description = description;
        (*courselist_ptr)[i].head = NULL;
        (*courselist_ptr)[i].tail = NULL;       
        (*courselist_ptr)[i].helped = 0;
        (*courselist_ptr)[i].bailed = 0;
        (*courselist_ptr)[i].wait_time = 0;
        (*courselist_ptr)[i].help_time = 0;
    }
   
    int error = fclose(config);
    if (error != 0) {
       fprintf(stderr, "fclose failed\n");
       return 1;
    }
    
    return num;
}


