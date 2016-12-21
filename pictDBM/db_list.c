/*
 * @file db_list.c
 * @brief file for do_list function.
 */

#include "pictDB.h" //lieu du prototype
#include "error.h" /* not needed here, but provides it as required by
* all functions of this lib.
*/
#include <string.h>
#include <stdio.h> // for FILE
#include <stdint.h> // for uint32_t, uint64_t
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH
#include <json-c/json.h>// for json methods.


/*void do_list(struct pictdb_file const* file)
{
    print_header(&file->header);
    int emptyDatab = EMPTY; // helper value to verify if the database is empty
    for (unsigned int i = 0; i < file->header.max_files && emptyDatab==EMPTY; i++) { // if an element of the array has a validity NON_EMPTY the we can go out of the for-loop.
        if((file->metadata[i]).is_valid == NON_EMPTY) {
            emptyDatab = NON_EMPTY;
        }
    }
    if (emptyDatab == EMPTY) {
        printf("<< empty database >> \n");
    }
    for (unsigned int i = 0; i < file->header.max_files; i++) {
        if (file->metadata[i].is_valid == NON_EMPTY) {
            print_metadata(&file->metadata[i]);
        }
    }
}*/
char* do_list(struct pictdb_file const* file, do_list_mode mode)
{
	
		if(mode == STDOUT){
			print_header(&file->header);
			int emptyDatab = EMPTY; // helper value to verify if the database is empty
			for (unsigned int i = 0; i < file->header.max_files && emptyDatab==EMPTY; i++) { // if an element of the array has a validity NON_EMPTY the we can go out of the for-loop.
				if((file->metadata[i]).is_valid == NON_EMPTY) {
					emptyDatab = NON_EMPTY;
				}
			}
			if (emptyDatab == EMPTY) {
				printf("<< empty database >> \n");
			}
			for (unsigned int i = 0; i < file->header.max_files; i++) {
				if (file->metadata[i].is_valid == NON_EMPTY) {
					print_metadata(&file->metadata[i]);
				}
			}
			return NULL;
		}
		if(mode == JSON){
			
			struct json_object* j_array = json_object_new_array();
			//struct json_object* val = malloc(sizeof(struct json_object));
			//val = json_object_new_string(...a mettre toutes les pict_id.
			for(unsigned int i=0; i<file->header.max_files;i++) {
				if(file->metadata[i].is_valid == NON_EMPTY && file->metadata[i].pict_id!=NULL){
					struct json_object* val = json_object_new_string(file->metadata[i].pict_id);
					//val = json_object_new_string(file->metadata[i].pict_id);
				    json_object_array_add(j_array, val);// que retourne json_object_array et comment le testé.
				
				}
			}
				struct json_object* j_obj = json_object_new_object();//cree un nouvel json_object
				json_object_object_add(j_obj, "Pictures", j_array);//on ajoute l'array de pict_id a j_obj. Pas du tout sur pour Pictures!!!!!!
				const char  * retour = (json_object_to_json_string(j_obj));// convertit json_object en json_string.
				char * tmp = malloc(strlen(retour)+1);
				strcpy(tmp, retour);
				json_object_put(j_obj);//decrement the reference_count return 1 if j_obj has been freed(ref_count==0).
				return tmp;
			}
			
			
	
	return "unimplemented do_list mode";	
}
