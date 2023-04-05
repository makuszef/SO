#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include<signal.h>
#include <string.h>
#include<ctype.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<pthread.h>
#include<sys/msg.h>
#include<sys/types.h>
#include<semaphore.h>
#define BUF_SIZE 40
#define MAX_LINE_LENGTH 10000
#define MAX_MSG_LENGTH 80
#define SHM_KEY 0x1234

#define size sizeof(struct shared_memory)	//boro
struct shared_memory {
int P[3];
int flaga_m;
int flaga_p[3];
};


struct msgbuf {
	long mtype;
	char mtext[MAX_MSG_LENGTH];
};
pid_t pid_tab[3];
int msgid;
void handle_sigchild1(int sig, siginfo_t* info, void* context) {		//proces 1
	printf("Sygnal P1\n");
	printf("Sygnal %d catched by process %d send by process %d\n", sig, getpid(), info->si_pid);
	int shm_id = shmget(SHM_KEY,size,0666|IPC_CREAT);
	struct shared_memory *shared_mem_ptr = (struct shared_memory *) shmat (shm_id,NULL,0);
	if (sig == SIGUSR1) { 	//reaguj tylko na macierzysty i SIGUSR1
		//sprawdz czy macierzysty wyslal sygnal
		if (info->si_pid == getppid()) {
			printf("Sygnal od PM\n");
			//czytaj kolejke
			struct msgbuf child_msg;
			
			printf("Odczytuje wiadomosc...\n");
			
			if (msgrcv(msgid, &child_msg, sizeof(child_msg), 1, 0) == -1) { //czytaj wiadomosc z kolejki mtype = 1
				printf("msgrcv");
			}
			printf("Wyswietlam wiadomosc...\n");
			printf("Otrzymana wiadomosc %s\n", child_msg.mtext);
			//kill(getpid() + 1, SIGUSR1);		//przeslij sygnal do procesu 2
			int sig_num = atoi(child_msg.mtext); 	//konewrtuj sygnal z KK
			
			kill(shared_mem_ptr -> P[1], SIGUSR1);		//przeslij sygnal do procesu 2	
			
			signal(sig_num, SIG_DFL);			//przeslij sygnal do siebie z KK
			raise(sig_num);
			struct sigaction sa;		//przywroc signal handling
			sa.sa_flags = SA_SIGINFO|SA_RESTART;
    			sa.sa_sigaction = handle_sigchild1; //sighandler
    			sigaction(sig_num, &sa, NULL);
    			
		}
	}
	
}
void handle_sigchild2(int sig, siginfo_t* info, void* context) {		//proces 2
	printf("Sygnal P2\n");
	
	//key_t shm_key = ftok("/dev/nul",'k');
	int shm_id = shmget(SHM_KEY,size,0666|IPC_CREAT);
	struct shared_memory *shared_mem_ptr = (struct shared_memory *) shmat (shm_id,NULL,0);
	//printf ("\n jestem P. Pidy innych:	%i\n",shared_mem_ptr -> P[0]);
	printf("Sygnal %d catched by process %d send by process %d\n", sig, getpid(), info->si_pid);
	if (sig == SIGUSR1) { 	//reaguj tylko na P1 i SIGUSR1
		//sprawdz czy P1 wyslal sygnal
		if (info->si_pid == shared_mem_ptr -> P[0]) {
			printf("Sygnal od P1\n");
			//czytaj kolejke
			struct msgbuf child_msg;
			
			printf("Odczytuje wiadomosc...\n");
			if (msgrcv(msgid, &child_msg, sizeof(child_msg), 2, 0) == -1) { //czytaj wiadomosc z kolejki mtype = 2
				printf("msgrcv");
			}
			printf("Wyswietlam wiadomosc...\n");
			printf("Otrzymana wiadomosc %s\n", child_msg.mtext);
			int sig_num = atoi(child_msg.mtext); 	//konewrtuj sygnal z KK
			kill(shared_mem_ptr -> P[2], SIGUSR1);		//przeslij sygnal do procesu 3
			
			
			signal(sig_num, SIG_DFL);			//przeslij sygnal do siebie z KK
			raise(sig_num);
			struct sigaction sa;		//przywroc signal handling
			sa.sa_flags = SA_SIGINFO|SA_RESTART;
    			sa.sa_sigaction = handle_sigchild2; //sighandler
    			sigaction(sig_num, &sa, NULL);
    			
    			
		}
	}
	else if (sig == SIGCONT || sig == SIGTERM || sig == SIGTSTP) { 	//te sygnaly mozna odberac z zewnatrz
		
		printf("Przesylam do PM\n");
		
		kill(getppid(), sig);		//przeslij sygnal do procesu macierzystego
		
		
	}
	
}
void handle_sigchild3(int sig, siginfo_t* info, void* context) {		//proces 3
	printf("Sygnal P3\n");
	
	//key_t shm_key = ftok("/dev/nul",'k');
	int shm_id = shmget(SHM_KEY,size,0666|IPC_CREAT);
	struct shared_memory *shared_mem_ptr = (struct shared_memory *) shmat (shm_id,NULL,0);
	
	printf("Sygnal %d catched by process %d send by process %d\n", sig, getpid(), info->si_pid);
	if (sig == SIGUSR1) { 	//reaguj tylko na P2 i SIGUSR1
		//sprawdz czy P2 wyslal sygnal
		if (info->si_pid == shared_mem_ptr -> P[1]) {
			printf("Sygnal od P2\n");
			//czytaj kolejke
			struct msgbuf child_msg;
			
			printf("Odczytuje wiadomosc...\n");
			if (msgrcv(msgid, &child_msg, sizeof(child_msg), 3, 0) == -1) { //czytaj wiadomosc z kolejki mtype = 3
				printf("msgrcv");
			}
			printf("Wyswietlam wiadomosc...\n");
			printf("Otrzymana wiadomosc %s\n", child_msg.mtext);
			int sig_num = atoi(child_msg.mtext); 	//konewrtuj sygnal z KK
			
			
			signal(sig_num, SIG_DFL);			//przeslij sygnal do siebie z KK
			raise(sig_num);
			struct sigaction sa;		//przywroc signal handling
			sa.sa_flags = SA_SIGINFO|SA_RESTART;
    			sa.sa_sigaction = handle_sigchild3; //sighandler
    			sigaction(sig_num, &sa, NULL);
    			
		}
	}
}
void handle_sigparent(int sig, siginfo_t* info, void* context) {	
	printf("Sygnal PM\n");
	
	//key_t shm_key = ftok("/dev/nul",'k');
	int shm_id = shmget(SHM_KEY,size,0666|IPC_CREAT);;
	struct shared_memory *shared_mem_ptr = (struct shared_memory *) shmat (shm_id,NULL,0);
	
	printf("Sygnal %d catched by process %d send by process %d\n", sig, getpid(), info->si_pid);
	if (info->si_pid == shared_mem_ptr -> P[1]) {	//reaguj tylko na sygnal od P2
		//ustaw atrybuty wiadomosci
		struct msgbuf msg; //struktura wiadomosci
		msg.mtype = 1;	
		
		sprintf(msg.mtext,"%d", sig);//kopiuj nr sygnalu do msg.text
		printf("Wysylam wiadomosc %s do P1P2P3...\n", msg.mtext);
		if (sig == SIGTERM || sig == SIGTSTP) {
			if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) { //wyslij wiadomosc do p1
					printf("msgsnd");
			}
			msg.mtype = 2;	
			if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) { //wyslij wiadomosc do p2
					printf("msgsnd");
			}
			msg.mtype = 3;	
			if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) { //wyslij wiadomosc do p3
					printf("msgsnd");
			}
			printf("Wyslano wiadomosc do P1P2P3\n");
			kill(shared_mem_ptr -> P[0], SIGUSR1);		//przeslij sygnal do P1 aby zaczal czytac KK i powiadomil pozostale procesy
		}
		if (sig == SIGTERM) {
			wait(NULL);
			wait(NULL);
			wait(NULL);
			//printf("Procesy potomne zakonczyly dzialanie\n");
			signal(sig, SIG_DFL);			//przeslij sygnal do siebie z KK
			raise(sig);
		}
		else if (sig == SIGCONT) {
			//kill(shared_mem_ptr -> P[1], SIGCONT);
			kill(shared_mem_ptr -> P[0], SIGCONT);
			kill(shared_mem_ptr -> P[2], SIGCONT);
		}
	}
	
}
int main(int argc, char* argv[]) {
	pid_t pid;
	
	//key_t shm_key = ftok("/dev/nul",'o');
	int shm_id = shmget(SHM_KEY,size,0666|IPC_CREAT);
	//printf ("%d\n",shm_id);
	struct shared_memory *shared_mem_ptr = (struct shared_memory *) shmat (shm_id,NULL,0);
	for(int i=0; i<3; i++)
		shared_mem_ptr -> P[i]=0;
	shared_mem_ptr -> flaga_m=0;
	
	sem_t semaphore[5];
	struct msgbuf msg; //struktura wiadomosci
	
	int shmid; //pamiec dzielona id
	char* bufptr;
	char tekst[MAX_LINE_LENGTH];
	
	shmid = shmget(IPC_PRIVATE, sizeof(tekst), 0666|IPC_CREAT); //stworz pamiec dzielona, zwroc idenryfikator ,-1 jesli porazka
	if (shmid == -1) {
		printf("Dzielona pamiec");
		return 1;
	}
	int shmid_int = shmget(IPC_PRIVATE, sizeof(int), 0666|IPC_CREAT); //stworz pamiec dzielona, zwroc idenryfikator ,-1 jesli porazka
	if (shmid_int == -1) {
		printf("Dzielona pamiec");
		return 1;
	}
	char *str = shmat(shmid, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
	if (str == (void*) -1) {
		printf("Shared memmory attach");
	}
	
	
	int sem_int = shmget(IPC_PRIVATE, sizeof(semaphore), 0666|IPC_CREAT); //stworz pamiec dzielona, zwroc idenryfikator ,-1 jesli porazka
	if (shmid_int == -1) {
		printf("Dzielona pamiec");
		return 1;
	}
	sem_t *sem_ptr = shmat(sem_int, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
	if (str == (void*) -1) {
		printf("Shared memmory attach");
	}
	//identyfikator semafora tworz  5 semafor
	//inicjalizuj semafor
	sem_init(sem_ptr, 1, 1);
	sem_init(sem_ptr+1, 1, 0);
	sem_init(sem_ptr+2, 1, 0);
	sem_init(sem_ptr+3, 1, 0);
	sem_init(sem_ptr+4, 1, 1);
	
	//tworz kolejke komunikatow
	msgid = msgget(IPC_PRIVATE, 0666|IPC_CREAT);
	if (msgid == -1) {
		printf("msgid");
		exit(1);
	}
	
	
	int k;
	for (k = 0; k < 3; k++) {/*petla w ktorej tworze procesy potomne*/
	    	switch(pid = fork()) {
		case 0:
		/*proces potomny 1*/
			if (k == 0) {
			
			while (shared_mem_ptr -> flaga_p[0]==0
			||shared_mem_ptr -> flaga_p[1]==0
			||shared_mem_ptr -> flaga_p[2]==0
			||shared_mem_ptr -> flaga_m==0)
			{
				if (shared_mem_ptr -> flaga_m==1)
				shared_mem_ptr -> flaga_p[0]=1;
			}
			
			
			
			struct sigaction sa;
			sa.sa_flags = SA_SIGINFO|SA_RESTART;
    			sa.sa_sigaction = handle_sigchild1; //sighandler
    			
    			sigaction(SIGUSR1, &sa, NULL);
    			sigaction(SIGTERM, &sa, NULL);
    			sigaction(SIGCONT, &sa, NULL);
    			sigaction(SIGTSTP, &sa, NULL);
    			
			sem_t *sem_ptr = shmat(sem_int, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
				if (str == (void*) -1) {
					printf("Shared memmory attach");
				}
			struct msgbuf child_msg;
			int wybor;			//struktura wiadomosci z kk
			char line[MAX_LINE_LENGTH];
			char *shmpointer = shmat(shmid, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
			if (shmpointer == (void*) -1) {
				printf("Shared memmory attach");
			}
			
			while (1) {
			
				sem_wait(sem_ptr+1);
				
				
				shared_mem_ptr -> flaga_p[0]=0;
				FILE* file = NULL;
				char filename[20] = "tekst.txt";
				printf("co chcesz zrobic:\n1 Czytaj z pliku \n2 standardowy strumien wejsciowy\n");
				scanf("%d", &wybor);
				
				fflush(stdin);
				if (wybor == 1) {		//czytaj z pliku
					printf("Podaj nazwe pliku\n");
					scanf("%s", filename);
					fflush(stdin);
					file = fopen(filename, "r"); //otworz plik
				}
				else if (wybor == 2) {		//czytaj strumien wejsciowy
					printf("Podaj ciag wyrazow\n");
					char singleline[100];
					memset(line, 0, sizeof(line));//czysc stringa
					while (1) {
						scanf("%s", singleline);
						//printf("wpisales %s\n", singleline);
						if (strcmp(singleline, ".") == 0) 
							break;
						strcat(line, singleline);
						strcat(line, "\n");
					}
					//scanf("%[^.]s", line); //
					fflush(stdin);
					file = NULL;
					//printf("Wpisales %s\n", line);
				}
				else {
					printf("Nie ma takiej opcji\n");
				}
				if (file != NULL && wybor == 1) {		//czytaj z pliku
						
					while (fgets(line, MAX_LINE_LENGTH, file)) { //czytaj wiersze z pliku
						sem_wait(sem_ptr+4);
						fflush(file);
						//sekcja krytyczna
						
						strcpy(shmpointer, line);//kopiuj zawartosc line do shmpointer
						printf("Proces 1 (%d) zapisalem wiersz do shared memory \n", getpid());
			
						//koniec sekcji krytycznej
						//sem_post(&semaphore[4]);
						sem_post(sem_ptr+2);
						
					}
					//printf("Koniec pliku\n");
					sem_post(sem_ptr+4);
					
					fclose(file);
					
				}
				else {		//czytaj stdin
					
					char* token = strtok(line, "\n");	//podziel stringa
					while (token != NULL) {
						sem_wait(sem_ptr+4);
						
						//printf("%s\n", token);
						//sekcja krytyczna
						strcpy(shmpointer, token);//kopiuj zawartosc line do shmpointer
						printf("Proces 1 (%d) zapisalem wiersz do shared memory \n", getpid());
						token = strtok(NULL, "\n");
					
						//koniec sekcji krytycznej
						//sem_post(&semaphore[4]);
						sem_post(sem_ptr+2);
						
					}
					//printf("Koniec ftok\n");
					sem_post(sem_ptr+4);
					
					
				
				}	
			}
			if (shmdt(shmpointer) == -1) { //odlacz pamiec dzielona
				printf("shmdt");
				return 1;
			}
			
			}
			else if (k == 1){
				/*proces potomny 2*/
				
				while (shared_mem_ptr -> flaga_p[0]==0
				||shared_mem_ptr -> flaga_p[1]==0
				||shared_mem_ptr -> flaga_p[2]==0
				||shared_mem_ptr -> flaga_m==0)
			{
				if (shared_mem_ptr -> flaga_m==1)
				shared_mem_ptr -> flaga_p[1]=1;
			}
				//printf ("\n\n\n\n jestem P[%i]. Pidy innych:	\n%i\n%i\n%i\n",k+1,shared_mem_ptr -> P[0],shared_mem_ptr -> P[1],shared_mem_ptr -> P[2]);
				
				//dzialanie na sygnalach
				struct sigaction sa;
				sa.sa_flags = SA_SIGINFO|SA_RESTART;
	    			sa.sa_sigaction = handle_sigchild2; //sighandler
	    			
	    			sigaction(SIGUSR1, &sa, NULL);
	    			sigaction(SIGTERM, &sa, NULL);
	    			sigaction(SIGCONT, &sa, NULL);
	    			sigaction(SIGTSTP, &sa, NULL);
	    			sem_t *sem_ptr = shmat(sem_int, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
				if (str == (void*) -1) {
					printf("Shared memmory attach");
				}
				struct msgbuf child_msg;
				char *shmpointer = shmat(shmid, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
				if (shmpointer == (void*) -1) {
					printf("Shared memmory attach");
				}
				
				int l, counter = 0;	//counter ->ile lini w wierszu
				int *shm_int = shmat(shmid_int, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
				while (1) {
				
					sem_wait(sem_ptr+2);
					
					//sekcja krytyczna
					
					for (l = 0; l < MAX_LINE_LENGTH; l++) { //oblicz ilosc znakow w wierszu
						counter++;
						if (shmpointer[l] == '\n' || shmpointer[l] == '\0') {
							counter--;
							break;
						}
					}
					
					
					
					*shm_int = counter;		//zapisz do pamieci dzielonej
					printf("Proces 2 (%d) obliczylem ile lini w wierszu\n", getpid());
					counter = 0;
						
					//koniec sekcji krytycznej
					//sem_post(&semaphore[2]);
					sem_post(sem_ptr+3);
					
				
				}
				if (shmdt(shmpointer) == -1) { //odlacz pamiec dzielona
					printf("shmdt");
					return 1;
				}
			}
			else{
				/*proces potomny 3*/
				
				while (shared_mem_ptr -> flaga_p[0]==0
				||shared_mem_ptr -> flaga_p[1]==0
				||shared_mem_ptr -> flaga_p[2]==0
				||shared_mem_ptr -> flaga_m==0)
			{
				if (shared_mem_ptr -> flaga_m==1)
				shared_mem_ptr -> flaga_p[2]=1;
			}
			//printf ("\n\n\n\n jestem P[%i]. Pidy innych:	\n%i\n%i\n%i\n",k+1,shared_mem_ptr -> P[0],shared_mem_ptr -> P[1],shared_mem_ptr -> P[2]);
				
				//dzialanie na sygnalach
				struct sigaction sa;
				sa.sa_flags = SA_SIGINFO|SA_RESTART;
	    			sa.sa_sigaction = handle_sigchild3; //sighandler
	    			
	    			sigaction(SIGUSR1, &sa, NULL);
	    			sigaction(SIGTERM, &sa, NULL);
	    			sigaction(SIGCONT, &sa, NULL);
	    			sigaction(SIGTSTP, &sa, NULL);
	    			sem_t *sem_ptr = shmat(sem_int, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
				if (str == (void*) -1) {
					printf("Shared memmory attach");
				}
				
				int l, counter = 0;
				int *shm_int = shmat(shmid_int, NULL, 0); //zwroc adres pamieci dzielonej, -1 porazka
				while (1) {
					sem_wait(sem_ptr+3);
					
					//sekcja krytyczna
					
					printf("Proces 3 (%d) w danej lini znajduje sie %d znakow \n", getpid(), *shm_int);
					//koniec sekcji krytycznej
					
					int wartoscsem;
					sem_getvalue(sem_ptr+4, &wartoscsem);
					if (wartoscsem == 0) { //koniec pliku
						//sem_post(&semaphore[3]);
						sem_post(sem_ptr+4);
						//sem_post(&semaphore[1]);
						
					}
					else {
						//sem_post(&semaphore[3]);
						//sem_post(sem_ptr+4);
						
						sem_post(sem_ptr+1);
						
					}
				
				}
			}
			exit(0);
		case -1:
			printf("Blad funkcji fork\n");
			exit(1);
		default:
		/*Proces macierzysty*/
		pid_tab[k] = pid;/*zapisz id procesu potomnego*/
		
		shared_mem_ptr -> P[k]=pid;
		}
	    }
    	/*Proces macierzysty*/
    	
    	//printf ("\n\n\n\n jestem PM. Pidy innych:	\n%i\n%i\n%i\n",shared_mem_ptr -> P[0],shared_mem_ptr -> P[1],shared_mem_ptr -> P[2]);
    	shared_mem_ptr -> flaga_m=1;
    	
    	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO|SA_RESTART;
	sa.sa_sigaction = handle_sigparent; //sighandler
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGCONT, &sa, NULL);
	sigaction(SIGTSTP, &sa, NULL);
    	int a;
    	for (a = 0; a < 3; a++) {		//wypisz pid-y
    		printf("P%d pid=%d ", a+1, pid_tab[a]);
    	}
    	printf("\n");
	while (1) {
		sem_wait(sem_ptr+0);
		
		//sekcja krytyczna
		printf("Proces macierzysty %d\n",  getpid());
		//koniec sekcji krytycznej
		
		sem_post(sem_ptr+1);
		
	}
	wait(NULL);
	wait(NULL);
	wait(NULL);
	//kontrola operacji
	if (shmdt(str) == -1) { //odlacz pamiec dzielona
		printf("shmdt");
		return 1;
	}
	if (shmctl(shmid, IPC_RMID, 0) == -1) {//zniszcz pamiec
		printf("shmctl");
		return 1;
	}
return 0;
}