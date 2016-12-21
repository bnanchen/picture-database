/**
 * @file pictDBM.c
 * @brief pictDB Manager: command line interpretor for pictDB core commands.
 *
 * Picture Database Management Tool
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <stdlib.h>
#include <string.h>
#include  <vips/vips.h>
#include <stdint.h>

// définition du type command: pointeur sur fonction:
typedef int (*command) (int args, char *argv[]);

/*
 * @brief structure command_mapping qui contient le nom de la fonction: const char* name; et le pointeur sur la fonction (la-même): command cmd.
 *
 */
struct command_mapping {
    const char* name;
    command cmd;
};

/*
 * Prototypes
 */
int create_name(char * original_prefix, char * resolution_suffix, char ** filename);

/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int
do_list_cmd (int args, char *argv[])
{
    if (args < 1) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    
    struct pictdb_file myfile;
    do_open(argv[0], "rb", &myfile); // ouverture du fichier
    do_list(&myfile, STDOUT);
    do_close(&myfile); // fermeture du fichier
    return 0;
}

/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int
do_create_cmd (int args, char *argv[])
{
    if (args < 1) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    struct pictdb_file db_file;
    // initialisation des variables à leur valeur "défaut":
    db_file.header.max_files = 10;
    db_file.header.res_resized[0] = 64; // the x element of the resolution (x x y) of thumbnail
    db_file.header.res_resized[1] = 64; // the y element of the resolution of thumbnail
    db_file.header.res_resized[2] = 256; // the x element of the resolution of small
    db_file.header.res_resized[3] = 256; // the y element of the resolution of small
    char* filename = malloc(sizeof(char));
    if (filename == NULL) {
        return ERR_OUT_OF_MEMORY; // erreur si problème avec le malloc.
    }
    strncpy(filename, argv[0], strlen(argv[0])); // copie du nom du fichier dans filename
    argv++;
    args--;
    while (args != 0) {
        if (strcmp(argv[0], "-max_files") == 0) { // test si l'option est -max_files
            if (args < 2) {
                free(filename);
                return ERR_NOT_ENOUGH_ARGUMENTS; // erreur retournée s'il manque des arguments à l'option max_files
            } else {
                db_file.header.max_files = (uint32_t)strtol(argv[1], NULL, 10);
                if (db_file.header.max_files > MAX_MAX_FILES || db_file.header.max_files == 0) {
                    return ERR_MAX_FILES; // erreur retournée si le nombre pour max_files est plus grand que MAX_MAX_FILES ou égal à 0
                }
                args -= 2;
                argv++;
                argv++;
            }
        } else if (strcmp(argv[0], "-thumb_res") == 0) { // test si l'option est -thumb_res
            if (args < 3) {
                return ERR_NOT_ENOUGH_ARGUMENTS; // erreur retournée s'il manque des arguments à l'option thumb_res
            } else {
                db_file.header.res_resized[0] = (uint16_t)strtol(argv[1], NULL, 10);
                db_file.header.res_resized[1] = (uint16_t)strtol(argv[2], NULL, 10);
                if (db_file.header.res_resized[0] > 128 || db_file.header.res_resized[1] > 128 || db_file.header.res_resized[0] == 0 || db_file.header.res_resized[1] == 0) {
                    return ERR_RESOLUTIONS; // erreur retournée si la résolution est plus grande que 128x128 ou égale à 0x0
                }
            }
            args -= 3;
            argv++;
            argv++;
            argv++;
        } else if (strcmp(argv[0], "-small_res") == 0) { // test si l'option est -small_res
            if (args < 3) {
                return ERR_NOT_ENOUGH_ARGUMENTS; // erreur retournée s'il manque des arguments à l'option small_res
            } else {
                db_file.header.res_resized[2] = (uint16_t)strtol(argv[1], NULL, 10);
                db_file.header.res_resized[3] = (uint16_t)strtol(argv[2], NULL, 10);
                if (db_file.header.res_resized[2] > 512 || db_file.header.res_resized[3] > 512 || db_file.header.res_resized[2] == 0 || db_file.header.res_resized[3] == 0) {
                    return ERR_RESOLUTIONS; // erreur retournée si la résolution est plus grande que 512x512 ou égale à 0x0
                }
                args -= 3;
                argv++;
                argv++;
                argv++;
            }
        } else {
            return ERR_INVALID_ARGUMENT; // erreur retournée si l'option utilisée n'a pas été reconnue
        }
    }
    puts("Create");
    int ret = do_create(filename, &db_file);
    if(ret == 0) {
        print_header(&(db_file.header));
    }
    free(filename);
    filename = NULL;
    return ret;
}

/********************************************************************//**
 * Displays some explanations.
 ********************************************************************** */
int
help (int args, char *argv[])
{
    puts("pictDBM [COMMAND] [ARGUMENTS]\nhelp: displays this help.\nlist <dbfilename>: list pictDB content.\ncreate <dbfilename> [options]: create a new pictDB.\n\toptions are:\n\t\t-max_files <MAX_FILES>: maximum number of files.\n\t\t\t\t\tdefault value is 10\n\t\t\t\t\tmaximum value is 100000\n\t\t-thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n\t\t\t\t\tdefault value is 64x64\n\t\t\t\t\tmaximum value is 128x128\n\t\t-small_res <X_RES> <Y_RES>: resolution for small images.\n\t\t\t\t\tdefault value is 256x256\n\t\t\t\t\tmaximum value is 512x512\nread <dbfilename> <pictID> [original|orig|thumbnail|thumb|small]:\n\tread an image from the pictDB and save it to a file.\n\tdefault resolution is \"original\".\ninsert <dbfilename> <pictID> <filename>: insert a new image in the pictDB.\ndelete <dbfilename> <pictID>: delete picture pictID from pictDB.");
    return 0;
}

/********************************************************************//**
 * Deletes a picture from the database.
 */
int
do_delete_cmd (int args, char *argv[])
{
    if (args < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    int ret1 = 0; // valeurs de retour
    int ret2 = 0;

    if(argv[1] == NULL ||strlen(argv[1]) > MAX_PIC_ID) { // empty pictID ou non trop long
        return ERR_INVALID_PICID;
    } else {
        struct pictdb_file f;
        ret1 = do_open(argv[0], "rb+", &f); // ouverture du fichier
        ret2 = do_delete(argv[1], &f); // supprime la pictID
        do_close(&f); // fermeture du fichier
    }
    if (ret1 != 0) {
        return ret1;
    } else {
        return ret2;
    }
    return 0;
}

/********************************************************************//**
 * Inserts a picture in the database.
 *
 */
int do_insert_cmd (int args, char *argv[])
{
    if (args < 3) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    int ret1 = 0; // valeurs de retour
    int ret2 = 0;
    if (argv[0] == NULL || strlen(argv[0]) > MAX_DB_NAME) {
        return ERR_INVALID_FILENAME; // erreur retournée si le nom dbfilename est vide ou trop long
    }
    struct pictdb_file db_file;
    ret1 = do_open(argv[0], "rb+", &db_file);
    if (ret1 != 0) {
        return ret1;
    }
    args--;
    argv++;
    char* pictID = calloc(strlen(argv[0])+1, sizeof(char));
    if (pictID == NULL) {
        return ERR_OUT_OF_MEMORY; // erreur retournée si problème lors du calloc
    }
    strncpy(pictID, argv[0], strlen(argv[0]));
    pictID[strlen(argv[0])] = '\0';
    if (pictID == NULL || strlen(pictID) > MAX_PIC_ID) {
		free(pictID);
        return ERR_INVALID_PICID; // erreur retournée si le nom pictID est vide ou trop long
    }
    args--;
    argv++;
    char* filename = calloc(strlen(argv[0])+1, sizeof(char));
    if (filename == NULL) {
        return ERR_OUT_OF_MEMORY; // erreur retournée si problème lors du calloc
    }
    strncpy(filename, argv[0], strlen(argv[0]));
    filename[strlen(argv[0])] = '\0';
    args--;
    argv++;

    // on lit l'image .jpg et on l'écrit dans pict_table:
    FILE* file;
    file = fopen(filename, "rb");
    if (file == NULL) {
		free(filename);
        return ERR_IO;
    }
    fseek(file, 0, SEEK_END);
    size_t pict_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* pict_table = calloc(pict_size, sizeof(char));
    if (pict_table == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    fread(pict_table, pict_size, sizeof(char), file);
    fclose(file);
    ret2 = do_insert(pict_table, pict_size, pictID, &db_file);
    if (ret1 != 0) {
		free(pict_table);
        return ret1;
    } else {
        return ret2;
    }
    do_close(&db_file);
    free(pictID);
    free(filename);
    free(pict_table);
    return 0;
}

/********************************************************************//**
* Reads a picture of the database.
*
*/

int
do_read_cmd (int args, char *argv[])
{
    if(args<2) { //si 2 argument alors resolution par defaut sinon resolution passé en paramètre.
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    char* resolution =  calloc( 9+1, sizeof(char));// 9+1 pour _original et \0

    if(resolution == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    strncpy(resolution, "orig",5);//resolution par default: original -> _orig (resolution_atoi).

    char * dbfilename = argv[0];
    struct pictdb_file db_file;

    int joue = do_open(dbfilename, "rb+", &db_file);
    if(joue != 0) {
		free(resolution);
        return joue;//erreur retournée si le ficchier ne peut pas etre ouvert.
    }

    argv++;
    args--;
    char* pictID = calloc(strlen(argv[0])+1, sizeof(char));//original_prefix.
    if (pictID == NULL) {
		free(resolution);
        return ERR_OUT_OF_MEMORY; // erreur retournée s'il y a un problème lors du calloc
    }
    strncpy(pictID, argv[0], strlen(argv[0]));
    pictID[strlen(argv[0])] = '\0';
    if (pictID == NULL || strlen(pictID) > MAX_PIC_ID) {
		if(strlen(pictID)>MAX_PIC_ID && pictID!=NULL){
			free(pictID);
		}
		free(resolution);
        return ERR_INVALID_PICID; // erreur retournée si le nom pictID est vide ou trop long
    }

    args--;
    if (args!=0) {
        argv++;
        resolution = realloc(resolution, strlen(argv[0])+1);//si 3 arguments alors pas de resolution par defaut.
        if(resolution==NULL) {
			free(pictID);
            return ERR_OUT_OF_MEMORY;
        }
        int new_res = resolution_atoi(argv[0]);
        if(new_res == 0) {
            strncpy(resolution , "thumb", 6);
        } else if(new_res == 1) {
            strncpy(resolution, "small", 6);
        } else if(new_res == 2) {
            strncpy(resolution, "orig",5);
        }
    }

   

    int resolution_bis = resolution_atoi(resolution);//il faut un int pour do_read et une string pour create_name.

    char * table = malloc(sizeof(char));
    uint32_t image_size;

    int ok_2 = do_read(pictID, resolution_bis, &table, &image_size, &db_file);//lire l'image

    if(ok_2!=0) {
		free(resolution);
        return ok_2;
    }


    char * filename;
    filename= malloc(sizeof(char*));
    int ok = create_name(pictID, resolution, &filename);//filename du type pic1_small.jpeg
    if(ok!=0) {
		free(filename);
		free(pictID);
		free(resolution);
        return ERR_IO;//pas sur de l'erreur.
    }
    /*Equivalent de write_disk_image*/
    FILE* result;
    result = fopen(filename,"wb");//cree fichier avec comme nom type pic1_small.jpeg

    if(result == NULL) {
	free(resolution);
	free(table);
	free(filename);
	free(pictID);
        return ERR_IO;
    }
    int ok_3 = fwrite(table, image_size, 1, result);

    if(ok_3!=1) {
		free(resolution);
		free(table);
		free(filename);
		free(pictID);
        return ERR_IO;
    }


    free(table);
    free(filename);
	free(resolution);
    free(pictID);
    pictID=NULL;
    do_close(&db_file);
    return 0;
}

/**
 * @brief lit une image depuis un filename et la met dans image.
 *
 * @param filename: nom de fichier, image : l'image a retourner.
 */
/*int read_disk_image(const char* filename, char * image)
{
	 FILE* result ;
	 result = fopen(filename, "rb"); //ouvrir en mode lecture.

	 if(result == NULL){
		 return ERR_IO;
	 }
	image= malloc(sizeof(FILE));
	if(image==NULL) {
            return ERR_OUT_OF_MEMORY;
    }
	int ok = fread(image,sizeof(FILE),1, result);
	if(ok!=1){
		return ERR_IO;
	}
	return 0;//retourne int pour gestion d'erreur dans do_insert_cmd.
}*/

int create_name(char * original_prefix, char * resolution_suffix, char ** filename)
{

    if( original_prefix == NULL || resolution_suffix==NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    size_t length_prefix =0;
    length_prefix=  strlen(original_prefix);

    size_t length_suffix = strlen(resolution_suffix);
    //~ filename = malloc(sizeof(char));
    *filename = calloc(length_prefix + length_suffix + 5 + 1, sizeof(char));//+5 pour .jpeg

    if(length_prefix <= MAX_PIC_ID) {
        if(strncmp(resolution_suffix, "orig", length_suffix)==0 ||
           strncmp(resolution_suffix, "small", length_suffix)==0 ||
           strncmp(resolution_suffix, "thumb", length_suffix)==0) {
            // size_t total_length = length_prefix + length_suffix +1;

            *filename = strncat(*filename,original_prefix, length_prefix);
            *filename = strncat(*filename, "_", 2);//2 pour \0
            *filename = strncat(*filename, resolution_suffix, length_suffix);
            *filename = strncat(*filename, ".jpeg", 6);//6 pour \0

            return 0;
        }
        return ERR_INVALID_ARGUMENT;
    }
    return ERR_INVALID_ARGUMENT;
}

int gc_command(int args, char* argv[]){
	
	if(args<2){
		return ERR_NOT_ENOUGH_ARGUMENTS;
	}
	struct pictdb_file file;
	
	
	
	
}


/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[])
{


    struct command_mapping* commands = calloc(6, sizeof(struct command_mapping)); // ne pas oublier d'ajouter read!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if (commands == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    // instanciation du tableau commands
    commands[0].name = "list";
    commands[0].cmd = do_list_cmd;
    commands[1].name = "create";
    commands[1].cmd = do_create_cmd;
    commands[2].name = "delete";
    commands[2].cmd = do_delete_cmd;
    commands[3].name = "help";
    commands[3].cmd = help;
    commands[4].name = "insert";
    commands[4].cmd = do_insert_cmd;
    commands[5].name = "read";
    commands[5].cmd = do_read_cmd;

    int ret = 0;
    unsigned int br_loop = 0; // si br_loop = 1 alors la fonction a été trouvée et la boucle for peut s'arrêter.
    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        argc--;
        argv++; // skip command call name
        for (unsigned int i = 0; i < 6 && br_loop == 0; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                argv++;
                argc--; // skip function call name

                ret = commands[i].cmd(argc, argv);
                br_loop = 1;
            }
        }
        if (br_loop == 0) { // si aucune fonction n'a été appelée
            ret = ERR_INVALID_COMMAND;
        }
        // en cas d'erreur: retourne l'erreur et la fonction help:
        if (ret) {
            fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
            (void)help(argc, argv);
        }
    }
    return ret;
}
