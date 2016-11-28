#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

/* prog a0 r n
Fiul trimite catre parinte SIGUSR1 dupa 1 sec, intre timp calculeaza a[n+1] = a[n] + 1/r, a[0] = a0. 
Dupa ce termina calculul, fiul afiseaza rezultatul, se termina si transmite parintelui valoarea 14 prin codul de retur.
Parintele afiseaza ****, dupa SIGUSR1 afiseaza +++++ si se termina dupa ce preia starea fiului si afiseaza codul de retur al acestuia.

Pentru ca executia fiului sa depaseasca 1 sec, se poate apela 
./p1 1 10000 100000000
*/

int a0,r;
double an;
int n;
char c='*';

void child_alarm(int s)
{	
	if (kill(getppid(), SIGUSR1) < 0)
	{
		printf("eroare kill\n");
		exit(2);
	}
}

void fiu()
{
	struct sigaction sa;

	sa.sa_handler = child_alarm;
	sa.sa_flags = 0;
	if (sigaction(SIGALRM,&sa,NULL) < 0)
	{
		printf("Eroare sigaction\n");
		exit(3);
	}

	alarm(1);

	an = a0;
	int i = 0;
	while(i < n)
	{
		i++;
		an += 1/(double)r;
	}
	printf("a[%d]=%f\n",n,an);
	exit(14);
}

void parent_end(int s)
{
	int status;
	int pid = wait(&status);
	if (pid < 0)
	{
		printf("eroare wait\n");
		exit(1);
	}
	if (WIFEXITED(status))
		printf("Child %d ended with status %d.\n",pid,WEXITSTATUS(status));
	printf("Parent also ends.\n");
	exit(0);
}

void parent_changechar(int s)
{
	c = '+';
}

int main(int argc, char *argv[])
{
	pid_t pid;
	struct sigaction sa1, sa2;

	if (argc != 4)
	{
		printf("Usage: %s a0 r n\n", argv[0]);
		exit(1);
	}

	a0 = atoi(argv[1]);
	r = atoi(argv[2]);
	n = atoi(argv[3]);

	if ((pid = fork()) < 0)
	{
		printf("Eroare fork\n");
		exit(4);
	}
	else if (pid == 0)
	{
		fiu();
	}

	// parent	
	sa1.sa_handler = parent_changechar;
	sa1.sa_flags = 0;
	if (sigaction(SIGUSR1,&sa1,NULL) < 0)
	{
		printf("Eroare sigaction\n");
		exit(5);
	}

	sa2.sa_handler = parent_end;
	sa2.sa_flags = 0;
	if (sigaction(SIGCHLD,&sa2,NULL) < 0)
	{
		printf("Eroare sigaction\n");
		exit(6);
	}
	
	while(1)
	{
		printf("%c",c);
	}
	
	return 0;
}

