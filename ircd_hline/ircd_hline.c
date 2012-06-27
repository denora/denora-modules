/*
 * ircd_hline module for Denora 1.2+
 * (c) Trystan
 */

#include "denora.h"
#define AUTHOR "Trystan"
#define VERSION "1.1.1"
#define MYNAME "ircd_hline"

int DenoraInit(int argc, char **argv);
void DenoraFini(void);
int my_server(int argc, char **argv);
int get_hline(char *source, int ac, char **av);
void create_field(void);

int DenoraInit(int argc, char **argv)
{
    EvtHook *hook = NULL;
    Message *msg = NULL;
    int status;

    hook = createEventHook(EVENT_SERVER, my_server);
    status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        /* something went wrong say something about */
        alog(LOG_NORMAL, "[%s%s] unable to bind to EVENT_SERVER error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
        return MOD_STOP;
    }

    if (denora_get_ircd() == IRC_ULTIMATE3) {
        msg = createMessage("244", get_hline);
        status = moduleAddMessage(msg, MOD_HEAD);
        if (status != MOD_ERR_OK) {
            /* something went wrong say something about */
            alog(LOG_NORMAL, "[%s%s] unable to bind to 244 error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
            return MOD_STOP;
        }
    } else {
        return MOD_STOP;
    }

    create_field();

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void DenoraFini(void)
{
    alog(LOG_NORMAL, "Unloading ircd_hline%s", MODULE_EXT);
}

int my_server(int argc, char **argv)
{
    if (argc >= 1) {
        if (stricmp(ServerName, argv[0])) {
            denora_cmd_stats(s_StatServ, "H", argv[0]);
        }
    }
    return MOD_CONT;
}

void create_field(void)
{
	rdb_query(QUERY_LOW, "ALTER TABLE `%s` ADD `hline` ENUM( 'Y', 'N' ) DEFAULT 'N' NOT NULL;", ServerTable);
}

int get_hline(char *source, int ac, char **av)
{
    Server *s;
    int id;

    if (denora_get_ircd() == IRC_ULTIMATE3) {
        s = server_find(source);
        if (!s) {
            return MOD_CONT;
        }
        if (!strcmp(av[1], "H")) {
            if (*av[4] != '*') {
                id = db_getserver(av[4]);
                if (id) {
                    rdb_query(QUERY_LOW, "UPDATE %s SET hline=1 WHERE servid=%d", ServerTable, id);
                }
            }
        }
    }
    return MOD_CONT;
}
