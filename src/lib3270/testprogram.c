
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include <lib3270/macros.h>

#define MAX_ARGS 10

int main(int numpar, char *param[])
{
	H3270	* h;
	char 	  line[4096];

	lib3270_initialize();

	h = lib3270_session_new("");
	printf("3270 session %p created\n]",h);

	while(fgets(line,4095,stdin))
	{
		const LIB3270_MACRO_LIST *cmd = get_3270_calls();

		int		 	  f;
		int			  argc = 0;
		const char	* argv[MAX_ARGS];
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
		argc--;

		if(!strcmp(argv[0],"quit"))
			break;

		for(f=0;cmd[f].name;f++)
		{
			if(!strcmp(cmd[f].name,argv[0]))
			{
				char *str = cmd[f].exec(h,argc,argv);
				if(str)
				{
					printf("\n%s\n",str);
					free(str);
				}
				else
				{
					printf("\nNo response\n");
				}
				break;
			}
		}

		printf("\n]");

	}

	printf("Ending 3270 session %p\n",h);
	lib3270_session_free(h);

	return 0;
}
