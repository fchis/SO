#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

/* Parintele trimite catre un fiu litere, catre alt fiu litere mici, iar fiii numara caractere si transmit valorile rezultate parintelui prin codul de retur.
Al treilea fiu afiseaza numarul de cuvinte din fisierul dat.
Parintele si fiii se executa in paralel!
Parintele asteapta terminarea fiilor, afiseaza codurile de retur si calculeaza nr de litere mari prin diferenta.
*/

void fiu3(char *f)
{
	printf("pid = %d\n",getpid());
	execlp("wc", "wc", "-w", f, (char*)0);
	printf("Eroare exec\n");
	exit(3);
}

void fiu(int infd)
{
	char c;
	int count = 0;
	while(read(infd,&c,1) > 0)
		count++;
	close(infd);
	exit(count);
}

void parent(int infd, int outfd1, int outfd2)
{
	char c;
	while(read(infd,&c,1) > 0)
	{
		if (isalpha(c))
		{
			write(outfd1,&c,1);
			if (islower(c))
				write(outfd2,&c,1);
		}
	}
	close(infd);
	close(outfd1);
	close(outfd2);
}

int main(int argc, char *argv[])
{
	pid_t pid[3];
	int i,st[3], pfd1[2], pfd2[2];
	
	if (argc < 2)
	{
		printf("Usage: %s file\n", argv[0]);
		exit(1);
	}

	if (pipe(pfd1) < 0)
	{
		printf("Eroare pipe 1\n");
		exit(1);
	}
	if (pipe(pfd2) < 0)
	{
		printf("Eroare pipe 2\n");
		exit(1);
	}

	if ((pid[0] = fork()) < 0)
	{
		printf("Eroare fork 1\n");
		exit(2);
	}
	else if (pid[0] == 0) // fiu 1
	{
		close(pfd1[1]);
		close(pfd2[0]);
		close(pfd2[1]);
		fiu(pfd1[0]);
	}

	if ((pid[1] = fork()) < 0)
	{
		printf("Eroare fork 2\n");
		exit(3);
	}
	else if (pid[1] == 0) // fiu 2
	{
		close(pfd1[1]);
		close(pfd1[0]);
		close(pfd2[1]);
		fiu(pfd2[0]);
	}

	if ((pid[2] = fork()) < 0)
	{
		printf("Eroare fork 2\n");
		exit(3);
	}
	else if (pid[2] == 0) // fiu 3
	{
		close(pfd1[1]);
		close(pfd1[0]);
		close(pfd2[1]);
		close(pfd2[0]);
		fiu3(argv[1]);
	}

	close(pfd1[0]);
	close(pfd2[0]);
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		printf("Eroare open\n");
		exit(3);
	}

	parent(fd, pfd1[1], pfd2[1]);

	for(i = 0; i < 3; i++)
	{
		printf("Call %d: process %d ended.\n", i, waitpid(pid[i],&st[i],0));
		if (WIFEXITED(st[i]))
		{
			printf("Exit status: %d\n",  WEXITSTATUS(st[i]));
		}
	}

	printf("%d capital letters.\n", WEXITSTATUS(st[0]) - WEXITSTATUS(st[1]));
	return 0;
}

