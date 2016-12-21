/**
* @file db_insert.c
* @brief implémentation de la fonction do_insert (voir pictDB.h pour plus d'informations)
*
*/

#include "pictDB.h"
#include "dedup.h"
#include "image_content.h"

#include <openssl/sha.h> // pour SHA256()


int do_insert(const char* pict_table, size_t pict_size, char* pict_name, struct pictdb_file* file) 
{
    // trouver une position de libre dans l'index:
    if (file->header.num_files >= file->header.max_files) {
        return ERR_FULL_DATABASE; // erreur retournée si le nombre actuel d'images est égal ou supérieur à max_files
    }
    uint32_t empty_find_index = 0; // index de la place libre trouvée dans le tableau de metadatas
    for (size_t i = 0; i < file->header.max_files && empty_find_index == 0; i++) {
        if (file->metadata[i].is_valid == EMPTY) {
            empty_find_index = i;
            file->metadata[empty_find_index].is_valid = NON_EMPTY;
        }
    }
    SHA256((unsigned char *)pict_table, pict_size, file->metadata[empty_find_index].SHA);
    if (pict_name == NULL) {
        return ERR_INVALID_PICID; // erreur retournée si le nouvel pict_id est NULL
    }
    strncpy(file->metadata[empty_find_index].pict_id, pict_name, strlen(pict_name));
    file->metadata[empty_find_index].size[RES_ORIG] = (uint32_t)pict_size;
    // de-duplication de l'image:
    unsigned int ret = 0; // valeur utilisée comme valeur de retour s'il y a un problème
    ret = do_name_and_content_dedup(file, empty_find_index);
    if (ret != 0) {
        return ret; // retourne le même code d'erreur que la do_name_and_content_dedup (s'il y a erreur)
    }
    // fin de l'initialisation des metadata de l'image:
    // écriture de l'image (pas de la metadata) sur le disque:
    if (file->metadata[empty_find_index].offset[RES_ORIG] == 0) { // si l'offset à RES_ORIG est 0 alors aucun doublon n'a été trouvé.
        // donc c'est une nouvelle image que l'on écrit sur le disque (à la fin):
        file->metadata[empty_find_index].offset[RES_ORIG] = (uint64_t)ftell(file->fpdb);
        if (file->metadata[empty_find_index].offset[RES_ORIG] == -1) {
            return ERR_IO; // erreur retournée s'il y a un problème avec ftell
        }
        // écriture à la suite du fichier de la nouvelle image:
        if (fseek(file->fpdb, 0, SEEK_END) != 0) {
            return ERR_IO; // erreur retournée si problème avec fseek
        }
        if (fwrite(pict_table, sizeof(char), pict_size, file->fpdb) != pict_size) {
            return ERR_IO; // erreur retournée si le nombre d'élément écrit n'est pas égal à pict_size
        }
    }
    
    // mise à jour des données de la base d'images (header + metadata):
    file->header.db_version += 1;
    file->header.num_files += 1;
    // calcul de la hauteur et de la largeur de l'image:
    ret = get_resolution(&(file->metadata[empty_find_index].res_orig[1]), &(file->metadata[empty_find_index].res_orig[0]), pict_table, pict_size);
    if (ret != 0) {
        return ret; // erreur retournée s'il y a une erreur lors du get_resolution
    }
    if (fseek(file->fpdb, 0, SEEK_SET) != 0) { // on replace la tête au début du fichier
        return ERR_IO; // erreur retournée si problème avec fseek
    }
    if (fwrite(&(file->header), sizeof(struct pictdb_header), 1, file->fpdb) != 1) { // écriture du header
        return ERR_IO; // erreur retournée si le nombre d'élément écrit n'est pas égal à 1
    }
    if (fseek(file->fpdb, empty_find_index*sizeof(struct pict_metadata), SEEK_CUR) != 0) {
        return ERR_IO; // erreur retournée si problème avec fseek
    }
    if (fwrite(&(file->metadata[empty_find_index]), sizeof(struct pict_metadata), 1, file->fpdb) != 1) { // écriture de la metadata de l'image concernée
        return ERR_IO; // erreur retournée si le nombre d'élément écrit n'est pas égal à 1
    }
    return 0;

}