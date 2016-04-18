
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <pthread.h>

#include "private.h"
#include <lib3270/macros.h>

#define MAX_ARGS 10

static H3270 *session = NULL;

static void * mainloop(void *dunno)
{
	while(session)
	{
		lib3270_main_iterate(session,1);
	}
	return NULL;
}

int main(int numpar, char *param[])
{
	H3270		* h;
	int			  rc	= 0;
	const char  * url	= getenv("LIB3270HOST");
//	char	 	  line[4096];
//	pthread_t	  thread;

	lib3270_initialize();

	session = h = lib3270_session_new("");
	printf("3270 session %p created\n]",h);

//	lib3270_set_toggle(session,LIB3270_TOGGLE_DS_TRACE,1);

//	pthread_create(&thread, NULL, mainloop, NULL);
//	pthread_detach(thread);

	lib3270_set_url(h,url ? url : "tn3270://fandezhi.efglobe.com");
	rc = lib3270_connect(h,1);

	printf("\nConnect exits with rc=%d\n",rc);

	mainloop(0);

/*
	while(fgets(line,4095,stdin))
	{
//		const LIB3270_MACRO_LIST *cmd = get_3270_calls();

		int		 	  f;
		int			  argc = 0;
		const char	* argv[MAX_ARGS+1];
		char		* ptr;

		line[4095] = 0;	// Just in case.

		for(ptr = line;ptr && *ptr != '\n';ptr++);
		*ptr = 0;

		if(!*line)
			break;

		argv[argc++] = strtok(line," ");
		for(f=1;f<MAX_ARGS;f++)
		{
			if( (argv[argc++] = strtok(NULL," ")) == NULL)
				break;
		}
		argv[argc] = NULL;

		if(!strcmp(argv[0],"quit"))
			break;

		ptr = lib3270_run_macro(h,argv);
		if(ptr)
		{
			printf("\n%s\n",ptr);
			lib3270_free(ptr);
		}
		else
		{
			printf("\nNo response\n");
		}

		printf("\n]");

	}

	session = 0;
	pthread_cancel(thread);

	printf("Ending 3270 session %p\n",h);
	lib3270_session_free(h);
*/

	return 0;
}
