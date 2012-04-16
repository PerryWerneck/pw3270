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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como telnet.c e possui 3477 linhas de código.
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

#include "globals.h"
#include <errno.h>

#if defined(_WIN32) /*[*/
#include <winsock2.h>
#include <ws2tcpip.h>
#else /*][*/
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#endif /*]*/
#define TELCMDS 1
#define TELOPTS 1
#include "arpa_telnet.h"
#if !defined(_WIN32) /*[*/
#include <arpa/inet.h>
#endif /*]*/
#include <errno.h>
#include <fcntl.h>
#if !defined(_WIN32) /*[*/
#include <netdb.h>
#endif /*]*/
#include <stdarg.h>

#if defined(HAVE_LIBSSL) /*[*/
	#include <openssl/ssl.h>
	#include <openssl/err.h>
#endif /*]*/
#include "tn3270e.h"
#include "3270ds.h"

#include "appres.h"

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

#if !defined(TELOPT_NAWS) /*[*/
#define TELOPT_NAWS	31
#endif /*]*/

#if !defined(TELOPT_STARTTLS) /*[*/
#define TELOPT_STARTTLS	46
#endif /*]*/
#define TLS_FOLLOWS	1

#define BUFSZ		16384
#define TRACELINE	72

#define N_OPTS		256

/* Globals */
// char    	*hostname = CN;
time_t          ns_time;
int             ns_brcvd;
int             ns_rrcvd;
int             ns_bsent;
int             ns_rsent;
unsigned char  *obuf;		/* 3270 output buffer */
unsigned char  *obptr = (unsigned char *) NULL;
int             linemode = 1;

/*
#if defined(LOCAL_PROCESS)
Boolean		local_process = False;
#endif
// char           *termtype;
*/

/* Externals */
extern struct timeval ds_ts;

/* Statics */
// static int      		sock 			= -1;	/* active socket */

#if defined(HAVE_LIBSSL) /*[*/
static unsigned long last_ssl_error	= 0;
#endif

//#if defined(_WIN32) /*[*/
//static HANDLE	sock_handle = NULL;
//#endif /*]*/

static unsigned char myopts[N_OPTS], hisopts[N_OPTS];
			/* telnet option flags */
static unsigned char *ibuf = (unsigned char *) NULL;
			/* 3270 input buffer */
static unsigned char *ibptr;
static int      ibuf_size = 0;	/* size of ibuf */
static unsigned char *obuf_base = (unsigned char *)NULL;
static int	obuf_size = 0;
static unsigned char *netrbuf = (unsigned char *)NULL;
			/* network input buffer */
static unsigned char *sbbuf = (unsigned char *)NULL;
			/* telnet sub-option buffer */
static unsigned char *sbptr;
static unsigned char telnet_state;
static int      syncing;
#if !defined(_WIN32) /*[*/
static unsigned long output_id = 0L;
#endif /*]*/
static char     ttype_tmpval[13];

#if defined(X3270_TN3270E) /*[*/
static unsigned long e_funcs;	/* negotiated TN3270E functions */
#define E_OPT(n)	(1 << (n))
static unsigned short e_xmit_seq; /* transmit sequence number */
static int response_required;
#endif /*]*/

#if defined(X3270_ANSI) /*[*/
static int      ansi_data = 0;
static unsigned char *lbuf = (unsigned char *)NULL;
			/* line-mode input buffer */
static unsigned char *lbptr;
static int      lnext = 0;
static int      backslashed = 0;
static int      t_valid = 0;
static char     vintr;
static char     vquit;
static char     verase;
static char     vkill;
static char     veof;
static char     vwerase;
static char     vrprnt;
static char     vlnext;
#endif /*]*/

static int	tn3270e_negotiated = 0;
static enum { E_NONE, E_3270, E_NVT, E_SSCP } tn3270e_submode = E_NONE;
static int	tn3270e_bound = 0;
static char	plu_name[BIND_PLU_NAME_MAX+1];
static char	**lus = (char **)NULL;
static char	**curr_lu = (char **)NULL;
static char	*try_lu = CN;

static int	proxy_type = 0;
static char	*proxy_host = CN;
static char	*proxy_portname = CN;
static unsigned short proxy_port = 0;

static int telnet_fsm(unsigned char c);
static void net_rawout(unsigned const char *buf, int len);
static void check_in3270(void);
static void store3270in(unsigned char c);
static void check_linemode(Boolean init);
static int non_blocking(Boolean on);
static void net_connected(H3270 *session);
#if defined(X3270_TN3270E) /*[*/
static int tn3270e_negotiate(void);
#endif /*]*/
static int process_eor(void);
#if defined(X3270_TN3270E) /*[*/
#if defined(X3270_TRACE) /*[*/
static const char *tn3270e_function_names(const unsigned char *, int);
#endif /*]*/
static void tn3270e_subneg_send(unsigned char, unsigned long);
static unsigned long tn3270e_fdecode(const unsigned char *, int);
static void tn3270e_ack(void);
static void tn3270e_nak(enum pds);
#endif /*]*/

#if defined(X3270_ANSI) /*[*/
static void do_data(char c);
static void do_intr(char c);
static void do_quit(char c);
static void do_cerase(char c);
static void do_werase(char c);
static void do_kill(char c);
static void do_rprnt(char c);
static void do_eof(char c);
static void do_eol(char c);
static void do_lnext(char c);
static char parse_ctlchar(char *s);
static void cooked_init(void);
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
static unsigned char	do_opt[]	= {
	IAC, DO, '_' };
static unsigned char	dont_opt[]	= {
	IAC, DONT, '_' };
static unsigned char	will_opt[]	= {
	IAC, WILL, '_' };
static unsigned char	wont_opt[]	= {
	IAC, WONT, '_' };
#if defined(X3270_TN3270E) /*[*/
static unsigned char	functions_req[] = {
	IAC, SB, TELOPT_TN3270E, TN3270E_OP_FUNCTIONS };
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

#if defined(HAVE_LIBSSL) /*[*/
static SSL_CTX *ssl_ctx;
static SSL *ssl_con;
static Boolean need_tls_follows = False;
static void ssl_init(void);
#if OPENSSL_VERSION_NUMBER >= 0x00907000L /*[*/
#define INFO_CONST const
#else /*][*/
#define INFO_CONST
#endif /*]*/
static void client_info_callback(INFO_CONST SSL *s, int where, int ret);
static void continue_tls(unsigned char *sbbuf, int len);
#endif /*]*/

#if !defined(_WIN32) /*[*/
static void output_possible(H3270 *session);
#endif /*]*/

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
								N_( "Can´t use winsock version %d.%d" ), LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
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
 * @return The file descriptor of the connected socket.
 */
int net_connect(H3270 *session, const char *host, char *portname, Boolean ls, Boolean *resolving, Boolean *pending)
{
	struct servent	* sp;
	struct hostent	* hp;
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

#if defined(_WIN32)
	sockstart(session);
#endif

	if (netrbuf == (unsigned char *)NULL)
		netrbuf = (unsigned char *)Malloc(BUFSZ);

#if defined(X3270_ANSI) /*[*/
	if (!t_valid) {
		vintr   = parse_ctlchar(appres.intr);
		vquit   = parse_ctlchar(appres.quit);
		verase  = parse_ctlchar(appres.erase);
		vkill   = parse_ctlchar(appres.kill);
		veof    = parse_ctlchar(appres.eof);
		vwerase = parse_ctlchar(appres.werase);
		vrprnt  = parse_ctlchar(appres.rprnt);
		vlnext  = parse_ctlchar(appres.lnext);
		t_valid = 1;
	}
#endif /*]*/

	*resolving = False;
	*pending = False;

	Replace(session->hostname, NewString(host));

	/* get the passthru host and port number */
	if (session->passthru_host) {
		const char *hn;

		hn = getenv("INTERNET_HOST");
		if (hn == CN)
			hn = "internet-gateway";

		hp = gethostbyname(hn);
		if (hp == (struct hostent *) 0) {
			popup_an_error(NULL,"Unknown passthru host: %s", hn);
			return -1;
		}
		(void) memmove(passthru_haddr, hp->h_addr, hp->h_length);
		passthru_len = hp->h_length;

		sp = getservbyname("telnet-passthru","tcp");
		if (sp != (struct servent *)NULL)
			passthru_port = sp->s_port;
		else
			passthru_port = htons(3514);
	} else if (appres.proxy != CN && !proxy_type) {
	    	proxy_type = proxy_setup(&proxy_host, &proxy_portname);
		if (proxy_type > 0) {
		    	unsigned long lport;
			char *ptr;
			struct servent *sp;

			lport = strtoul(portname, &ptr, 0);
			if (ptr == portname || *ptr != '\0' || lport == 0L ||
				    lport & ~0xffff) {
				if (!(sp = getservbyname(portname, "tcp"))) {
					popup_an_error(NULL,"Unknown port number "
						"or service: %s", portname);
					return -1;
				}
				session->current_port = ntohs(sp->s_port);
			} else
				session->current_port = (unsigned short)lport;
		}
		if (proxy_type < 0)
		    	return -1;
	}

	/* fill in the socket address of the given host */
	(void) memset((char *) &haddr, 0, sizeof(haddr));
	if (session->passthru_host) {
		haddr.sin.sin_family = AF_INET;
		(void) memmove(&haddr.sin.sin_addr, passthru_haddr,
			       passthru_len);
		haddr.sin.sin_port = passthru_port;
		ha_len = sizeof(struct sockaddr_in);
	} else if (proxy_type > 0) {
	    	if (resolve_host_and_port(&h3270,proxy_host, proxy_portname,
			    &proxy_port, &haddr.sa, &ha_len, errmsg,
			    sizeof(errmsg)) < 0) {
		    	popup_an_error(session,errmsg);
		    	return -1;
		}
	} else {
			if (resolve_host_and_port(&h3270,host, portname,
				    &session->current_port, &haddr.sa, &ha_len,
				    errmsg, sizeof(errmsg)) < 0) {
			    	popup_an_error(session,errmsg);
			    	return -1;
			}
	}

	/* create the socket */
	if ((session->sock = socket(haddr.sa.sa_family, SOCK_STREAM, 0)) == -1)
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

	/* set the socket to be non-delaying */
#if defined(_WIN32)
	if (non_blocking(False) < 0)
#else
	if (non_blocking(True) < 0)
#endif
		close_fail;

#if !defined(_WIN32)
	/* don't share the socket with our children */
	(void) fcntl(session->sock, F_SETFD, 1);
#endif

	/* init ssl */
#if defined(HAVE_LIBSSL)
	last_ssl_error = 0;
	if (session->ssl_host)
		ssl_init();
#endif

	/* connect */
	status_connecting(session,1);

	switch(connect_sock(session, session->sock, &haddr.sa,ha_len))
	{
	case 0:					// Connected
		trace_dsn("Connected.\n");
		if(non_blocking(False) < 0)
			close_fail;
		net_connected(session);
		break;

	case SE_EWOULDBLOCK:	// Connection in progress
	case SE_EINPROGRESS:
		*pending = True;
		trace_dsn("Connection pending.\n");
#if !defined(_WIN32)
		output_id = AddOutput(session->sock, session, output_possible);
#endif
		break;

	default:
		popup_a_sockerr(session, N_( "Can't connect to %s:%d" ),session->hostname, session->current_port);
		close_fail;

	}

/*
	if (connect(session->sock, &haddr.sa, ha_len) == -1)
	{
		if (socket_errno() == SE_EWOULDBLOCK
#if defined(SE_EINPROGRESS)
			|| socket_errno() == SE_EINPROGRESS
#endif
						   ) {
			trace_dsn("Connection pending.\n");
			*pending = True;
#if !defined(_WIN32)
			output_id = AddOutput(session->sock, session, output_possible);
#endif
		} else {
			popup_a_sockerr(session, N_( "Can't connect to %s:%d" ),session->hostname, session->current_port);
			close_fail;
		}
	} else {
		if (non_blocking(False) < 0)
			close_fail;
		net_connected(session);
	}
	*/

	/* set up temporary termtype */
	if (appres.termname == CN && session->std_ds_host) {
		(void) sprintf(ttype_tmpval, "IBM-327%c-%d",
		    appres.m3279 ? '9' : '8', session->model_num);
		session->termtype = ttype_tmpval;
	}

	/* all done */
#if defined(_WIN32) /*[*/
	if (session->sock_handle == NULL) {
		char ename[256];

		sprintf(ename, "wc3270-%d", getpid());

		session->sock_handle = CreateEvent(NULL, TRUE, FALSE, ename);
		if (session->sock_handle == NULL)
		{
			lib3270_popup_dialog(	session,
									LIB3270_NOTIFY_CRITICAL,
									N_( "Network startup error" ),
									N_( "Cannot create socket handle" ),
									"%s", win32_strerror(GetLastError()) );

			_exit(1);
		}
	}

	if (WSAEventSelect(session->sock, session->sock_handle, FD_READ | FD_CONNECT | FD_CLOSE) != 0)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_CRITICAL,
								N_( "Network startup error" ),
								N_( "WSAEventSelect failed" ),
								"%s", win32_strerror(GetLastError()) );
		_exit(1);
	}

	return (int) session->sock_handle;
#else /*][*/
	return session->sock;
#endif /*]*/
}
#undef close_fail

/* Set up the LU list. */
static void
setup_lus(void)
{
	char *lu;
	char *comma;
	int n_lus = 1;
	int i;

	h3270.connected_lu = CN;
	h3270.connected_type = CN;

	if (!h3270.luname[0]) {
		Replace(lus, NULL);
		curr_lu = (char **)NULL;
		try_lu = CN;
		return;
	}

	/*
	 * Count the commas in the LU name.  That plus one is the
	 * number of LUs to try.
	 */
	lu = h3270.luname;
	while ((comma = strchr(lu, ',')) != CN) {
		n_lus++;
		lu++;
	}

	/*
	 * Allocate enough memory to construct an argv[] array for
	 * the LUs.
	 */
	Replace(lus,
	    (char **)Malloc((n_lus+1) * sizeof(char *) + strlen(h3270.luname) + 1));

	/* Copy each LU into the array. */
	lu = (char *)(lus + n_lus + 1);
	(void) strcpy(lu, h3270.luname);
	i = 0;
	do {
		lus[i++] = lu;
		comma = strchr(lu, ',');
		if (comma != CN) {
			*comma = '\0';
			lu = comma + 1;
		}
	} while (comma != CN);
	lus[i] = CN;
	curr_lu = lus;
	try_lu = *curr_lu;
}

static void net_connected(H3270 *session)
{
	if (proxy_type > 0)
	{
		/* Negotiate with the proxy. */
		trace_dsn("Connected to proxy server %s, port %u.\n",proxy_host, proxy_port);

		if (proxy_negotiate(proxy_type, session->sock, session->hostname,session->current_port) < 0)
		{
			host_disconnect(session,True);
			return;
		}
	}

	trace_dsn("Connected to %s, port %u%s.\n", session->hostname, session->current_port,session->ssl_host? " via SSL": "");

#if defined(HAVE_LIBSSL) /*[*/
	/* Set up SSL. */
	if(session->ssl_host && !session->secure_connection)
	{
		if (SSL_set_fd(ssl_con, session->sock) != 1)
		{
			trace_dsn("Can't set fd!\n");
		}
		if (SSL_connect(ssl_con) != 1)
		{
			/*
			 * No need to trace the error, it was already
			 * displayed.
			 */
			host_disconnect(session,True);
			return;
		}
		session->secure_connection = True;
		trace_dsn("TLS/SSL tunneled connection complete. Connection is now secure.\n");

		/* Tell everyone else again. */
		host_connected(session);
	}
#endif /*]*/

	/* set up telnet options */
	(void) memset((char *) myopts, 0, sizeof(myopts));
	(void) memset((char *) hisopts, 0, sizeof(hisopts));

#if defined(X3270_TN3270E) /*[*/
	e_funcs = E_OPT(TN3270E_FUNC_BIND_IMAGE) |
		  E_OPT(TN3270E_FUNC_RESPONSES) |
		  E_OPT(TN3270E_FUNC_SYSREQ);
	e_xmit_seq = 0;
	response_required = TN3270E_RSF_NO_RESPONSE;
#endif /*]*/

#if defined(HAVE_LIBSSL) /*[*/
	need_tls_follows = False;
#endif /*]*/
	telnet_state = TNS_DATA;
	ibptr = ibuf;

	/* clear statistics and flags */
	(void) time(&ns_time);
	ns_brcvd = 0;
	ns_rrcvd = 0;
	ns_bsent = 0;
	ns_rsent = 0;
	syncing = 0;
	tn3270e_negotiated = 0;
	tn3270e_submode = E_NONE;
	tn3270e_bound = 0;

	setup_lus();

	check_linemode(True);

	/* write out the passthru hostname and port nubmer */
	if (session->passthru_host)
	{
		char *buf;

		buf = Malloc(strlen(session->hostname) + 32);
		(void) sprintf(buf, "%s %d\r\n", session->hostname, session->current_port);
		(void) send(session->sock, buf, strlen(buf), 0);
		Free(buf);
	}
}

/*
 * connection_complete
 *	The connection appears to be complete (output is possible or input
 *	appeared ready but recv() returned EWOULDBLOCK).  Complete the
 *	connection-completion processing.
 */
static void
connection_complete(void)
{
#if !defined(_WIN32) /*[*/
	if (non_blocking(False) < 0) {
		host_disconnect(&h3270,True);
		return;
	}
#endif /*]*/
	host_connected(&h3270);
	net_connected(&h3270);
}

#if !defined(_WIN32) /*[*/
/*
 * output_possible
 *	Output is possible on the socket.  Used only when a connection is
 *	pending, to determine that the connection is complete.
 */
static void output_possible(H3270 *session)
{
	trace("%s: %s",__FUNCTION__,HALF_CONNECTED ? "Half connected" : "Connected");
	if (HALF_CONNECTED)
	{
		connection_complete();
	}

	if (output_id)
	{
		RemoveInput(output_id);
		output_id = 0L;
	}
}
#endif /*]*/

/*
 * net_disconnect
 *	Shut down the socket.
 */
void
net_disconnect(void)
{
#if defined(HAVE_LIBSSL) /*[*/
	if (ssl_con != NULL) {
		SSL_shutdown(ssl_con);
		SSL_free(ssl_con);
		ssl_con = NULL;
	}
	h3270.secure_connection = False;
#endif /*]*/
	if (CONNECTED)
		(void) shutdown(h3270.sock, 2);
	(void) SOCK_CLOSE(h3270.sock);
	h3270.sock = -1;
	trace_dsn("SENT disconnect\n");

	/* Restore terminal type to its default. */
	if (appres.termname == CN)
		h3270.termtype = h3270.full_model_name;

	/* We're not connected to an LU any more. */
	h3270.connected_lu = CN;
	status_lu(&h3270,CN);

#if !defined(_WIN32) /*[*/
	/* We have no more interest in output buffer space. */
	if (output_id != 0L)
	{
		RemoveInput(output_id);
		output_id = 0L;
	}
#endif /*]*/
}


/*
 * net_input
 *	Called by the toolkit whenever there is input available on the
 *	socket.  Reads the data, processes the special telnet commands
 *	and calls process_ds to process the 3270 data stream.
 */
void net_input(H3270 *session)
{
	register unsigned char	*cp;
	int	nr;

	if(!session)
		session = &h3270;

#if defined(_WIN32) /*[*/
	for (;;)
#endif /*]*/
	{
		if (session->sock < 0)
			return;

#if defined(_WIN32) /*[*/
		if (HALF_CONNECTED) {

			if (connect(session->sock, &haddr.sa, sizeof(haddr)) < 0) {
				int err = GetLastError();

				switch (err) {
				case WSAEISCONN:
					connection_complete();
					/* and go get data...? */
					break;
				case WSAEALREADY:
				case WSAEWOULDBLOCK:
				case WSAEINVAL:
					return;
				default:
					lib3270_popup_dialog(	&h3270,
											LIB3270_NOTIFY_CRITICAL,
											N_( "Network startup error" ),
											N_( "Second connect() failed" ),
											"%s", win32_strerror(GetLastError()) );
					_exit(1);
				}
			}
		}
#endif /*]*/

#if defined(X3270_ANSI) /*[*/
		ansi_data = 0;
#endif /*]*/

#if defined(_WIN32) /*[*/
		(void) ResetEvent(session->sock_handle);
#endif /*]*/

#if defined(HAVE_LIBSSL)
		if (ssl_con != NULL)
			nr = SSL_read(ssl_con, (char *) netrbuf, BUFSZ);
		else
#endif // HAVE_LIBSSL
/*
#if defined(LOCAL_PROCESS)
		if (local_process)
		    	nr = read(sock, (char *) netrbuf, BUFSZ);
		else
#endif
*/
			nr = recv(session->sock, (char *) netrbuf, BUFSZ, 0);
		if (nr < 0) {
			if (socket_errno() == SE_EWOULDBLOCK) {
				return;
			}
#if defined(HAVE_LIBSSL) /*[*/
			if (ssl_con != NULL) {
				unsigned long e;
				char err_buf[120];

				e = ERR_get_error();
				if (e != 0)
					(void) ERR_error_string(e, err_buf);
				else
					strcpy(err_buf, "unknown error");
				trace_dsn("RCVD SSL_read error %ld (%s)\n", e,
				    err_buf);

				popup_an_error(NULL,"SSL_read:\n%s", err_buf);
				host_disconnect(session,True);
				return;
			}
#endif /*]*/
			if (HALF_CONNECTED && socket_errno() == SE_EAGAIN) {
				connection_complete();
				return;
			}
/*
#if defined(LOCAL_PROCESS) /
			if (errno == EIO && local_process) {
				trace_dsn("RCVD local process disconnect\n");
				host_disconnect(session,False);
				return;
			}
#endif
*/
			trace_dsn("RCVD socket error %d\n", errno);
			if (HALF_CONNECTED) {
				popup_a_sockerr(NULL, N_( "%s:%d" ),h3270.hostname, h3270.current_port);
			} else if (socket_errno() != SE_ECONNRESET) {
				popup_a_sockerr(NULL, N_( "Socket read error" ) );
			}
			host_disconnect(session,True);
			return;
		} else if (nr == 0) {
			/* Host disconnected. */
			trace_dsn("RCVD disconnect\n");
			host_disconnect(session,False);
			return;
		}

		/* Process the data. */

		if (HALF_CONNECTED)
		{
			if (non_blocking(False) < 0)
			{
				host_disconnect(session,True);
				return;
			}
			host_connected(session);
			net_connected(session);
		}

#if defined(X3270_TRACE) /*[*/
		trace_netdata('<', netrbuf, nr);
#endif /*]*/

		ns_brcvd += nr;
		for (cp = netrbuf; cp < (netrbuf + nr); cp++) {
/*
#if defined(LOCAL_PROCESS)
			if (local_process) {
				// More to do here, probably.
				if (IN_NEITHER) {	// now can assume ANSI mode
					host_in3270(CONNECTED_ANSI);
					hisopts[TELOPT_ECHO] = 1;
					check_linemode(False);
					kybdlock_clr(KL_AWAITING_FIRST, "telnet_fsm");
					status_reset();
					ps_process();
				}
				ansi_process((unsigned int) *cp);
			} else {
#endif
*/
				if (telnet_fsm(*cp)) {
					(void) ctlr_dbcs_postprocess();
					host_disconnect(&h3270,True);
					return;
				}
/*
#if defined(LOCAL_PROCESS)
			}
#endif
*/
		}

#if defined(X3270_ANSI) /*[*/
		if (IN_ANSI) {
			(void) ctlr_dbcs_postprocess();
		}
		if (ansi_data) {
			trace_dsn("\n");
			ansi_data = 0;
		}
#endif /*]*/

#if defined(X3270_TRACE) /*[*/
		/* See if it's time to roll over the trace file. */
		trace_rollover_check();
#endif /*]*/

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
static void
send_naws(void)
{
	char naws_msg[14];
	int naws_len = 0;

	(void) sprintf(naws_msg, "%c%c%c", IAC, SB, TELOPT_NAWS);
	naws_len += 3;
	naws_len += set16(naws_msg + naws_len, XMIT_COLS);
	naws_len += set16(naws_msg + naws_len, XMIT_ROWS);
	(void) sprintf(naws_msg + naws_len, "%c%c", IAC, SE);
	naws_len += 2;
	net_rawout((unsigned char *)naws_msg, naws_len);
	trace_dsn("SENT %s NAWS %d %d %s\n", cmd(SB), XMIT_COLS, XMIT_ROWS, cmd(SE));
}



/* Advance 'try_lu' to the next desired LU name. */
static void
next_lu(void)
{
	if (curr_lu != (char **)NULL && (try_lu = *++curr_lu) == CN)
		curr_lu = (char **)NULL;
}

/*
 * telnet_fsm
 *	Telnet finite-state machine.
 *	Returns 0 for okay, -1 for errors.
 */
static int
telnet_fsm(unsigned char c)
{
#if defined(X3270_ANSI) /*[*/
	char	*see_chr;
	int	sl;
#endif /*]*/

	switch (telnet_state) {
	    case TNS_DATA:	/* normal data processing */
		if (c == IAC) {	/* got a telnet command */
			telnet_state = TNS_IAC;
#if defined(X3270_ANSI) /*[*/
			if (ansi_data) {
				trace_dsn("\n");
				ansi_data = 0;
			}
#endif /*]*/
			break;
		}
		if (IN_NEITHER) {	/* now can assume ANSI mode */
#if defined(X3270_ANSI)/*[*/
			if (linemode)
				cooked_init();
#endif /*]*/
			host_in3270(&h3270,CONNECTED_ANSI);
			kybdlock_clr(KL_AWAITING_FIRST, "telnet_fsm");
			status_reset(NULL);
			ps_process();
		}
		if (IN_ANSI && !IN_E) {
#if defined(X3270_ANSI) /*[*/
			if (!ansi_data) {
				trace_dsn("<.. ");
				ansi_data = 4;
			}
			see_chr = ctl_see((int) c);
			ansi_data += (sl = strlen(see_chr));
			if (ansi_data >= TRACELINE) {
				trace_dsn(" ...\n... ");
				ansi_data = 4 + sl;
			}
			trace_dsn("%s",see_chr);
			if (!syncing) {
				if (linemode && appres.onlcr && c == '\n')
					ansi_process((unsigned int) '\r');
				ansi_process((unsigned int) c);
//				sms_store(c);
			}
#endif /*]*/
		} else {
			store3270in(c);
		}
		break;
	    case TNS_IAC:	/* process a telnet command */
		if (c != EOR && c != IAC) {
			trace_dsn("RCVD %s ", cmd(c));
		}
		switch (c) {
		    case IAC:	/* escaped IAC, insert it */
			if (IN_ANSI && !IN_E) {
#if defined(X3270_ANSI) /*[*/
				if (!ansi_data) {
					trace_dsn("<.. ");
					ansi_data = 4;
				}
				see_chr = ctl_see((int) c);
				ansi_data += (sl = strlen(see_chr));
				if (ansi_data >= TRACELINE) {
					trace_dsn(" ...\n ...");
					ansi_data = 4 + sl;
				}
				trace_dsn("%s",see_chr);
				ansi_process((unsigned int) c);
//				sms_store(c);
#endif /*]*/
			} else
				store3270in(c);
			telnet_state = TNS_DATA;
			break;
		    case EOR:	/* eor, process accumulated input */
			if (IN_3270 || (IN_E && tn3270e_negotiated)) {
				ns_rrcvd++;
				if (process_eor())
					return -1;
			} else
				Warning(NULL, _( "EOR received when not in 3270 mode, ignored." ));
			trace_dsn("RCVD EOR\n");
			ibptr = ibuf;
			telnet_state = TNS_DATA;
			break;
		    case WILL:
			telnet_state = TNS_WILL;
			break;
		    case WONT:
			telnet_state = TNS_WONT;
			break;
		    case DO:
			telnet_state = TNS_DO;
			break;
		    case DONT:
			telnet_state = TNS_DONT;
			break;
		    case SB:
			telnet_state = TNS_SB;
			if (sbbuf == (unsigned char *)NULL)
				sbbuf = (unsigned char *)Malloc(1024);
			sbptr = sbbuf;
			break;
		    case DM:
			trace_dsn("\n");
			if (syncing) {
				syncing = 0;
				x_except_on(&h3270,h3270.sock);
			}
			telnet_state = TNS_DATA;
			break;
		    case GA:
		    case NOP:
			trace_dsn("\n");
			telnet_state = TNS_DATA;
			break;
		    default:
			trace_dsn("???\n");
			telnet_state = TNS_DATA;
			break;
		}
		break;
	    case TNS_WILL:	/* telnet WILL DO OPTION command */
		trace_dsn("%s\n", opt(c));
		switch (c) {
		    case TELOPT_SGA:
		    case TELOPT_BINARY:
		    case TELOPT_EOR:
		    case TELOPT_TTYPE:
		    case TELOPT_ECHO:
#if defined(X3270_TN3270E) /*[*/
		    case TELOPT_TN3270E:
#endif /*]*/
			if (c != TELOPT_TN3270E || !h3270.non_tn3270e_host) {
				if (!hisopts[c]) {
					hisopts[c] = 1;
					do_opt[2] = c;
					net_rawout(do_opt, sizeof(do_opt));
					trace_dsn("SENT %s %s\n",
						cmd(DO), opt(c));

					/*
					 * For UTS, volunteer to do EOR when
					 * they do.
					 */
					if (c == TELOPT_EOR && !myopts[c]) {
						myopts[c] = 1;
						will_opt[2] = c;
						net_rawout(will_opt,
							sizeof(will_opt));
						trace_dsn("SENT %s %s\n",
							cmd(WILL), opt(c));
					}

					check_in3270();
					check_linemode(False);
				}
				break;
			}
		    default:
			dont_opt[2] = c;
			net_rawout(dont_opt, sizeof(dont_opt));
			trace_dsn("SENT %s %s\n", cmd(DONT), opt(c));
			break;
		}
		telnet_state = TNS_DATA;
		break;
	    case TNS_WONT:	/* telnet WONT DO OPTION command */
		trace_dsn("%s\n", opt(c));
		if (hisopts[c]) {
			hisopts[c] = 0;
			dont_opt[2] = c;
			net_rawout(dont_opt, sizeof(dont_opt));
			trace_dsn("SENT %s %s\n", cmd(DONT), opt(c));
			check_in3270();
			check_linemode(False);
		}
		telnet_state = TNS_DATA;
		break;
	    case TNS_DO:	/* telnet PLEASE DO OPTION command */
		trace_dsn("%s\n", opt(c));
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
			if (c == TELOPT_TN3270E && h3270.non_tn3270e_host)
				goto wont;
			if (c == TELOPT_TM && !appres.bsd_tm)
				goto wont;

			if (!myopts[c]) {
				if (c != TELOPT_TM)
					myopts[c] = 1;
				will_opt[2] = c;
				net_rawout(will_opt, sizeof(will_opt));
				trace_dsn("SENT %s %s\n", cmd(WILL),
					opt(c));
				check_in3270();
				check_linemode(False);
			}
			if (c == TELOPT_NAWS)
				send_naws();
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
				net_rawout(follows_msg,
						sizeof(follows_msg));
				trace_dsn("SENT %s %s FOLLOWS %s\n",
						cmd(SB),
						opt(TELOPT_STARTTLS),
						cmd(SE));
				need_tls_follows = True;
			}
#endif /*]*/
			break;
		    default:
		    wont:
			wont_opt[2] = c;
			net_rawout(wont_opt, sizeof(wont_opt));
			trace_dsn("SENT %s %s\n", cmd(WONT), opt(c));
			break;
		}
		telnet_state = TNS_DATA;
		break;
	    case TNS_DONT:	/* telnet PLEASE DON'T DO OPTION command */
		trace_dsn("%s\n", opt(c));
		if (myopts[c]) {
			myopts[c] = 0;
			wont_opt[2] = c;
			net_rawout(wont_opt, sizeof(wont_opt));
			trace_dsn("SENT %s %s\n", cmd(WONT), opt(c));
			check_in3270();
			check_linemode(False);
		}
		telnet_state = TNS_DATA;
		break;
	    case TNS_SB:	/* telnet sub-option string command */
		if (c == IAC)
			telnet_state = TNS_SB_IAC;
		else
			*sbptr++ = c;
		break;
	    case TNS_SB_IAC:	/* telnet sub-option string command */
		*sbptr++ = c;
		if (c == SE) {
			telnet_state = TNS_DATA;
			if (sbbuf[0] == TELOPT_TTYPE &&
			    sbbuf[1] == TELQUAL_SEND) {
				int tt_len, tb_len;
				char *tt_out;

				trace_dsn("%s %s\n", opt(sbbuf[0]),
				    telquals[sbbuf[1]]);
				if (lus != (char **)NULL && try_lu == CN) {
					/* None of the LUs worked. */
					popup_an_error(NULL,"Cannot connect to specified LU");
					return -1;
				}

				tt_len = strlen(h3270.termtype);
				if (try_lu != CN && *try_lu) {
					tt_len += strlen(try_lu) + 1;
					h3270.connected_lu = try_lu;
				} else
					h3270.connected_lu = CN;
				status_lu(&h3270,h3270.connected_lu);

				tb_len = 4 + tt_len + 2;
				tt_out = Malloc(tb_len + 1);
				(void) sprintf(tt_out, "%c%c%c%c%s%s%s%c%c",
				    IAC, SB, TELOPT_TTYPE, TELQUAL_IS,
				    h3270.termtype,
				    (try_lu != CN && *try_lu) ? "@" : "",
				    (try_lu != CN && *try_lu) ? try_lu : "",
				    IAC, SE);
				net_rawout((unsigned char *)tt_out, tb_len);

				trace_dsn("SENT %s %s %s %.*s %s\n",
				    cmd(SB), opt(TELOPT_TTYPE),
				    telquals[TELQUAL_IS],
				    tt_len, tt_out + 4,
				    cmd(SE));
				Free(tt_out);

				/* Advance to the next LU name. */
				next_lu();
			}
#if defined(X3270_TN3270E) /*[*/
			else if (myopts[TELOPT_TN3270E] &&
				   sbbuf[0] == TELOPT_TN3270E) {
				if (tn3270e_negotiate())
					return -1;
			}
#endif /*]*/
#if defined(HAVE_LIBSSL) /*[*/
			else if (need_tls_follows &&
				   myopts[TELOPT_STARTTLS] &&
				   sbbuf[0] == TELOPT_STARTTLS) {
				continue_tls(sbbuf, sbptr - sbbuf);
			}
#endif /*]*/

		} else {
			telnet_state = TNS_SB;
		}
		break;
	}
	return 0;
}

#if defined(X3270_TN3270E) /*[*/
/* Send a TN3270E terminal type request. */
static void
tn3270e_request(void)
{
	int tt_len, tb_len;
	char *tt_out;
	char *t;

	tt_len = strlen(h3270.termtype);
	if (try_lu != CN && *try_lu)
		tt_len += strlen(try_lu) + 1;

	tb_len = 5 + tt_len + 2;
	tt_out = Malloc(tb_len + 1);
	t = tt_out;
	t += sprintf(tt_out, "%c%c%c%c%c%s",
	    IAC, SB, TELOPT_TN3270E, TN3270E_OP_DEVICE_TYPE,
	    TN3270E_OP_REQUEST, h3270.termtype);

	/* Convert 3279 to 3278, per the RFC. */
	if (tt_out[12] == '9')
		tt_out[12] = '8';

	if (try_lu != CN && *try_lu)
		t += sprintf(t, "%c%s", TN3270E_OP_CONNECT, try_lu);

	(void) sprintf(t, "%c%c", IAC, SE);

	net_rawout((unsigned char *)tt_out, tb_len);

	trace_dsn("SENT %s %s DEVICE-TYPE REQUEST %.*s%s%s %s\n",
	    cmd(SB), opt(TELOPT_TN3270E), (int) strlen(h3270.termtype), tt_out + 5,
	    (try_lu != CN && *try_lu) ? " CONNECT " : "",
	    (try_lu != CN && *try_lu) ? try_lu : "",
	    cmd(SE));

	Free(tt_out);
}

/*
 * Back off of TN3270E.
 */
static void
backoff_tn3270e(const char *why)
{
	trace_dsn("Aborting TN3270E: %s\n", why);

	/* Tell the host 'no'. */
	wont_opt[2] = TELOPT_TN3270E;
	net_rawout(wont_opt, sizeof(wont_opt));
	trace_dsn("SENT %s %s\n", cmd(WONT), opt(TELOPT_TN3270E));

	/* Restore the LU list; we may need to run it again in TN3270 mode. */
	setup_lus();

	/* Reset our internal state. */
	myopts[TELOPT_TN3270E] = 0;
	check_in3270();
}

/*
 * Negotiation of TN3270E options.
 * Returns 0 if okay, -1 if we have to give up altogether.
 */
static int
tn3270e_negotiate(void)
{
#define LU_MAX	32
	static char reported_lu[LU_MAX+1];
	static char reported_type[LU_MAX+1];
	int sblen;
	unsigned long e_rcvd;

	/* Find out how long the subnegotiation buffer is. */
	for (sblen = 0; ; sblen++) {
		if (sbbuf[sblen] == SE)
			break;
	}

	trace_dsn("TN3270E ");

	switch (sbbuf[1]) {

	case TN3270E_OP_SEND:

		if (sbbuf[2] == TN3270E_OP_DEVICE_TYPE) {

			/* Host wants us to send our device type. */
			trace_dsn("SEND DEVICE-TYPE SE\n");

			tn3270e_request();
		} else {
			trace_dsn("SEND ??%u SE\n", sbbuf[2]);
		}
		break;

	case TN3270E_OP_DEVICE_TYPE:

		/* Device type negotiation. */
		trace_dsn("DEVICE-TYPE ");

		switch (sbbuf[2]) {
		case TN3270E_OP_IS: {
			int tnlen, snlen;

			/* Device type success. */

			/* Isolate the terminal type and session. */
			tnlen = 0;
			while (sbbuf[3+tnlen] != SE &&
			       sbbuf[3+tnlen] != TN3270E_OP_CONNECT)
				tnlen++;
			snlen = 0;
			if (sbbuf[3+tnlen] == TN3270E_OP_CONNECT) {
				while(sbbuf[3+tnlen+1+snlen] != SE)
					snlen++;
			}
			trace_dsn("IS %.*s CONNECT %.*s SE\n",
				tnlen, &sbbuf[3],
				snlen, &sbbuf[3+tnlen+1]);

			/* Remember the LU. */
			if (tnlen) {
				if (tnlen > LU_MAX)
					tnlen = LU_MAX;
				(void)strncpy(reported_type,
				    (char *)&sbbuf[3], tnlen);
				reported_type[tnlen] = '\0';
				h3270.connected_type = reported_type;
			}
			if (snlen) {
				if (snlen > LU_MAX)
					snlen = LU_MAX;
				(void)strncpy(reported_lu,
				    (char *)&sbbuf[3+tnlen+1], snlen);
				reported_lu[snlen] = '\0';
				h3270.connected_lu = reported_lu;
				status_lu(&h3270,h3270.connected_lu);
			}

			/* Tell them what we can do. */
			tn3270e_subneg_send(TN3270E_OP_REQUEST, e_funcs);
			break;
		}
		case TN3270E_OP_REJECT:

			/* Device type failure. */

			trace_dsn("REJECT REASON %s SE\n", rsn(sbbuf[4]));
			if (sbbuf[4] == TN3270E_REASON_INV_DEVICE_TYPE ||
			    sbbuf[4] == TN3270E_REASON_UNSUPPORTED_REQ) {
				backoff_tn3270e("Host rejected device type or "
				    "request type");
				break;
			}

			next_lu();
			if (try_lu != CN) {
				/* Try the next LU. */
				tn3270e_request();
			} else if (lus != (char **)NULL) {
				/* No more LUs to try.  Give up. */
				backoff_tn3270e("Host rejected resource(s)");
			} else {
				backoff_tn3270e("Device type rejected");
			}

			break;
		default:
			trace_dsn("??%u SE\n", sbbuf[2]);
			break;
		}
		break;

	case TN3270E_OP_FUNCTIONS:

		/* Functions negotiation. */
		trace_dsn("FUNCTIONS ");

		switch (sbbuf[2]) {

		case TN3270E_OP_REQUEST:

			/* Host is telling us what functions they want. */
			trace_dsn("REQUEST %s SE\n",
			    tn3270e_function_names(sbbuf+3, sblen-3));

			e_rcvd = tn3270e_fdecode(sbbuf+3, sblen-3);
			if ((e_rcvd == e_funcs) || (e_funcs & ~e_rcvd)) {
				/* They want what we want, or less.  Done. */
				e_funcs = e_rcvd;
				tn3270e_subneg_send(TN3270E_OP_IS, e_funcs);
				tn3270e_negotiated = 1;
				trace_dsn("TN3270E option negotiation complete.\n");
				check_in3270();
			} else {
				/*
				 * They want us to do something we can't.
				 * Request the common subset.
				 */
				e_funcs &= e_rcvd;
				tn3270e_subneg_send(TN3270E_OP_REQUEST,e_funcs);
			}
			break;

		case TN3270E_OP_IS:

			/* They accept our last request, or a subset thereof. */
			trace_dsn("IS %s SE\n",
			    tn3270e_function_names(sbbuf+3, sblen-3));
			e_rcvd = tn3270e_fdecode(sbbuf+3, sblen-3);
			if (e_rcvd != e_funcs) {
				if (e_funcs & ~e_rcvd) {
					/*
					 * They've removed something.  This is
					 * technically illegal, but we can
					 * live with it.
					 */
					e_funcs = e_rcvd;
				} else {
					/*
					 * They've added something.  Abandon
					 * TN3270E, they're brain dead.
					 */
					backoff_tn3270e("Host illegally added "
					    "function(s)");
					break;
				}
			}
			tn3270e_negotiated = 1;
			trace_dsn("TN3270E option negotiation complete.\n");
			check_in3270();
			break;

		default:
			trace_dsn("??%u SE\n", sbbuf[2]);
			break;
		}
		break;

	default:
		trace_dsn("??%u SE\n", sbbuf[1]);
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

/* Expand the current TN3270E function codes into text. */
const char *
tn3270e_current_opts(void)
{
	int i;
	static char text_buf[1024];
	char *s = text_buf;

	if (!e_funcs || !IN_E)
		return CN;
	for (i = 0; i < 32; i++) {
		if (e_funcs & E_OPT(i))
		s += sprintf(s, "%s%s", (s == text_buf) ? "" : " ",
		    fnn(i));
	}
	return text_buf;
}

/* Transmit a TN3270E FUNCTIONS REQUEST or FUNCTIONS IS message. */
static void
tn3270e_subneg_send(unsigned char op, unsigned long funcs)
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
	net_rawout(proto_buf, proto_len);

	/* Complete and send out the trace text. */
	trace_dsn("SENT %s %s FUNCTIONS %s %s %s\n",
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
static void
process_bind(unsigned char *buf, int buflen)
{
	int namelen, i;

	(void) memset(plu_name, '\0', sizeof(plu_name));

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
		plu_name[i] = ebc2asc0[buf[BIND_OFF_PLU_NAME + i]];
	}
}
#endif /*]*/

static int
process_eor(void)
{
	if (syncing || !(ibptr - ibuf))
		return(0);

#if defined(X3270_TN3270E) /*[*/
	if (IN_E) {
		tn3270e_header *h = (tn3270e_header *)ibuf;
		unsigned char *s;
		enum pds rv;

		trace_dsn("RCVD TN3270E(%s%s %s %u)\n",
		    e_dt(h->data_type),
		    e_rq(h->data_type, h->request_flag),
		    e_rsp(h->data_type, h->response_flag),
		    h->seq_number[0] << 8 | h->seq_number[1]);

		switch (h->data_type) {
		case TN3270E_DT_3270_DATA:
			if ((e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)) &&
			    !tn3270e_bound)
				return 0;
			tn3270e_submode = E_3270;
			check_in3270();
			response_required = h->response_flag;
			rv = process_ds(ibuf + EH_SIZE,
			    (ibptr - ibuf) - EH_SIZE);
			if (rv < 0 &&
			    response_required != TN3270E_RSF_NO_RESPONSE)
				tn3270e_nak(rv);
			else if (rv == PDS_OKAY_NO_OUTPUT &&
			    response_required == TN3270E_RSF_ALWAYS_RESPONSE)
				tn3270e_ack();
			response_required = TN3270E_RSF_NO_RESPONSE;
			return 0;
		case TN3270E_DT_BIND_IMAGE:
			if (!(e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
				return 0;
			process_bind(ibuf + EH_SIZE, (ibptr - ibuf) - EH_SIZE);
			trace_dsn("< BIND PLU-name '%s'\n", plu_name);
			tn3270e_bound = 1;
			check_in3270();
			return 0;
		case TN3270E_DT_UNBIND:
			if (!(e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
				return 0;
			tn3270e_bound = 0;
			if (tn3270e_submode == E_3270)
				tn3270e_submode = E_NONE;
			check_in3270();
			return 0;
		case TN3270E_DT_NVT_DATA:
			/* In tn3270e NVT mode */
			tn3270e_submode = E_NVT;
			check_in3270();
			for (s = ibuf; s < ibptr; s++) {
				ansi_process(*s++);
			}
			return 0;
		case TN3270E_DT_SSCP_LU_DATA:
			if (!(e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
				return 0;
			tn3270e_submode = E_SSCP;
			check_in3270();
			ctlr_write_sscp_lu(ibuf + EH_SIZE,
			                   (ibptr - ibuf) - EH_SIZE);
			return 0;
		default:
			/* Should do something more extraordinary here. */
			return 0;
		}
	} else
#endif /*]*/
	{
		(void) process_ds(ibuf, ibptr - ibuf);
	}
	return 0;
}


/*
 * net_exception
 *	Called when there is an exceptional condition on the socket.
 */
void net_exception(H3270 *session)
{
/*
#if defined(LOCAL_PROCESS)
	if (local_process) {
		trace_dsn("RCVD exception\n");
	} else
#endif
*/
	{
		trace_dsn("RCVD urgent data indication\n");
		if (!syncing) {
			syncing = 1;
			x_except_off(session);
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


/*
 * net_rawout
 *	Send out raw telnet data.  We assume that there will always be enough
 *	space to buffer what we want to transmit, so we don't handle EAGAIN or
 *	EWOULDBLOCK.
 */
static void
net_rawout(unsigned const char *buf, int len)
{
	int	nw;

#if defined(X3270_TRACE) /*[*/
	trace_netdata('>', buf, len);
#endif /*]*/

	while (len) {
#if defined(OMTU) /*[*/
		int n2w = len;
		int pause = 0;

		if (n2w > OMTU) {
			n2w = OMTU;
			pause = 1;
		}
#else
#		define n2w len
#endif
#if defined(HAVE_LIBSSL) /*[*/
		if (ssl_con != NULL)
			nw = SSL_write(ssl_con, (const char *) buf, n2w);
		else
#endif /*]*/

/*
#if defined(LOCAL_PROCESS)
		if (local_process)
			nw = write(sock, (const char *) buf, n2w);
		else
#endif
*/
			nw = send(h3270.sock, (const char *) buf, n2w, 0);
		if (nw < 0) {
#if defined(HAVE_LIBSSL) /*[*/
			if (ssl_con != NULL) {
				unsigned long e;
				char err_buf[120];

				e = ERR_get_error();
				(void) ERR_error_string(e, err_buf);
				trace_dsn("RCVD SSL_write error %ld (%s)\n", e,err_buf);
				popup_an_error(NULL,"SSL_write:\n%s", err_buf);
				host_disconnect(&h3270,False);
				return;
			}
#endif /*]*/
			trace_dsn("RCVD socket error %d\n", errno);
			if (socket_errno() == SE_EPIPE || socket_errno() == SE_ECONNRESET) {
				host_disconnect(&h3270,False);
				return;
			} else if (socket_errno() == SE_EINTR) {
				goto bot;
			} else {
				popup_a_sockerr(NULL, N_( "Socket write error" ) );
				host_disconnect(&h3270,True);
				return;
			}
		}
		ns_bsent += nw;
		len -= nw;
		buf += nw;
	    bot:
#if defined(OMTU) /*[*/
		if (pause)
			sleep(1);
#endif /*]*/
		;
	}
}


#if defined(X3270_ANSI) /*[*/
/*
 * net_hexansi_out
 *	Send uncontrolled user data to the host in ANSI mode, performing IAC
 *	and CR quoting as necessary.
 */
void
net_hexansi_out(unsigned char *buf, int len)
{
	unsigned char *tbuf;
	unsigned char *xbuf;

	if (!len)
		return;

#if defined(X3270_TRACE) /*[*/
	/* Trace the data. */
	if (toggled(DS_TRACE)) {
		int i;

		trace_dsn(">");
		for (i = 0; i < len; i++)
			trace_dsn(" %s", ctl_see((int) *(buf+i)));
		trace_dsn("\n");
	}
#endif /*]*/

	/* Expand it. */
	tbuf = xbuf = (unsigned char *)Malloc(2*len);
	while (len) {
		unsigned char c = *buf++;

		*tbuf++ = c;
		len--;
		if (c == IAC)
			*tbuf++ = IAC;
		else if (c == '\r' && (!len || *buf != '\n'))
			*tbuf++ = '\0';
	}

	/* Send it to the host. */
	net_rawout(xbuf, tbuf - xbuf);
	Free(xbuf);
}

/*
 * net_cookedout
 *	Send user data out in ANSI mode, without cooked-mode processing.
 */
static void
net_cookedout(const char *buf, int len)
{
#if defined(X3270_TRACE) /*[*/
	if (toggled(DS_TRACE)) {
		int i;

		trace_dsn(">");
		for (i = 0; i < len; i++)
			trace_dsn(" %s", ctl_see((int) *(buf+i)));
		trace_dsn("\n");
	}
#endif /*]*/
	net_rawout((unsigned const char *) buf, len);
}


/*
 * net_cookout
 *	Send output in ANSI mode, including cooked-mode processing if
 *	appropriate.
 */
static void
net_cookout(const char *buf, int len)
{

	if (!IN_ANSI || (kybdlock & KL_AWAITING_FIRST))
		return;

	if (linemode) {
		register int	i;
		char	c;

		for (i = 0; i < len; i++) {
			c = buf[i];

			/* Input conversions. */
			if (!lnext && c == '\r' && appres.icrnl)
				c = '\n';
			else if (!lnext && c == '\n' && appres.inlcr)
				c = '\r';

			/* Backslashes. */
			if (c == '\\' && !backslashed)
				backslashed = 1;
			else
				backslashed = 0;

			/* Control chars. */
			if (c == '\n')
				do_eol(c);
			else if (c == vintr)
				do_intr(c);
			else if (c == vquit)
				do_quit(c);
			else if (c == verase)
				do_cerase(c);
			else if (c == vkill)
				do_kill(c);
			else if (c == vwerase)
				do_werase(c);
			else if (c == vrprnt)
				do_rprnt(c);
			else if (c == veof)
				do_eof(c);
			else if (c == vlnext)
				do_lnext(c);
			else if (c == 0x08 || c == 0x7f) /* Yes, a hack. */
				do_cerase(c);
			else
				do_data(c);
		}
		return;
	} else
		net_cookedout(buf, len);
}


/*
 * Cooked mode input processing.
 */

static void
cooked_init(void)
{
	if (lbuf == (unsigned char *)NULL)
		lbuf = (unsigned char *)Malloc(BUFSZ);
	lbptr = lbuf;
	lnext = 0;
	backslashed = 0;
}

static void
ansi_process_s(const char *data)
{
	while (*data)
		ansi_process((unsigned int) *data++);
}

static void
forward_data(void)
{
	net_cookedout((char *) lbuf, lbptr - lbuf);
	cooked_init();
}

static void
do_data(char c)
{
	if (lbptr+1 < lbuf + BUFSZ) {
		*lbptr++ = c;
		if (c == '\r')
			*lbptr++ = '\0';
		if (c == '\t')
			ansi_process((unsigned int) c);
		else
			ansi_process_s(ctl_see((int) c));
	} else
		ansi_process_s("\007");
	lnext = 0;
	backslashed = 0;
}

static void
do_intr(char c)
{
	if (lnext) {
		do_data(c);
		return;
	}
	ansi_process_s(ctl_see((int) c));
	cooked_init();
	net_interrupt();
}

static void
do_quit(char c)
{
	if (lnext) {
		do_data(c);
		return;
	}
	ansi_process_s(ctl_see((int) c));
	cooked_init();
	net_break();
}

static void
do_cerase(char c)
{
	int len;

	if (backslashed) {
		lbptr--;
		ansi_process_s("\b");
		do_data(c);
		return;
	}
	if (lnext) {
		do_data(c);
		return;
	}
	if (lbptr > lbuf) {
		len = strlen(ctl_see((int) *--lbptr));

		while (len--)
			ansi_process_s("\b \b");
	}
}

static void
do_werase(char c)
{
	int any = 0;
	int len;

	if (lnext) {
		do_data(c);
		return;
	}
	while (lbptr > lbuf) {
		char ch;

		ch = *--lbptr;

		if (ch == ' ' || ch == '\t') {
			if (any) {
				++lbptr;
				break;
			}
		} else
			any = 1;
		len = strlen(ctl_see((int) ch));

		while (len--)
			ansi_process_s("\b \b");
	}
}

static void
do_kill(char c)
{
	int i, len;

	if (backslashed) {
		lbptr--;
		ansi_process_s("\b");
		do_data(c);
		return;
	}
	if (lnext) {
		do_data(c);
		return;
	}
	while (lbptr > lbuf) {
		len = strlen(ctl_see((int) *--lbptr));

		for (i = 0; i < len; i++)
			ansi_process_s("\b \b");
	}
}

static void
do_rprnt(char c)
{
	unsigned char *p;

	if (lnext) {
		do_data(c);
		return;
	}
	ansi_process_s(ctl_see((int) c));
	ansi_process_s("\r\n");
	for (p = lbuf; p < lbptr; p++)
		ansi_process_s(ctl_see((int) *p));
}

static void
do_eof(char c)
{
	if (backslashed) {
		lbptr--;
		ansi_process_s("\b");
		do_data(c);
		return;
	}
	if (lnext) {
		do_data(c);
		return;
	}
	do_data(c);
	forward_data();
}

static void
do_eol(char c)
{
	if (lnext) {
		do_data(c);
		return;
	}
	if (lbptr+2 >= lbuf + BUFSZ) {
		ansi_process_s("\007");
		return;
	}
	*lbptr++ = '\r';
	*lbptr++ = '\n';
	ansi_process_s("\r\n");
	forward_data();
}

static void
do_lnext(char c)
{
	if (lnext) {
		do_data(c);
		return;
	}
	lnext = 1;
	ansi_process_s("^\b");
}
#endif /*]*/



/*
 * check_in3270
 *	Check for switches between NVT, SSCP-LU and 3270 modes.
 */
static void
check_in3270(void)
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
	if (myopts[TELOPT_TN3270E]) {
		if (!tn3270e_negotiated)
			new_cstate = CONNECTED_INITIAL_E;
		else switch (tn3270e_submode) {
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
	if (myopts[TELOPT_BINARY] &&
	           myopts[TELOPT_EOR] &&
	           myopts[TELOPT_TTYPE] &&
	           hisopts[TELOPT_BINARY] &&
	           hisopts[TELOPT_EOR]) {
		new_cstate = CONNECTED_3270;
	} else if (h3270.cstate == CONNECTED_INITIAL) {
		/* Nothing has happened, yet. */
		return;
	} else {
		new_cstate = CONNECTED_ANSI;
	}

	if (new_cstate != h3270.cstate) {
#if defined(X3270_TN3270E) /*[*/
		int was_in_e = IN_E;
#endif /*]*/

#if defined(X3270_TN3270E) /*[*/
		/*
		 * If we've now switched between non-TN3270E mode and
		 * TN3270E mode, reset the LU list so we can try again
		 * in the new mode.
		 */
		if (lus != (char **)NULL && was_in_e != IN_E) {
			curr_lu = lus;
			try_lu = *curr_lu;
		}
#endif /*]*/

		/* Allocate the initial 3270 input buffer. */
		if (new_cstate >= CONNECTED_INITIAL && !ibuf_size) {
			ibuf = (unsigned char *)Malloc(BUFSIZ);
			ibuf_size = BUFSIZ;
			ibptr = ibuf;
		}

#if defined(X3270_ANSI) /*[*/
		/* Reinitialize line mode. */
		if ((new_cstate == CONNECTED_ANSI && linemode) ||
		    new_cstate == CONNECTED_NVT)
			cooked_init();
#endif /*]*/

#if defined(X3270_TN3270E) /*[*/
		/* If we fell out of TN3270E, remove the state. */
		if (!myopts[TELOPT_TN3270E]) {
			tn3270e_negotiated = 0;
			tn3270e_submode = E_NONE;
			tn3270e_bound = 0;
		}
#endif /*]*/
		trace_dsn("Now operating in %s mode.\n",state_name[new_cstate]);
		host_in3270(&h3270,new_cstate);
	}
}

/*
 * store3270in
 *	Store a character in the 3270 input buffer, checking for buffer
 *	overflow and reallocating ibuf if necessary.
 */
static void
store3270in(unsigned char c)
{
	if (ibptr - ibuf >= ibuf_size) {
		ibuf_size += BUFSIZ;
		ibuf = (unsigned char *)Realloc((char *)ibuf, ibuf_size);
		ibptr = ibuf + ibuf_size - BUFSIZ;
	}
	*ibptr++ = c;
}

/*
 * space3270out
 *	Ensure that <n> more characters will fit in the 3270 output buffer.
 *	Allocates the buffer in BUFSIZ chunks.
 *	Allocates hidden space at the front of the buffer for TN3270E.
 */
void
space3270out(int n)
{
	unsigned nc = 0;	/* amount of data currently in obuf */
	unsigned more = 0;

	if (obuf_size)
		nc = obptr - obuf;

	while ((nc + n + EH_SIZE) > (obuf_size + more)) {
		more += BUFSIZ;
	}

	if (more) {
		obuf_size += more;
		obuf_base = (unsigned char *)Realloc((char *)obuf_base,
			obuf_size);
		obuf = obuf_base + EH_SIZE;
		obptr = obuf + nc;
	}
}


/*
 * check_linemode
 *	Set the global variable 'linemode', which says whether we are in
 *	character-by-character mode or line mode.
 */
static void
check_linemode(Boolean init)
{
	int wasline = linemode;

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
	linemode = !hisopts[TELOPT_ECHO] /* && !hisopts[TELOPT_SGA] */;

	if (init || linemode != wasline) {
		st_changed(ST_LINE_MODE, linemode);
		if (!init) {
			trace_dsn("Operating in %s mode.\n",
			    linemode ? "line" : "character-at-a-time");
		}
#if defined(X3270_ANSI) /*[*/
		if (IN_ANSI && linemode)
			cooked_init();
#endif /*]*/
	}
}


#if defined(X3270_TRACE) /*[*/

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
static const char *
cmd(int c)
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

void
trace_netdata(char direction, unsigned const char *buf, int len)
{
	int offset;
	struct timeval ts;
	double tdiff;

	if (!toggled(DS_TRACE))
		return;
	(void) gettimeofday(&ts, (struct timezone *)NULL);
	if (IN_3270) {
		tdiff = ((1.0e6 * (double)(ts.tv_sec - ds_ts.tv_sec)) +
			(double)(ts.tv_usec - ds_ts.tv_usec)) / 1.0e6;
		trace_dsn("%c +%gs\n", direction, tdiff);
	}
	ds_ts = ts;
	for (offset = 0; offset < len; offset++) {
		if (!(offset % LINEDUMP_MAX))
			trace_dsn("%s%c 0x%-3x ",
			    (offset ? "\n" : ""), direction, offset);
		trace_dsn("%02x", buf[offset]);
	}
	trace_dsn("\n");
}
#endif /*]*/


/*
 * net_output
 *	Send 3270 output over the network:
 *	- Prepend TN3270E header
 *	- Expand IAC to IAC IAC
 *	- Append IAC EOR
 */
void
net_output(void)
{
	static unsigned char *xobuf = NULL;
	static int xobuf_len = 0;
	int need_resize = 0;
	unsigned char *nxoptr, *xoptr;

#if defined(X3270_TN3270E) /*[*/
#define BSTART	((IN_TN3270E || IN_SSCP) ? obuf_base : obuf)
#else /*][*/
#define BSTART	obuf
#endif /*]*/

#if defined(X3270_TN3270E) /*[*/
	/* Set the TN3720E header. */
	if (IN_TN3270E || IN_SSCP) {
		tn3270e_header *h = (tn3270e_header *)obuf_base;

		/* Check for sending a TN3270E response. */
		if (response_required == TN3270E_RSF_ALWAYS_RESPONSE) {
			tn3270e_ack();
			response_required = TN3270E_RSF_NO_RESPONSE;
		}

		/* Set the outbound TN3270E header. */
		h->data_type = IN_TN3270E ?
			TN3270E_DT_3270_DATA : TN3270E_DT_SSCP_LU_DATA;
		h->request_flag = 0;
		h->response_flag = 0;
		h->seq_number[0] = (e_xmit_seq >> 8) & 0xff;
		h->seq_number[1] = e_xmit_seq & 0xff;

		trace_dsn("SENT TN3270E(%s NO-RESPONSE %u)\n",
			IN_TN3270E ? "3270-DATA" : "SSCP-LU-DATA", e_xmit_seq);
		if (e_funcs & E_OPT(TN3270E_FUNC_RESPONSES))
			e_xmit_seq = (e_xmit_seq + 1) & 0x7fff;
	}
#endif /*]*/

	/* Reallocate the expanded output buffer. */
	while (xobuf_len <  (obptr - BSTART + 1) * 2) {
		xobuf_len += BUFSZ;
		need_resize++;
	}
	if (need_resize) {
		Replace(xobuf, (unsigned char *)Malloc(xobuf_len));
	}

	/* Copy and expand IACs. */
	xoptr = xobuf;
	nxoptr = BSTART;
	while (nxoptr < obptr) {
		if ((*xoptr++ = *nxoptr++) == IAC) {
			*xoptr++ = IAC;
		}
	}

	/* Append the IAC EOR and transmit. */
	*xoptr++ = IAC;
	*xoptr++ = EOR;
	net_rawout(xobuf, xoptr - xobuf);

	trace_dsn("SENT EOR\n");
	ns_rsent++;
#undef BSTART
}

#if defined(X3270_TN3270E) /*[*/
/* Send a TN3270E positive response to the server. */
static void
tn3270e_ack(void)
{
	unsigned char rsp_buf[10];
	tn3270e_header *h_in = (tn3270e_header *)ibuf;
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
	trace_dsn("SENT TN3270E(RESPONSE POSITIVE-RESPONSE "
		"%u) DEVICE-END\n",
		h_in->seq_number[0] << 8 | h_in->seq_number[1]);
	net_rawout(rsp_buf, rsp_len);
}

/* Send a TN3270E negative response to the server. */
static void
tn3270e_nak(enum pds rv)
{
	unsigned char rsp_buf[10];
	tn3270e_header *h_in = (tn3270e_header *)ibuf;
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
	trace_dsn("SENT TN3270E(RESPONSE NEGATIVE-RESPONSE %u) %s\n",h_in->seq_number[0] << 8 | h_in->seq_number[1], neg);
	net_rawout(rsp_buf, rsp_len);
}

#if defined(X3270_TRACE) /*[*/
/* Add a dummy TN3270E header to the output buffer. */
Boolean
net_add_dummy_tn3270e(void)
{
	tn3270e_header *h;

	if (!IN_E || tn3270e_submode == E_NONE)
		return False;

	space3270out(EH_SIZE);
	h = (tn3270e_header *)obptr;

	switch (tn3270e_submode) {
	case E_NONE:
		break;
	case E_NVT:
		h->data_type = TN3270E_DT_NVT_DATA;
		break;
	case E_SSCP:
		h->data_type = TN3270E_DT_SSCP_LU_DATA;
		break;
	case E_3270:
		h->data_type = TN3270E_DT_3270_DATA;
		break;
	}
	h->request_flag = 0;
	h->response_flag = TN3270E_RSF_NO_RESPONSE;
	h->seq_number[0] = 0;
	h->seq_number[1] = 0;
	obptr += EH_SIZE;
	return True;
}
#endif /*]*/
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
/*
 * net_sendc
 *	Send a character of user data over the network in ANSI mode.
 */
void
net_sendc(char c)
{
	if (c == '\r' && !linemode
/*
#if defined(LOCAL_PROCESS)
				   && !local_process
#endif
*/
						    ) {
		/* CR must be quoted */
		net_cookout("\r\0", 2);
	} else {
		net_cookout(&c, 1);
	}
}


/*
 * net_sends
 *	Send a null-terminated string of user data in ANSI mode.
 */
void
net_sends(const char *s)
{
	net_cookout(s, strlen(s));
}


/*
 * net_send_erase
 *	Sends the KILL character in ANSI mode.
 */
void
net_send_erase(void)
{
	net_cookout(&verase, 1);
}


/*
 * net_send_kill
 *	Sends the KILL character in ANSI mode.
 */
void
net_send_kill(void)
{
	net_cookout(&vkill, 1);
}


/*
 * net_send_werase
 *	Sends the WERASE character in ANSI mode.
 */
void
net_send_werase(void)
{
	net_cookout(&vwerase, 1);
}
#endif /*]*/


#if defined(X3270_MENUS) /*[*/
/*
 * External entry points to negotiate line or character mode.
 */
void
net_linemode(void)
{
	if (!CONNECTED)
		return;
	if (hisopts[TELOPT_ECHO]) {
		dont_opt[2] = TELOPT_ECHO;
		net_rawout(dont_opt, sizeof(dont_opt));
		trace_dsn("SENT %s %s\n", cmd(DONT), opt(TELOPT_ECHO));
	}
	if (hisopts[TELOPT_SGA]) {
		dont_opt[2] = TELOPT_SGA;
		net_rawout(dont_opt, sizeof(dont_opt));
		trace_dsn("SENT %s %s\n", cmd(DONT), opt(TELOPT_SGA));
	}
}

void
net_charmode(void)
{
	if (!CONNECTED)
		return;
	if (!hisopts[TELOPT_ECHO]) {
		do_opt[2] = TELOPT_ECHO;
		net_rawout(do_opt, sizeof(do_opt));
		trace_dsn("SENT %s %s\n", cmd(DO), opt(TELOPT_ECHO));
	}
	if (!hisopts[TELOPT_SGA]) {
		do_opt[2] = TELOPT_SGA;
		net_rawout(do_opt, sizeof(do_opt));
		trace_dsn("SENT %s %s\n", cmd(DO), opt(TELOPT_SGA));
	}
}
#endif /*]*/


/*
 * net_break
 *	Send telnet break, which is used to implement 3270 ATTN.
 *
 */
void
net_break(void)
{
	static unsigned char buf[] = { IAC, BREAK };

	/* I don't know if we should first send TELNET synch ? */
	net_rawout(buf, sizeof(buf));
	trace_dsn("SENT BREAK\n");
}

/*
 * net_interrupt
 *	Send telnet IP.
 *
 */
void
net_interrupt(void)
{
	static unsigned char buf[] = { IAC, IP };

	/* I don't know if we should first send TELNET synch ? */
	net_rawout(buf, sizeof(buf));
	trace_dsn("SENT IP\n");
}

/*
 * net_abort
 *	Send telnet AO.
 *
 */
#if defined(X3270_TN3270E) /*[*/
void
net_abort(void)
{
	static unsigned char buf[] = { IAC, AO };

	if (e_funcs & E_OPT(TN3270E_FUNC_SYSREQ)) {
		/*
		 * I'm not sure yet what to do here.  Should the host respond
		 * to the AO by sending us SSCP-LU data (and putting us into
		 * SSCP-LU mode), or should we put ourselves in it?
		 * Time, and testers, will tell.
		 */
		switch (tn3270e_submode) {
		case E_NONE:
		case E_NVT:
			break;
		case E_SSCP:
			net_rawout(buf, sizeof(buf));
			trace_dsn("SENT AO\n");
			if (tn3270e_bound ||
			    !(e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE))) {
				tn3270e_submode = E_3270;
				check_in3270();
			}
			break;
		case E_3270:
			net_rawout(buf, sizeof(buf));
			trace_dsn("SENT AO\n");
			tn3270e_submode = E_SSCP;
			check_in3270();
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

#if (defined(X3270_MENUS) || defined(C3270)) && defined(X3270_ANSI) /*[*/
/*
 * net_linemode_chars
 *	Report line-mode characters.
 */
struct ctl_char *
net_linemode_chars(void)
{
	static struct ctl_char c[9];

	c[0].name = "intr";	(void) strcpy(c[0].value, ctl_see(vintr));
	c[1].name = "quit";	(void) strcpy(c[1].value, ctl_see(vquit));
	c[2].name = "erase";	(void) strcpy(c[2].value, ctl_see(verase));
	c[3].name = "kill";	(void) strcpy(c[3].value, ctl_see(vkill));
	c[4].name = "eof";	(void) strcpy(c[4].value, ctl_see(veof));
	c[5].name = "werase";	(void) strcpy(c[5].value, ctl_see(vwerase));
	c[6].name = "rprnt";	(void) strcpy(c[6].value, ctl_see(vrprnt));
	c[7].name = "lnext";	(void) strcpy(c[7].value, ctl_see(vlnext));
	c[8].name = 0;

	return c;
}
#endif /*]*/

#if defined(X3270_TRACE) /*[*/
/*
 * Construct a string to reproduce the current TELNET options.
 * Returns a Boolean indicating whether it is necessary.
 */
Boolean
net_snap_options(void)
{
	Boolean any = False;
	int i;
	static unsigned char ttype_str[] = {
		IAC, DO, TELOPT_TTYPE,
		IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE
	};

	if (!CONNECTED)
		return False;

	obptr = obuf;

	/* Do TTYPE first. */
	if (myopts[TELOPT_TTYPE]) {
		unsigned j;

		space3270out(sizeof(ttype_str));
		for (j = 0; j < sizeof(ttype_str); j++)
			*obptr++ = ttype_str[j];
	}

	/* Do the other options. */
	for (i = 0; i < N_OPTS; i++) {
		space3270out(6);
		if (i == TELOPT_TTYPE)
			continue;
		if (hisopts[i]) {
			*obptr++ = IAC;
			*obptr++ = WILL;
			*obptr++ = (unsigned char)i;
			any = True;
		}
		if (myopts[i]) {
			*obptr++ = IAC;
			*obptr++ = DO;
			*obptr++ = (unsigned char)i;
			any = True;
		}
	}

#if defined(X3270_TN3270E) /*[*/
	/* If we're in TN3270E mode, snap the subnegotations as well. */
	if (myopts[TELOPT_TN3270E]) {
		any = True;

		space3270out(5 +
			((h3270.connected_type != CN) ? strlen(h3270.connected_type) : 0) +
			((h3270.connected_lu != CN) ? + strlen(h3270.connected_lu) : 0) +
			2);
		*obptr++ = IAC;
		*obptr++ = SB;
		*obptr++ = TELOPT_TN3270E;
		*obptr++ = TN3270E_OP_DEVICE_TYPE;
		*obptr++ = TN3270E_OP_IS;
		if (h3270.connected_type != CN) {
			(void) memcpy(obptr, h3270.connected_type,
					strlen(h3270.connected_type));
			obptr += strlen(h3270.connected_type);
		}
		if (h3270.connected_lu != CN) {
			*obptr++ = TN3270E_OP_CONNECT;
			(void) memcpy(obptr, h3270.connected_lu,
					strlen(h3270.connected_lu));
			obptr += strlen(h3270.connected_lu);
		}
		*obptr++ = IAC;
		*obptr++ = SE;

		space3270out(38);
		(void) memcpy(obptr, functions_req, 4);
		obptr += 4;
		*obptr++ = TN3270E_OP_IS;
		for (i = 0; i < 32; i++) {
			if (e_funcs & E_OPT(i))
				*obptr++ = i;
		}
		*obptr++ = IAC;
		*obptr++ = SE;

		if (tn3270e_bound) {
			tn3270e_header *h;

			space3270out(EH_SIZE + 3);
			h = (tn3270e_header *)obptr;
			h->data_type = TN3270E_DT_BIND_IMAGE;
			h->request_flag = 0;
			h->response_flag = 0;
			h->seq_number[0] = 0;
			h->seq_number[1] = 0;
			obptr += EH_SIZE;
			*obptr++ = 1; /* dummy */
			*obptr++ = IAC;
			*obptr++ = EOR;
		}
	}
#endif /*]*/
	return any;
}
#endif /*]*/

/*
 * Set blocking/non-blocking mode on the socket.  On error, pops up an error
 * message, but does not close the socket.
 */
static int
non_blocking(Boolean on)
{
#if !defined(BLOCKING_CONNECT_ONLY) /*[*/
# if defined(FIONBIO) /*[*/
	int i = on ? 1 : 0;

	if (SOCK_IOCTL(h3270.sock, FIONBIO, (int *) &i) < 0) {
		popup_a_sockerr(NULL, N_( "ioctl(%s)" ), "FIONBIO");
		return -1;
	}
# else /*][*/
	int f;

	if ((f = fcntl(sock, F_GETFL, 0)) == -1) {
		popup_an_errno(NULL,errno, N_( "fcntl(%s)" ), "F_GETFL" );
		return -1;
	}
	if (on)
		f |= O_NDELAY;
	else
		f &= ~O_NDELAY;
	if (fcntl(sock, F_SETFL, f) < 0) {
		popup_an_errno(NULL,errno, N_( "fcntl(%s)" ), "F_GETFL");
		return -1;
	}
# endif /*]*/
#endif /*]*/
	return 0;
}

#if defined(HAVE_LIBSSL) /*[*/

/* Initialize the OpenSSL library. */
static void ssl_init(void)
{
	static Boolean ssl_initted = False;

	if (!ssl_initted) {
		SSL_load_error_strings();
		SSL_library_init();
		ssl_initted = True;
		ssl_ctx = SSL_CTX_new(SSLv23_method());
		if (ssl_ctx == NULL) {
			popup_an_error(NULL,"SSL_CTX_new failed");
			h3270.ssl_host = False;
			return;
		}
		SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL);
	}

	ssl_con = SSL_new(ssl_ctx);
	if (ssl_con == NULL) {
		popup_an_error(NULL,"SSL_new failed");
		h3270.ssl_host = False;
	}
	SSL_set_verify(ssl_con, 0/*xxx*/, NULL);

	SSL_CTX_set_info_callback(ssl_ctx, client_info_callback);

	/* XXX: May need to get key file and password. */
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

	SSL_CTX_set_default_verify_paths(ssl_ctx);
}

/* Callback for tracing protocol negotiation. */
static void client_info_callback(INFO_CONST SSL *s, int where, int ret)
{
	if (where == SSL_CB_CONNECT_LOOP)
	{
		trace_dsn("SSL_connect: %s %s\n",SSL_state_string(s), SSL_state_string_long(s));
	}
	else if (where == SSL_CB_CONNECT_EXIT)
	{
		if (ret == 0)
		{
			trace_dsn("SSL_connect: failed in %s\n",SSL_state_string_long(s));
			WriteLog("SSL","connect failed in %s (Alert: %s)",SSL_state_string_long(s),SSL_alert_type_string_long(ret));
		}
		else if (ret < 0)
		{
			static int showing = 0;

			unsigned long e;
			char err_buf[1024];

			e = ERR_get_error();
			while(ERR_peek_error() == e)	// Remove other messages with the same error
				e = ERR_get_error();

			if(e != 0)
			{
				if(e == last_ssl_error)
					return;
				last_ssl_error = e;
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

			trace_dsn("SSL Connect error in %s\nState: %s\nAlert: %s\n",err_buf,SSL_state_string_long(s),SSL_alert_type_string_long(ret));

			if(showing)
			{
				Log("SSL Connect error message %d discarded: \"%s\" state=%s alert=%s",showing++,err_buf,SSL_state_string_long(s),SSL_alert_type_string_long(ret));
			}
			else
			{
				showing++;
				show_3270_popup_dialog(	&h3270,										// H3270 *session,
										PW3270_DIALOG_CRITICAL,						//	PW3270_DIALOG type,
										_( "SSL Connect error" ),					// Title
										err_buf,									// Message
										_( "<b>Connection state:</b> %s\n<b>Alert message:</b> %s" ),
										SSL_state_string_long(s),
										SSL_alert_type_string_long(ret));
				showing = 0;
			}

//			popup_system_error(_( "SSL Connect error" ), SSL_state_string_long(s), err_buf);

		}
	}
}

/* Process a STARTTLS subnegotiation. */
static void
continue_tls(unsigned char *sbbuf, int len)
{
	int rv;

	/* Whatever happens, we're not expecting another SB STARTTLS. */
	need_tls_follows = False;

	/* Make sure the option is FOLLOWS. */
	if (len < 2 || sbbuf[1] != TLS_FOLLOWS) {
		/* Trace the junk. */
		trace_dsn("%s ? %s\n", opt(TELOPT_STARTTLS), cmd(SE));
		popup_an_error(NULL,"TLS negotiation failure");
		net_disconnect();
		return;
	}

	/* Trace what we got. */
	trace_dsn("%s FOLLOWS %s\n", opt(TELOPT_STARTTLS), cmd(SE));

	/* Initialize the SSL library. */
	ssl_init();
	if (ssl_con == NULL) {
		/* Failed. */
		net_disconnect();
		return;
	}

	/* Set up the TLS/SSL connection. */
	if (SSL_set_fd(ssl_con, h3270.sock) != 1) {
		trace_dsn("Can't set fd!\n");
	}

#if defined(_WIN32) /*[*/
	/* Make the socket blocking for SSL. */
	(void) WSAEventSelect(h3270.sock, h3270.sock_handle, 0);
	(void) non_blocking(False);
#endif /*]*/

	rv = SSL_connect(ssl_con);

#if defined(_WIN32) /*[*/
	/* Make the socket non-blocking again for event processing. */
	(void) WSAEventSelect(h3270.sock, h3270.sock_handle, FD_READ | FD_CONNECT | FD_CLOSE);
#endif /*]*/

	if (rv != 1) {
		/* Error already displayed. */
		trace_dsn("continue_tls: SSL_connect failed\n");
		net_disconnect();
		return;
	}

	h3270.secure_connection = True;

	/* Success. */
	trace_dsn("TLS/SSL negotiated connection complete. Connection is now secure.\n");

	/* Tell the world that we are (still) connected, now in secure mode. */
	host_connected(&h3270);
}

#endif /*]*/

/* Return the local address for the socket. */
int net_getsockname(const H3270 *session, void *buf, int *len)
{
	if (session->sock < 0)
		return -1;
	return getsockname(session->sock, buf, (socklen_t *)(void *)len);
}

/* Return a text version of the current proxy type, or NULL. */
char *
net_proxy_type(void)
{
    	if (proxy_type > 0)
	    	return proxy_type_name(proxy_type);
	else
	    	return NULL;
}

/* Return the current proxy host, or NULL. */
char *
net_proxy_host(void)
{
    	if (proxy_type > 0)
	    	return proxy_host;
	else
	    	return NULL;
}

/* Return the current proxy port, or NULL. */
char *
net_proxy_port(void)
{
    	if (proxy_type > 0)
	    	return proxy_portname;
	else
	    	return NULL;
}

LIB3270_EXPORT int lib3270_get_ssl_state(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);

#if defined(HAVE_LIBSSL)
		return (h->secure_connection != 0);
#else
		return 0;
#endif
}

int Get3270Socket(void)
{
        return h3270.sock;
}
