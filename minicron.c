#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <time.h>

char *tab[10];
int licznik = 0;
int hour_s, minute_s, hour_e, minute_e; // czas z pliku
int wypisz_plik(const char *nazwa)
{
        FILE *stream;
        stream = fopen(nazwa, "r");
        if (stream == NULL)
        {
                printf("Plik NIE istnieje");
                // return 1;
        }
        char line[100];
        fgets(line, sizeof(line), stream);
        sscanf(line, "%d:%d;%d:%d", &hour_s, &minute_s, &hour_e, &minute_e);
        printf("%d:%d;%d:%d\n", hour_s, minute_s, hour_e, minute_e);
        while (fgets(line, sizeof(line), stream) != NULL)
        {
                licznik++;
        }
        fseek(stream, 0, SEEK_SET);
        char *command[licznik];
        fgets(line, sizeof(line), stream);
        for (int i = 0; i < licznik; i++)
        {
                fgets(line, sizeof(line), stream);
                command[i] = strdup(line);
                tab[i] = strdup(line);
        }
        for (int i = 0; i < licznik; i++)
        {
                printf("%s", command[i]);
        }
        fclose(stream);
        return licznik;
}
void handler(int signum)
{
        printf("signal!\n");
        syslog(LOG_NOTICE, "otrzymano sygnał SIGINT");
        exit(signum);
}
int main(int argc, char **argv)
{
        srand(time(NULL));
        if (argc < 2)
        {
                printf("Uzycie pliku o nazwie: %s\n", argv[0]);
                return EXIT_FAILURE;
        }
        for (int i = 1; i < argc; i++)
        {
                printf("zawartosc pliku %s: \n", argv[i]);
                wypisz_plik(argv[i]);
                // przeliczanie czasu z pliku
                int sekundy = 0;
                sekundy += (hour_e - hour_s) * 60;
                if (minute_e < minute_s && hour_e > hour_s)
                {
                        sekundy += 60 * (60 - minute_s + minute_e);
                }
                else
                        sekundy += 60 * (minute_e - minute_s);
                printf("Sekundy %d\n", sekundy);
                if (10 * licznik > sekundy)
                {
                        printf("Za krótki czas");
                        return 0;
                }
                // start procesu
                pid_t pid, sid;
                pid = fork();
                if (pid < 0)
                {
                        exit(EXIT_FAILURE);
                }
                if (pid > 0)
                {
                        exit(EXIT_SUCCESS);
                }
                umask(0);
                sid = setsid();
                if (sid < 0)
                {
                        exit(EXIT_FAILURE);
                }
                if ((chdir("/")) < 0)
                {
                        exit(EXIT_FAILURE);
                }
                // close(STDIN_FILENO);
                // close(STDOUT_FILENO);
                // close(STDERR_FILENO);
                /*  time_t czas;
                  struct tm *aktualny;
                  time(&czas);
                  aktualny = localtime(&czas);

                          int godzina_a = aktualny->tm_hour;
                          int minuta_a = aktualny->tm_min;*/
                // while()
                while (1)
                {
                        time_t czas;
                        struct tm *aktualny;
                        time(&czas);
                        aktualny = localtime(&czas);
                        int godzina_a = aktualny->tm_hour;
                        int minuta_a = aktualny->tm_min;

                        // printf("%d : %d\n",godzina_a,minuta_a);
                        // printf("%s",asctime(aktualny));
                        if (hour_s == godzina_a && minute_s == minuta_a)
                        {
                                printf("rozpoczęto program\n");
                                int pom = 0;
                                int *liczby = (int *)malloc(licznik * sizeof(int));
                                int j = 0;
                                for (int i = 0; i < licznik; i++)
                                        liczby[i] = -1;
                                int r = rand() % licznik;
                                while (j != licznik)
                                {
                                        int pom = 0;
                                        r = rand() % licznik;
                                        for (int k = 0; k <= j; k++)
                                        {
                                                if (liczby[k] == r)
                                                {
                                                        pom++;
                                                }
                                        }
                                        if (pom == 0)
                                        {
                                                liczby[j] = r;
                                                j++;
                                        }
                                }
                                for (int i = 0; i < licznik; i++)
                                        printf("%d\n", liczby[i]);
                                printf("start");

                                signal(SIGINT, handler);

                                FILE *plik;
                                plik = fopen("outfile.txt", "w");

                                for (int i = 0; i < licznik; i++) // zamiast while w demonie
                                {
                                        char *komenda = tab[i];
                                        // sizeof liczy razem ze znakiem końca linii \0 a strlen tylko ilosc znakow np. ala to w 1 bedzie 4 a w 2 bedzie 3
                                        int ile = sizeof(komenda);
                                        int mode;
                                        mode = (int)(komenda[ile - 1] - 48);
                                        printf("%d\n", mode);
                                        char polecenie[ile - 1];
                                        strncpy(polecenie, komenda, ile - 2); // działa i to i for
                                        /*for(int j=0; j<ile-2;j++){
                                                polecenie[j]=komenda[j];
                                               // printf("%c",polecenie[j]);
                                        }*/
                                        polecenie[ile - 2] = '\0';
                                        printf("\n%s\n", polecenie);
                                        switch (mode)
                                        {
                                        case 0:
                                                printf("\nTryb %d\n\n", mode);
                                                execlp("/bin/sh", "sh", "-c", polecenie, NULL); //"1>","/home/student/systemy/projekt",NULL);
                                                sleep(10);
                                                break;
                                        case 1:
                                                break;
                                        case 2:
                                                printf("\nTryb %d\n\n", mode);
                                                execlp("/bin/sh", "sh", "-c", polecenie, ">", plik, NULL); //"/home/student/systemy/projekt/outfile.txt", NULL);
                                                sleep(10);
                                                break;
                                        }
                                }

                                exit(EXIT_SUCCESS);
                                return 0;
                        }
                }
        }
}
