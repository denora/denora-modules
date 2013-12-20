#include "denora.h"
#define AUTHOR "Trystan"
#define VERSION "1.1.2"
#define MYNAME "php_run"


int php_exec(User * u, int ac, char **av);
int DenoraInit(int argc, char **argv);
void DenoraFini(void);

int DenoraInit(int argc, char **argv)
{
	Command *c;
	c = createCommand("PHP", php_exec, is_oper, -1, -1, -1, -1);
	moduleAddCommand(STATSERV, c, MOD_HEAD);

	if (denora->debug)
	{
		protocol_debug(NULL, argc, argv);
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void DenoraFini(void)
{
	alog(LOG_NORMAL, "Unloading ss_uptime.%s", MODULE_EXT);
}


int php_exec(User * u, int ac, char **av)
{

  FILE *fp;
  int status;
  char path[1035];
  char buf[256];

   snprintf(buf, sizeof(buf), "php -f %s/%s", STATS_DIR, av[0]);

  /* Open the command for reading. */
  fp = popen(buf, "r");
  if (fp == NULL) {
    notice(s_StatServ, u->nick, "Failed to run command\n" );
    return MOD_CONT;
  }

  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    notice(s_StatServ, u->nick, "%s", path);
  }

  /* close */
  pclose(fp);

 return   MOD_CONT;
}
