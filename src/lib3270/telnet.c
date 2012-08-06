/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como telnet.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */


/*
 *	telnet.c
 *		This module initializes and manages a telnet socket to
 *		the given IBM host.
 */

#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
#endif

#ifndef ANDROID
	#include <stdlib.h>
#endif // !ANDROID

#include <lib3270/config.h>
#if defined(HAVE_LIBSSL)
	#include <openssl/ssl.h>
	#include <openssl/err.h>
#endif

#include "globals.h"
#include <errno.h>

#if defined(_WIN32)
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
#endif

#define TELCMDS 1
#define TELOPTS 1
#include "arpa_telnet.h"

#if !defined(_WIN32)
	#include <arpa/inet.h>
#endif

#include <errno.h>
#include <fcntl.h>

#if !defined(_WIN32)
	#include <netdb.h>
#endif

// #include <stdarg.h>

#include "tn3270e.h"
#include "3270ds.h"

// #include "appres.h"

#include "ansic.h"
#include "ctlrc.h"
#include "hostc.h"
#include "kybdc.h"
// #include "macrosc.h"
#include "popupsc.h"
#include "proxyc.h"
#include "resolverc.h"
#include "statusc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "w3miscc.h"
#include "xioc.h"
#include "screen.h"

#include <lib3270/internals.h>

#if !defined(TELOPT_NAWS) /*[*/
#define TELOPT_NAWS	31
#endif /*]*/

#if !defined(TELOPT_STARTTLS) /*[*/
#define TELOPT_STARTTLS	46
#endif /*]*/
#define TLS_FOLLOWS	1

#define BUFSZ		16384
#define TRACELINE	72

/* Globals */
// char    	*hostname = CN;
// time_t          ns_time;
//int             ns_brcvd;
//int             ns_rrcvd;
//int             ns_bsent;
//int             ns_rsent;
// unsigned char  *obuf;		/* 3270 output buffer */
// unsigned char  *obptr = (unsigned char *) NULL;
//int             linemode = 1;

/*
#if defined(LOCAL_PROCESS)
Boolean		local_process = False;
#endif
// char           *termtype;
*/

/* Externals */
// extern struct timeval ds_ts;

/* Statics */
// static int      		sock 			= -1;	/* active socket */

//#if defined(HAVE_LIBSSL) /*[*/
//static unsigned long last_ssl_error	= 0;
//#endif

//#if defined(_WIN32) /*[*/
//static HANDLE	sock_handle = NULL;
//#endif /*]*/

// static unsigned char myopts[LIB3270_TELNET_N_OPTS], hisopts[LIB3270_TELNET_N_OPTS];

/* telnet option flags */
// static unsigned char *ibuf = (unsigned char *) NULL;
// static int      ibuf_size = 0;	/* size of ibuf */

/* 3270 input buffer */
// static unsigned char *ibptr;
// static unsigned char *obuf_base = (unsigned char *)NULL;
// static int	obuf_size = 0;
// static unsigned char *netrbuf = (unsigned char *)NULL;

/* network input buffer */
// static unsigned char *sbbuf = (unsigned char *)NULL;

/* telnet sub-option buffer */
// static unsigned char *sbptr;
// static unsigned char telnet_state;
// static char     ttype_tmpval[13];

#if defined(X3270_TN3270E)
	#define E_OPT(n)	(1 << (n))
#endif // X3270_TN3270E

//#if defined(X3270_TN3270E)
//static unsigned long e_funcs;	/* negotiated TN3270E functions */
//static unsigned short e_xmit_seq; /* transmit sequence number */
//static int response_required;
//#endif

#if defined(X3270_ANSI) /*[*/
//static int      ansi_data = 0;
// static unsigned char *lbuf = (unsigned char *)NULL;
/* line-mode input buffer */
// static unsigned char *lbptr;
// static int      lnext = 0;
// static int      backslashed = 0;
//static int      t_valid = 0;
static char     vintr;
static char     vquit;
static char     verase;
static char     vkill;
static char     veof;
static char     vwerase;
static char     vrprnt;
static char     vlnext;
#endif /*]*/

// static int	tn3270e_negotiated = 0;
//static enum { E_NONE, E_3270, E_NVT, E_SSCP } tn3270e_submode = E_NONE;
// static int	tn3270e_bound = 0;
// static char	plu_name[BIND_PLU_NAME_MAX+1];
// static char	**lus = (char **)NULL;
// static char	**curr_lu = (char **)NULL;
//static char	*try_lu = CN;

// static int	proxy_type = 0;
// static char	*proxy_host = CN;
//static char	*proxy_portname = CN;
// static unsigned short proxy_port = 0;

static int telnet_fsm(H3270 *session, unsigned char c);
static void net_rawout(H3270 *session, unsigned const char *buf, size_t len);
static void check_in3270(H3270 *session);
static void store3270in(H3270 *hSession, unsigned char c);
static void check_linemode(H3270 *hSession, Boolean init);
static int non_blocking(H3270 *session, Boolean on);
static void net_connected(H3270 *session);
#if defined(X3270_TN3270E) /*[*/
static int tn3270e_negotiate(H3270 *hSession);
#endif /*]*/
static int process_eor(H3270 *hSession);
#if defined(X3270_TN3270E) /*[*/
#if defined(X3270_TRACE) /*[*/
static const char *tn3270e_function_names(const unsigned char *, int);
#endif /*]*/
static void tn3270e_subneg_send(H3270 *hSession, unsigned char, unsigned long);
static unsigned long tn3270e_fdecode(const unsigned char *, int);
static void tn3270e_ack(H3270 *hSession);
static void tn3270e_nak(H3270 *hSession, enum pds);
#endif /*]*/

#if defined(X3270_ANSI) /*[*/
static void do_data(H3270 *hSession, char c);
static void do_intr(H3270 *hSession, char c);
static void do_quit(H3270 *hSession, char c);
static void do_cerase(H3270 *hSession, char c);
static void do_werase(H3270 *hSession, char c);
static void do_kill(H3270 *hSession, char c);
static void do_rprnt(H3270 *hSession, char c);
static void do_eof(H3270 *hSession, char c);
static void do_eol(H3270 *hSession, char c);
static void do_lnext(H3270 *hSession, char c);
static char parse_ctlchar(char *s);
static void cooked_init(H3270 *hSession);
#endif /*]*/

#if defined(X3270_TRACE) /*[*/
static const char *cmd(int c);
static const char *opt(unsigned char c);
static const char *nnn(int c);
#else /*][*/
#if defined(__GNUC__) /*[*/
#else /*][*/
#endif /*]*/
#define cmd(x) 0
#define opt(x) 0
#define nnn(x) 0
#endif /*]*/

/* telnet states */
#define TNS_DATA	0	/* receiving data */
#define TNS_IAC		1	/* got an IAC */
#define TNS_WILL	2	/* got an IAC WILL */
#define TNS_WONT	3	/* got an IAC WONT */
#define TNS_DO		4	/* got an IAC DO */
#define TNS_DONT	5	/* got an IAC DONT */
#define TNS_SB		6	/* got an IAC SB */
#define TNS_SB_IAC	7	/* got an IAC after an IAC SB */

/* telnet predefined messages */
static unsigned char	do_opt[]	= 	{ 	IAC, DO, '_' };
static unsigned char	dont_opt[]	= 	{	IAC, DONT, '_' };
static unsigned char	will_opt[]	= 	{	IAC, WILL, '_' };
static unsigned char	wont_opt[]	= 	{	IAC, WONT, '_' };

#if defined(X3270_TN3270E) /*[*/
static const unsigned char	functions_req[] = {	IAC, SB, TELOPT_TN3270E, TN3270E_OP_FUNCTIONS };
#endif /*]*/

#if defined(X3270_TRACE) /*[*/
static const char *telquals[2] = { "IS", "SEND" };
#endif /*]*/
#if defined(X3270_TN3270E) /*[*/
#if defined(X3270_TRACE) /*[*/
static const char *reason_code[8] = { "CONN-PARTNER", "DEVICE-IN-USE",
	"INV-ASSOCIATE", "INV-NAME", "INV-DEVICE-TYPE", "TYPE-NAME-ERROR",
	"UNKNOWN-ERROR", "UNSUPPORTED-REQ" };
#define rsn(n)	(((n) <= TN3270E_REASON_UNSUPPORTED_REQ) ? \
			reason_code[(n)] : "??")
#endif /*]*/
static const char *function_name[5] = { "BIND-IMAGE", "DATA-STREAM-CTL",
	"RESPONSES", "SCS-CTL-CODES", "SYSREQ" };
#define fnn(n)	(((n) <= TN3270E_FUNC_SYSREQ) ? \
			function_name[(n)] : "??")
#if defined(X3270_TRACE) /*[*/
static const char *data_type[9] = { "3270-DATA", "SCS-DATA", "RESPONSE",
	"BIND-IMAGE", "UNBIND", "NVT-DATA", "REQUEST", "SSCP-LU-DATA",
	"PRINT-EOJ" };
#define e_dt(n)	(((n) <= TN3270E_DT_PRINT_EOJ) ? \
			data_type[(n)] : "??")
static const char *req_flag[1] = { " ERR-COND-CLEARED" };
#define e_rq(fn, n) (((fn) == TN3270E_DT_REQUEST) ? \
			(((n) <= TN3270E_RQF_ERR_COND_CLEARED) ? \
			req_flag[(n)] : " ??") : "")
static const char *hrsp_flag[3] = { "NO-RESPONSE", "ERROR-RESPONSE",
	"ALWAYS-RESPONSE" };
#define e_hrsp(n) (((n) <= TN3270E_RSF_ALWAYS_RESPONSE) ? \
			hrsp_flag[(n)] : "??")
static const char *trsp_flag[2] = { "POSITIVE-RESPONSE", "NEGATIVE-RESPONSE" };
#define e_trsp(n) (((n) <= TN3270E_RSF_NEGATIVE_RESPONSE) ? \
			trsp_flag[(n)] : "??")
#define e_rsp(fn, n) (((fn) == TN3270E_DT_RESPONSE) ? e_trsp(n) : e_hrsp(n))
#endif /*]*/
#endif /*]*/

// #if defined(C3270) && defined(C3270_80_132) /*[*/
// #define XMIT_ROWS	((appres.altscreen != CN)? 24: maxROWS)
// #define XMIT_COLS	((appres.altscreen != CN)? 80: maxCOLS)
// #else /*][*/
#define XMIT_ROWS	h3270.maxROWS
#define XMIT_COLS	h3270.maxCOLS
// #endif /*]*/

// #if defined(HAVE_LIBSSL)
// static SSL *ssl_con;
// #endif

#if defined(HAVE_LIBSSL) /*[*/
// static Boolean need_tls_follows = False;
static void ssl_init(H3270 *session);
#if OPENSSL_VERSION_NUMBER >= 0x00907000L /*[*/
#define INFO_CONST const
#else /*][*/
#define INFO_CONST
#endif /*]*/
static void ssl_info_callback(INFO_CONST SSL *s, int where, int ret);
static void continue_tls(H3270 *hSession, unsigned char *sbbuf, int len);
#endif /*]*/

// #if !defined(_WIN32) /*[*/
// static void output_possible(H3270 *session);
// #endif /*]*/

#if defined(_WIN32) /*[*/
	#define socket_errno()	WSAGetLastError()
	#define SE_EWOULDBLOCK	WSAEWOULDBLOCK
	#define SE_ECONNRESET	WSAECONNRESET
	#define SE_EINTR	WSAEINTR
	#define SE_EAGAIN	WSAEINPROGRESS
	#define SE_EPIPE	WSAECONNABORTED
	#define SE_EINPROGRESS	WSAEINPROGRESS
	#define SOCK_CLOSE(s)	closesocket(s)
	#define SOCK_IOCTL(s, f, v)	ioctlsocket(s, f, (void *)v)
#else /*][*/
	#define socket_errno()	errno
	#define SE_EWOULDBLOCK	EWOULDBLOCK
	#define SE_ECONNRESET	ECONNRESET
	#define SE_EINTR	EINTR
	#define SE_EAGAIN	EAGAIN
	#define SE_EPIPE	EPIPE

	#if defined(EINPROGRESS) /*[*/
		#define SE_EINPROGRESS	EINPROGRESS
	#endif /*]*/

	#define SOCK_CLOSE(s)	close(s)
	#define SOCK_IOCTL	ioctl
#endif /*]*/


/*--[ Implement ]------------------------------------------------------------------------------------*/

static void set_ssl_state(H3270 *session, LIB3270_SSL_STATE state)
{
	if(state == session->secure)
		return;

	trace_dsn(session,"SSL state changes to %d\n",(int) state);

	session->update_ssl(session,session->secure = state);
}

#if defined(_WIN32) /*[*/
void sockstart(H3270 *session)
{
	static int initted = 0;
	WORD wVersionRequested;
	WSADATA wsaData;

	if (initted)
		return;

	initted = 1;

	wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_CRITICAL,
								N_( "Network startup error" ),
								N_( "WSAStartup failed" ),
								"%s", win32_strerror(GetLastError()) );

		_exit(1);
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_CRITICAL,
								N_( "Network startup error" ),
								N_( "Bad winsock version" ),
								N_( "Can't use winsock version %d.%d" ), LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		_exit(1);
	}
}
#endif /*]*/

static union {
	struct sockaddr sa;
	struct sockaddr_in sin;
#if defined(AF_INET6) /*[*/
	struct sockaddr_in6 sin6;
#endif /*]*/
} haddr;
socklen_t ha_len = sizeof(haddr);

void popup_a_sockerr(H3270 *session, char *fmt, ...)
{
#if defined(_WIN32)
	const char *msg = win32_strerror(socket_errno());
#else
	const char *msg = strerror(errno);
#endif // WIN32
	va_list args;
	char buffer[4096];

	va_start(args, fmt);
	vsnprintf(buffer, 4095, fmt, args);
	va_end(args);

	popup_system_error(session, N_( "Network error" ), buffer, "%s", msg);

}

#pragma pack(1)
struct connect_parm
{
	unsigned short			  sz;
	int 					  sockfd;
	const struct sockaddr	* addr;
	socklen_t 				  addrlen;
	int						  err;
};
#pragma pack()

static int do_connect_sock(H3270 *h, struct connect_parm *p)
{

	if(connect(p->sockfd, p->addr, p->addrlen) == -1)
		p->err = socket_errno();
	else
		p->err = 0;

	return 0;
}

static int connect_sock(H3270 *hSession, int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	struct connect_parm p = { sizeof(struct connect_parm), sockfd, addr, addrlen, -1 };

	trace("%s: Connect begin sock=%d",__FUNCTION__,p.sockfd);
	lib3270_call_thread((int (*)(H3270 *, void *)) do_connect_sock,hSession,&p);
	trace("%s: Connect ends, rc=%d",__FUNCTION__,p.err);

	return p.err;
}


/**
 *  Establish a telnet socket to the given host passed as an argument.
 *
 *	Called only once and is responsible for setting up the telnet
 *	variables.
 *
 * @param session	Handle to the session descriptor.
 *
 * @return 0 if ok, non zero if failed
 */
int net_connect(H3270 *session, const char *host, char *portname, Boolean ls, Boolean *resolving, Boolean *pending)
{
//	struct servent	* sp;
//	struct hostent	* hp;

#if defined(X3270_ANSI)
	static int		  t_valid = 0;
#endif // X3270_ANSI

	char	          passthru_haddr[8];
	int				  passthru_len = 0;
	unsigned short	  passthru_port = 0;
	int				  on = 1;
	char			  errmsg[1024];
	int				  rc;
#if defined(OMTU) /*[*/
	int			mtu = OMTU;
#endif /*]*/

#define close_fail { (void) SOCK_CLOSE(session->sock); session->sock = -1; return -1; }

	set_ssl_state(session,LIB3270_SSL_UNSECURE);

#if defined(_WIN32)
	sockstart(session);
#endif

//	if (session->netrbuf == (unsigned char *)NULL)
//		session->netrbuf = (unsigned char *)lib3270_malloc(BUFSZ);

#if defined(X3270_ANSI) /*[*/
	if (!t_valid)
	{
		vintr   = parse_ctlchar("^C");
		vquit   = parse_ctlchar("^\\");
		verase  = parse_ctlchar("^H");
		vkill   = parse_ctlchar("^U");
		veof    = parse_ctlchar("^D");
		vwerase = parse_ctlchar("^W");
		vrprnt  = parse_ctlchar("^R");
		vlnext  = parse_ctlchar("^V");

/*
		vintr   = parse_ctlchar(appres.intr);
		vquit   = parse_ctlchar(appres.quit);
		verase  = parse_ctlchar(appres.erase);
		vkill   = parse_ctlchar(appres.kill);
		veof    = parse_ctlchar(appres.eof);
		vwerase = parse_ctlchar(appres.werase);
		vrprnt  = parse_ctlchar(appres.rprnt);
		vlnext  = parse_ctlchar(appres.lnext);
*/
		t_valid = 1;
	}
#endif /*]*/

	*resolving = False;
	*pending = False;

	Replace(session->hostname, NewString(host));

	/* get the passthru host and port number */
	if (session->passthru_host)
	{
#if defined(HAVE_GETADDRINFO)

		popup_an_error(session,"%s",_( "Unsupported passthru host session" ) );

#else
		struct hostent	* hp = NULL;
		struct servent	* sp = NULL;
		const char 		* hn = CN;

		hn = getenv("INTERNET_HOST");

		if (hn == CN)
			hn = "internet-gateway";

		hp = gethostbyname(hn);
		if (hp == (struct hostent *) 0)
		{
			popup_an_error(session,_( "Unknown passthru host: %s" ), hn);
			return -1;
		}
		memmove(passthru_haddr, hp->h_addr, hp->h_length);
		passthru_len = hp->h_length;

		sp = getservbyname("telnet-passthru","tcp");
		if (sp != (struct servent *)NULL)
			passthru_port = sp->s_port;
		else
			passthru_port = htons(3514);

#endif // HAVE_GETADDRINFO
	}
	else if(session->proxy != CN && !session->proxy_type)
	{
	   	session->proxy_type = proxy_setup(session, &session->proxy_host, &session->proxy_portname);

		if (session->proxy_type > 0)
		{
		   	unsigned long lport;
			char *ptr;
			struct servent *sp;

			lport = strtoul(portname, &ptr, 0);
			if (ptr == portname || *ptr != '\0' || lport == 0L || lport & ~0xffff)
			{
				if (!(sp = getservbyname(portname, "tcp")))
				{
					popup_an_error(session, _( "Unknown port number or service: %s" ), portname);
					return -1;
				}
				session->current_port = ntohs(sp->s_port);
			}
			else
			{
				session->current_port = (unsigned short)lport;
			}
		}

		if (session->proxy_type < 0)
			return -1;
	}

	/* fill in the socket address of the given host */
	(void) memset((char *) &haddr, 0, sizeof(haddr));
	if (session->passthru_host)
	{
		haddr.sin.sin_family = AF_INET;
		(void) memmove(&haddr.sin.sin_addr, passthru_haddr,passthru_len);
		haddr.sin.sin_port = passthru_port;
		ha_len = sizeof(struct sockaddr_in);
	}
	else if (session->proxy_type > 0)
	{
		if (resolve_host_and_port(session,session->proxy_host, session->proxy_portname,&session->proxy_port, &haddr.sa, &ha_len, errmsg,sizeof(errmsg)) < 0)
		{
			popup_an_error(session,errmsg);
			return -1;
		}
	}
	else
	{
		if (resolve_host_and_port(session,host, portname,&session->current_port, &haddr.sa, &ha_len,errmsg, sizeof(errmsg)) < 0)
		{
			popup_an_error(session,errmsg);
			return -1;
		}
	}

	/* create the socket */
	if((session->sock = socket(haddr.sa.sa_family, SOCK_STREAM, 0)) == -1)
	{
		popup_a_sockerr(session, N_( "socket" ) );
		return -1;
	}

	/* set options for inline out-of-band data and keepalives */
	if (setsockopt(session->sock, SOL_SOCKET, SO_OOBINLINE, (char *)&on,sizeof(on)) < 0)
	{
		popup_a_sockerr(session, N_( "setsockopt(%s)" ), "SO_OOBINLINE");
		close_fail;
	}

	if (setsockopt(session->sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on)) < 0)
	{
		popup_a_sockerr(session, N_( "setsockopt(%s)" ), "SO_KEEPALIVE");
		close_fail;
	}

#if defined(OMTU)
	if (setsockopt(session->sock, SOL_SOCKET, SO_SNDBUF, (char *)&mtu,sizeof(mtu)) < 0)
	{
		popup_a_sockerr(session, N_( "setsockopt(%s)" ), "SO_SNDBUF");
		close_fail;
	}
#endif

	/* set the socket to be non-delaying during connect */
	if(non_blocking(session,False) < 0)
		close_fail;

#if !defined(_WIN32)
	/* don't share the socket with our children */
	(void) fcntl(session->sock, F_SETFD, 1);
#endif

	/* init ssl */
#if defined(HAVE_LIBSSL)
	session->last_ssl_error = !0;
	if (session->ssl_host)
		ssl_init(session);
#endif

	/* connect */
	status_connecting(session,1);
	rc = connect_sock(session, session->sock, &haddr.sa,ha_len);

	if(!rc)
	{
		trace_dsn(session,"Connected.\n");
		net_connected(session);
	}
	else
	{
		char *msg = xs_buffer( _( "Can't connect to %s:%d" ), session->hostname, session->current_port);

		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_ERROR,
								_( "Network error" ),
								msg,
								"%s",strerror(rc) );

		lib3270_free(msg);
		close_fail;

	}

	/* set up temporary termtype */
	if (session->termname == CN && session->std_ds_host)
	{
		sprintf(session->ttype_tmpval, "IBM-327%c-%d",session->m3279 ? '9' : '8', session->model_num);
		session->termtype = session->ttype_tmpval;
	}

	/* all done */
#if defined(_WIN32)
	if(session->sockEvent == NULL)
	{
		char ename[256];

		snprintf(ename, 255, "%s-%d", PACKAGE_NAME, getpid());

		session->sockEvent = CreateEvent(NULL, TRUE, FALSE, ename);
		if(session->sockEvent == NULL)
		{
			lib3270_popup_dialog(	session,
									LIB3270_NOTIFY_CRITICAL,
									N_( "Network startup error" ),
									N_( "Cannot create socket handle" ),
									"%s", win32_strerror(GetLastError()) );
			_exit(1);
		}
	}

	if (WSAEventSelect(session->sock, session->sockEvent, FD_READ | FD_CONNECT | FD_CLOSE) != 0)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_CRITICAL,
								N_( "Network startup error" ),
								N_( "WSAEventSelect failed" ),
								"%s", win32_strerror(GetLastError()) );
		_exit(1);
	}

//	trace("Socket: %d Event: %ld",session->sock,(unsigned long) session->sockEvent);

#endif // WIN32

	non_blocking(session,1);

	return 0;
}
#undef close_fail

/* Set up the LU list. */
static void setup_lus(H3270 *hSession)
{
	char *lu;
	char *comma;
	int n_lus = 1;
	int i;

	hSession->connected_lu = CN;
	hSession->connected_type = CN;

	if (!hSession->luname[0])
	{
		Replace(hSession->lus, NULL);
		hSession->curr_lu = (char **)NULL;
		hSession->try_lu = CN;
		return;
	}

	/*
	 * Count the commas in the LU name.  That plus one is the
	 * number of LUs to try.
	 */
	lu = hSession->luname;
	while ((comma = strchr(lu, ',')) != CN)
	{
		n_lus++;
		lu++;
	}

	/*
	 * Allocate enough memory to construct an argv[] array for
	 * the LUs.
	 */
	Replace(hSession->lus,(char **)lib3270_malloc((n_lus+1) * sizeof(char *) + strlen(hSession->luname) + 1));

	/* Copy each LU into the array. */
	lu = (char *)(hSession->lus + n_lus + 1);
	(void) strcpy(lu, hSession->luname);

	i = 0;
	do
	{
		hSession->lus[i++] = lu;
		comma = strchr(lu, ',');
		if (comma != CN)
		{
			*comma = '\0';
			lu = comma + 1;
		}
	} while (comma != CN);

	hSession->lus[i]	= CN;
	hSession->curr_lu	= hSession->lus;
	hSession->try_lu	= *hSession->curr_lu;
}

static void net_connected(H3270 *hSession)
{
	if(hSession->proxy_type > 0)
	{
		/* Negotiate with the proxy. */
		trace_dsn(hSession,"Connected to proxy server %s, port %u.\n",hSession->proxy_host, hSession->proxy_port);

		if (proxy_negotiate(hSession, hSession->proxy_type, hSession->sock, hSession->hostname,hSession->current_port) < 0)
		{
			host_disconnect(hSession,True);
			return;
		}
	}

	trace_dsn(hSession,"Connected to %s, port %u%s.\n", hSession->hostname, hSession->current_port,hSession->ssl_host? " via SSL": "");

#if defined(HAVE_LIBSSL) /*[*/
	/* Set up SSL. */
	if(hSession->ssl_con && hSession->secure == LIB3270_SSL_UNDEFINED)
	{
		int rc;

		set_ssl_state(hSession,LIB3270_SSL_NEGOTIATING);

		if (SSL_set_fd(hSession->ssl_con, hSession->sock) != 1)
		{
			trace_dsn(hSession,"Can't set fd!\n");
			popup_system_error(hSession,_( "Connection failed" ), _( "Can't set SSL socket file descriptor" ), "%s", SSL_state_string_long(hSession->ssl_con));
			set_ssl_state(hSession,LIB3270_SSL_UNSECURE);
		}
		else
		{
			rc = SSL_connect(hSession->ssl_con);

			if(rc != 1)
			{
				unsigned long 	  e		= ERR_get_error();
				const char  	* state	= SSL_state_string_long(hSession->ssl_con);

				trace_dsn(hSession,"TLS/SSL tunneled connection failed with error %ld, rc=%d and state=%s",e,rc,state);

				host_disconnect(hSession,True);

				if(e != hSession->last_ssl_error)
				{
					hSession->message(hSession,LIB3270_NOTIFY_ERROR,_( "Connection failed" ),_( "SSL negotiation failed" ),state);
					hSession->last_ssl_error = e;
				}
				return;

			}
		}

//		hSession->secure_connection = True;
		trace_dsn(hSession,"TLS/SSL tunneled connection complete. Connection is now secure.\n");

		/* Tell everyone else again. */
		lib3270_set_connected(hSession);
	}
#endif /*]*/

	lib3270_setup_session(hSession);

}

/**
 * Set up telnet options.
 *
 * Called just after a sucessfull connect to setup tn3270 state.
 *
 * @param hSession	3270 session to setup.
 *
 */
LIB3270_EXPORT void lib3270_setup_session(H3270 *hSession)
{
	(void) memset((char *) hSession->myopts, 0, sizeof(hSession->myopts));
	(void) memset((char *) hSession->hisopts, 0, sizeof(hSession->hisopts));

#if defined(X3270_TN3270E) /*[*/
	hSession->e_funcs = E_OPT(TN3270E_FUNC_BIND_IMAGE) | E_OPT(TN3270E_FUNC_RESPONSES) | E_OPT(TN3270E_FUNC_SYSREQ);
	hSession->e_xmit_seq = 0;
	hSession->response_required = TN3270E_RSF_NO_RESPONSE;
#endif /*]*/

#if defined(HAVE_LIBSSL) /*[*/
	hSession->need_tls_follows = 0;
#endif /*]*/
	hSession->telnet_state = TNS_DATA;
	hSession->ibptr = hSession->ibuf;

	/* clear statistics and flags */
	time(&hSession->ns_time);
	hSession->ns_brcvd  = 0;
	hSession->ns_rrcvd  = 0;
	hSession->ns_bsent  = 0;
	hSession->ns_rsent  = 0;
	hSession->syncing   = 0;
	hSession->tn3270e_negotiated = 0;
	hSession->tn3270e_submode = E_NONE;
	hSession->tn3270e_bound = 0;

	setup_lus(hSession);

	check_linemode(hSession,True);

	/* write out the passthru hostname and port nubmer */
	if (hSession->passthru_host)
	{
		unsigned char *buffer = (unsigned char *) xs_buffer("%s %d\r\n", hSession->hostname, hSession->current_port);
		hSession->write(hSession, buffer, strlen((char *) buffer));
		lib3270_free(buffer);
		trace_ds(hSession,"SENT HOSTNAME %s:%d\n", hSession->hostname, hSession->current_port);
	}
}

/**
 * Connection_complete.
 *
 * The connection appears to be complete (output is possible or input
 * appeared ready but recv() returned EWOULDBLOCK).  Complete the
 * connection-completion processing.
 *
 */
static void connection_complete(H3270 *session)
{
	if (non_blocking(session,False) < 0)
	{
		host_disconnect(session,True);
		return;
	}
	lib3270_set_connected(session);
	net_connected(session);
}


LIB3270_INTERNAL void lib3270_sock_disconnect(H3270 *session)
{
	trace("%s",__FUNCTION__);

#if defined(HAVE_LIBSSL)
	if(session->ssl_con != NULL)
	{
		SSL_shutdown(session->ssl_con);
		SSL_free(session->ssl_con);
		session->ssl_con = NULL;
	}
#endif

	if(session->sock >= 0)
	{
		shutdown(session->sock, 2);
		SOCK_CLOSE(session->sock);
		session->sock = -1;
	}
}

/*
 * net_disconnect
 *	Shut down the socket.
 */
void net_disconnect(H3270 *session)
{
	set_ssl_state(session,LIB3270_SSL_UNSECURE);

	session->disconnect(session);

	trace_dsn(session,"SENT disconnect\n");

	/* Restore terminal type to its default. */
	if (session->termname == CN)
		session->termtype = session->full_model_name;

	/* We're not connected to an LU any more. */
	session->connected_lu = CN;
	status_lu(session,CN);

}

LIB3270_EXPORT void lib3270_data_recv(H3270 *hSession, size_t nr, const unsigned char *netrbuf)
{
	register const unsigned char * cp;

	trace("%s: nr=%d",__FUNCTION__,nr);

	trace_netdata(hSession, '<', netrbuf, nr);

	hSession->ns_brcvd += nr;
	for (cp = netrbuf; cp < (netrbuf + nr); cp++)
	{
		if(telnet_fsm(hSession,*cp))
		{
			(void) ctlr_dbcs_postprocess(hSession);
			host_disconnect(hSession,True);
			return;
		}
	}

#if defined(X3270_ANSI)
	if (IN_ANSI)
	{
		(void) ctlr_dbcs_postprocess(hSession);
	}

	if (hSession->ansi_data)
	{
		trace_dsn(hSession,"\n");
		hSession->ansi_data = 0;
	}
#endif // X3270_ANSI
}


/*
 * net_input
 *	Called by the toolkit whenever there is input available on the
 *	socket.  Reads the data, processes the special telnet commands
 *	and calls process_ds to process the 3270 data stream.
 */
void net_input(H3270 *session)
{
//	register unsigned char	* cp;
	int						  nr;
	unsigned char			  buffer[BUFSZ];

	CHECK_SESSION_HANDLE(session);

#if defined(_WIN32)
 	for (;;)
#endif
	{
		if (session->sock < 0)
			return;

#if defined(X3270_ANSI)
		session->ansi_data = 0;
#endif

#if defined(_WIN32)
		ResetEvent(session->sockEvent);
#endif

#if defined(HAVE_LIBSSL)
		if (session->ssl_con != NULL)
			nr = SSL_read(session->ssl_con, (char *) buffer, BUFSZ);
		else
			nr = recv(session->sock, (char *) buffer, BUFSZ, 0);
#else
			nr = recv(session->sock, (char *) buffer, BUFSZ, 0);
#endif // HAVE_LIBSSL

		if (nr < 0)
		{
			if (socket_errno() == SE_EWOULDBLOCK)
				return;

#if defined(HAVE_LIBSSL) /*[*/
			if(session->ssl_con != NULL)
			{
				unsigned long e;
				char err_buf[120];

				e = ERR_get_error();
				if (e != 0)
					(void) ERR_error_string(e, err_buf);
				else
					strcpy(err_buf, _( "unknown error" ) );

				trace_dsn(session,"RCVD SSL_read error %ld (%s)\n", e,err_buf);

				session->message( session,LIB3270_NOTIFY_ERROR,_( "SSL Error" ),_( "SSL Read error" ),err_buf );

				host_disconnect(session,True);
				return;
			}
#endif /*]*/

			if (HALF_CONNECTED && socket_errno() == SE_EAGAIN)
			{
				connection_complete(session);
				return;
			}

			trace_dsn(session,"RCVD socket error %d\n", errno);

			if (HALF_CONNECTED)
			{
				popup_a_sockerr(session, N_( "%s:%d" ),session->hostname, session->current_port);
			}
			else if (socket_errno() != SE_ECONNRESET)
			{
				popup_a_sockerr(session, N_( "Socket read error" ) );
			}

			host_disconnect(session,True);
			return;
		} else if (nr == 0)
		{
			/* Host disconnected. */
			trace_dsn(session,"RCVD disconnect\n");
			host_disconnect(session,False);
			return;
		}

		/* Process the data. */
		if (HALF_CONNECTED)
		{
			if (non_blocking(session,False) < 0)
			{
				host_disconnect(session,True);
				return;
			}
			lib3270_set_connected(session);
			net_connected(session);
		}

		lib3270_data_recv(session, nr, buffer);

/*
		trace_netdata('<', session->netrbuf, nr);

		session->ns_brcvd += nr;
		for (cp = session->netrbuf; cp < (session->netrbuf + nr); cp++)
		{
			if (telnet_fsm(session,*cp))
			{
				(void) ctlr_dbcs_postprocess(hSession);
				host_disconnect(session,True);
				return;
			}
		}

#if defined(X3270_ANSI)
		if (IN_ANSI)
		{
			(void) ctlr_dbcs_postprocess(hSession);
		}

		if (session->ansi_data)
		{
			trace_dsn(session,"\n");
			session->ansi_data = 0;
		}
#endif // X3270_ANSI
*/
	}

}


/*
 * set16
 *	Put a 16-bit value in a buffer.
 *	Returns the number of bytes required.
 */
static int
set16(char *buf, int n)
{
	char *b0 = buf;

	n %= 256 * 256;
	if ((n / 256) == IAC)
		*(unsigned char *)buf++ = IAC;
	*buf++ = (n / 256);
	n %= 256;
	if (n == IAC)
		*(unsigned char *)buf++ = IAC;
	*buf++ = n;
	return buf - b0;
}

/*
 * send_naws
 *	Send a Telnet window size sub-option negotation.
 */
static void send_naws(H3270 *hSession)
{
	char naws_msg[14];
	int naws_len = 0;

	(void) sprintf(naws_msg, "%c%c%c", IAC, SB, TELOPT_NAWS);
	naws_len += 3;
	naws_len += set16(naws_msg + naws_len, XMIT_COLS);
	naws_len += set16(naws_msg + naws_len, XMIT_ROWS);
	(void) sprintf(naws_msg + naws_len, "%c%c", IAC, SE);
	naws_len += 2;
	net_rawout(hSession,(unsigned char *)naws_msg, naws_len);
	trace_dsn(hSession,"SENT %s NAWS %d %d %s\n", cmd(SB), XMIT_COLS, XMIT_ROWS, cmd(SE));
}



/* Advance 'try_lu' to the next desired LU name. */
static void next_lu(H3270 *hSession)
{
	if (hSession->curr_lu != (char **)NULL && (hSession->try_lu = *++hSession->curr_lu) == CN)
		hSession->curr_lu = (char **)NULL;
}

/*
 * telnet_fsm
 *	Telnet finite-state machine.
 *	Returns 0 for okay, -1 for errors.
 */
static int telnet_fsm(H3270 *session, unsigned char c)
{
#if defined(X3270_ANSI) /*[*/
	char	*see_chr;
	int	sl;
#endif /*]*/

	switch (session->telnet_state)
	{
	    case TNS_DATA:	/* normal data processing */
		if (c == IAC) {	/* got a telnet command */
			session->telnet_state = TNS_IAC;
#if defined(X3270_ANSI) /*[*/
			if (session->ansi_data) {
				trace_dsn(session,"\n");
				session->ansi_data = 0;
			}
#endif /*]*/
			break;
		}
		if (IN_NEITHER) {	/* now can assume ANSI mode */
#if defined(X3270_ANSI)/*[*/
			if (session->linemode)
				cooked_init(session);
#endif /*]*/
			host_in3270(session,CONNECTED_ANSI);
			lib3270_kybdlock_clear(session,KL_AWAITING_FIRST);
			status_reset(session);
			ps_process(session);
		}
		if (IN_ANSI && !IN_E) {
#if defined(X3270_ANSI) /*[*/
			if (!session->ansi_data) {
				trace_dsn(session,"<.. ");
				session->ansi_data = 4;
			}
			see_chr = ctl_see((int) c);
			session->ansi_data += (sl = strlen(see_chr));
			if (session->ansi_data >= TRACELINE) {
				trace_dsn(session," ...\n... ");
				session->ansi_data = 4 + sl;
			}
			trace_dsn(session,"%s",see_chr);
			if (!session->syncing)
			{
				if (session->linemode && session->onlcr && c == '\n')
					ansi_process(session,(unsigned int) '\r');
				ansi_process(session,(unsigned int) c);
//				sms_store(c);
			}
#endif /*]*/
		} else {
			store3270in(session,c);
		}
		break;
	    case TNS_IAC:	/* process a telnet command */
		if (c != EOR && c != IAC)
		{
			trace_dsn(session,"RCVD %s ", cmd(c));
		}

		switch (c)
		{
	    case IAC:	/* escaped IAC, insert it */
			if (IN_ANSI && !IN_E)
			{
#if defined(X3270_ANSI) /*[*/
				if (!session->ansi_data)
				{
					trace_dsn(session,"<.. ");
					session->ansi_data = 4;
				}
				see_chr = ctl_see((int) c);
				session->ansi_data += (sl = strlen(see_chr));
				if (session->ansi_data >= TRACELINE)
				{
					trace_dsn(session," ...\n ...");
					session->ansi_data = 4 + sl;
				}
				trace_dsn(session,"%s",see_chr);
				ansi_process(session,(unsigned int) c);
#endif /*]*/
			}
			else
			{
				store3270in(session,c);
			}
			session->telnet_state = TNS_DATA;
			break;

	    case EOR:	/* eor, process accumulated input */

			if (IN_3270 || (IN_E && session->tn3270e_negotiated))
			{
				session->ns_rrcvd++;
				if (process_eor(session))
					return -1;
			} else
				Warning(session, _( "EOR received when not in 3270 mode, ignored." ));

			trace_dsn(session,"RCVD EOR\n");
			session->ibptr = session->ibuf;
			session->telnet_state = TNS_DATA;
			break;

	    case WILL:
			session->telnet_state = TNS_WILL;
			break;

	    case WONT:
			session->telnet_state = TNS_WONT;
			break;

	    case DO:
			session->telnet_state = TNS_DO;
			break;

	    case DONT:
			session->telnet_state = TNS_DONT;
			break;

	    case SB:
			session->telnet_state = TNS_SB;
			if (session->sbbuf == (unsigned char *)NULL)
				session->sbbuf = (unsigned char *)lib3270_malloc(1024);
			session->sbptr = session->sbbuf;
			break;

	    case DM:
			trace_dsn(session,"\n");
			if (session->syncing)
			{
				session->syncing = 0;
				x_except_on(session);
			}
			session->telnet_state = TNS_DATA;
			break;

	    case GA:
	    case NOP:
			trace_dsn(session,"\n");
			session->telnet_state = TNS_DATA;
			break;

	    default:
			trace_dsn(session,"???\n");
			session->telnet_state = TNS_DATA;
			break;
		}
		break;
	    case TNS_WILL:	/* telnet WILL DO OPTION command */
		trace_dsn(session,"%s\n", opt(c));
		switch (c) {
		    case TELOPT_SGA:
		    case TELOPT_BINARY:
		    case TELOPT_EOR:
		    case TELOPT_TTYPE:
		    case TELOPT_ECHO:
#if defined(X3270_TN3270E) /*[*/
		    case TELOPT_TN3270E:
#endif /*]*/
			if (c != TELOPT_TN3270E || !session->non_tn3270e_host) {
				if (!session->hisopts[c]) {
					session->hisopts[c] = 1;
					do_opt[2] = c;
					net_rawout(session,do_opt, sizeof(do_opt));
					trace_dsn(session,"SENT %s %s\n",
						cmd(DO), opt(c));

					/*
					 * For UTS, volunteer to do EOR when
					 * they do.
					 */
					if (c == TELOPT_EOR && !session->myopts[c]) {
						session->myopts[c] = 1;
						will_opt[2] = c;
						net_rawout(session,will_opt,sizeof(will_opt));
						trace_dsn(session,"SENT %s %s\n",cmd(WILL), opt(c));
					}

					check_in3270(session);
					check_linemode(session,False);
				}
				break;
			}
		    default:
			dont_opt[2] = c;
			net_rawout(session,dont_opt, sizeof(dont_opt));
			trace_dsn(session,"SENT %s %s\n", cmd(DONT), opt(c));
			break;
		}
		session->telnet_state = TNS_DATA;
		break;
	    case TNS_WONT:	/* telnet WONT DO OPTION command */
		trace_dsn(session,"%s\n", opt(c));
		if (session->hisopts[c])
		{
			session->hisopts[c] = 0;
			dont_opt[2] = c;
			net_rawout(session, dont_opt, sizeof(dont_opt));
			trace_dsn(session,"SENT %s %s\n", cmd(DONT), opt(c));
			check_in3270(session);
			check_linemode(session,False);
		}

		session->telnet_state = TNS_DATA;
		break;
	    case TNS_DO:	/* telnet PLEASE DO OPTION command */
		trace_dsn(session,"%s\n", opt(c));
		switch (c) {
		    case TELOPT_BINARY:
		    case TELOPT_EOR:
		    case TELOPT_TTYPE:
		    case TELOPT_SGA:
		    case TELOPT_NAWS:
		    case TELOPT_TM:
#if defined(X3270_TN3270E) /*[*/
		    case TELOPT_TN3270E:
#endif /*]*/
#if defined(HAVE_LIBSSL) /*[*/
		    case TELOPT_STARTTLS:
#endif /*]*/
			if (c == TELOPT_TN3270E && session->non_tn3270e_host)
				goto wont;
			if (c == TELOPT_TM && !session->bsd_tm)
				goto wont;

			trace("session->myopts[c]=%d",session->myopts[c]);
			if (!session->myopts[c])
			{
				if (c != TELOPT_TM)
					session->myopts[c] = 1;
				will_opt[2] = c;
				net_rawout(session, will_opt, sizeof(will_opt));
				trace_dsn(session,"SENT %s %s\n", cmd(WILL), opt(c));
				check_in3270(session);
				check_linemode(session,False);
			}
			if (c == TELOPT_NAWS)
				send_naws(session);
#if defined(HAVE_LIBSSL) /*[*/
			if (c == TELOPT_STARTTLS) {
				static unsigned char follows_msg[] = {
					IAC, SB, TELOPT_STARTTLS,
					TLS_FOLLOWS, IAC, SE
				};

				/*
				 * Send IAC SB STARTTLS FOLLOWS IAC SE
				 * to announce that what follows is TLS.
				 */
				net_rawout(session, follows_msg, sizeof(follows_msg));
				trace_dsn(session,"SENT %s %s FOLLOWS %s\n",
						cmd(SB),
						opt(TELOPT_STARTTLS),
						cmd(SE));
				session->need_tls_follows = 1;
			}
#endif /*]*/
			break;
		    default:
		    wont:
			wont_opt[2] = c;
			net_rawout(session, wont_opt, sizeof(wont_opt));
			trace_dsn(session,"SENT %s %s\n", cmd(WONT), opt(c));
			break;
		}
		session->telnet_state = TNS_DATA;
		break;
	    case TNS_DONT:	/* telnet PLEASE DON'T DO OPTION command */
		trace_dsn(session,"%s\n", opt(c));
		if (session->myopts[c]) {
			session->myopts[c] = 0;
			wont_opt[2] = c;
			net_rawout(session, wont_opt, sizeof(wont_opt));
			trace_dsn(session,"SENT %s %s\n", cmd(WONT), opt(c));
			check_in3270(session);
			check_linemode(session,False);
		}
		session->telnet_state = TNS_DATA;
		break;
	    case TNS_SB:	/* telnet sub-option string command */
		if (c == IAC)
			session->telnet_state = TNS_SB_IAC;
		else
			*session->sbptr++ = c;
		break;
	    case TNS_SB_IAC:	/* telnet sub-option string command */
		*session->sbptr++ = c;
		if (c == SE) {
			session->telnet_state = TNS_DATA;
			if (session->sbbuf[0] == TELOPT_TTYPE && session->sbbuf[1] == TELQUAL_SEND)
			{
				int tt_len, tb_len;
				char *tt_out;

				trace_dsn(session,"%s %s\n", opt(session->sbbuf[0]),telquals[session->sbbuf[1]]);

				if (session->lus != (char **)NULL && session->try_lu == CN)
				{
					/* None of the LUs worked. */
					popup_an_error(NULL,"Cannot connect to specified LU");
					return -1;
				}

				tt_len = strlen(session->termtype);
				if (session->try_lu != CN && *session->try_lu)
				{
					tt_len += strlen(session->try_lu) + 1;
					session->connected_lu = session->try_lu;
				}
				else
					session->connected_lu = CN;

				status_lu(session,session->connected_lu);

				tb_len = 4 + tt_len + 2;
				tt_out = lib3270_malloc(tb_len + 1);
				(void) sprintf(tt_out, "%c%c%c%c%s%s%s%c%c",
				    IAC, SB, TELOPT_TTYPE, TELQUAL_IS,
				    session->termtype,
				    (session->try_lu != CN && *session->try_lu) ? "@" : "",
				    (session->try_lu != CN && *session->try_lu) ? session->try_lu : "",
				    IAC, SE);
				net_rawout(session, (unsigned char *)tt_out, tb_len);

				trace_dsn(session,"SENT %s %s %s %.*s %s\n",
				    cmd(SB), opt(TELOPT_TTYPE),
				    telquals[TELQUAL_IS],
				    tt_len, tt_out + 4,
				    cmd(SE));
				lib3270_free(tt_out);

				/* Advance to the next LU name. */
				next_lu(session);
			}
#if defined(X3270_TN3270E) /*[*/
			else if (session->myopts[TELOPT_TN3270E] && session->sbbuf[0] == TELOPT_TN3270E)
			{
				if (tn3270e_negotiate(session))
					return -1;
			}
#endif /*]*/
#if defined(HAVE_LIBSSL) /*[*/
			else if (session->need_tls_follows && session->myopts[TELOPT_STARTTLS] && session->sbbuf[0] == TELOPT_STARTTLS)
			{
				continue_tls(session,session->sbbuf, session->sbptr - session->sbbuf);
			}
#endif /*]*/

		} else {
			session->telnet_state = TNS_SB;
		}
		break;
	}
	return 0;
}

#if defined(X3270_TN3270E) /*[*/
/* Send a TN3270E terminal type request. */
static void tn3270e_request(H3270 *hSession)
{
	int tt_len, tb_len;
	char *tt_out;
	char *t;

	tt_len = strlen(hSession->termtype);
	if (hSession->try_lu != CN && *hSession->try_lu)
		tt_len += strlen(hSession->try_lu) + 1;

	tb_len = 5 + tt_len + 2;
	tt_out = lib3270_malloc(tb_len + 1);
	t = tt_out;
	t += sprintf(tt_out, "%c%c%c%c%c%s",
	    IAC, SB, TELOPT_TN3270E, TN3270E_OP_DEVICE_TYPE,
	    TN3270E_OP_REQUEST, hSession->termtype);

	/* Convert 3279 to 3278, per the RFC. */
	if (tt_out[12] == '9')
		tt_out[12] = '8';

	if (hSession->try_lu != CN && *hSession->try_lu)
		t += sprintf(t, "%c%s", TN3270E_OP_CONNECT, hSession->try_lu);

	(void) sprintf(t, "%c%c", IAC, SE);

	net_rawout(hSession, (unsigned char *)tt_out, tb_len);

	trace_dsn(hSession,"SENT %s %s DEVICE-TYPE REQUEST %.*s%s%s %s\n",
	    cmd(SB), opt(TELOPT_TN3270E), (int) strlen(hSession->termtype), tt_out + 5,
	    (hSession->try_lu != CN && *hSession->try_lu) ? " CONNECT " : "",
	    (hSession->try_lu != CN && *hSession->try_lu) ? hSession->try_lu : "",
	    cmd(SE));

	lib3270_free(tt_out);
}

/*
 * Back off of TN3270E.
 */
static void backoff_tn3270e(H3270 *hSession, const char *why)
{
	trace_dsn(hSession,"Aborting TN3270E: %s\n", why);

	/* Tell the host 'no'. */
	wont_opt[2] = TELOPT_TN3270E;
	net_rawout(hSession, wont_opt, sizeof(wont_opt));
	trace_dsn(hSession,"SENT %s %s\n", cmd(WONT), opt(TELOPT_TN3270E));

	/* Restore the LU list; we may need to run it again in TN3270 mode. */
	setup_lus(hSession);

	/* Reset our internal state. */
	hSession->myopts[TELOPT_TN3270E] = 0;
	check_in3270(hSession);
}

/*
 * Negotiation of TN3270E options.
 * Returns 0 if okay, -1 if we have to give up altogether.
 */
static int tn3270e_negotiate(H3270 *hSession)
{
//	#define LU_MAX	32
//	static char reported_lu[LU_MAX+1];
//	static char reported_type[LU_MAX+1];

	int sblen;
	unsigned long e_rcvd;

	/* Find out how long the subnegotiation buffer is. */
	for (sblen = 0; ; sblen++) {
		if (hSession->sbbuf[sblen] == SE)
			break;
	}

	trace_dsn(hSession,"TN3270E ");

	switch (hSession->sbbuf[1]) {

	case TN3270E_OP_SEND:

		if (hSession->sbbuf[2] == TN3270E_OP_DEVICE_TYPE) {

			/* Host wants us to send our device type. */
			trace_dsn(hSession,"SEND DEVICE-TYPE SE\n");

			tn3270e_request(hSession);
		} else {
			trace_dsn(hSession,"SEND ??%u SE\n", hSession->sbbuf[2]);
		}
		break;

	case TN3270E_OP_DEVICE_TYPE:

		/* Device type negotiation. */
		trace_dsn(hSession,"DEVICE-TYPE ");

		switch (hSession->sbbuf[2]) {
		case TN3270E_OP_IS: {
			int tnlen, snlen;

			/* Device type success. */

			/* Isolate the terminal type and session. */
			tnlen = 0;
			while (hSession->sbbuf[3+tnlen] != SE &&
			       hSession->sbbuf[3+tnlen] != TN3270E_OP_CONNECT)
				tnlen++;
			snlen = 0;
			if (hSession->sbbuf[3+tnlen] == TN3270E_OP_CONNECT) {
				while(hSession->sbbuf[3+tnlen+1+snlen] != SE)
					snlen++;
			}
			trace_dsn(hSession,"IS %.*s CONNECT %.*s SE\n",
				tnlen, &hSession->sbbuf[3],
				snlen, &hSession->sbbuf[3+tnlen+1]);

			/* Remember the LU. */
			if (tnlen) {
				if (tnlen > LIB3270_LU_MAX)
					tnlen = LIB3270_LU_MAX;
				(void)strncpy(hSession->reported_type,(char *)&hSession->sbbuf[3], tnlen);
				hSession->reported_type[tnlen] = '\0';
				hSession->connected_type = hSession->reported_type;
			}
			if (snlen) {
				if (snlen > LIB3270_LU_MAX)
					snlen = LIB3270_LU_MAX;
				(void)strncpy(hSession->reported_lu,(char *)&hSession->sbbuf[3+tnlen+1], snlen);
				hSession->reported_lu[snlen] = '\0';
				hSession->connected_lu = hSession->reported_lu;
				status_lu(hSession,hSession->connected_lu);
			}

			/* Tell them what we can do. */
			tn3270e_subneg_send(hSession, TN3270E_OP_REQUEST, hSession->e_funcs);
			break;
		}
		case TN3270E_OP_REJECT:

			/* Device type failure. */

			trace_dsn(hSession,"REJECT REASON %s SE\n", rsn(hSession->sbbuf[4]));
			if (hSession->sbbuf[4] == TN3270E_REASON_INV_DEVICE_TYPE ||
			    hSession->sbbuf[4] == TN3270E_REASON_UNSUPPORTED_REQ) {
				backoff_tn3270e(hSession,_( "Host rejected device type or request type" ));
				break;
			}

			next_lu(hSession);
			if (hSession->try_lu != CN)
			{
				/* Try the next LU. */
				tn3270e_request(hSession);
			}
			else if (hSession->lus != (char **)NULL)
			{
				/* No more LUs to try.  Give up. */
				backoff_tn3270e(hSession,_("Host rejected resource(s)"));
			}
			else
			{
				backoff_tn3270e(hSession,_("Device type rejected"));
			}

			break;
		default:
			trace_dsn(hSession,"??%u SE\n", hSession->sbbuf[2]);
			break;
		}
		break;

	case TN3270E_OP_FUNCTIONS:

		/* Functions negotiation. */
		trace_dsn(hSession,"FUNCTIONS ");

		switch (hSession->sbbuf[2]) {

		case TN3270E_OP_REQUEST:

			/* Host is telling us what functions they want. */
			trace_dsn(hSession,"REQUEST %s SE\n",tn3270e_function_names(hSession->sbbuf+3, sblen-3));

			e_rcvd = tn3270e_fdecode(hSession->sbbuf+3, sblen-3);
			if ((e_rcvd == hSession->e_funcs) || (hSession->e_funcs & ~e_rcvd)) {
				/* They want what we want, or less.  Done. */
				hSession->e_funcs = e_rcvd;
				tn3270e_subneg_send(hSession, TN3270E_OP_IS, hSession->e_funcs);
				hSession->tn3270e_negotiated = 1;
				trace_dsn(hSession,"TN3270E option negotiation complete.\n");
				check_in3270(hSession);
			} else {
				/*
				 * They want us to do something we can't.
				 * Request the common subset.
				 */
				hSession->e_funcs &= e_rcvd;
				tn3270e_subneg_send(hSession, TN3270E_OP_REQUEST,hSession->e_funcs);
			}
			break;

		case TN3270E_OP_IS:

			/* They accept our last request, or a subset thereof. */
			trace_dsn(hSession,"IS %s SE\n",tn3270e_function_names(hSession->sbbuf+3, sblen-3));
			e_rcvd = tn3270e_fdecode(hSession->sbbuf+3, sblen-3);
			if (e_rcvd != hSession->e_funcs) {
				if (hSession->e_funcs & ~e_rcvd) {
					/*
					 * They've removed something.  This is
					 * technically illegal, but we can
					 * live with it.
					 */
					hSession->e_funcs = e_rcvd;
				} else {
					/*
					 * They've added something.  Abandon
					 * TN3270E, they're brain dead.
					 */
					backoff_tn3270e(hSession,_( "Host illegally added function(s)" ));
					break;
				}
			}
			hSession->tn3270e_negotiated = 1;
			trace_dsn(hSession,"TN3270E option negotiation complete.\n");
			check_in3270(hSession);
			break;

		default:
			trace_dsn(hSession,"??%u SE\n", hSession->sbbuf[2]);
			break;
		}
		break;

	default:
		trace_dsn(hSession,"??%u SE\n", hSession->sbbuf[1]);
	}

	/* Good enough for now. */
	return 0;
}

#if defined(X3270_TRACE) /*[*/
/* Expand a string of TN3270E function codes into text. */
static const char *
tn3270e_function_names(const unsigned char *buf, int len)
{
	int i;
	static char text_buf[1024];
	char *s = text_buf;

	if (!len)
		return("(null)");
	for (i = 0; i < len; i++) {
		s += sprintf(s, "%s%s", (s == text_buf) ? "" : " ",
		    fnn(buf[i]));
	}
	return text_buf;
}
#endif /*]*/

/* Expand the current TN3270E function codes into text. */ /*
const char *
tn3270e_current_opts(void)
{
	int i;
	static char text_buf[1024];
	char *s = text_buf;

	if (!h3270.e_funcs || !IN_E)
		return CN;
	for (i = 0; i < 32; i++) {
		if (h3270.e_funcs & E_OPT(i))
		s += sprintf(s, "%s%s", (s == text_buf) ? "" : " ",
		    fnn(i));
	}
	return text_buf;
}
*/

/* Transmit a TN3270E FUNCTIONS REQUEST or FUNCTIONS IS message. */
static void tn3270e_subneg_send(H3270 *hSession, unsigned char op, unsigned long funcs)
{
	unsigned char proto_buf[7 + 32];
	int proto_len;
	int i;

	/* Construct the buffers. */
	(void) memcpy(proto_buf, functions_req, 4);
	proto_buf[4] = op;
	proto_len = 5;
	for (i = 0; i < 32; i++) {
		if (funcs & E_OPT(i))
			proto_buf[proto_len++] = i;
	}

	/* Complete and send out the protocol message. */
	proto_buf[proto_len++] = IAC;
	proto_buf[proto_len++] = SE;
	net_rawout(hSession, proto_buf, proto_len);

	/* Complete and send out the trace text. */
	trace_dsn(hSession,"SENT %s %s FUNCTIONS %s %s %s\n",
	    cmd(SB), opt(TELOPT_TN3270E),
	    (op == TN3270E_OP_REQUEST)? "REQUEST": "IS",
	    tn3270e_function_names(proto_buf + 5, proto_len - 7),
	    cmd(SE));
}

/* Translate a string of TN3270E functions into a bit-map. */
static unsigned long
tn3270e_fdecode(const unsigned char *buf, int len)
{
	unsigned long r = 0L;
	int i;

	/* Note that this code silently ignores options >= 32. */
	for (i = 0; i < len; i++) {
		if (buf[i] < 32)
			r |= E_OPT(buf[i]);
	}
	return r;
}
#endif /*]*/

#if defined(X3270_TN3270E) /*[*/
static void process_bind(H3270 *hSession, unsigned char *buf, int buflen)
{
	int namelen, i;

	(void) memset(hSession->plu_name, '\0', sizeof(hSession->plu_name));

	/* Make sure it's a BIND. */
	if (buflen < 1 || buf[0] != BIND_RU) {
		return;
	}
	buf++;
	buflen--;

	/* Extract the PLU name. */
	if (buflen < BIND_OFF_PLU_NAME + BIND_PLU_NAME_MAX)
		return;
	namelen = buf[BIND_OFF_PLU_NAME_LEN];
	if (namelen > BIND_PLU_NAME_MAX)
		namelen = BIND_PLU_NAME_MAX;
	for (i = 0; i < namelen; i++) {
		hSession->plu_name[i] = ebc2asc0[buf[BIND_OFF_PLU_NAME + i]];
	}
}
#endif /*]*/

static int
process_eor(H3270 *hSession)
{
	if (hSession->syncing || !(hSession->ibptr - hSession->ibuf))
		return(0);

#if defined(X3270_TN3270E) /*[*/
	if (IN_E) {
		tn3270e_header *h = (tn3270e_header *) hSession->ibuf;
		unsigned char *s;
		enum pds rv;

		trace_dsn(hSession,"RCVD TN3270E(%s%s %s %u)\n",
		    e_dt(h->data_type),
		    e_rq(h->data_type, h->request_flag),
		    e_rsp(h->data_type, h->response_flag),
		    h->seq_number[0] << 8 | h->seq_number[1]);

		switch (h->data_type) {
		case TN3270E_DT_3270_DATA:
			if ((hSession->e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)) &&
			    !hSession->tn3270e_bound)
				return 0;
			hSession->tn3270e_submode = E_3270;
			check_in3270(hSession);
			hSession->response_required = h->response_flag;
			rv = process_ds(hSession, hSession->ibuf + EH_SIZE,(hSession->ibptr - hSession->ibuf) - EH_SIZE);
			if (rv < 0 &&
			    hSession->response_required != TN3270E_RSF_NO_RESPONSE)
				tn3270e_nak(hSession,rv);
			else if (rv == PDS_OKAY_NO_OUTPUT &&
			    hSession->response_required == TN3270E_RSF_ALWAYS_RESPONSE)
				tn3270e_ack(hSession);
			hSession->response_required = TN3270E_RSF_NO_RESPONSE;
			return 0;
		case TN3270E_DT_BIND_IMAGE:
			if (!(hSession->e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
				return 0;
			process_bind(hSession, hSession->ibuf + EH_SIZE, (hSession->ibptr - hSession->ibuf) - EH_SIZE);
			trace_dsn(hSession,"< BIND PLU-name '%s'\n", hSession->plu_name);
			hSession->tn3270e_bound = 1;
			check_in3270(hSession);
			return 0;
		case TN3270E_DT_UNBIND:
			if (!(hSession->e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
				return 0;
			hSession->tn3270e_bound = 0;
			if (hSession->tn3270e_submode == E_3270)
				hSession->tn3270e_submode = E_NONE;
			check_in3270(hSession);
			return 0;
		case TN3270E_DT_NVT_DATA:
			/* In tn3270e NVT mode */
			hSession->tn3270e_submode = E_NVT;
			check_in3270(hSession);
			for (s = hSession->ibuf; s < hSession->ibptr; s++) {
				ansi_process(hSession,*s++);
			}
			return 0;
		case TN3270E_DT_SSCP_LU_DATA:
			if (!(hSession->e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
				return 0;
			hSession->tn3270e_submode = E_SSCP;
			check_in3270(hSession);
			ctlr_write_sscp_lu(hSession, hSession->ibuf + EH_SIZE,(hSession->ibptr - hSession->ibuf) - EH_SIZE);
			return 0;
		default:
			/* Should do something more extraordinary here. */
			return 0;
		}
	} else
#endif /*]*/
	{
		(void) process_ds(hSession, hSession->ibuf, hSession->ibptr - hSession->ibuf);
	}
	return 0;
}


/*
 * net_exception
 *	Called when there is an exceptional condition on the socket.
 */
void net_exception(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	trace_dsn(session,"RCVD urgent data indication\n");
	if (!session->syncing)
	{
		session->syncing = 1;

		if(session->excepting)
		{
			RemoveInput(session->ns_exception_id);
			session->ns_exception_id = NULL;
			session->excepting = 0;
		}
	}
}

/*
 * Flavors of Network Output:
 *
 *   3270 mode
 *	net_output	send a 3270 record
 *
 *   ANSI mode; call each other in turn
 *	net_sendc	net_cookout for 1 byte
 *	net_sends	net_cookout for a null-terminated string
 *	net_cookout	send user data with cooked-mode processing, ANSI mode
 *	net_cookedout	send user data, ANSI mode, already cooked
 *	net_rawout	send telnet protocol data, ANSI mode
 *
 */

LIB3270_INTERNAL int lib3270_sock_send(H3270 *hSession, unsigned const char *buf, int len)
{
	int rc;

#if defined(HAVE_LIBSSL)
	if(hSession->ssl_con != NULL)
		rc = SSL_write(hSession->ssl_con, (const char *) buf, len);
	else
		rc = send(hSession->sock, (const char *) buf, len, 0);
#else
		rc = send(hSession->sock, (const char *) buf, len, 0);
#endif // HAVE_LIBSSL

	if(rc > 0)
		return rc;

	// Recv error, notify

#if defined(HAVE_LIBSSL)
	if(hSession->ssl_con != NULL)
	{
		unsigned long e;
		char err_buf[120];

		e = ERR_get_error();
		(void) ERR_error_string(e, err_buf);
		trace_dsn(hSession,"RCVD SSL_write error %ld (%s)\n", e,err_buf);
		popup_an_error(hSession,_( "SSL_write:\n%s" ), err_buf);
		return -1;
	}
#endif // HAVE_LIBSSL

	trace_dsn(hSession,"RCVD socket error %d\n", socket_errno());

	switch(socket_errno())
	{
	case SE_EPIPE:
		popup_an_error(hSession, "%s", N_( "Broken pipe" ));
		break;

	case SE_ECONNRESET:
		popup_an_error(hSession, "%s", N_( "Connection reset by peer" ));
		break;

	case SE_EINTR:
		return 0;

	default:
		popup_a_sockerr(NULL, "%s", N_( "Socket write error" ) );

	}

	return -1;
}

/**
 * Send out raw telnet data.
 *
 * We assume that there will always be enough space to buffer what we want to transmit,
 * so we don't handle EAGAIN or EWOULDBLOCK.
 *
 * @param hSession	Session handle.
 * @param buf		Buffer to send.
 * @param len		Buffer length
 *
 */
static void net_rawout(H3270 *hSession, unsigned const char *buf, size_t len)
{
	trace_netdata(hSession, '>', buf, len);

	while (len)
	{
		int nw = hSession->write(hSession,buf,len);

		if (nw > 0)
		{
			// Data received
			hSession->ns_bsent += nw;
			len -= nw;
			buf += nw;
		}
		else if(nw < 0)
		{
			host_disconnect(hSession,True);
			return;
		}
	}
}

#if defined(X3270_ANSI)

/*
 * net_cookedout
 *	Send user data out in ANSI mode, without cooked-mode processing.
 */
static void
net_cookedout(H3270 *hSession, const char *buf, int len)
{
#if defined(X3270_TRACE)
	if (lib3270_get_toggle(hSession,LIB3270_TOGGLE_DS_TRACE))
	{
		int i;

		trace_dsn(hSession,">");
		for (i = 0; i < len; i++)
			trace_dsn(hSession," %s", ctl_see((int) *(buf+i)));
		trace_dsn(hSession,"\n");
	}
#endif
	net_rawout(hSession,(unsigned const char *) buf, len);
}


/*
 * net_cookout
 *	Send output in ANSI mode, including cooked-mode processing if
 *	appropriate.
 */
static void net_cookout(H3270 *hSession, const char *buf, int len)
{

	if (!IN_ANSI || (hSession->kybdlock & KL_AWAITING_FIRST))
		return;

	if (hSession->linemode) {
		register int	i;
		char	c;

		for (i = 0; i < len; i++) {
			c = buf[i];

			/* Input conversions. */
			if (!hSession->lnext && c == '\r' && hSession->icrnl)
				c = '\n';
			else if (!hSession->lnext && c == '\n' && hSession->inlcr)
				c = '\r';

			/* Backslashes. */
			if (c == '\\' && !hSession->backslashed)
				hSession->backslashed = 1;
			else
				hSession->backslashed = 0;

			/* Control chars. */
			if (c == '\n')
				do_eol(hSession,c);
			else if (c == vintr)
				do_intr(hSession, c);
			else if (c == vquit)
				do_quit(hSession,c);
			else if (c == verase)
				do_cerase(hSession,c);
			else if (c == vkill)
				do_kill(hSession,c);
			else if (c == vwerase)
				do_werase(hSession,c);
			else if (c == vrprnt)
				do_rprnt(hSession,c);
			else if (c == veof)
				do_eof(hSession,c);
			else if (c == vlnext)
				do_lnext(hSession,c);
			else if (c == 0x08 || c == 0x7f) /* Yes, a hack. */
				do_cerase(hSession,c);
			else
				do_data(hSession,c);
		}
		return;
	} else
		net_cookedout(hSession, buf, len);
}


/*
 * Cooked mode input processing.
 */

static void cooked_init(H3270 *hSession)
{
	if (hSession->lbuf == (unsigned char *)NULL)
		hSession->lbuf = (unsigned char *)lib3270_malloc(BUFSZ);
	hSession->lbptr = hSession->lbuf;
	hSession->lnext = 0;
	hSession->backslashed = 0;
}

static void ansi_process_s(H3270 *hSession, const char *data)
{
	while (*data)
		ansi_process(hSession,(unsigned int) *data++);
}

static void forward_data(H3270 *hSession)
{
	net_cookedout(hSession, (char *) hSession->lbuf, hSession->lbptr - hSession->lbuf);
	cooked_init(hSession);
}

static void do_data(H3270 *hSession, char c)
{
	if (hSession->lbptr+1 < hSession->lbuf + BUFSZ)
	{
		*hSession->lbptr++ = c;
		if (c == '\r')
			*hSession->lbptr++ = '\0';
		if (c == '\t')
			ansi_process(hSession,(unsigned int) c);
		else
			ansi_process_s(hSession,ctl_see((int) c));
	}
	else
		ansi_process_s(hSession,"\007");

	hSession->lnext = 0;
	hSession->backslashed = 0;
}

static void do_intr(H3270 *hSession, char c)
{
	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}
	ansi_process_s(hSession,ctl_see((int) c));
	cooked_init(hSession);
	net_interrupt(hSession);
}

static void do_quit(H3270 *hSession, char c)
{
	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}
	ansi_process_s(hSession,ctl_see((int) c));
	cooked_init(hSession);
	net_break(hSession);
}

static void do_cerase(H3270 *hSession, char c)
{
	int len;

	if (hSession->backslashed)
	{
		hSession->lbptr--;
		ansi_process_s(hSession,"\b");
		do_data(hSession,c);
		return;
	}

	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}

	if (hSession->lbptr > hSession->lbuf)
	{
		len = strlen(ctl_see((int) *--hSession->lbptr));

		while (len--)
			ansi_process_s(hSession,"\b \b");
	}
}

static void do_werase(H3270 *hSession, char c)
{
	int any = 0;
	int len;

	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}

	while (hSession->lbptr > hSession->lbuf) {
		char ch;

		ch = *--hSession->lbptr;

		if (ch == ' ' || ch == '\t') {
			if (any) {
				++hSession->lbptr;
				break;
			}
		} else
			any = 1;
		len = strlen(ctl_see((int) ch));

		while (len--)
			ansi_process_s(hSession,"\b \b");
	}
}

static void do_kill(H3270 *hSession, char c)
{
	int i, len;

	if (hSession->backslashed)
	{
		hSession->lbptr--;
		ansi_process_s(hSession,"\b");
		do_data(hSession,c);
		return;
	}

	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}

	while (hSession->lbptr > hSession->lbuf)
	{
		len = strlen(ctl_see((int) *--hSession->lbptr));

		for (i = 0; i < len; i++)
			ansi_process_s(hSession,"\b \b");
	}
}

static void do_rprnt(H3270 *hSession, char c)
{
	unsigned char *p;

	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}

	ansi_process_s(hSession,ctl_see((int) c));
	ansi_process_s(hSession,"\r\n");
	for (p = hSession->lbuf; p < hSession->lbptr; p++)
		ansi_process_s(hSession,ctl_see((int) *p));
}

static void do_eof(H3270 *hSession, char c)
{
	if (hSession->backslashed)
	{
		hSession->lbptr--;
		ansi_process_s(hSession,"\b");
		do_data(hSession,c);
		return;
	}

	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}

	do_data(hSession,c);
	forward_data(hSession);
}

static void do_eol(H3270 *hSession, char c)
{
	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}

	if (hSession->lbptr+2 >= hSession->lbuf + BUFSZ)
	{
		ansi_process_s(hSession,"\007");
		return;
	}

	*hSession->lbptr++ = '\r';
	*hSession->lbptr++ = '\n';
	ansi_process_s(hSession,"\r\n");
	forward_data(hSession);
}

static void do_lnext(H3270 *hSession, char c)
{
	if (hSession->lnext)
	{
		do_data(hSession,c);
		return;
	}
	hSession->lnext = 1;
	ansi_process_s(hSession,"^\b");
}
#endif /*]*/



/*
 * check_in3270
 *	Check for switches between NVT, SSCP-LU and 3270 modes.
 */
static void
check_in3270(H3270 *session)
{
	LIB3270_CSTATE new_cstate = NOT_CONNECTED;

#if defined(X3270_TRACE) /*[*/
	static const char *state_name[] = {
		"unconnected",
		"resolving",
		"pending",
		"connected initial",
		"TN3270 NVT",
		"TN3270 3270",
		"TN3270E",
		"TN3270E NVT",
		"TN3270E SSCP-LU",
		"TN3270E 3270"
	};
#endif /*]*/

#if defined(X3270_TN3270E) /*[*/
	if (session->myopts[TELOPT_TN3270E]) {
		if (!session->tn3270e_negotiated)
			new_cstate = CONNECTED_INITIAL_E;
		else switch (session->tn3270e_submode) {
		case E_NONE:
			new_cstate = CONNECTED_INITIAL_E;
			break;
		case E_NVT:
			new_cstate = CONNECTED_NVT;
			break;
		case E_3270:
			new_cstate = CONNECTED_TN3270E;
			break;
		case E_SSCP:
			new_cstate = CONNECTED_SSCP;
			break;
		}
	} else
#endif /*]*/
	if (session->myopts[TELOPT_BINARY] &&
	           session->myopts[TELOPT_EOR] &&
	           session->myopts[TELOPT_TTYPE] &&
	           session->hisopts[TELOPT_BINARY] &&
	           session->hisopts[TELOPT_EOR]) {
		new_cstate = CONNECTED_3270;
	} else if (session->cstate == CONNECTED_INITIAL) {
		/* Nothing has happened, yet. */
		return;
	} else {
		new_cstate = CONNECTED_ANSI;
	}

	if (new_cstate != session->cstate) {
#if defined(X3270_TN3270E) /*[*/
		int was_in_e = IN_E;
#endif /*]*/

#if defined(X3270_TN3270E) /*[*/
		/*
		 * If we've now switched between non-TN3270E mode and
		 * TN3270E mode, reset the LU list so we can try again
		 * in the new mode.
		 */
		if (session->lus != (char **)NULL && was_in_e != IN_E) {
			session->curr_lu = session->lus;
			session->try_lu = *session->curr_lu;
		}
#endif /*]*/

		/* Allocate the initial 3270 input buffer. */
		if(new_cstate >= CONNECTED_INITIAL && !(session->ibuf_size && session->ibuf))
		{
			session->ibuf 		= (unsigned char *) lib3270_malloc(BUFSIZ);
			session->ibuf_size	= BUFSIZ;
			session->ibptr		= session->ibuf;
		}

#if defined(X3270_ANSI) /*[*/
		/* Reinitialize line mode. */
		if ((new_cstate == CONNECTED_ANSI && session->linemode) ||
		    new_cstate == CONNECTED_NVT)
			cooked_init(session);
#endif /*]*/

#if defined(X3270_TN3270E) /*[*/
		/* If we fell out of TN3270E, remove the state. */
		if (!session->myopts[TELOPT_TN3270E]) {
			session->tn3270e_negotiated = 0;
			session->tn3270e_submode = E_NONE;
			session->tn3270e_bound = 0;
		}
#endif /*]*/
		trace_dsn(session,"Now operating in %s mode.\n",state_name[new_cstate]);
		host_in3270(session,new_cstate);
	}
}

/*
 * store3270in
 *	Store a character in the 3270 input buffer, checking for buffer
 *	overflow and reallocating ibuf if necessary.
 */
static void store3270in(H3270 *hSession, unsigned char c)
{
	if(hSession->ibptr - hSession->ibuf >= hSession->ibuf_size)
	{
		hSession->ibuf_size += BUFSIZ;
		hSession->ibuf = (unsigned char *) lib3270_realloc((char *) hSession->ibuf, hSession->ibuf_size);
		hSession->ibptr = hSession->ibuf + hSession->ibuf_size - BUFSIZ;
	}
	*hSession->ibptr++ = c;
}

/**
 * Ensure that <n> more characters will fit in the 3270 output buffer.
 *
 * Allocates the buffer in BUFSIZ chunks.
 * Allocates hidden space at the front of the buffer for TN3270E.
 *
 * @param hSession	3270 session handle.
 * @param n			Number of characters to set.
 */
void space3270out(H3270 *hSession, int n)
{
	unsigned nc = 0;	/* amount of data currently in obuf */
	unsigned more = 0;

	if (hSession->obuf_size)
		nc = hSession->obptr - hSession->obuf;

	while ((nc + n + EH_SIZE) > (hSession->obuf_size + more))
	{
		more += BUFSIZ;
	}

	if (more)
	{
		hSession->obuf_size += more;
		hSession->obuf_base = (unsigned char *)Realloc((char *) hSession->obuf_base,hSession->obuf_size);
		hSession->obuf = hSession->obuf_base + EH_SIZE;
		hSession->obptr = hSession->obuf + nc;
	}
}


/**
 *	Set the session variable 'linemode', which says whether we are in
 *	character-by-character mode or line mode.
 */
static void check_linemode(H3270 *hSession, Boolean init)
{
	int wasline = hSession->linemode;

	/*
	 * The next line is a deliberate kluge to effectively ignore the SGA
	 * option.  If the host will echo for us, we assume
	 * character-at-a-time; otherwise we assume fully cooked by us.
	 *
	 * This allows certain IBM hosts which volunteer SGA but refuse
	 * ECHO to operate more-or-less normally, at the expense of
	 * implementing the (hopefully useless) "character-at-a-time, local
	 * echo" mode.
	 *
	 * We still implement "switch to line mode" and "switch to character
	 * mode" properly by asking for both SGA and ECHO to be off or on, but
	 * we basically ignore the reply for SGA.
	 */
	hSession->linemode = hSession->hisopts[TELOPT_ECHO] ? 0 : 1 /* && !hisopts[TELOPT_SGA] */;

	if (init || hSession->linemode != wasline)
	{
		lib3270_st_changed(hSession,LIB3270_STATE_LINE_MODE, hSession->linemode);
		if (!init)
		{
			trace_dsn(hSession,"Operating in %s mode.\n",hSession->linemode ? "line" : "character-at-a-time");
		}
#if defined(X3270_ANSI) /*[*/
		if (IN_ANSI && hSession->linemode)
			cooked_init(hSession);
#endif /*]*/
	}
}


#if defined(X3270_TRACE)

/*
 * nnn
 *	Expands a number to a character string, for displaying unknown telnet
 *	commands and options.
 */
static const char *
nnn(int c)
{
	static char	buf[64];

	(void) sprintf(buf, "%d", c);
	return buf;
}

/*
 * cmd
 *	Expands a TELNET command into a character string.
 */
static const char * cmd(int c)
{
	if (TELCMD_OK(c))
		return TELCMD(c);
	else
		return nnn(c);
}

/*
 * opt
 *	Expands a TELNET option into a character string.
 */
static const char *
opt(unsigned char c)
{
	if (TELOPT_OK(c))
		return TELOPT(c);
	else if (c == TELOPT_TN3270E)
		return "TN3270E";
#if defined(HAVE_LIBSSL) /*[*/
	else if (c == TELOPT_STARTTLS)
		return "START-TLS";
#endif /*]*/
	else
		return nnn((int)c);
}

#define LINEDUMP_MAX	32

void trace_netdata(H3270 *hSession, char direction, unsigned const char *buf, int len)
{
	int offset;
	struct timeval ts;
	double tdiff;

	if (!lib3270_get_toggle(hSession,LIB3270_TOGGLE_DS_TRACE))
		return;

	(void) gettimeofday(&ts, (struct timezone *)NULL);
	if (IN_3270)
	{
		tdiff = ((1.0e6 * (double)(ts.tv_sec - hSession->ds_ts.tv_sec)) +
			(double)(ts.tv_usec - hSession->ds_ts.tv_usec)) / 1.0e6;
		trace_dsn(hSession,"%c +%gs\n", direction, tdiff);
	}

	hSession->ds_ts = ts;
	for (offset = 0; offset < len; offset++)
	{
		if (!(offset % LINEDUMP_MAX))
			trace_dsn(hSession,"%s%c 0x%-3x ",(offset ? "\n" : ""), direction, offset);
		trace_dsn(hSession,"%02x", buf[offset]);
	}
	trace_dsn(hSession,"\n");
}
#endif // X3270_TRACE


/**
 * Send 3270 output over the network.
 *
 * Send 3270 output over the network:
 *	- Prepend TN3270E header
 *	- Expand IAC to IAC IAC
 *	- Append IAC EOR
 *
 * @param hSession Session handle
 *
 */
void net_output(H3270 *hSession)
{
	static unsigned char *xobuf = NULL;
	static int xobuf_len = 0;
	int need_resize = 0;
	unsigned char *nxoptr, *xoptr;

#if defined(X3270_TN3270E)
	#define BSTART	((IN_TN3270E || IN_SSCP) ? hSession->obuf_base : hSession->obuf)
#else
	#define BSTART	obuf
#endif

#if defined(X3270_TN3270E) /*[*/
	/* Set the TN3720E header. */
	if (IN_TN3270E || IN_SSCP)
	{
		tn3270e_header *h = (tn3270e_header *) hSession->obuf_base;

		/* Check for sending a TN3270E response. */
		if (hSession->response_required == TN3270E_RSF_ALWAYS_RESPONSE)
		{
			tn3270e_ack(hSession);
			hSession->response_required = TN3270E_RSF_NO_RESPONSE;
		}

		/* Set the outbound TN3270E header. */
		h->data_type = IN_TN3270E ? TN3270E_DT_3270_DATA : TN3270E_DT_SSCP_LU_DATA;
		h->request_flag = 0;
		h->response_flag = 0;
		h->seq_number[0] = (hSession->e_xmit_seq >> 8) & 0xff;
		h->seq_number[1] = hSession->e_xmit_seq & 0xff;

		trace_dsn(hSession,"SENT TN3270E(%s NO-RESPONSE %u)\n",IN_TN3270E ? "3270-DATA" : "SSCP-LU-DATA", hSession->e_xmit_seq);
		if (hSession->e_funcs & E_OPT(TN3270E_FUNC_RESPONSES))
			hSession->e_xmit_seq = (hSession->e_xmit_seq + 1) & 0x7fff;
	}
#endif /*]*/

	/* Reallocate the expanded output buffer. */
	while (xobuf_len <  (hSession->obptr - BSTART + 1) * 2)
	{
		xobuf_len += BUFSZ;
		need_resize++;
	}

	if (need_resize)
	{
		Replace(xobuf, (unsigned char *)lib3270_malloc(xobuf_len));
	}

	/* Copy and expand IACs. */
	xoptr = xobuf;
	nxoptr = BSTART;
	while (nxoptr < hSession->obptr)
	{
		if ((*xoptr++ = *nxoptr++) == IAC)
		{
			*xoptr++ = IAC;
		}
	}

	/* Append the IAC EOR and transmit. */
	*xoptr++ = IAC;
	*xoptr++ = EOR;
	net_rawout(hSession,xobuf, xoptr - xobuf);

	trace_dsn(hSession,"SENT EOR\n");
	hSession->ns_rsent++;
#undef BSTART
}

#if defined(X3270_TN3270E) /*[*/
/* Send a TN3270E positive response to the server. */
static void tn3270e_ack(H3270 *hSession)
{
	unsigned char rsp_buf[10];
	tn3270e_header *h_in = (tn3270e_header *) hSession->ibuf;
	int rsp_len = 0;

	rsp_len = 0;
	rsp_buf[rsp_len++] = TN3270E_DT_RESPONSE;	    /* data_type */
	rsp_buf[rsp_len++] = 0;				    /* request_flag */
	rsp_buf[rsp_len++] = TN3270E_RSF_POSITIVE_RESPONSE; /* response_flag */
	rsp_buf[rsp_len++] = h_in->seq_number[0];	    /* seq_number[0] */

	if (h_in->seq_number[0] == IAC)
		rsp_buf[rsp_len++] = IAC;

	rsp_buf[rsp_len++] = h_in->seq_number[1];	    /* seq_number[1] */

	if (h_in->seq_number[1] == IAC)
		rsp_buf[rsp_len++] = IAC;

	rsp_buf[rsp_len++] = TN3270E_POS_DEVICE_END;
	rsp_buf[rsp_len++] = IAC;
	rsp_buf[rsp_len++] = EOR;
	trace_dsn(hSession,"SENT TN3270E(RESPONSE POSITIVE-RESPONSE %u) DEVICE-END\n",h_in->seq_number[0] << 8 | h_in->seq_number[1]);
	net_rawout(hSession, rsp_buf, rsp_len);
}

/* Send a TN3270E negative response to the server. */
static void tn3270e_nak(H3270 *hSession, enum pds rv)
{
	unsigned char rsp_buf[10];
	tn3270e_header *h_in = (tn3270e_header *) hSession->ibuf;
	int rsp_len = 0;
	char *neg = NULL;

	rsp_buf[rsp_len++] = TN3270E_DT_RESPONSE;	    /* data_type */
	rsp_buf[rsp_len++] = 0;				    /* request_flag */
	rsp_buf[rsp_len++] = TN3270E_RSF_NEGATIVE_RESPONSE; /* response_flag */
	rsp_buf[rsp_len++] = h_in->seq_number[0];	    /* seq_number[0] */
	if (h_in->seq_number[0] == IAC)
		rsp_buf[rsp_len++] = IAC;
	rsp_buf[rsp_len++] = h_in->seq_number[1];	    /* seq_number[1] */
	if (h_in->seq_number[1] == IAC)
		rsp_buf[rsp_len++] = IAC;
	switch (rv) {
	default:
	case PDS_BAD_CMD:
		rsp_buf[rsp_len++] = TN3270E_NEG_COMMAND_REJECT;
		neg = "COMMAND-REJECT";
		break;
	case PDS_BAD_ADDR:
		rsp_buf[rsp_len++] = TN3270E_NEG_OPERATION_CHECK;
		neg = "OPERATION-CHECK";
		break;
	}
	rsp_buf[rsp_len++] = IAC;
	rsp_buf[rsp_len++] = EOR;
	trace_dsn(hSession,"SENT TN3270E(RESPONSE NEGATIVE-RESPONSE %u) %s\n",h_in->seq_number[0] << 8 | h_in->seq_number[1], neg);
	net_rawout(hSession, rsp_buf, rsp_len);
}
#endif /*]*/

#if defined(X3270_TRACE) /*[*/
/*
 * Add IAC EOR to a buffer.
 */
void
net_add_eor(unsigned char *buf, int len)
{
	buf[len++] = IAC;
	buf[len++] = EOR;
}
#endif /*]*/


#if defined(X3270_ANSI) /*[*/
/**
 * Send a character of user data over the network in ANSI mode.
 *
 * @param hSession	Session handle.
 * @param c			Character to send.
 *
 */
void
net_sendc(H3270 *hSession, char c)
{
	if (c == '\r' && !hSession->linemode)
	{
		/* CR must be quoted */
		net_cookout(hSession,"\r\0", 2);
	}
	else
	{
		net_cookout(hSession,&c, 1);
	}
}

/**
 * Send a null-terminated string of user data in ANSI mode.
 *
 * @param hSession	Session handle.
 * @param s			String to send.
 */
void net_sends(H3270 *hSession,const char *s)
{
	net_cookout(hSession, s, strlen(s));
}

/**
 * Sends the ERASE character in ANSI mode.
 *
 */
void net_send_erase(H3270 *hSession)
{
	net_cookout(hSession, &verase, 1);
}

/**
 *	Sends the KILL character in ANSI mode.
 */
void net_send_kill(H3270 *hSession)
{
	net_cookout(hSession, &vkill, 1);
}

/**
 * Sends the WERASE character in ANSI mode.
 */
void net_send_werase(H3270 *hSession)
{
	net_cookout(hSession, &vwerase, 1);
}
#endif /*]*/

/*
#if defined(X3270_MENUS)

void net_charmode(H3270 *hSession)
{
	if (!CONNECTED)
		return;

	if (!hisopts[TELOPT_ECHO])
	{
		do_opt[2] = TELOPT_ECHO;
		net_rawout(do_opt, sizeof(do_opt));
		trace_dsn(hSession,"SENT %s %s\n", cmd(DO), opt(TELOPT_ECHO));
	}

	if (!hisopts[TELOPT_SGA])
	{
		do_opt[2] = TELOPT_SGA;
		net_rawout(do_opt, sizeof(do_opt));
		trace_dsn(hSession,"SENT %s %s\n", cmd(DO), opt(TELOPT_SGA));
	}
}
#endif
*/

/*
 * net_break
 *	Send telnet break, which is used to implement 3270 ATTN.
 *
 */
void net_break(H3270 *hSession)
{
	static const unsigned char buf[] = { IAC, BREAK };

	/* I don't know if we should first send TELNET synch ? */
	net_rawout(hSession, buf, sizeof(buf));
	trace_dsn(hSession,"SENT BREAK\n");
}

/*
 * net_interrupt
 *	Send telnet IP.
 *
 */
void net_interrupt(H3270 *hSession)
{
	static const unsigned char buf[] = { IAC, IP };

	/* I don't know if we should first send TELNET synch ? */
	net_rawout(hSession, buf, sizeof(buf));
	trace_dsn(hSession,"SENT IP\n");
}

/*
 * net_abort
 *	Send telnet AO.
 *
 */
#if defined(X3270_TN3270E) /*[*/
void net_abort(H3270 *hSession)
{
	static const unsigned char buf[] = { IAC, AO };

	if (hSession->e_funcs & E_OPT(TN3270E_FUNC_SYSREQ))
	{
		/*
		 * I'm not sure yet what to do here.  Should the host respond
		 * to the AO by sending us SSCP-LU data (and putting us into
		 * SSCP-LU mode), or should we put ourselves in it?
		 * Time, and testers, will tell.
		 */
		switch (hSession->tn3270e_submode)
		{
		case E_NONE:
		case E_NVT:
			break;

		case E_SSCP:
			net_rawout(hSession, buf, sizeof(buf));
			trace_dsn(hSession,"SENT AO\n");
			if (hSession->tn3270e_bound || !(hSession->e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
			{
				hSession->tn3270e_submode = E_3270;
				check_in3270(hSession);
			}
			break;

		case E_3270:
			net_rawout(hSession, buf, sizeof(buf));
			trace_dsn(hSession,"SENT AO\n");
			hSession->tn3270e_submode = E_SSCP;
			check_in3270(hSession);
			break;
		}
	}
}
#endif /*]*/

#if defined(X3270_ANSI) /*[*/
/*
 * parse_ctlchar
 *	Parse an stty control-character specification.
 *	A cheap, non-complaining implementation.
 */
static char
parse_ctlchar(char *s)
{
	if (!s || !*s)
		return 0;
	if ((int) strlen(s) > 1) {
		if (*s != '^')
			return 0;
		else if (*(s+1) == '?')
			return 0177;
		else
			return *(s+1) - '@';
	} else
		return *s;
}
#endif /*]*/

/*
 * Set blocking/non-blocking mode on the socket.  On error, pops up an error
 * message, but does not close the socket.
 */
static int non_blocking(H3270 *session, Boolean on)
{
# if defined(FIONBIO)

	int i = on ? 1 : 0;

	if (SOCK_IOCTL(session->sock, FIONBIO, (int *) &i) < 0)
	{
		popup_a_sockerr(session,N_( "ioctl(%s)" ), "FIONBIO");
		return -1;
	}

#else

	int f;

	if ((f = fcntl(session->sock, F_GETFL, 0)) == -1)
	{
		popup_an_errno(session,errno, N_( "fcntl(%s)" ), "F_GETFL" );
		return -1;
	}

	if (on)
		f |= O_NDELAY;
	else
		f &= ~O_NDELAY;

	if (fcntl(session->sock, F_SETFL, f) < 0)
	{
		popup_an_errno(session,errno, N_( "fcntl(%s)" ), "F_GETFL");
		return -1;
	}

#endif // FIONBIO

	trace("Socket %d is %s",session->sock, on ? "non-blocking" : "blocking");

	return 0;
}

#if defined(HAVE_LIBSSL) /*[*/

/* Initialize the OpenSSL library. */
static void ssl_init(H3270 *session)
{
	static SSL_CTX *ssl_ctx = NULL;

	set_ssl_state(session,LIB3270_SSL_UNDEFINED);

	if(ssl_ctx == NULL)
	{
		lib3270_write_log(session,"%s","Initializing SSL context");
		SSL_load_error_strings();
		SSL_library_init();
		ssl_ctx = SSL_CTX_new(SSLv23_method());
		if(ssl_ctx == NULL)
		{
			popup_an_error(NULL,"SSL_CTX_new failed");
			session->ssl_host = False;
			return;
		}
		SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL);
		SSL_CTX_set_info_callback(ssl_ctx, ssl_info_callback);
		SSL_CTX_set_default_verify_paths(ssl_ctx);
	}

	if(session->ssl_con)
		SSL_free(session->ssl_con);

	session->ssl_con = SSL_new(ssl_ctx);
	if(session->ssl_con == NULL)
	{
		popup_an_error(session,"SSL_new failed");
		session->ssl_host = False;
		return;
	}

	SSL_set_verify(session->ssl_con, 0/*xxx*/, NULL);

	/* XXX: May need to get key file and password. */
	/*
	if (appres.cert_file)
	{
		if (!(SSL_CTX_use_certificate_chain_file(ssl_ctx,
						appres.cert_file))) {
			unsigned long e;
			char err_buf[120];

			e = ERR_get_error();
			(void) ERR_error_string(e, err_buf);

			popup_an_error(NULL,"SSL_CTX_use_certificate_chain_file("
					"\"%s\") failed:\n%s",
					appres.cert_file, err_buf);
		}
	}
	*/
}

/* Callback for tracing protocol negotiation. */
static void ssl_info_callback(INFO_CONST SSL *s, int where, int ret)
{
	H3270 *hSession = &h3270; // TODO: Find a better way!

	switch(where)
	{
	case SSL_CB_CONNECT_LOOP:
		trace_dsn(hSession,"SSL_connect: %s %s\n",SSL_state_string(s), SSL_state_string_long(s));
		break;

	case SSL_CB_CONNECT_EXIT:

		trace_dsn(hSession,"%s: SSL_CB_CONNECT_EXIT\n",__FUNCTION__);

		if (ret == 0)
		{
			trace_dsn(hSession,"SSL_connect: failed in %s\n",SSL_state_string_long(s));
		}
		else if (ret < 0)
		{
			unsigned long e = ERR_get_error();
			char err_buf[1024];

			while(ERR_peek_error() == e)	// Remove other messages with the same error
				e = ERR_get_error();

			if(e != 0)
			{
				if(e == hSession->last_ssl_error)
					return;
				hSession->last_ssl_error = e;
				(void) ERR_error_string_n(e, err_buf, 1023);
			}
#if defined(_WIN32)
			else if (GetLastError() != 0)
			{
				strncpy(err_buf,win32_strerror(GetLastError()),1023);
			}
#else
			else if (errno != 0)
			{
				strncpy(err_buf, strerror(errno),1023);
			}
#endif
			else
			{
				err_buf[0] = '\0';
			}

			trace_dsn(hSession,"SSL Connect error in %s\nState: %s\nAlert: %s\n",err_buf,SSL_state_string_long(s),SSL_alert_type_string_long(ret));

			show_3270_popup_dialog(	hSession,									// H3270 *session,
									PW3270_DIALOG_CRITICAL,						//	PW3270_DIALOG type,
									_( "SSL Connect error" ),					// Title
									err_buf,									// Message
									_( "<b>Connection state:</b> %s\n<b>Alert message:</b> %s" ),
									SSL_state_string_long(s),
									SSL_alert_type_string_long(ret));


		}


	default:
		trace_dsn(hSession,"SSL Current state is \"%s\"\n",SSL_state_string_long(s));
	}

//	trace("%s: state=%04x where=%04x ret=%d",__FUNCTION__,SSL_state(s),where,ret);

#ifdef DEBUG
	if(where & SSL_CB_EXIT)
	{
		trace("%s: SSL_CB_EXIT ret=%d\n",__FUNCTION__,ret);
	}
#endif

	if(where & SSL_CB_ALERT)
		trace_dsn(hSession,"SSL ALERT: %s\n",SSL_alert_type_string_long(ret));

	if(where & SSL_CB_HANDSHAKE_DONE)
	{
		trace_dsn(hSession,"%s: SSL_CB_HANDSHAKE_DONE state=%04x\n",__FUNCTION__,SSL_state(s));
		if(SSL_state(s) == 0x03)
			set_ssl_state(hSession,LIB3270_SSL_SECURE);
		else
			set_ssl_state(hSession,LIB3270_SSL_UNSECURE);
	}
}

/**
 * Process a STARTTLS subnegotiation.
 */
static void continue_tls(H3270 *hSession, unsigned char *sbbuf, int len)
{
	int rv;

	/* Whatever happens, we're not expecting another SB STARTTLS. */
	hSession->need_tls_follows = 0;

	/* Make sure the option is FOLLOWS. */
	if (len < 2 || sbbuf[1] != TLS_FOLLOWS)
	{
		/* Trace the junk. */
		trace_dsn(hSession,"%s ? %s\n", opt(TELOPT_STARTTLS), cmd(SE));
		popup_an_error(hSession,_( "TLS negotiation failure"));
		net_disconnect(hSession);
		return;
	}

	/* Trace what we got. */
	trace_dsn(hSession,"%s FOLLOWS %s\n", opt(TELOPT_STARTTLS), cmd(SE));

	/* Initialize the SSL library. */
	ssl_init(hSession);
	if(hSession->ssl_con == NULL)
	{
		/* Failed. */
		net_disconnect(hSession);
		return;
	}

	/* Set up the TLS/SSL connection. */
	if(SSL_set_fd(hSession->ssl_con, hSession->sock) != 1)
	{
		trace_dsn(hSession,"SSL_set_fd failed!\n");
	}

//#if defined(_WIN32)
//	/* Make the socket blocking for SSL. */
//	(void) WSAEventSelect(hSession->sock, hSession->sock_handle, 0);
//	(void) non_blocking(False);
//#endif

	rv = SSL_connect(hSession->ssl_con);

//#if defined(_WIN32)
//	// Make the socket non-blocking again for event processing
//	(void) WSAEventSelect(hSession->sock, hSession->sock_handle, FD_READ | FD_CONNECT | FD_CLOSE);
//#endif

	if (rv != 1)
	{
		trace_dsn(hSession,"continue_tls: SSL_connect failed\n");
		net_disconnect(hSession);
		return;
	}

//	hSession->secure_connection = True;

	/* Success. */
//	trace_dsn(hSession,"TLS/SSL negotiated connection complete. Connection is now secure.\n");

	/* Tell the world that we are (still) connected, now in secure mode. */
	lib3270_set_connected(hSession);
}

#endif /*]*/

/* Return the local address for the socket. */
int net_getsockname(const H3270 *session, void *buf, int *len)
{
	if (session->sock < 0)
		return -1;
	return getsockname(session->sock, buf, (socklen_t *)(void *)len);
}

/* Return a text version of the current proxy type, or NULL. */ /*
char *
net_proxy_type(void)
{
   	if(h3270.proxy_type > 0)
		return proxy_type_name(h3270.proxy_type);
	else
		return NULL;
}
*/

/* Return the current proxy host, or NULL. */ /*
char *
net_proxy_host(void)
{
	if(h3270.proxy_type > 0)
		return h3270.proxy_host;
	else
		return NULL;
}
*/

/* Return the current proxy port, or NULL. */ /*
char *
net_proxy_port(void)
{
	if (h3270.proxy_type > 0)
		return h3270.proxy_portname;
	else
		return NULL;
}
*/

LIB3270_EXPORT LIB3270_SSL_STATE lib3270_get_secure(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);
	return session->secure;
}

/*
LIB3270_EXPORT int lib3270_get_ssl_state(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);

#if defined(HAVE_LIBSSL)
		return (h->secure_connection != 0);
#else
		return 0;
#endif
}
*/

/*
int Get3270Socket(void)
{
        return h3270.sock;
}
*/
