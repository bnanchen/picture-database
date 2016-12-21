/**
 * @file pictDB.h
 * @brief Main header file for pictDB core library.
 *
 * Defines the format of the data structures that will be stored on the disk
 * and provides interface functions.
 *
 * The picture database starts with exactly one header structure
 * followed by exactly pictdb_header.max_files metadata
 * structures. The actual content is not defined by these structures
 * because it should be stored as raw bytes appended at the end of the
 * database file and addressed by offsets in the metadata structure.
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#ifndef PICTDBPRJ_PICTDB_H
#define PICTDBPRJ_PICTDB_H

#include "error.h" /* not needed here, but provides it as required by
                    * all functions of this lib.
                    */
#include <stdio.h> // for FILE
#include <stdint.h> // for uint32_t, uint64_t
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH
#include <string.h>
#include <stdlib.h> // ajouté pour l'allocation dynamique
#include <inttypes.h> //ajouté pour gérer les PRIu32,...

#define CAT_TXT "EPFL PictDB binary"

/* constraints */
#define MAX_DB_NAME 31  // max. size of a PictDB name
#define MAX_PIC_ID 127  // max. size of a picture id
#define MAX_MAX_FILES 100000  // will be increased later in the project

/* For is_valid in pictdb_metadata */
#define EMPTY 0
#define NON_EMPTY 1

// pictDB library internal codes for different picture resolutions.
#define RES_THUMB 0
#define RES_SMALL 1
#define RES_ORIG  2
#define NB_RES    3

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief header (structure) du pictDB qui rassemble les éléments de configuration du système.
 *
 */


struct pictdb_header {
    char db_name[MAX_DB_NAME + 1];/*!<tableau de MAX_DB_NAME plus un caractères, contenant le nom de la base d’images*/
    uint32_t db_version;/*!<version de la base de données ; elle est augmentée après chaque modification de la base ;*/
    uint32_t num_files;/*!<nombre d'images présentes dans la base ;*/
    uint32_t max_files;/*!<nombre maximal d'images possibles dans la base ; ce champ est spécifié lors de la création de la base, mais ne doit pas être modifié par la suite ;*/
    uint16_t res_resized [2*(NB_RES-1)];/*!< tableaux des résolutions maximales des images « thumbnail » et « small » (dans l'ordre: « thumbnail X », « thumbnail Y », « small X », « small Y ») ; tout comme le nombre maximum d’images, ces valeurs sont également spécifiées lors de création de la base d’image et ne doivent pas être modifiées par la suite. */
    uint32_t unused_32; /*!<non utilisé */
    uint64_t unused_64; /*!<non utilisé */

};

/*
 * @brief structure qui rassemble les métadonnées d'une pictDB.
 *
 */
struct pict_metadata {
    char pict_id [MAX_PIC_ID+1]; /*!<tableau de taille MAX_PIC_ID+1 contenant un identificateur unique (nom) de l'image*/
    unsigned char SHA [SHA256_DIGEST_LENGTH]; /*!<un tableau de taille SHA256_DIGEST_LENGTH contenant le <<hash code>> de l'image*/
    uint32_t res_orig [2]; /*!<un tableau de taille 2 contenant la résolution de l'image d'origine*/
    uint32_t size [NB_RES]; /*!<un tableau de taille NB_RES contenant les tailles mémoires (en octets) des images aux différentes résolutions*/
    uint64_t offset [NB_RES]; /*!<un tableau de taille NB_RES contenant les positions dans le fichier <<base de donnée d'images>> des images aux différentes résolutions possibles*/
    uint16_t is_valid; /*!<indique si l'image est encore utilisée (NON_EMPTY) ou a été effacée (valeur EMPTY)*/
    uint16_t unused_16; /*!<non utilisé*/
};

struct pictdb_file {
    FILE* fpdb;
    struct pictdb_header header;
    struct pict_metadata* metadata; // pour créer un tableau de metadata.
};

typedef enum { STDOUT, JSON } do_list_mode;

/**
 * @brief Prints database header informations.
 *
 * @param header The header to be displayed.
 */
void print_header(struct pictdb_header const* header);

/**
 * @brief Prints picture metadata informations.
 *
 * @param metadata The metadata of one picture.
 */
void print_metadata (struct pict_metadata const* metadata);

/**
 * @brief Displays (on stdout) pictDB metadata.
 *
 * @param db_file In memory structure with header and metadata.
 */
char* do_list(struct pictdb_file const* file,do_list_mode mode);

/**
 * @brief Creates the database called db_filename. Writes the header and the
 *        preallocated empty metadata array to database file.
 *
 * @param db_file In memory structure with header and metadata.
 */

int do_create(const char* file_name, struct pictdb_file* db_file);

/**
* @brief Open the file called filename and read the content of header and metadata.
*
* @param filename: name of the file we will open, opening_mode the open mode of the file, file: structure with header and metadata.
*/
int do_open(const char* filename, const char* opening_mode, struct pictdb_file* file);

/**
* @brief Close the file.
*
* @param file: structure with header and metadata.
*/
void do_close(struct pictdb_file* file);

/**
* @brief Delete the pictID from the file
*
* @param file: structure with header and metadata
*/

int do_delete(const char* pictID, struct pictdb_file* file);

/**
 * @brief transforme une chaine de caractères spécifiant une résolution d'image dans une des énumérations spécifiant un type de résolution, retourne le nombre correspondant au nom de la
 * résolution spécifié (chain) sinon retourne -1.
 *
 * @param une chaine de caractères: const char* chain.
 */

int resolution_atoi(const char* chain);

/**
 * @brief
 *
 * @param un identifiant d'image: char* pict_id; un entier représentant le code d'une résolution d'image: int code_res;
 * un pointeur de pointeur sur des caractères (char) (utilisés en tant qu'octets), c'est l'adresse d'un tableau d'octets: char** table;
 * un entier non signé sur 32 bits représentant la taille de l'image: uint32_t pict_size; une structure pictdb_file: struct pictdb_file file.
 */

int do_read(char* pict_id, int code_res, char** table, uint32_t* pict_size, struct pictdb_file* file);

/**
 * @brief insère une image de type .jpg dans la base d'images. 
 *
 * @param une image sous forme d'un pointeur (tableau) de caractères: char* pict_tabe; la taille de l'image: size_t pict_size; un identifiant d'image: char* pict_name;
 * une structure pictdb_file dans laquelle on ajoutera l'image: struct pictdb_file* file.
 */

int do_insert(const char* pict_table, size_t pict_size, char* pict_name, struct pictdb_file* file);

#ifdef __cplusplus
}
#endif
#endif
