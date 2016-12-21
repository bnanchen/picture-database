/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */

int do_create(const char* filename, struct pictdb_file*  db_file)
{

    // Sets the DB header name
    strncpy(db_file->header.db_name, filename,  MAX_DB_NAME);
    db_file->header.db_name[MAX_DB_NAME] = '\0';

    int taille_lue=0;
    taille_lue=strlen(filename); // length of the name
    if(taille_lue==0) {
        return ERR_INVALID_FILENAME;
    }


    db_file->fpdb = fopen(filename,"wb");
    fflush(stdout);

    if(db_file->fpdb == NULL) {
        return ERR_IO;
    }

    db_file->header.db_version = 0; // initialisation de db_version
    db_file->header.num_files = 0; // initialisation de num_files
    if(db_file->header.max_files > MAX_MAX_FILES) {
        return ERR_IO; // erreur envoyée si le champ max_files est plus grand que MAX_MAX_FILES
    }
    db_file->metadata = calloc(MAX_MAX_FILES, sizeof(struct pict_metadata));
    if (db_file->metadata == NULL) {
        return ERR_OUT_OF_MEMORY; // erreur envoyée si problème lors du calloc
    }
    for(int i=0; i<MAX_MAX_FILES; ++i) {
        (db_file->metadata[i]).pict_id[0]='\0';
        db_file->metadata[i].SHA[0] = '\0';

        memset((db_file->metadata[i]).res_orig, 0,sizeof(db_file->metadata[i].res_orig)); // initialization of the resolution
        memset((db_file->metadata[i]).size, 0, sizeof(db_file->metadata[i].size)); // initiallization of the size to 0
        memset((db_file->metadata[i]).offset, 0, sizeof(db_file->metadata[i].offset)); // initialize every values of offset to 0
        (db_file->metadata[i]).is_valid = EMPTY; // on utilise toujours l'image
    }
    int number_write_header=0;
    number_write_header = fwrite(&(db_file->header), sizeof(struct pictdb_header), 1,db_file->fpdb); // write the header in the FILE of db_file

	int number_write_metadata=0;
	number_write_metadata = fwrite(db_file->metadata, sizeof(struct pict_metadata), db_file->header.max_files, db_file->fpdb); // write the metadata int the FILE of db_file 
	
	if((1 + db_file->header.max_files) != number_write_header+number_write_metadata){
        free(db_file->metadata);
		return ERR_IO; // erreur retournée si le nombre écrit n'est pas celui attendu
	}
	printf("%d items written\n" ,number_write_header+number_write_metadata);
    
	fclose(db_file->fpdb);
	return 0;
}
