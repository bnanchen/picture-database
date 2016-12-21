/**
 * @file db_utils.c
 * @brief implementation of several tool functions for pictDB
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <stdint.h> // for uint8_t
#include <stdio.h> // for sprintf
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH
#include <string.h>

/********************************************************************//**
 * Human-readable SHA
 */
static void
sha_to_string (const unsigned char* SHA,
               char* sha_string)
{
    if (SHA == NULL) {
        return;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(&sha_string[i*2], "%02x", SHA[i]);
    }

    sha_string[2*SHA256_DIGEST_LENGTH] = '\0';
}

/********************************************************************//**
 * pictDB header display.
 */
void print_header(struct pictdb_header const* header)
{
    printf("*****************************************\n");
    printf("**********DATABASE HEADER START**********\n");
    printf("DB NAME: %31s\n", header->db_name);
    printf("VERSION: %"PRIu32"\n", header->db_version);
    printf("IMAGE COUNT: %"PRIu32"\t \t MAX IMAGES: %"PRIu32"\n", header->num_files, header->max_files);
    printf("THUMBNAIL: %"PRIu16" x %"PRIu16" \t SMALL: %"PRIu16" x %"PRIu16"\n", header->res_resized[0], header->res_resized[1], header->res_resized[2], header->res_resized[3]);
    printf("***********DATABASE HEADER END***********\n");
    printf("*****************************************\n");
}


/********************************************************************//**
 * Metadata display.
 */


void
print_metadata (struct pict_metadata const* metadata)
{
    char sha_printable[2*SHA256_DIGEST_LENGTH+1];
    sha_to_string(metadata->SHA, sha_printable);

    printf("PICTURE ID: %-31s\n", metadata->pict_id);
    printf("SHA: ");
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        printf("%02x", metadata->SHA[i]);
    }
    printf("\n");

    printf("VALID: %"PRIu16" \n", metadata->is_valid);
    printf("UNUSED: %"PRIu16"\n", metadata->unused_16);
    printf("OFFSET ORIG. : %"PRIu64"\t\tSIZE ORIG. : %"PRIu32"\n", metadata->offset[RES_ORIG], metadata->size[RES_ORIG]);
    printf("OFFSET THUMB.: %"PRIu64"\t\tSIZE THUMB.: %"PRIu32"\n", metadata->offset[RES_THUMB], metadata->size[RES_THUMB]);
    printf("OFFSET SMALL : %"PRIu64"\t\tSIZE SMALL : %"PRIu32"\n", metadata->offset[RES_SMALL], metadata->size[RES_SMALL]);
    printf("ORIGINAL: %"PRIu32" x %"PRIu32"\n", metadata->res_orig[0], metadata->res_orig[1]);
    printf("*****************************************\n");
}

// fonction d'ouverture d'un fichier (voir pictDB.h pour plus d'informations):
int do_open(const char* filename, const char* opening_mode, struct pictdb_file* file)
{
    if (strcmp("w", opening_mode) == 0 || strcmp("wb", opening_mode) == 0) { // si opening_mode = "wb" ou "w", on retourne un erreur
        return ERR_INVALID_ARGUMENT;
    }
    if (filename == NULL) {
        return ERR_IO;
    }
    file->fpdb = fopen(filename, opening_mode); // ouvre le fichier
    if (file->fpdb == NULL) {
        return ERR_IO;
    } else {
        size_t size = 0;
        size = strlen(filename); // longueur du nom du fichier
        if(size < 1 || size > MAX_DB_NAME) {
            return ERR_INVALID_FILENAME; // longueur du nom du fichier non valide
        }
        // lit le contenu du header:
        if (fread(&(file->header), sizeof(struct pictdb_header), 1, file->fpdb) != 1) {
            return ERR_IO; // erreur envoyée si le nombre d'élément lu ne correspond pas à 1
        }
        // allocation dynamique:
        file->metadata = calloc(file->header.max_files, sizeof(struct pict_metadata));
        if (file->metadata == NULL) {
            return ERR_OUT_OF_MEMORY; // erreur envoyée si problème lors du calloc
        }
        // lit le contenu des metadata:
        if (fread(file->metadata, sizeof(struct pict_metadata), file->header.max_files, file->fpdb) != file->header.max_files) {
            free(file->metadata); 
            return ERR_IO; // erreur envoyée si le nombre d'éléments lus ne correspond pas au nombre max_files
        }
    }
    return 0;
}


// fonction de fermeture d'un fichier (voir pictDB.h pour plus d'informations):
void do_close(struct pictdb_file* file)
{
    if (file != NULL) {
        free(file->metadata);// dedans le if? Oui dans le if on free que si non vide.
        file->metadata = NULL;
        fclose(file->fpdb); // ferme le fichier du pictdb_file passé en paramètre
    } else {
        free(file->metadata);
    }
}


// fonction utilitaire (voir pictDB.h pour plus d'informations):
int resolution_atoi(const char* chain)
{
    if (strcmp(chain, "thumb") == 0 || strcmp(chain, "thumbnail") == 0) {
        return RES_THUMB;
    } else if (strcmp(chain, "small") == 0) {
        return RES_SMALL;
    } else if (strcmp(chain, "orig") == 0 || strcmp(chain, "original") == 0) {
        return RES_ORIG;
    } else {
        return -1;
    }
}

