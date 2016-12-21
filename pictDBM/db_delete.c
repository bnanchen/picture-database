/**
 * @file db_delete.c
 * @brief file for do_delete function.
 *
 */

#include "pictDB.h" // lieu du prototype
#include "error.h" /* not needed here, but provides it as required by
* all functions of this lib.
*/
#include <string.h>
#include <stdio.h>

// création de la fonction do_delete (voir documentation dans pictDB.h):
int do_delete(const char* pictID, struct pictdb_file* file)
{
    unsigned int id_found = 0; // met à 1 si une image a été trouvée du nom de id mais d'abord on utilise cette valeur pour la vérification que la db n'est pas vide.
    for (unsigned int i = 0; i < file->header.max_files && id_found == 0; i++) {
        if (file->metadata[i].is_valid == NON_EMPTY) {
            id_found = 1;
        }
    }
    if (id_found == 0) {
        return 0; // si id_found alors la db est vide et donc on n'a rien à supprimer pour conséquent on peut directement retourner 0.
    }
    id_found = 0; // on réinitialise à 0 pour sa réelle utilisation (savoir si une image a été trouvée).
    if (fseek(file->fpdb, sizeof(struct pictdb_header), SEEK_SET) != 0) {
        return ERR_IO;
    } // on survole le header dans le fichier.
    for (unsigned int i = 0; i < file->header.max_files && id_found == 0; i++) {

        if (strcmp(pictID, file->metadata[i].pict_id) == 0) { // test si pictID = id de la metadata i
            id_found = 1;
            if (file->metadata[i].is_valid == 0) { // Image a déjà été deletée
                return ERR_FILE_NOT_FOUND;
            } else {
                file->metadata[i].is_valid = 0; // sinon on la delete
                if (fwrite(&(file->metadata[i]), sizeof(struct pict_metadata), 1, file->fpdb) != 1) { // test si l'écriture du metadata modifié est un succès
                    return ERR_IO;
                }
                file->header.db_version += 1; // version mis a jour
                file->header.num_files -= 1; // nombre de fichiers mis a jour
                if (fseek(file->fpdb, 0, SEEK_SET) != 0) { // on se repositionne au début
                    return ERR_IO; // échec du repositionnement au debut
                }
                if (fwrite(&(file->header), sizeof(struct pictdb_header), 1, file->fpdb) != 1) { // écrit le header
                    return ERR_IO;
                }

            }
        }
        if (fseek(file->fpdb, sizeof(struct pict_metadata), SEEK_CUR) != 0) { // on se positionne au debut du prochain metadata pour la prochaine itération du for.
            return ERR_IO;
        }
    }
    if (id_found == 0) { // si aucune image n'a été trouvé au nom id alors on retourne une erreur.
        return ERR_FILE_NOT_FOUND;
    }
    return 0;
}
