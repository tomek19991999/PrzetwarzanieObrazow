#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#define MAXSTRLEN 100
#define DEKLARACJATABLICY tab = malloc(dlugosc_tablicy * sizeof(*tab))

struct obraz {
	char nazwa[MAXSTRLEN];
	int szerokosc;
	int wysokosc;
	int maksWartoscSzarosci;
	int standard; //2 to P2 (ASCII), 5 to P5 (binary)
	
	int* tablicaPikseli;
}; typedef struct obraz obraz;

int WczytajLiczbe(FILE* plik);
int CzytajPlik(FILE* plik, obraz* wczytObraz);
void WydrukujObraz(obraz* ObrazDoDruku);
int WczytajKonsola(char komunikat[], int granica_dolna, int granica_gorna, int* kontrolka);

int main(int argc, char* argv[]) {
	char nazwa[MAXSTRLEN];
	int krok_programu = 0;
	int wyswietl = 0;
	obraz NowyObraz;

	while (1)
	{
		switch (krok_programu)
		{
		case 0:

			printf("Podaj nazwe pliku ktory chcesz otworzyc lub wpisz 'exit' zeby wyjsc\n");
			while (!scanf("%s", nazwa))
			{
				char c;
				while (c = getchar() != '\n');
			}
			if (!strcmp(nazwa, "exit")) {
				krok_programu = 3;
				continue;
			}
			krok_programu = 1;
			break;

		case 1:
			printf("Wczytuje plik %s\n", nazwa);
			strcpy(NowyObraz.nazwa, nazwa);

			if (WczytajPlik(&NowyObraz))
			{
				krok_programu = 0;
				continue;
			}
			krok_programu = 2;
			break;

		case 2:
			if (WczytajKonsola("Czy chcesz wyswietlic wczytany obraz?\n", 0, 1, &wyswietl))
			{
				krok_programu = 0;
			}

			if (wyswietl)
			{
				WydrukujObraz(&NowyObraz);
			}
			else {
				krok_programu = 0;
			}
			break;
		case 3:
			return 0;
		}
	}
}

int WczytajPlik(obraz* wczytObraz) {
	int test = 0;
	int status_skanu = 0;
	fpos_t pozycja_wskaznika = 0;
	FILE* plik = NULL;

	system("cls");

	fopen_s(&plik, wczytObraz->nazwa, "r");
	if (plik == NULL) {
		printf("Blad przy otwieraniu pliku.\n");
		return 2;
	}
	if ((test = CzytajPlik(plik, wczytObraz)) != 8) {
		printf("\nBlad odczytu danych z pliku\nKod Bledu: %d\n", test);
		return -1;
	}

	fclose(plik);

	printf("Wczytano plik %s\n", wczytObraz->nazwa);

	return 0;
}

int CzytajPlik(FILE* plik, obraz* wczytObraz) {
	int krok_wczytania = 1;
	int poprzedni_krok = 0;
	int koniec_odczytu = 0;
	int i = 0;
	int j = 0;
	char znak;
	int* tab = NULL;
	int dlugosc_tablicy = 0;
	char liczba[MAXSTRLEN];

	while (!feof(plik)) {
		if ((znak = fgetc(plik)) > 32 || (znak == '\n' && krok_wczytania == 0)) {
			if (znak == '#') {
				if (krok_wczytania != 0)
				{
					poprzedni_krok = krok_wczytania;
				}
				krok_wczytania = 0;
			}
			switch (krok_wczytania) {
			case 0://tryb komentarza
				if (znak == '\n') {
					krok_wczytania = poprzedni_krok;
					continue;
				}
				break;
			case 1://SprawdŸ pierwszy element standardu - sposób kodowania
				if (znak == 'P' || znak == 'p') {
					krok_wczytania = 2;
					continue;
				}
				else {
					printf("\nNie znaleziono standardu kodowania obrazu (powinien sie znalezc na pierwszym miejscu w pliku)\n");
					return krok_wczytania;
				}
				break;
			case 2://SprawdŸ czy numer standardu kodowania jest prawid³owy...
				if (znak - 48 != 2) {
					printf("\nNiepoprawny standard kodowania - obraz musi byc zakodowany w standardzie P2.\n");
					return krok_wczytania;
				}
				else {
					wczytObraz->standard = znak - 48;
					krok_wczytania = 3;
					continue;
				}
				break;
			case 3://Wczytaj szerokosc obrazu...
				i = WczytajLiczbe(plik, znak);
				if (i > 0) {
					wczytObraz->szerokosc = i;
					krok_wczytania = 4;
				}
				else {
					if (!i)
					{
						printf("\nSzerokosc obrazu nie moze byc 0\n");
					}
					return krok_wczytania;
				}
				i = 0;
				break;
			case 4://Wczytaj wysokosc obrazu...
				i = WczytajLiczbe(plik, znak);
				if (i > 0) {
					wczytObraz->wysokosc = i;
					krok_wczytania = 5;
				}
				else {
					if (!i)
					{
						printf("\nWysokosc obrazu nie moze byc 0\n");
					}
					return krok_wczytania;
				}
				i = 0;
				break;
			case 5://Wczytaj maksymalny odcien szarosci...
				i = WczytajLiczbe(plik, znak);
				if (i >= 0) {
					wczytObraz->maksWartoscSzarosci = i;
					krok_wczytania = 6;
				}
				else {
					return krok_wczytania;
				}
				i = 0;
				j = 0;
				break;
			case 6:
				dlugosc_tablicy = wczytObraz->szerokosc * wczytObraz->wysokosc < INT_MAX ? wczytObraz->szerokosc * wczytObraz->wysokosc : 0;

				if (!dlugosc_tablicy) {
					printf("\nPrzekroczenie maksymalnej wartosci zmiennej typu int\n");
					return krok_wczytania;
				}

				DEKLARACJATABLICY;
				if (!tab) {
					printf("\nBlad przy przydzielaniu pamieci do tablicy.\n");
					return krok_wczytania;
				}

				krok_wczytania = 7;
				//fseek(plik, -1, SEEK_CUR);
			case 7://Wczytaj tablice pikseli...

				if (j >= wczytObraz->szerokosc) {
					i++;
					j = 0;
				}

				if (i < wczytObraz->wysokosc) {
					int test = 0;
					*(tab + i * wczytObraz->szerokosc + j) = (((test = WczytajLiczbe(plik, znak)) >= 0) && (test <= wczytObraz->maksWartoscSzarosci)) ? test : -2;

					if (*(tab + i * wczytObraz->szerokosc + j) < 0) {
						printf("\nBlad wartosci pikseli w pliku obrazu.\n");
						return krok_wczytania;
					}
					j++;
					continue;
				}

				krok_wczytania = 8;
				continue;
			case 8:
				printf("\nNiepoprawna tablica pikseli w pliku - niejezdoznacznosc wysokosci i szerokosci z podanymi danymi.\n");
				return 0;
			}
		}
	}
	if (i == wczytObraz->wysokosc - 1 && j == wczytObraz->szerokosc)
	{
		wczytObraz->tablicaPikseli = tab;
		krok_wczytania = 8;
		printf("\nPoprawnie wczytano tablice o szerokosci %d i wysokosci %d\n", wczytObraz->szerokosc, wczytObraz->wysokosc);
	}
	else {
		printf("\nNiepoprawna tablica pikseli w pliku - niejezdoznacznosc wysokosci i szerokosci z podanymi danymi.\n");
		return krok_wczytania;
	}
	return krok_wczytania;
}

int WczytajLiczbe(FILE* plik, char cyfra) {
	int i = 0;
	char liczba[MAXSTRLEN];
	int liczbaInt = -2;

	do {
		if (i < MAXSTRLEN - 1) {
			liczba[i] = cyfra;
			i++;
		}
		else {
			printf("\nPrzekroczono dlugosc bufora odczytu\n");
			return -4;
		}
	} while ((cyfra = fgetc(plik)) - 48 <= 9 && cyfra - 48 >= 0);
	
	//	fseek(plik, -1, SEEK_CUR);
	//}

	if (i){
		liczba[i] = '\0';
	}
	else {
		printf("\nBlad przy pobieraniu danej liczbowej.\n");
		return -1;
	}


	if (!(liczbaInt = strtol(liczba, NULL, 10) <= INT_MAX ? (int)strtol(liczba, NULL, 0) : 0) && strcmp("0", liczba)) {
		printf("\nBlad przy pobieraniu danej liczbowej.\n");
		return -3;
	}

	return liczbaInt;
}

void WydrukujObraz(obraz* ObrazDoDruku) {
	int i = 0;
	int j = 0;

	system("cls");
	/*printf("\nTablica pikseli obrazu:\n");
	for (i = 0; i < ObrazDoDruku->wysokosc; i++) {
		for (j = 0; j < ObrazDoDruku->szerokosc; j++) {
			printf("%d ", (ObrazDoDruku->tablicaPikseli)[i * ObrazDoDruku->szerokosc + j]);
		}
		printf("\n");
	}*/
	//Eksperymentalne
	printf("\nTablica pikseli obrazu eksperymentalna:\n");
	for (i = 0; i < ObrazDoDruku->wysokosc; i++) {
		for (j = 0; j < ObrazDoDruku->szerokosc; j++) {
			char c;
			int wartosc;
			switch ((int) (((float)(ObrazDoDruku->tablicaPikseli)[i * ObrazDoDruku->szerokosc + j] / (float)ObrazDoDruku->maksWartoscSzarosci) * 10)) {
			case 10:
				c = '@';
				break;
			case 9:
				c = '@';
				break;
			case 8:
				c = '%';
				break;
			case 7:
				c = '#';
				break;
			case 6:
				c = '*';
				break;
			case 5:
				c = '+';
				break;
			case 4:
				c = '=';
				break;
			case 3:
				c = '-';
				break;
			case 2:
				c = ':';
				break;
			case 1:
				c = '.';
				break;
			case 0:
				c = ' ';
			}
			printf("%c", c);
		}
		printf("%d\n", i);
	}
}

int WczytajKonsola(char komunikat[], int granica_dolna, int granica_gorna, int* kontrolka) {
	char c;

	printf("\n %s \nWcisnij ',' aby powrocic do menu\nZakres: %d do %d\n", komunikat, granica_dolna, granica_gorna);
	while (scanf("%d", kontrolka) == 0 || (*kontrolka < granica_dolna || *kontrolka > granica_gorna)) {
		while ((c = getchar()) != '\n') {
			if (c == ',') {
				printf("\nWracam do menu\n");
				return -1;
			}
		}
		printf("Podaj prawidlowa cyfre z odpowiedniego zakresu\n");
	}

	return 0;
}