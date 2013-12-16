/* ConnectServ Module
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

int DenoraInit(int argc, char **argv);
void DenoraFini(void);

int ConnectServConnected;

int do_reload(int argc, char **argv);
int load_config(void);
void my_add_languages(void);

void cs_addClient(void);
void cs_delClient(void);

int my_privmsg(char *source, int ac, char **av);

void ConnectServ(User * u, char *buf);
void cs_addMessageList(void);

CommandHash *ConnectServ_cmdTable[MAX_CMD_HASH];

int cs_about(User * u, int ac, char **av);
int cs_version(User * u, int ac, char **av);

char *s_ConnectServ;
char *ConnectServ_ident;
char *ConnectServ_host;
char *ConnectServ_realname;
char *ConnectServ_modes;
int Log_ConnectServ;
int CsJoinLogChan;
int SignOnMessage;
int UseColorfulMessages;
int SignOffMessage;
int NickChangeMessage;

#define LNG_NUM_STRINGS 4

#define CS_DEF_MSG          0
#define CS_BLANK_LINE	    1
#define CS_HELP_ABOUT	    2
#define CS_DISPLAY_VERSION	3

int do_newnick(int ac, char **av);
int cs_user_nick(int ac, char **av);
int cs_server_join(int ac, char **av);
int cs_server_quit(int ac, char **av);
int cs_server_kill(int ac, char **av);
int cs_global_kill(int ac, char **av);
int cs_report_mode(User* u, int add, const char *mode, const char* mode_desc);
int cs_user_modes(int ac, char **av);
int cs_user_logoff(int ac, char **av);


