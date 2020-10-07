/*
 * =====================================================================================
 *
 *        Filename:  mld.c
 *
 *        Description:  This file implements the functions and routines 
 *                      for Memory Leak Detector library
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

#include <stdio.h>
#include <stdlib.h>
#include "mld.h"
#include <assert.h>
#include <memory.h>

char *DATA_TYPE[] = {"UINT8", "UINT32", "INT32",  "CHAR", "OBJ_PTR", "VOID_PTR", "FLOAT", "DOUBLE"};


void print_structure_rec(struct_db_rec_t *struct_rec)
{
    if (!struct_rec)
        return;
    field_info_t *currunt_field = NULL;
    printf(ANSI_COLOR_BLUE "|-----------------------------------------------------------------------------------|\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_YELLOW "| struct_name = %-20s | struct_size = %-8d | No. of Fields = %-3d |\n" ANSI_COLOR_RESET, struct_rec->struct_name, struct_rec->struct_size, struct_rec->n_fields);
    printf(ANSI_COLOR_BLUE "|-----------------------------------------------------------------------------------|\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BLUE "----------------------------------------------------------------------------------------------------------------------------------------------------|\n" ANSI_COLOR_RESET);
    for (int i = 0; i < struct_rec->n_fields; i++)
    {
        currunt_field = &struct_rec->fields[i];

        printf("| field[%d]   | field_name = %-20s | datatype = %-15s | field_size = %-5d | offset = %-6d|  stuct_ptr_name = %-10s  |\n",
               i, currunt_field->feldname, DATA_TYPE[currunt_field->datatype], currunt_field->field_size, currunt_field->offset, currunt_field->stuct_ptr_name);

        printf(ANSI_COLOR_BLUE "----------------------------------------------------------------------------------------------------------------------------------------------------|\n" ANSI_COLOR_RESET);
    }
}

void print_structure_db(struct_db_t *struct_db)
{
    if (!struct_db)
        return;
    printf( "\n------------------------------------------------------");    
    printf(ANSI_COLOR_GREEN "\nPrinting STRUCTURE DATABASE for Registered Structures:\n" ANSI_COLOR_RESET);
    printf( "------------------------------------------------------\n");
    int current_record = 0;
    struct_db_rec_t *current_struct_rec = NULL;
    current_struct_rec = struct_db->head;
    printf("\nNo of Registered Structures = %d\n", struct_db->count);
    while (current_struct_rec)
    {
        printf("\nStructure No : %d [ Address -> %p ]\n", current_record++, current_struct_rec);
        print_structure_rec(current_struct_rec);
        current_struct_rec = current_struct_rec->next;
    }
}

int add_structure_to_struct_db(struct_db_t *struct_db,struct_db_rec_t *struct_rec)
{
    struct_db_rec_t *head = struct_db->head;

    if (!head)
    {
        struct_db->head = struct_rec;
        struct_rec->next = NULL;
        struct_db->count++;
        return 0;
    }

    struct_rec->next = head;
    struct_db->head = struct_rec;
    struct_db->count++;
    return 0;
}

static struct_db_rec_t* struct_db_look_up(struct_db_t *struct_db,char *struct_name)
{

    struct_db_rec_t *curr_struct_rec = struct_db->head;
    if (!curr_struct_rec)
        return NULL;

    while(curr_struct_rec)
    {
        if (strncmp(curr_struct_rec->struct_name, struct_name, MAX_STRUCTURE_NAME_SIZE) == 0)
            return curr_struct_rec;
        curr_struct_rec = curr_struct_rec->next;    
    }
    return NULL;
}

static object_db_rec_t* object_db_look_up(object_db_t *object_db, void *ptr)
{

    object_db_rec_t *curr_obj_rec = object_db->head;
    if (!curr_obj_rec)
        return NULL;

    while(curr_obj_rec)
    {
        if (curr_obj_rec->calloc_ptr == ptr)
            return curr_obj_rec;
        curr_obj_rec = curr_obj_rec->next;    
    }    
    return NULL;
}

/*  API to add dynamically allocated object to object database */
static void add_object_to_object_db(object_db_t *object_db,
                        void *ptr,
                        int units,
                        struct_db_rec_t *struct_rec,
                        mld_boolean_t is_root)
{
    object_db_rec_t *obj_rec = object_db_look_up(object_db, ptr);
    /* Dont add same object twice */
    assert(!obj_rec);

    obj_rec = calloc(1, sizeof(object_db_rec_t));

    obj_rec->next = NULL;
    obj_rec->calloc_ptr = ptr;
    obj_rec->units = units;
    obj_rec->struct_rec = struct_rec;
    obj_rec->is_visited = MLD_FALSE;
    obj_rec->is_root = is_root;

    object_db_rec_t *head = object_db->head;

    if (!head)
    {
        object_db->head = obj_rec;
        obj_rec->next = NULL;
        object_db->count++;
        return;
    }

    obj_rec->next = head;
    object_db->head = obj_rec;
    object_db->count++;
}

void* xcalloc(object_db_t *object_db,char *struct_name,int units)
{
    struct_db_rec_t *struct_rec = struct_db_look_up(object_db->struct_db, struct_name);
    if(!struct_rec)
    {
        printf(ANSI_COLOR_RED "\nStructure -> %s not registered \n"ANSI_COLOR_RESET,struct_name);
        assert(struct_rec);
    }
    void *ptr = calloc(units, struct_rec->struct_size);

    add_object_to_object_db(object_db, ptr, units, struct_rec, MLD_FALSE);
    return ptr;
}

static void delete_object_record_from_object_db(object_db_t *object_db, object_db_rec_t *object_rec)
{
    assert(object_rec);

    object_db_rec_t *head = object_db->head;
    if (head == object_rec)
    {
        object_db->head = object_rec->next;
        free(object_rec);
        return;
    }

    object_db_rec_t *prev = head;
    head = head->next;

    while (head)
    {
        if (head != object_rec)
        {
            prev = head;
            head = head->next;
            continue;
        }

        prev->next = head->next;
        head->next = NULL;
        free(head);
        return;
    }
}

void xfree(object_db_t *object_db, void *ptr)
{
    if (!ptr)
        return;
    object_db_rec_t *object_rec = object_db_look_up(object_db, ptr);

    assert(object_rec);
    assert(object_rec->calloc_ptr);
    free(object_rec->calloc_ptr);
    object_rec->calloc_ptr = NULL;
    /* after freeing allocated object delete object record from object database */
    delete_object_record_from_object_db(object_db, object_rec);
}

/* Displaying Object record from Object database */
void print_object_rec(object_db_rec_t *obj_rec, int i)
{
    if (!obj_rec)
        return;
    printf(ANSI_COLOR_MAGENTA "---------------------------------------------------------------------------------------------------------------------------------------------------------------------|\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_YELLOW "Obj Rec = %-2d |  Obj Rec Add = %-3p |  calloc_ptr = %-10p |  next = %-15p |  units = %-4d |  struct_name = %-10s |  is_root = %s |\n" ANSI_COLOR_RESET,
           i, obj_rec,  obj_rec->calloc_ptr, obj_rec->next, obj_rec->units, obj_rec->struct_rec->struct_name, obj_rec->is_root ? "TRUE " : "FALSE");
    printf(ANSI_COLOR_MAGENTA "---------------------------------------------------------------------------------------------------------------------------------------------------------------------|\n" ANSI_COLOR_RESET);
}


/*  Dsiplaying whole object database */
void print_object_db(object_db_t *object_db)
{
    object_db_rec_t *head = object_db->head;
    unsigned int i = 0;
    printf( "\n------------------------------------------------------------"); 
    printf(ANSI_COLOR_GREEN "\nPrinting OBJECT DATABASE for Registered dynamically objects :\n"ANSI_COLOR_RESET);
    printf( "------------------------------------------------------------\n"); 
    for (; head; head = head->next)
    {
        print_object_rec(head, i++);
    }
    printf("\n\n");
}

/* API used to registere global object created by application */
void mld_register_global_object_as_root(object_db_t *object_db,
                                        void *objptr,
                                        char *struct_name,
                                        unsigned int units)
{

    struct_db_rec_t *struct_rec = struct_db_look_up(object_db->struct_db, struct_name);
    assert(struct_rec);

    /*Create a new object record and add to object database*/
    add_object_to_object_db(object_db, objptr, units, struct_rec, MLD_TRUE);
}

/* API to set perticular dynamic allocated object as a root */
void mld_set_dynamic_object_as_root(object_db_t *object_db, void *obj_ptr)
{
    object_db_rec_t *obj_rec = object_db_look_up(object_db, obj_ptr);
    assert(obj_rec);

    obj_rec->is_root = MLD_TRUE;
}

static object_db_rec_t* get_next_root_object(object_db_t *object_db,
                     object_db_rec_t *starting_from_here)
{
    object_db_rec_t *first = starting_from_here ? starting_from_here->next : object_db->head;
    while (first)
    {
        if (first->is_root)
            return first;
        first = first->next;
    }
    return NULL;
}

static void init_mld_algorithm(object_db_t *object_db)
{
    object_db_rec_t *obj_rec = object_db->head;
    while (obj_rec)
    {
        obj_rec->is_visited = MLD_FALSE;
        obj_rec = obj_rec->next;
    }
}

/* function to explore the direct childs of obj_rec and mark them visited. 
   Note that obj_rec must have already visted */
static void mld_explore_objects_recursively(object_db_t *object_db,
                                object_db_rec_t *parent_obj_rec)
{
    unsigned int i; 
    char *parent_obj_ptr = NULL;
    char *child_obj_offset = NULL;
    void *child_object_address = NULL;
    field_info_t *field_info = NULL;

    object_db_rec_t *child_object_rec = NULL;
    struct_db_rec_t *parent_struct_rec = parent_obj_rec->struct_rec;

    /*Parent object must have already visited*/
    assert(parent_obj_rec->is_visited);

    if (parent_struct_rec->n_fields == 0)
    {
        return;
    }

    for (i = 0; i < parent_obj_rec->units; i++)
    {

        parent_obj_ptr = (char *)(parent_obj_rec->calloc_ptr) + (i * parent_struct_rec->struct_size);

        for (int fields = 0; fields < parent_struct_rec->n_fields; fields++)
        {

            field_info = &parent_struct_rec->fields[fields];

            /*We are only concerned with fields which are pointer to
             * other objects*/
            switch (field_info->datatype)
            {
            case UINT8:
            case UINT32:
            case INT32:
            case CHAR:
            case FLOAT:
            case DOUBLE:
                break;
            case VOID_PTR:
            case OBJ_PTR:
            default:;

                /*child_obj_offset is the memory location inside parent object
                 * where address of next level object is stored*/
                child_obj_offset = parent_obj_ptr + field_info->offset;
                memcpy(&child_object_address, child_obj_offset, sizeof(void *));

                /*child_object_address now stores the address of the next object in the
                 * graph. It could be NULL, Handle that as well*/
                if (!child_object_address)
                    continue;

                child_object_rec = object_db_look_up(object_db, child_object_address);

                assert(child_object_rec);
                /* Since we are able to reach this child object "child_object_rec" 
                 * from parent object "parent_obj_ptr", mark this
                 * child object as visited and explore its children recirsively. 
                 * If this child object is already visited, then do nothing - avoid infinite loops*/
                if (!child_object_rec->is_visited)
                {
                    child_object_rec->is_visited = MLD_TRUE;
                    if (field_info->datatype != VOID_PTR) /*Explore next object only when it is not a VOID_PTR*/
                        mld_explore_objects_recursively(object_db, child_object_rec);
                }
                else
                {
                    continue; /*Do nothing, explore next child object*/
                }
            }
        }
    }
}

/* API used to Traverse the graph starting from root objects using DFS and mark all reachable nodes as visited */
void run_mld_algorithm(object_db_t *object_db)
{

    /*  Mark all objects in object databse as unvisited */
    init_mld_algorithm(object_db);

    /* Get the first root object from the object db */
    object_db_rec_t *root_obj = get_next_root_object(object_db, NULL);

    while (root_obj)
    {
        if (root_obj->is_visited)
        {
            /* if root object already been explored get the next root objet */
            root_obj = get_next_root_object(object_db, root_obj);
            continue;
        }

        /* make it as visited */
        root_obj->is_visited = MLD_TRUE;

        /* Explore all reachable objects from this root_obj recursively */
        mld_explore_objects_recursively(object_db, root_obj);

        /* get the next root object and explore it */
        root_obj = get_next_root_object(object_db, root_obj);
    }
}

/* funtion to display object record  */
static void mld_dump_object_rec_detail(object_db_rec_t *obj_rec)
{

    int fields = obj_rec->struct_rec->n_fields;
    field_info_t *field = NULL;

    int units = obj_rec->units;

    for (int obj_index = 0; obj_index < units; obj_index++)
    {
        char *current_object_ptr = (char *)(obj_rec->calloc_ptr) +
                                   (obj_index * obj_rec->struct_rec->struct_size);

        for (int field_index = 0; field_index < fields; field_index++)
        {

            field = &obj_rec->struct_rec->fields[field_index];

            switch (field->datatype)
            {
            case UINT8:
            case INT32:
            case UINT32:
                printf("%s[%d]->%s = %d\n", obj_rec->struct_rec->struct_name, obj_index, field->feldname, *(int *)(current_object_ptr + field->offset));
                break;
            case CHAR:
                printf("%s[%d]->%s = %s\n", obj_rec->struct_rec->struct_name, obj_index, field->feldname, (char *)(current_object_ptr + field->offset));
                break;
            case FLOAT:
                printf("%s[%d]->%s = %f\n", obj_rec->struct_rec->struct_name, obj_index, field->feldname, *(float *)(current_object_ptr + field->offset));
                break;
            case DOUBLE:
                printf("%s[%d]->%s = %f\n", obj_rec->struct_rec->struct_name, obj_index, field->feldname, *(double *)(current_object_ptr + field->offset));
                break;
            case OBJ_PTR:
                printf("%s[%d]->%s = %p\n", obj_rec->struct_rec->struct_name, obj_index, field->feldname, (void *)*(int *)(current_object_ptr + field->offset));
                break; 
            default:
                break;
            }
        }
    }
}

void report_leaked_objects(object_db_t *object_db)
{
    int i = 0;
    object_db_rec_t *head=object_db->head;
    mld_boolean_t isfound = MLD_FALSE;    
    printf( "\n----------------"); 
    printf(ANSI_COLOR_GREEN"\nLeaked Objects : "ANSI_COLOR_RESET);
    printf( "\n----------------\n"); 
    
    while(head)
    {
        if (!head->is_visited)
        {
            print_object_rec(head, i++);
            mld_dump_object_rec_detail(head);
            isfound = MLD_TRUE;
            printf("\n\n");
        }
        head=head->next;
    }

    if(!isfound)
    {
        printf("Number of Leaked object -> 0 \n\n");
    }
}


void report_and_free_leaked_objects(object_db_t *object_db)
{
    int i = 0;
    object_db_rec_t *head=object_db->head;
    mld_boolean_t isfound = MLD_FALSE;    
    printf( "\n---------------------------------------"); 
    printf(ANSI_COLOR_GREEN"\nDisplaying and Freeing Leaked Objects : "ANSI_COLOR_RESET);
    printf( "\n---------------------------------------\n"); 
    
    while(head)
    {
        //object which are not visited in object database consider as a leaked object 
        if (!head->is_visited)
        {
            print_object_rec(head, i++);
            mld_dump_object_rec_detail(head);
            //after displaying leaked object freeing it
            xfree(object_db,head->calloc_ptr);
            isfound = MLD_TRUE;
            printf("\n\n");
        }
        head=head->next;
    }

    if(!isfound)
    {
        printf("Number of Leaked object -> 0 \n\n");
    }
}

/* Support for primitive data types */
void mld_init_primitive_data_type_support(struct_db_t *struct_db)
{
    REG_STRUCT(struct_db, int, 0);
    REG_STRUCT(struct_db, float, 0);
    REG_STRUCT(struct_db, double, 0);
}

