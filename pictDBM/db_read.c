/**
 * @file db_read.c
 * @brief implementation of do_read function.
 *
 */

#include <stdint.h>
#include "pictDB.h"
#include "image_content.h"

/**
 *@brief retourne l'image dans un buffer table avec sa size dans image_size.
 *
 *@param un identifiant d'image, une resolution,une struct db_file.
 */

int do_read(char* pict_id, int resolution, char** table, uint32_t* image_size,struct pictdb_file* db_file)
{
	if(pict_id==NULL){
		return ERR_INVALID_PICID;
	}
    int index=-1;
    for(unsigned int i=0; i< db_file->header.max_files; ++i) {
        if(strcmp(pict_id, db_file->metadata[i].pict_id)==0) {//on cherche l'image correspondante à la pict_id.
            index=i;

            if(db_file->metadata[i].is_valid!=NON_EMPTY) {
                return ERR_INVALID_PICID;//image non valide.
            }
        }
    }
    if(index==-1) {
        return ERR_FILE_NOT_FOUND;//pas de pict_id correspondant dans les metadata.
    }

    if(db_file != NULL) {
        if(db_file->metadata[index].offset[resolution]==0 || db_file->metadata[index].size[resolution]==0) {
            int a = 0;
            if( (a = lazily_resize(resolution , db_file, index))!=0 ) {//image n'exiiste pas dans la resolution voulue.
                return a;
            }
        }
        char * image=NULL;
        image = malloc(db_file->metadata[index].size[resolution]);//allocation de l'espace pour image
        if(image==NULL) {
            return ERR_OUT_OF_MEMORY;
        }


        //~ image_size = malloc(sizeof(uint32_t));
        *image_size = db_file->metadata[index].size[resolution];//taille de l'image a retourné
        *table = calloc(*image_size, sizeof(char));//alloue pour table : la taille de l'image.
        if(*table==NULL) {
            return ERR_OUT_OF_MEMORY;
        }

        fseek(db_file->fpdb, db_file->metadata[index].offset[resolution],SEEK_SET);
        int ok = fread(image, *image_size ,1, db_file->fpdb);// utilisé taille de l'image pour la lecture
        if(ok!=1) {
            free(image);
            image=NULL;
            return ERR_IO;
        }
        *table = image;
    }
    return 0;
}
