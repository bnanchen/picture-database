/**
* @file dedup.h
* @brief
*
*/
#include "pictDB.h"

// prototypes:

/**
 * @brief
 *
 * @param resolution de la nouvelle image, pictDB file et l'index de la metadata de la nouvelle image.
 */
int lazily_resize(int resolution, struct pictdb_file * db_file,size_t index);

/**
 * @brief recuperer la resolution d'une image JPEG
 *
 * @param
 */
int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer, size_t image_size);
