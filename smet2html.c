/* smet2html
 *
 * Project homepage: http://ed2k-tools.sourceforge.net
 *
 * Please report bugs and supply patches there.
 *
 * --- DESCRIPTION ---
 *
 * this simple command line tool has been written for use with the
 * edonkey 2000 file-sharing program (www.edonkey2000.com)
 *
 * smet2html scans an eDonkey2000 server.met file which contains a
 * list of servers and outputs them in ed2k://|server| link format
 *
 *
 *
 * =====> it assumes that the server.met is in the working directory
 *
 *
 *
 * This code sucks, but it works for me. It was put together in
 *  a couple of minutes and fulfils its purpose. You are welcome to
 *  submit patches if you feel like it ;)
 *
 *
 * --- COPYRIGHT AND LICENCE ---
 *
 * Copyright (C) 2002 Tim-Philipp Müller <t.i.m@orange.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* --- DEFINE IF YOU COMPILE FOR 'LINUX' OR 'WINDOWS' here... !!! ---------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#define VERSION "0.1"


/* option_nohtml - if set: 
 *
 * Do not wrap <a href=...></a> around ed2k-links, 
 *  print only ed2k-links
 *
 */
static int option_nohtml = 0;

/* option_linksonly - if set:
 *
 * If html is on, print only the link lines,
 *  don't print html document header/body +
 *  footer
 */
static int option_linksonly = 0;

/* option_metfile
 *
 * The server.met file to process. If not set,
 *  then server.met in the current working
 *  directory will be processed
 */
static char *option_metfile = NULL;


/* read_and_dump_i4string
 *
 * Reads an i4_string from file
 *  and dumps it to /dev/null
 *  (ie. just skips the string)
 *
 */

static void
read_and_dump_i4string (FILE *smetfile)
{
	unsigned short	len, l;

	if (fread(&len,1,2,smetfile)<0)				/* 16 bits: no. of letters in string */
	{
		fprintf(stderr, "fread() error in %s: %s\n", __FUNCTION__, strerror(errno));
		return;
	}

	for (l=0; l<len; l++)
	{
		(void) fgetc(smetfile);	/* read letters + dump them to nirvana */
	}
}



/* print_help
 *
 * prints help text and exits
 *
 */

static void
print_help (void)
{
  printf ("\n");
	printf ("smet2html version %s Copyright (c) 2002 Tim-Philipp Müller\n", VERSION);
  printf ("\n");
	printf ("This program is free software, licensed under the GNU General\n");
	printf ("Public License, version 2 or higher, and comes with no warranty\n");
	printf ("at all, not even implied. See the License for details.\n");
	printf ("http://www.gnu.org.\n");
	printf ("\n");
  printf ("smet2html takes an eDonkey2000 server.met file and extracts the\n");
	printf ("server links from it in form of a simple html-file or just in\n");
	printf ("form of ed2k-links. The output can be redirected from stdout\n");
	printf ("to a file with '> serverlist.html' or can be processed via\n");
	printf ("pipes (on unix systems).\n");
	printf ("\n");
	printf ("Command line options:\n");
	printf ("\n");
	printf ("\t--nohtml      \tprint ed2k-links only, do not output html tags\n");
	printf ("\n");
	printf ("\t--linksonly   \tif outputting html, do only output the server lines,\n");
	printf ("\t              \t but no html document header or footer. This is useful\n");
	printf ("\t              \t if you want to pipe the output through sed and friends.\n");
	printf ("\n");
	printf ("\t--input=<file>\tspecify a server.met file to process (default: ./server.met)\n");
	printf ("\n");
	printf ("\t--help        \tprint this help screen\n");
	printf ("\n\n");
	exit(EXIT_SUCCESS);
}



/* scan_command_line_options
 *
 * scans the command line options
 *
 * returns 0 on error, otherwise 1
 *
 */

static int
scan_command_line_options (int argc, char **argv)
{
	int i;

	/* nothing to do? */
	if (!argv)
		return 1;

	/* skip first argument (program name ususally */
	for (i=1; i<argc; i++)
	{
		if (argv[i])
		{
			char *arg = argv[i];
			while (*arg == '-')
				arg++;

			if (*arg)
			{
				if (strncasecmp(arg,"help",4)==0)
					print_help();	/* will exit */

				else if (strncasecmp(arg,"nohtml",6)==0)
					option_nohtml = 1;

				else if (strncasecmp(arg,"linksonly",8)==0)
					option_linksonly = 1;

				else if (strncasecmp(arg,"input=",6)==0)
				{
					option_metfile = strdup(arg+6);
				}

				else
				{
					fprintf (stderr, "**** unknown command line option '%s'!? (ignored)\n", argv[i]);
					return 0;
				}

			} else fprintf (stderr, "**** dodgy command line option '%s'!? (ignored)\n", argv[i]);
		}
	}

	return 1;
}



/* main
 *
 *
 */

int
main (int argc, char *argv[])
{
	FILE		*smetfile;
	unsigned int	 servers;		/* no of servers */
	unsigned int	 i;
	size_t		 len;

	if (!scan_command_line_options (argc, argv))
		exit(EXIT_FAILURE);

	if (!option_metfile)
		option_metfile = strdup("server.met");

	if ( (smetfile = fopen (option_metfile, "rb")) == NULL )
	{
		fprintf (stderr,"*****\tCouldn't find or open file '%s'\n\n", option_metfile);
		fprintf (stderr,"*****\tuse the '--help' command line option for more information.\n\n");
#ifdef __WIN32
		fprintf (stderr,"\n\nWINDOWS USERS: This is a COMMAND LINE program.\n\n");
	/* TODO: sleep on windows, so people who didn't start from the command line see the message */
#endif
		exit(EXIT_FAILURE);
	}

	/* skip header byte */
	fseek (smetfile, 1, SEEK_SET);

	/* read no. of servers (4 bytes) */
	len = fread (&servers, 1, 4, smetfile);
	if (len!=4)
	{
		perror ("read error");
		exit (EXIT_FAILURE);
	}

	if (servers == 0)
	{
		fprintf (stderr, "*****\tserver.met file says it contains no records.\n");
		exit(EXIT_FAILURE);
	}

	if (servers > 1024)
	{
		fprintf (stderr, "*****\tWARNING: server.met file says it contains more than 1024 records!!!\n"
		 	"\t(slightly wrong .met syntax?)\n\n");
	}

	if (option_nohtml==0 && option_linksonly==0)
	{
		printf ("<html>\n<head></head>\n<body>\n");
	}

	for (i=0; i<servers; i++)
	{
		unsigned int  	ip, tags, t;
		unsigned short	port;
		char          	ed2klink[1024];	/* this IS enough for a server link */

		if (feof(smetfile) || ferror(smetfile))
		{
			perror ("*****\tPremature eof or error while reading the file");
			exit (EXIT_FAILURE);
		}
//		printf ("file pos = %8x\n", (unsigned int)ftell(smetfile));
		fread (&ip, 1, 4, smetfile);
		fread (&port, 1, 2, smetfile);
		fread (&tags, 1, 4, smetfile);

		snprintf (ed2klink, sizeof(ed2klink), "ed2k://|server|%u.%u.%u.%u|%u|", ip&0xff, (ip&0xff00)>>8, (ip&0xff0000)>>16, (ip&0xff000000)>>24, port);

		if (!option_nohtml)
		{
			printf ("<a href=\"%s\">%s</a><br>\n", ed2klink, ed2klink);
		} else printf ("%s\n", ed2klink);

		if (feof(smetfile) || ferror(smetfile))
		{
			perror ("*****\tPremature eof or error while reading the file");
			exit (EXIT_FAILURE);
		}
		for (t=0; t<tags; t++)
		{
			char		type;
			unsigned int	dummy;
			type = fgetc (smetfile);
			// read tagname first
			read_and_dump_i4string(smetfile);
			switch (type)
			{
				case 0x03:
				{
					fread (&dummy,1,4,smetfile);
				}
				break;

				case 0x02:
				{
					read_and_dump_i4string(smetfile);
				}
				break;

				default:	// something strange happened or file format is wrong
				{
					t=tags;		// => stop
					i=servers;
				}
				break;
			}
		}

	}

	if (option_nohtml==0 && option_linksonly==0)
	{
		printf ("<hr>\n<p><font size=\"-2\">created with <a href=\"http://ed2k-tools.sourceforge.net\">smet2html</a></font></p>\n");
		printf ("</body>\n</html>\n");
	}

	printf ("\n\n");
	fclose(smetfile);
	
	if (option_metfile)
		free(option_metfile);

	return (0);
}


