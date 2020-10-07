/*
 * =====================================================================================
 *
 *        Filename:  mld.h
 *
 *        Description: This file Provides definations to use Memory Leak Detector library
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


#ifndef __MLD__
#define __MLD__

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ANSI COLOR code MACRO */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_STRUCTURE_NAME_SIZE 100
#define MAX_FIELD_NAME_SIZE 100

/* Data Type enum for Structure field type */
typedef enum
{
    UINT8,
    UINT32,
    INT32,
    CHAR,
    OBJ_PTR,
    VOID_PTR,
    FLOAT,
    DOUBLE,
    OBJ_STRUCT
} data_type_t;

/* enum for bool data type */
typedef enum
{
    MLD_FALSE,
    MLD_TRUE
} mld_boolean_t;

/* macro to find structure field offset */
#define OFFSETOF(struct_name, fld_name) (unsigned long)&(((struct_name *)0)->fld_name)

/* macro to find structure individual field size */
#define FIELD_SIZE(struct_name, fld_name) sizeof(((struct_name *)0)->fld_name)

/* structure to store information of one field of a C structure */
typedef struct field_info
{
    char feldname[MAX_FIELD_NAME_SIZE]; // Name of the field
    data_type_t datatype;               // Data type of the field
    unsigned int field_size;            // Size of the field
    unsigned int offset;                // Offset of the field
    // Below field is meaningful only if dtype = OBJ_PTR, Or OBJ_STRUCT
    char stuct_ptr_name[MAX_STRUCTURE_NAME_SIZE]; // Name of type of OBJ_PTR or OBJ_STRUCT pointer
} field_info_t;

/* structure to store the information of one C structure which could have nubmer of fields */
typedef struct struct_db_rec
{
    struct struct_db_rec *next;                // Pointer to the next structure in the linked list
    char struct_name[MAX_STRUCTURE_NAME_SIZE]; // Structure name
    unsigned int struct_size;                  // Size of the structure
    unsigned int n_fields;                     // No of fields in the structure
    field_info_t *fields;                      // Pointer to the array of fields
} struct_db_rec_t;

/* structure database to hold information for all register structure */
typedef struct struct_db
{
    struct_db_rec_t *head;                     // Pointer to the head of registered structure linked list
    unsigned int count;                        // Number of record in registured structure linked list
} struct_db_t;

/* Print one registered strcuture record */
void print_structure_rec(struct_db_rec_t *struct_rec);

/* Print complete structure database */
void print_structure_db(struct_db_t *struct_db);

/* add structure to front of structure database linekd list */
int add_structure_to_struct_db(struct_db_t *struct_db, struct_db_rec_t *struct_rec);


/* Macro to register one structure field info Strcucture record */
#define FIELD_INFO(struct_name, fld_name, datatype, stuct_ptr_name)             \
    {                                                                           \
        #fld_name, datatype, FIELD_SIZE(struct_name, fld_name),                 \
        OFFSETOF(struct_name, fld_name), #stuct_ptr_name                        \
    }

/* Macro to Register structure into Structure Database */
#define REG_STRUCT(struct_db, st_name, fields_arr)                              \
    do                                                                          \
    {                                                                           \
        struct_db_rec_t *reg_record = calloc(1, sizeof(struct_db_rec_t));       \
        strncpy(reg_record->struct_name, #st_name, MAX_STRUCTURE_NAME_SIZE);    \
        reg_record->struct_size = sizeof(st_name);                              \
        reg_record->n_fields = sizeof(fields_arr) / sizeof(field_info_t);       \
        reg_record->fields = fields_arr;                                        \
        add_structure_to_struct_db(struct_db, reg_record);                      \
                                                                                \
    } while (0);

/* structure to store the information for one dynamicllay allocated object */
typedef struct object_db_rec
{
    struct object_db_rec *next;  // Pointer next object record in linked list
    void *calloc_ptr;            // Used to hold address for dynamically allcoated object
    unsigned int units;          // Used to hold number of Units of an allocated object
    struct_db_rec_t *struct_rec; // Used to point respective registered structure record
    mld_boolean_t is_visited;    // Used for Graph traversal
    mld_boolean_t is_root;       // Used to make object as a Root object
} object_db_rec_t;

/* object databse to hold information for all dynamically allocated object */
typedef struct object_db
{
    struct_db_t *struct_db;     // Pointer to the structure database
    object_db_rec_t *head;      // Pointer to the head of object database linked list
    unsigned int count;         // Number of object in linked list
} object_db_t;

/* Print one object record  */
void print_object_rec(object_db_rec_t *obj_rec, int i);

/* Print complete object database */
void print_object_db(object_db_t *object_db);

/* API to malloc the object */
void *xcalloc(object_db_t *object_db, char *struct_name, int units);

/* API to free dynamically allocated object */
void xfree(object_db_t *object_db, void *ptr);

/* APIs to register root objects */
void mld_register_root_object(object_db_t *object_db,
                              void *objptr,
                              char *struct_name,
                              unsigned int units);

/* API to set perticular object which is not created dynamically as a root object */
void set_mld_object_as_global_root(object_db_t *object_db, void *obj_ptr);

/* API to Run MLD */
void run_mld_algorithm(object_db_t *object_db);

/* API to report leaked objects */
void report_leaked_objects(object_db_t *object_db);

/* API to report as weel as free leaked objects */
void report_and_free_leaked_objects(object_db_t *object_db);

/* API to set perticular dynamic object as a root object */
void mld_set_dynamic_object_as_root(object_db_t *object_db, void *obj_ptr);

/* API to initialize primitive datatype support */
void mld_init_primitive_data_type_support(struct_db_t *struct_db);

#endif
