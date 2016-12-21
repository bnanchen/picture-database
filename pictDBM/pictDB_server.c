/**
* @file pictDB_server.c
* @brief web server code.
*
*/
#include  <vips/vips.h>

#include "pictDB.h"
#include "libmongoose/mongoose.h"

#define MAX_QUERY_PARAM 5

static const char* s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
struct pictdb_file db_file; // en global

/**
 * @brief fonction permettant de faire l'action correspondant (do_list) et de fournir une réponse HTTP valide à l'URI /pictDB/list.
 *
 * @param moongoose connection: struct mg_connection* conn.
 */
static void handle_list_call(struct mg_connection* conn)
{
    char* chain = do_list(&db_file, JSON);
    if (chain == NULL) {
        mg_printf(conn, "HTTP/1.1 500 Internal Servor Error\r\n");
    } else {
        mg_printf(conn, "HTTP/1.1 200\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", strlen(chain), chain);
    }
}

/**
 * @brief fonction permettant de retourner un code d'erreur HTTP.
 *
 * @param moongoose connection: struct mg_connection* nc; code d'erreur: int error.
 */
void mg_error(struct mg_connection* nc, int error)
{
    mg_printf(nc, "HTTP/1.1 500 : %d\r\n", error);
}

/**
 * @brief une fonction qui va séparer la query string en morceaux.
 *
 * @param tableau de pointeurs de chaines de caractères de taille MAX_QUERY_PARAM: char* result[]; chaine de caractères de taille (MAX_PIC_ID + 1) * MAX_QUERY_PARAM;
 * la query string à découper: const char* src; chaine de caractère contenant la liste des délimiteurs de token: char* delim; la longueur de la query string: size_t len.
 */
void split(char* result[], char * tmp , const char * src, char* delim, size_t len)
{
    strncpy(tmp, src, len);//max copy len

    char * tok =  strtok((char*) src, delim);

    int i = 0;
    while(tok!=NULL && i<MAX_QUERY_PARAM) {
        strncpy(result[i], tok, strlen(tok));
        tok= strtok(NULL, delim);
        i++;
    }
}
/**
 * @brief fonction permettant de faire l'action correspondant (do_read) et de fournir une réponse HTTP valide à l'URI /pictDB/read.
 *
 * @param moongoose connection: struct mg_connection* conn; HTTP message: struct http_message* http_mess.
 */
static void handle_read_call(struct mg_connection* conn, struct http_message* http_mess)
{
    char * result[MAX_QUERY_PARAM];
    char * tmp= calloc(MAX_QUERY_PARAM, MAX_PIC_ID+1);
    if (tmp == NULL) {
        mg_error(conn, ERR_OUT_OF_MEMORY); // erreur retournée si problème lors du calloc
    }
    char * delim = calloc(3, sizeof(char));
    if (delim == NULL) {
        mg_error(conn, ERR_OUT_OF_MEMORY); // erreur retournée si problème lors du calloc
    }
    for (size_t i = 0; i < MAX_QUERY_PARAM; i++) {
        result[i] = calloc(MAX_PIC_ID+1, sizeof(char)); // allocation de chaque élément du tableau result
    }
    strcpy(delim,"& =");


    split(result, tmp, http_mess->query_string.p, delim, http_mess->query_string.len);
    char * table = NULL;
    uint32_t image_size;
    int new_res;
    char* pict_id = calloc(MAX_PIC_ID, sizeof(char));
    if (pict_id == NULL) {
        mg_error(conn, ERR_OUT_OF_MEMORY); // erreur retournée si problème lors du calloc
    }

    for(size_t i = 0; i < MAX_QUERY_PARAM && result[i]!= NULL; ++i) {
        if((strcmp(result[i],"res") == 0) && result[i+1] != NULL) {
            new_res = resolution_atoi(result[i+1]); // result[i+1] contient ici la resolution
        }

        if((strcmp(result[i], "pict_id") == 0) && result[i+1] != NULL) {
            strcpy(pict_id, result[i+1]); // copie la pict_id delimite par split
        }
    }
    int ok = do_read(pict_id, new_res, &table, &image_size, &db_file);
    if(ok == 0) {
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: <%u>\r\n\r\n" , image_size);
        mg_send(conn, table, image_size);

    } else {
        mg_error(conn, ok); // appelle mg_error avec l'erreur de do_read
    }
    conn->flags |= MG_F_SEND_AND_CLOSE;

    // libération de la mémoire:
    for(size_t i=0; i<MAX_QUERY_PARAM; ++i) {
        free(result[i]);
        result[i] = NULL;
    }
    free(tmp);
    tmp = NULL;
    free(pict_id);
    pict_id = NULL;
    free(table);
    table = NULL;
    free(delim);
    delim = NULL;
}

/**
 * @brief fonction permettant de faire l'action correspondant (do_insert) et de fournir une réponse HTTP valide à l'URI /pictDB/insert.
 *
 * @param moongoose connection: struct mg_connection* conn; HTTP message: struct http_message* http_mess.
 */
static void handle_insert_call(struct mg_connection* conn, struct http_message* http_mess)  // gérer les erreurs!
{
    int ret = 0; // valeur utile en cas d'erreur
    char var_name[100];
    char filename[100];
    const char* chunk;
    size_t chunk_len;
    size_t n1 = 0;
    size_t n2 = 0;
    while ((n2 = mg_parse_multipart(http_mess->body.p + n1, http_mess->body.len - n1, var_name, sizeof(var_name), filename, sizeof(filename), &chunk, &chunk_len)) > 0) {
        n1 += n2;
    }
    ret = do_insert(chunk, chunk_len, filename, &db_file);
    if (ret != 0) {
        mg_error(conn, ret);
    } else {
        mg_printf(conn, "HTTP/1.1 302 Found\r\nLocation: http://localhost:%s/index.html\r\n\r\n", s_http_port);
    }
    conn->flags |= MG_F_SEND_AND_CLOSE;
}

/**
 * @brief fonction permettant de faire l'action correspondant (do_delete) et de fournir une réponse HTTP valide à l'URI /pictDB/delete.
 *
 * @param moongoose connection: struct mg_connection* conn; HTTP message: struct http_message* http_mess.
 */
static void handle_delete_call(struct mg_connection* conn, struct http_message* http_mess)
{
    char * result[MAX_QUERY_PARAM];
    char * tmp= calloc(MAX_QUERY_PARAM, MAX_PIC_ID+1);
    if (tmp == NULL) {
        mg_error(conn, ERR_OUT_OF_MEMORY); // erreur retournée si problème lors du calloc
    }
    const char * delim = "& =";
    /*if (delim == NULL) {
        mg_error(conn, ERR_OUT_OF_MEMORY); // erreur retournée si problème lors du calloc
    }
    strcpy(delim, "& =");*/

    char * pict_id= calloc(MAX_PIC_ID, sizeof(char));
    if (pict_id == NULL) {
        mg_error(conn, ERR_OUT_OF_MEMORY); // erreur retournée si problème lors du calloc
    }
    for (size_t i = 0; i < MAX_QUERY_PARAM; i++) {
        result[i] = calloc(MAX_PIC_ID+1, sizeof(char)); // allocation de chaque élément du tableau result
        if (result[i] == NULL) {
            mg_error(conn, ERR_OUT_OF_MEMORY); // erreur retournée si problème lors du calloc
        }
    }
    split(result, tmp, http_mess->query_string.p, (char*) delim, http_mess->query_string.len);
    for(size_t i = 0; i < MAX_QUERY_PARAM && result[i]!= NULL; ++i) {
        if((strcmp(result[i], "pict_id")==0) && result[i+1]!= NULL) {
            strcpy(pict_id, result[i+1]);// recupere la pict_id
        }
    }
    int ok = do_delete(pict_id, &db_file);
    if(ok == 0) {
        mg_printf(conn, "HTTP/1.1 302 Found\r\nLocation: http://localhost:%s/index.html\r\n\r\n", s_http_port);
    } else {
        mg_error(conn, ok);
    }
    conn->flags |= MG_F_SEND_AND_CLOSE;

    // libération de la mémoire:
    for(size_t i=0; i<MAX_QUERY_PARAM; ++i) {
        free(result[i]);
        result[i] = NULL;
    }
    free(tmp);
    tmp = NULL;
    free(pict_id);
    pict_id = NULL;
    //free(delim);
    //delim = NULL;
}



/**
 * @brief fonction permettant de choisir que faire par rapport au message de l'URI.
 *
 * @param moongoose connection: struct mg_connection* conn, nombre correspondant à l'event: int event; les données correspondant à l'event: void* event_data.
 */
static void event_handler(struct mg_connection* conn, int event, void* event_data)
{
    struct http_message* http_mess = (struct http_message*) event_data;
    if (event == MG_EV_HTTP_REQUEST) {

        // on inspecte pour choisir le bon handle le message reçu envoyé par l'URI:
        if (mg_vcmp(&http_mess->uri, "/pictDB/list") == 0) { // handle du list
            handle_list_call(conn);
        } else if(mg_vcmp(&http_mess->uri, "/pictDB/read") == 0) { //handle du read
            handle_read_call(conn,http_mess);
        } else if(mg_vcmp(&http_mess->uri, "/pictDB/insert") == 0) { //handle du insert
            handle_insert_call(conn, http_mess);
        } else if(mg_vcmp(&http_mess->uri, "/pictDB/delete") == 0) { //handle du delete
            handle_delete_call(conn, http_mess);
        } else {
            mg_serve_http(conn, http_mess, s_http_server_opts); // serve static content
        }
    }
}

/********************************************************************//**
* MAIN
*/
int main(int argc, char* argv[])  // ne pas oublier les vérifications
{
    struct mg_mgr mgr; // mongoose event manager
    struct mg_connection* conn; // mongoose connection

    if (VIPS_INIT(argv[0])) {
        vips_error_exit("unable to start VIPS");
    }
    mg_mgr_init(&mgr, NULL);
    conn = mg_bind(&mgr, s_http_port, event_handler); // create listening connection

    // Set up HTTP parameters:
    mg_set_protocol_http_websocket(conn); // Attach built-in HTTP event handler to the given connection
    s_http_server_opts.document_root = ".";  // Serve current directory
    s_http_server_opts.dav_document_root = ".";  // Allow access via WebDav
    s_http_server_opts.enable_directory_listing = "yes";
    // Set up pictDB:
    if (argc < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    int ret = 0; // valeur de retour
    argc--;
    argv++; // on soustrait ./pictDB_server
    do_open(argv[0], "rb+", &db_file); // ouverture du db_file
    if (ret != 0) {
        return ret;
    }
    print_header(&(db_file.header)); // imprime le header dans le terminal

    for(;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    // Shutdown:
    vips_shutdown();
    do_close(&db_file);
    mg_mgr_free(&mgr);
    return 0;
}
