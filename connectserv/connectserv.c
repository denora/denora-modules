/* ConnnectServ Module
** Trystan Scott Lee trystan@nomadirc.net
** 
** based off code by 
** NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
*/

#include "denora.h"
#include "connectserv.h"
#define AUTHOR "Trystan"
#define VERSION "1.0.4"

/* 
  See changelog.txt included with the tar for what has changed
  See readme.txt included with the tar for how to configure
*/

/*******************************************************************************************/

int DenoraInit(int argc, char **argv)
{
    Message *msg = NULL;
    EvtHook *hook;
    PrivMsg *p;
    int status;

    ConnectServConnected = 0;

    hook = createEventHook(EVENT_RELOAD, do_reload);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_NEWNICK, do_newnick);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_CHANGE_NICK, cs_user_nick);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_SERVER, cs_server_join);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_SQUIT, cs_server_quit);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_GLOBAL_KILL, cs_global_kill);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_SERVER_KILL, cs_server_kill);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_USER_LOGOFF, cs_user_logoff);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_USER_MODE, cs_user_modes);
    status = moduleAddEventHook(hook);


    if (!load_config()) {
        return MOD_STOP;
    }

    if (status == MOD_ERR_OK) {
        cs_addClient();
        ConnectServConnected = 1;
        cs_addMessageList();
        p = createPrivmsg(s_ConnectServ, ConnectServ);
        addPrivMsg(p);
    } else {
        alog(LOG_NORMAL, "Error status was [%d]", status);
        return MOD_STOP;
    }
    my_add_languages();

    if (CsJoinLogChan) {
        denora_cmd_join(s_ConnectServ, LogChannel, time(NULL));
    }

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    alog(LOG_NORMAL, "ConnectServ%s: loaded", MODULE_EXT);
    return MOD_CONT;
}

/*******************************************************************************************/

void cs_addMessageList(void)
{
    Command *c;
    c = createCommand("ABOUT", cs_about, NULL, -1, -1, -1, -1);
    moduleAddCommand(ConnectServ_cmdTable, c, MOD_UNIQUE);

    c = createCommand("VERSION", cs_version, NULL, -1, -1, -1, -1);
    moduleAddCommand(ConnectServ_cmdTable, c, MOD_UNIQUE);

}

/*******************************************************************************************/

void DenoraFini(void)
{
    if (ConnectServConnected) {
        cs_delClient();
    }
}

/*******************************************************************************************/

void cs_addClient(void)
{
    if (ConnectServ_modes) {
        denora_cmd_bot_nick(s_ConnectServ, ConnectServ_ident, ConnectServ_host,
                           ConnectServ_realname, ConnectServ_modes);
    } else {
        denora_cmd_bot_nick(s_ConnectServ, ConnectServ_ident, ConnectServ_host,
                           ConnectServ_realname, (char *) ircd->statservmode);
    }
}

/*******************************************************************************************/

void cs_delClient(void)
{
    send_cmd(s_ConnectServ, "QUIT :Module Unloaded!");
}

/*******************************************************************************************/

void ConnectServ(User * u, char *buf)
{
    char *cmd, *buf2;
    char *str;

    buf2 = sstrdup(buf);
    cmd = myStrGetToken(buf, ' ', 0);
    str = myStrGetTokenRemainder(buf, ' ', 1);

    SET_SEGV_LOCATION();

    if (!cmd) {
        free(buf2);
        return;
    } else if (stricmp(cmd, "\1PING") == 0) {
        denora_cmd_ctcp(s_StatServ, u->nick, "PING %s", str);
    } else {
        SET_SEGV_LOCATION();
        mod_run_cmd(s_StatServ, u, STATSERV, cmd, str);
    }
    free(cmd);
    if (str) {
        free(str);
    }
    free(buf2);
}

/*******************************************************************************************/

int cs_about(User * u, int ac, char **av)
{
    if (Log_ConnectServ) {
        alog(LOG_NORMAL, "%s : %s used ABOUT", s_ConnectServ, u->nick);
    }
    moduleNoticeLang(s_ConnectServ, u, CS_HELP_ABOUT);
    return MOD_CONT;
}

/*******************************************************************************************/

int cs_version(User * u, int ac, char **av)
{
    if (Log_ConnectServ) {
        alog(LOG_NORMAL, "%s : %s used VERSION", s_ConnectServ, u->nick);
    }
    moduleNoticeLang(s_ConnectServ, u, CS_DISPLAY_VERSION, s_ConnectServ, VERSION);
    return MOD_CONT;
}

/*******************************************************************************************/

int do_reload(int argc, char **argv)
{
    load_config();
    return MOD_CONT;
}

/*******************************************************************************************/

int do_newnick(int ac, char **av)
{
	User *u;

	SET_SEGV_LOCATION();

	if (!ConnectServConnected) {
		return 1;
    }

	u = user_find(av[0]);
	
	
	if (!u) {
		return MOD_CONT;
    }

	if (!stricmp(u->nick, s_ConnectServ)) {
		return MOD_CONT;
	}

	/* Print Connection Notice */
	if (SignOnMessage) {
        if (!UseColorfulMessages) {
            alog(LOG_NORMAL, "\2SIGNON\2 %s (%s@%s - %s) has signed on at %s", u->nick, u->username, u->host, u->realname, u->server->name);
        } else {
            alog(LOG_NORMAL, "\2\0034SIGNED ON\2 user: \2%s\2 (%s@%s - %s) at: \2%s\2\003", u->nick, u->username, u->host, u->realname, u->server->name);
        }   
	}
	return MOD_CONT;
}


/* 
 * Echo nick changes
 */
int cs_user_nick(int ac, char **av)
{
	User *u;

	SET_SEGV_LOCATION();

	if (!ConnectServConnected) {
		return MOD_CONT;
    }

	u = user_find(av[1]);
	if (!u) {
		return MOD_CONT;
    }

	if (!stricmp(u->nick, s_ConnectServ)) {
		return MOD_CONT;
	}
	if (NickChangeMessage) {
        if (!UseColorfulMessages) {
            alog(LOG_NORMAL, "\2NICK\2 %s (%s@%s) changed their nick to %s", av[0], u->username, u->host, av[1]);
        } else {
            alog(LOG_NORMAL, "\2\0037Nick Change\2 user: \2%s\2 (%s@%s) Changed their nick to \2%s\2\003", av[0], u->username, u->host, av[1]);
        }   
	}
	return 1;
}

int cs_server_join(int ac, char **av)
{
	Server *s;

	SET_SEGV_LOCATION();
	if (!ConnectServConnected)
		return MOD_CONT;

	s = server_find(av[0]);
	if (!s) {
		return MOD_CONT;
    }

	alog(LOG_NORMAL, "\2SERVER\2 %s has joined the Network at %s", s->name, s->uplink->name);

	return MOD_CONT;
}

/*******************************************************************************************/

int cs_server_quit(int ac, char **av)
{
	Server *s;

	SET_SEGV_LOCATION();
	if (!ConnectServConnected)
		return MOD_CONT;

	s = server_find(av[0]);
	if (!s) {
		return MOD_CONT;
    }

	alog(LOG_NORMAL, "\2SERVER\2 %s has left the Network at %s for %s", s->name, s->uplink->name, (ac == 2) ? av[1] : "");
	return MOD_CONT;
}

int cs_server_kill(int ac, char **av)
{
	User *u;

	SET_SEGV_LOCATION();

	if (!ConnectServConnected)
		return 1;

	u = user_find(av[1]);
	if (!u) /* User not found */
		return 1;

	if (!stricmp(u->nick, s_ConnectServ)) {
		return MOD_CONT;
	}
    if (UseColorfulMessages) {
            alog(LOG_NORMAL, "\2SERVER KILL\2 user: \2%s\2 (%s@%s) was Killed by the Server \2%s\2 - Reason sighted: \2%s\2", u->nick, u->username, u->host, av[0], av[2]);
    } else {
            alog(LOG_NORMAL, "\2SERVER KILL\2 %s (%s@%s) was killed by the server %s - Reason sighted: \2%s\2", u->nick, u->username, u->host, av[0], av[2]);
    }   
	return 1;
}

int cs_global_kill(int ac, char **av)
{
	User *u;

	SET_SEGV_LOCATION();

	if (!ConnectServConnected)
		return 1;

	u = user_find(av[1]);
	if (!u) /* User not found */
		return 1;

	if (!stricmp(u->nick, s_ConnectServ)) {
		return MOD_CONT;
	}
    if (UseColorfulMessages) {
            alog(LOG_DEBUG,  "\2\00312GLOBAL KILL\2 user: \2%s\2 (%s@%s) was Killed by \2%s\2 - Reason sighted: \2%s\2\003", u->nick, u->username, u->host, av[0], av[2]);
    } else {
            alog(LOG_DEBUG, "\2GLOBAL KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2", u->nick, u->username, u->host, av[0], av[2]);
    }   
	return MOD_CONT;
}


int cs_report_mode(User* u, int add, const char *mode, const char* mode_desc)
{
        if (UseColorfulMessages) {
    		alog(LOG_DEBUG, "\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c)\003", u->nick, 
	    		add?"now":"no longer", 
		    	mode_desc,
			    add?'+':'-',
			    *mode);
        } else {
    		alog(LOG_DEBUG, "\2MODE\2 %s is %s a %s (%c%c)", u->nick, 
	    		add?"now":"no longer", 
		    	mode_desc,
			    add?'+':'-',
			    *mode);
        }
	return MOD_CONT;
}

int cs_user_modes(int ac, char **av)
{
	int add;
	char *modes;
	User *u;

	SET_SEGV_LOCATION();

	if (!ConnectServConnected) {
        return MOD_CONT;
    }

	u = user_find(av[1]);
	if (!u) { 
		return MOD_CONT;
	}

	if (!stricmp(u->nick, s_ConnectServ)) {
		return MOD_CONT;
	}

    add  = (!stricmp(EVENT_MODE_ADD, av[0]) ? 1 : 0);

	modes = (char *)(av[2]);

    switch (*modes) {
		case 'N':
            if (denora_get_ircd() == IRC_UNREAL32) {
    			cs_report_mode(u, add, "N", "Network Administrator");
            }
			break;
		case 'C':
            if (denora_get_ircd() == IRC_UNREAL32) {
    			cs_report_mode(u, add, "C", "Co-Network Administrator");
            }
			break;
		case 'A':
            if (denora_get_ircd() == IRC_UNREAL32) {
    			cs_report_mode(u, add, "A", "Server Administrator");
            }
			break;
		case 'B':
            if (denora_get_ircd() == IRC_UNREAL32) {
    			cs_report_mode(u, add, "B", "Bot");
            }
			break;
		case 'a':
            if (denora_get_ircd() == IRC_UNREAL32) {
    			cs_report_mode(u, add, "a", "Services Administrator");
            }
			break;
		case 'o':
   			cs_report_mode(u, add, "o", "global operator");
            break;
		case 'O':
   			cs_report_mode(u, add, "O", "local operator");
            break;
		case 'S':
            if (denora_get_ircd() == IRC_UNREAL32) {
    			cs_report_mode(u, add, "S", "Network Service");
            }
			break;
		default:
			break;
	}
	return MOD_CONT;
}


int cs_user_logoff(int ac, char **av)
{
	User *u;

	SET_SEGV_LOCATION();

	if (!ConnectServConnected) {
        return MOD_CONT;
    }

    if (ac <= 1) {
        return MOD_CONT;
    }

	u = user_find(av[0]);
	if (!u) {
		return MOD_CONT;
    }

	if (!stricmp(u->nick, s_ConnectServ)) {
		return MOD_CONT;
	}

	if (SignOffMessage) {
        if (UseColorfulMessages) {
  		  alog(LOG_NORMAL,"\2\0033Signed Off\2 user: %s (%s@%s - %s) at: %s - %s\003",
			  u->nick, u->username, u->host, u->realname,
			  u->server->name, av[1]);
        } else {
  		  alog(LOG_NORMAL,"\2SIGNOFF\2 %s (%s@%s - %s) has signed off at %s - %s",
			  u->nick, u->username, u->host, u->realname,
			  u->server->name, av[1]);
        }
	}
	return MOD_CONT;
}

/*******************************************************************************************/

int load_config(void)
{
    int i;
    int retval = 1;

    Directive confvalues[][1] = {
        {{"LogEvents", {{PARAM_SET, 0, &Log_ConnectServ}}}},
        {{"JoinLogChan", {{PARAM_SET, 0, &CsJoinLogChan}}}},
        {{"ConnectServNick", {{PARAM_STRING, 0, &s_ConnectServ}}}},
        {{"ConnectServIdent", {{PARAM_STRING, 0, &ConnectServ_ident}}}},
        {{"ConnectServHost", {{PARAM_STRING, 0, &ConnectServ_host}}}},
        {{"ConnectServReal", {{PARAM_STRING, 0, &ConnectServ_realname}}}},
        {{"ConnectServModes", {{PARAM_STRING, 0, &ConnectServ_modes}}}},
        {{"SignOnMessage", {{PARAM_SET, 0, &SignOnMessage}}}},
        {{"SignOffMessage", {{PARAM_SET, 0, &SignOffMessage}}}},
        {{"UseColorfulMessages", {{PARAM_SET, 0, &UseColorfulMessages}}}},
        {{"NickChangeMessage", {{PARAM_SET, 0, &NickChangeMessage}}}},
    };
    for (i = 0; i < 6; i++)
        moduleGetConfigDirective((char *) "connectserv.conf", confvalues[i]);

    if (!s_ConnectServ) {
        alog(LOG_NORMAL, "Error: ConnectServNick missing");
        retval = 0;
    }
    if (!ConnectServ_ident) {
        alog(LOG_NORMAL, "Error: ConnectServIdent missing");
        retval = 0;
    }
    if (!ConnectServ_host) {
        alog(LOG_NORMAL, "Error: ConnectServHost missing");
        retval = 0;
    }
    if (!ConnectServ_realname) {
        alog(LOG_NORMAL, "Error: ConnectServReal missing");
        retval = 0;
    }
    return retval;
}

/*******************************************************************************************/

void my_add_languages(void)
{
    /* English (US) */
    const char *langtable_en_us[] = {
        "Unknown Lang index",
        " ",
	    "\2ConnectServ\2 tracks users joining and leaving the network,\n"
	    "kills, operator modes, nickname changes, server connects and\n"
	    "server quits. These events can be reported to the services\n"
	    "channel and logged.",
        "\2%s Version Information\2 : %s"
    };

    moduleInsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, (char **) langtable_en_us);
}



