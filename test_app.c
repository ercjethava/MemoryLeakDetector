/*
 * =====================================================================================
 *
 *        Filename:  test_app.c
 *
 *        Description:  This file shows test application for the usage of 
 *                      Memory Leak Detector library
 *
 *        Version:   1.0
 *        Created:   10/05/2020
 *        Revision:  1.0
 *        Compiler:  gcc
 *
 *        Author:  Er. Chirag Jethava
 *        Email ID: ercjethava@gmail.com
 *        
 *        This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by  
 *        the Free Software Foundation, version 3.
 *
 *        This program is distributed in the hope that it will be useful, but 
 *        WITHOUT ANY WARRANTY; without even the implied warranty of 
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 *        General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License 
 *        along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * =====================================================================================
 */


#include "mld_src/mld.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct employee {
    char employee_name[32];
    char company_name[50];
    char employee_job_title[50]; 
    unsigned int employee_id;
    float salary;
    struct employee* emp_ptr;
} employee_t;

typedef struct student{

    char student_name[32];
    uint32_t student_rollno;
    uint32_t student_total_marks;
    struct student* best_friend;
} student_t;


int main(int argc, char **argv){

    // create structure database structure database */
    struct_db_t *struct_db = calloc(1, sizeof(struct_db_t));

    // initialize primitive data type entry into structure database
    mld_init_primitive_data_type_support(struct_db);
        
    // adding structure record for structure employee_t 
    field_info_t emp_field[] = {
        FIELD_INFO(employee_t, employee_name,        CHAR,       0),
        FIELD_INFO(employee_t, company_name,         CHAR,       0),
        FIELD_INFO(employee_t, employee_job_title,   CHAR,       0),
        FIELD_INFO(employee_t, employee_id,          UINT32,     0),
        FIELD_INFO(employee_t, salary,               FLOAT,      0),
        FIELD_INFO(employee_t, emp_ptr,              OBJ_PTR, employee_t),
    };

    // register employee_t structure into structure database
    REG_STRUCT(struct_db, employee_t, emp_field);

    field_info_t stud_field[] = {
        FIELD_INFO(student_t, student_name,         CHAR,       0),
        FIELD_INFO(student_t, student_rollno,       UINT32,     0),
        FIELD_INFO(student_t, student_total_marks,  UINT32,     0),
        FIELD_INFO(student_t, best_friend,          OBJ_PTR,    student_t),
    };
    REG_STRUCT(struct_db, student_t, stud_field);

    // Display registered structure database
    print_structure_db(struct_db);


    // Initialize Object database 
    object_db_t *object_db = calloc(1, sizeof(object_db_t));
    object_db->struct_db = struct_db;
    
    // allocating memory using MLD API   
    student_t *chirag = xcalloc(object_db, "student_t", 1);
    strncpy(chirag->student_name, "chirag", strlen("chirag"));
    chirag->student_rollno = 20;
    chirag->student_total_marks = 400;

    student_t *prince = xcalloc(object_db, "student_t", 1);
    strncpy(prince->student_name, "prince", strlen("prince"));
    prince->student_rollno = 21;
    prince->student_total_marks = 480;

    employee_t *victor = xcalloc(object_db, "employee_t", 2);
    strncpy(victor->employee_name, "victor", strlen("victor"));
    strncpy(victor->company_name, "XYZ", strlen("XYZ"));
    victor->salary = 65000;
    
    
    // Display registered object database
    print_object_db(object_db);
  
   
    /* Case 1:  If developer forgot to  free all allocated object 
                report_leaked_objects will detect and display all the three allocated object
                chirag, prince , victor */

    /* Case 2:  If developer forgot to  free only one allocated object (ex. prince )
                report_leaked_objects will detect 1 leaked object and display it */
      
    xfree(object_db,chirag);
    xfree(object_db,victor);


    /* Case 3:  If developer freed all allocated object  
                report_leaked_objects will display leaked object as 0 */
    
    //xfree(object_db,chirag);
    //xfree(object_db,prince);
    //xfree(object_db,victor);

    /* run mld algorithm for graph based object leak detection 
       if we have have any tree based strcuture where one object is having reference of other one */

    //run_mld_algorithm(object_db);
  
    /* detect leaked object at the end of program */
    report_leaked_objects(object_db);

    /* detect and free leaked objects at the end of program */
    //report_and_free_leaked_objects(object_db);

    return 0;
}
