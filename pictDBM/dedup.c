/**
* @file dedup.c
* @brief implémentation de la fonction do_name_and_content_dedup
*
*/

#include "dedup.h"

/**
 * @brief compare deux SHA si ils sont égaux retourne 0 sinon retourne 1.
 *
 * @param le premier SHA: const unsigned char* sha_1; le second SHA: const unsigned char* sha_2.
 */
int comparison_sha(const unsigned char* sha_1, const unsigned char* sha_2)   // helper function pour do_name_and_content_dedup
{
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        if (sha_1[i] != sha_2[i]) {
            return 1; // s'ils ne sont pas égaux
        }
    }
    return 0; // s'ils sont égaux
}

// implémentation de la fonction:
int do_name_and_content_dedup(struct pictdb_file* file, uint32_t index)
{
    unsigned int br_loop = 0;
    for (size_t i = 0; i < file->header.max_files; i++) {
        if (strcmp(file->metadata[i].pict_id, file->metadata[index].pict_id) == 0 && i != index) {
            // si le pic_id du metadata à la place i est le même que celui du metadata à la place index:
            return ERR_DUPLICATE_ID;
        } else if (comparison_sha(file->metadata[i].SHA, file->metadata[index].SHA) == 0) {
            for (size_t j = 0; j < NB_RES; j++) {
                // si le SHA du metadata à la place i est le même que le SHA du metadata à la place index:
                file->metadata[index].offset[j] = file->metadata[i].offset[j];
                file->metadata[index].size[j] = file->metadata[i].size[j];
            }
            return 0;
        }
    }
    // si il n'y a aucun doublon:
    file->metadata[index].offset[RES_ORIG] = 0;
    return 0;
}