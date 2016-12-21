/**
* @file dedup.h
* @brief
*
*/

#include "pictDB.h"

// prototypes:

/**
 * @brief regarde s'il existe une image dupliquée à l'index index de la metadata.
 *
 * @param un fichier pictDB précédemment ouvert: struct pictdb_file* file; la position d'une image donnée dans le tableau metadata: uint32_t index.
 */

int do_name_and_content_dedup(struct pictdb_file* file, uint32_t index);