/*
 * @file image_content.c
 * @brief implémentation de la fonction image_content.c
 *
 */

#include "pictDB.h"
#include "image_content.h"
#include  <vips/vips.h>
#include <stdlib.h>
#include <string.h>




double
shrink_value(VipsImage *image,
             int max_thumbnail_width,
             int max_thumbnail_height)
{
    const double h_shrink = (double) max_thumbnail_width  / (double) image->Xsize ;
    const double v_shrink = (double) max_thumbnail_height / (double) image->Ysize ;
    return h_shrink > v_shrink ? v_shrink : h_shrink ;
}
/**
 * @brief creer une nouvelle image avec une la resolution donnée en param si l'image à l'index donné n'existe pas à la résolution souhaité.
 *
 * @param une resolution, une struct pictdb_file, et un index
 */

int lazily_resize(int resolution, struct pictdb_file * db_file,size_t index)
{
    int ancienne_resolution=0;
    if(resolution!= RES_SMALL && resolution!= RES_THUMB) {
        if(resolution==RES_ORIG) {
            return 0;
        } else {
            return ERR_INVALID_ARGUMENT;//Not a valid resolution
        }
    }
    fseek(db_file->fpdb, sizeof(struct pictdb_header),SEEK_SET);//saute le header.

    if(db_file->metadata[index].offset[resolution] == 0 || db_file->metadata[index].size[resolution] == 0) {
        //~ ancienne_resolution = db_file->metadata[index].res_orig[0];
        //~ struct pict_metadata* new_metadata = malloc(sizeof(struct pict_metadata));
        //struct pictdb_metadata new_metadata;
        /*   if(new_metadata!=NULL) {

               strcpy(new_metadata->pict_id, db_file->metadata[index].pict_id);
               for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                   new_metadata->SHA[i] = db_file->metadata[index].SHA[i];
               }
               for (size_t i = 0; i < 2; i++) {
                   new_metadata->res_orig[i] = db_file->metadata[index].res_orig[i];
               }
               for (size_t i = 0; i < NB_RES; i++) {
                   new_metadata->size[i] = db_file->metadata[index].size[i];
                   new_metadata->offset[i] = db_file->metadata[index].offset[i];
               }
               new_metadata->is_valid = db_file->metadata[index].is_valid;


               /*changer l'image originale en memoire*/
        //~ fseek(db_file->fpdb,0,SEEK_END);//on veut rajouter a la fin du fichier l'image avec la nouvelle resolution.
        /* int number_of_write=0;
          number_of_write = fwrite(new, sizeof(struct pictdb_metadata),1, db_file->fpdb);// pas bon je pense.
          if(number_of_write !=1) {
        	printf("That_fail\n");
              return ERR_IO;//Mauvaise ecriture.
          }*/// a enlever...
        //~ }
        //mettre a jour en memoire et sur disque
        char * image = malloc(db_file->metadata[index].size[RES_ORIG]);
        fseek(db_file->fpdb, db_file->metadata[index].offset[RES_ORIG], SEEK_SET);//aller a l'image cherché

        fread(image, db_file->metadata[index].size[RES_ORIG],1,db_file->fpdb);
        VipsObject* process = VIPS_OBJECT( vips_image_new() );
        VipsImage** thumbs =(VipsImage**) vips_object_local_array( process, 2);

        //~ double ratio = resolution/db_file->metadata[index].res_orig[0];//ratio de la nouvelle resolution sur l'ancienne

        vips_jpegload_buffer( image, db_file->metadata[index].size[RES_ORIG], &thumbs[0],NULL);
        double ratio = shrink_value(thumbs[0], db_file->header.res_resized[resolution*2], db_file->header.res_resized[resolution*2+1]);
        //charger l'image originale en memoire
        //~ double ratio = shrink_value(image, db_file->header.res_resized[2*resolution], db_file->header.res_resized[2*resolution]);
        //ratio pour image non carrée?

        vips_resize(thumbs[0], &thumbs[1], ratio, NULL);//resize l'image

        fseek(db_file->fpdb,0,SEEK_END);
        size_t taille=0;
        void * buffer2=NULL;
        size_t len ;

        taille = vips_jpegsave_buffer( thumbs[1], &buffer2, &len,NULL); //ecrire en fin de fichier pictDB
        uint64_t offset_reso = ftell(db_file->fpdb);

        if(len!=0) {
            int pk = fwrite(buffer2, len, 1,db_file->fpdb);
            if(pk!=1) {
                return ERR_IO;
            }
        }
        db_file->metadata[index].size[resolution] = len;

        db_file->metadata[index].offset[resolution] = offset_reso;
        fseek(db_file->fpdb, sizeof(struct pictdb_header) + index * sizeof(struct pict_metadata), SEEK_SET);
        fwrite(&db_file->metadata[index], sizeof(struct pict_metadata), 1, db_file->fpdb);

        fseek(db_file->fpdb, sizeof(struct pictdb_header) + index * sizeof(struct pict_metadata), SEEK_SET);

        struct pict_metadata tmp ;
        fread(&tmp, sizeof(struct pict_metadata), 1, db_file->fpdb);
        g_free(buffer2);
        buffer2=NULL;
        free(image);

        image= NULL;
    }
    return 0;
}



/**
 * @brief permet d'avoir la hauteur et la largeur (en pixels) d'une image.
 *
 * @param pointeur sur la valeur où l'on veut écrire la hauteur: uint32_t* height; pointeur sur la valeur où l'on veut écrire la hauteur: uint32_t* width;
 * une image sous forme d'un pointeur (tableau) de caractères: const char* image_buffer; la taille de l'image: size_t image_size.
 */

int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer, size_t image_size)
{
    VipsObject* process = VIPS_OBJECT(vips_image_new());
    VipsImage** thumbs = (VipsImage**) vips_object_local_array(process,2);
    if(height==NULL || width==NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if(vips_jpegload_buffer((char*) image_buffer, image_size, &thumbs[0],NULL)==-1) {
        return ERR_VIPS;
    }
    *width=thumbs[0]->Xsize;
    *height = thumbs[0]->Ysize;
    return 0;
}
