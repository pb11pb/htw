#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lendlibin.h"
#include "lendlibout.h"
#include "lendlibitem.h"

#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#define max(X,Y) ((X) > (Y) ? (X) : (Y))

const char *filename = "../lendlib.csv";

/**
 * @ingroup LendLibIn
 * @brief Liest Verleihmediendaten aus Datei aus
 */
void readfile(){
	libprint(status, "Datei '%s' wird geöffnet.", filename);
#ifndef CGI
	filename = "lendlib.csv";	/// Je nach Terminal- oder CGI-Anwendung ist der Dateipfad anders
#endif
	libdb = fopen(filename,"rt");	///- Datei öffnen
	if (libdb){
 		int listSize = getSize(libdb);
		int i;
		for (i=0 ; i < listSize; i++){	///- Datei Zeilenweise durchgehen und als Medium in Liste einfügen
// 			libprint(out, "Lese Zeile: %d ", i);
			medium *newMedium = createItemF(libdb);
			insertItem( newMedium, &myLib );
		}
	} else{
		libprint(error, "Datei konnte nicht geöffnet werden.");
	}
}

/**
 * @ingroup LendLibIn
 * @brief Gibt Größe der gespeicherten CSV-Datenbank aus
 * @param libdb Eine Datei
 * @return Anzahl von Linien (also Einträgen)
 * 
 * Quelle: http://stackoverflow.com/a/1910795
 */
int getSize(FILE *libdb){
	int c = EOF, c2, n = 0;
	do{	///- Anzahl der Zeilenumbrüche zählen
		c2 = c;
		c = fgetc(libdb);
		if(c == '\n')
			n++;
	} while (c != EOF);
	///- Im Fall, dass die letzte Zeile keinen Umbruch enthält oder leer ist (bis auf den Umbruch), wird diese dazu gezählt oder eben nicht
	if (c != '\n' && n != 0 && c2 != '\n'){
		n++;
	} 	
	rewind(libdb);
	return n;
}

/**
 * @ingroup LendLibIn
 * @brief Verarbeitet Nutzerinput im Terminal
 */
void getInput(){
	libprint(in, "Wähle Funktion (h für Hilfe): ");
	fgets(vbuf, 128, stdin);
	switch (vbuf[0]){
		case 'h':	/// Hilfezeilen mit Anweisungen anzeigen
			libprint(out, "  Mögliche Funktionen:");
			libprint(out, "l - Ausleihmedien erneut auflisten");
			libprint(out, "c - Ausleihmedium hinzufügen");
			libprint(out, "d - Ausleihmedium löschen");
			libprint(out, "s - Ausleihmedien sortieren");
			libprint(out, "f - Ausleihmedien finden");
			libprint(out, "alle anderen Eingaben beenden das Programm");
			printTLine('~',0); printf("\n");
			getInput();
			break;
		case 'l':	/// Dateien noch einmal auflisten
			printItems();
			getInput();
			break;
		case 'c':	/// Ausleihmedium hinzufügen
			libprint(in, "%16s: ", "Medienart");	///- Eingabe Medienart
			fgets(vbuf, 128, stdin);
			int type = vbuf[0];
			mType ntype;
			char *ntitle, *nauthor, *nlendee;
			///- (Festlegung der Medienart durch entsprechende Zahl aus mType oder Anfangsbuchstabe: Buch, Cd, Dvd)
			switch (type){
				case 'b':
				case 'B':
				case 1:
					libprint(out, "Medium ist Buch.");
					ntype = book;
					break;
				case 'c':
				case 'C':
				case 2:
					libprint(out, "Medium ist CD.");
					ntype = cd;
					break;
				case 'd':
				case 'D':
				case 3:
					libprint(out, "Medium ist DVD.");
					ntype = dvd;
					break;
				default:
					libprint(out, "Medium ist anderer oder unbestimmt.");
					ntype = other;
					break;
			}
			///- (Titel, Interpret/Autor und Ausleihender eingeben lassen) und jeweils in freigegebenen Speicher schreiben
			libprint(in, "%16s: ", "Medientitel");
			fgets(vbuf, 128, stdin);
			while (strlen(vbuf) < 2){
				libprint(out, "Bitte einen Medientitel angeben!");
				libprint(in, "%16s: ", "Medientitel");
				fgets(vbuf, 128, stdin);				
			}	
			vbuf[strlen(vbuf)-1]=0;
			ntitle = malloc (strlen(vbuf)+1);
			if ( ntitle == NULL){
				libprint(error, MALLOCERR);
				exit(-1);
			}
			strcpy(ntitle, vbuf);
			libprint(out, "Titel ist '%s'.", ntitle);
			libprint(in, "%16s: ", "Interpret/Autor");
			fgets(vbuf, 128, stdin);	
			vbuf[strlen(vbuf)-1]=0;
			nauthor = malloc (strlen(vbuf)+1);
			if ( nauthor == NULL){
				libprint(error, MALLOCERR);
				exit(-1);
			}
			strcpy(nauthor, vbuf);
			if (strlen(vbuf) != 0)
				libprint(out, "Interpret/Autor ist '%s'.", nauthor);
			libprint(in, "%16s: ", "Ausgeliehen an");
			fgets(vbuf, 128, stdin);
			while (strlen(vbuf) < 2){
				libprint(out, "Bitte Person angeben, an die das Medium ausgeliehen wurde!");
				libprint(in, "%16s: ", "Ausgeliehen an");
				fgets(vbuf, 128, stdin);				
			}
			vbuf[strlen(vbuf)-1]=0;
			nlendee = malloc (strlen(vbuf)+1);
			if ( nlendee == NULL){
				libprint(error, MALLOCERR);
				exit(-1);
			}
			strcpy(nlendee, vbuf);
			libprint(out, "Ausgeliehen an '%s'.", nlendee);
			
			///- Eingegebene Daten als Medium speicher und in Liste einfügen
			medium *newMedium = createItem(ntype, ntitle, nauthor, nlendee);
			insertItem( newMedium, &myLib );
			printTLine('~',0); printf("\n");			
			printItems();
			
			getInput();
			break;
		case 's':	/// Medien sortieren, dazu nach Sortierkategorie fragen
			libprint(in, "Sortieren nach (\e[1mT\e[0m\e[32mitel=0, \e[1mA\e[0m\e[32mus\e[1mL\e[0m\e[32meihender=1): ");
			fgets(vbuf, 128, stdin);
			int sortingBy = vbuf[0];
			printTLine('~',0); printf("\n");
			switch (sortingBy){
				case 'a':
				case 'A':
				case 'l':
				case 'L':
				case '1':
					sortItems(1, &myLib);
					printItems();
					break;
				case 't':
				case 'T':
				case '0':
					sortItems(0, &myLib);
					printItems();
				default:
					break;				
			}
			getInput();
			break;
		case 'd':	/// Medium löschen, dazu ID abfragen
			libprint(in, "Medium löschen (ID): ");
			fgets(vbuf, 128, stdin);
			deleteItem(atoi(vbuf), &myLib);
			printTLine('~',0); printf("\n");
			printItems();
			getInput();
			break;
		case 'f':	/// Medien finden, dazu nach Suchkategorie (Titel oder Ausleihender) und der zu suchenden Zeichenkette fragen
			libprint(in, "Medium suchen nach (\e[1mT\e[0m\e[32mitel=0, \e[1mA\e[0m\e[32mus\e[1mL\e[0m\e[32meihender=1): ");
			fgets(vbuf, 128, stdin);
			int fBy = vbuf[0];
			libprint(in, "Nach dieser Zeichenkette durchsuchen: ");
			char* fItem = fgets(vbuf, 128, stdin);
			printTLine('~',0); printf("\n");
			switch (fBy){
				case 'a':
				case 'A':
				case 'l':
				case 'L':
				case '1':
					findItem(fItem, 1, &myLib);
					break;
				case 't':
				case 'T':
				case '0':
					findItem(fItem, 0, &myLib);
				default:
					break;				
			}
			getInput();
			break;
		default: /// Ansonsten Programm beenden
			break;
	}
}

/**
 * @ingroup LendLibIn
 * @brief Verarbeitet POSTs des CGI
 * 
 * Der POST wird jeweils mit ::strtok2 auseinandergeschnitten und verarbeitet. Die vorgehensweise ist ähnlich wie bei der Terminaleingabe.
 */
void getPost(){
	fgets(posted,256,stdin);	/// POST einlesen
#ifdef DEBUG
	printf("Querry: %s", posted);
#endif
	switch (posted[0]){	/// POSTs unterscheiden sich durch den Anfangsbuchstaben (wie bei Terminaleingabe)
		case 's':	///- Sucheingabe verarbeiten
// 			printf("<p>sortieren</p>");
			printf("<h2>Ausgeliehene Medien: </h2>\n");
			if(posted[1] == 'l'){
// 				printf("<p>Sortiert nach Ausleihenden</p>");
				sortItems(1, &myLib);
				printItems();
			} else {
// 				printf("<p>Sortiert nach Titel</p>");
				sortItems(0, &myLib);
				printItems();
			}
			break;
		case 'f':	///- Etwas finden
// 			printf("<p>finden</p>");
			;
			strtok2(posted, "=");
			int fType = (strcmp(strtok2(NULL, "&"),"ti") == 0? 0 : 1 );
// 			printf("Typ: %d<br>", fType);
			char* tmpf = strtok2(NULL, "=");
			if ((tmpf+4)[0] != 0){
				char* fStr = strtok2(NULL, "=");
				printf("<h2>Ausgeliehene Medien: </h2>\n");
				findItem(fStr, fType, &myLib);
			} else {
				printf("<h2>Ausgeliehene Medien: </h2>\n");
				libprint(error, "Keinen Suchbegriff eingegeben.");
				currLib = &myLib;
				printItems();
			}
			break;
		case 'c':	///- Medium einfügen
// 			printf("<p>hinzufügen</p>");
			;
			strtok2(posted, "=");
			int cType;
			char* tmpc = strtok2(NULL, "&");
#ifdef DEBUG
			printf("<p> Medium: '%1s'</p>", tmpc);
#endif
			if ( strcmp(tmpc,"bu") == 0 ){
				cType = 1;
			}else if ( strcmp(tmpc,"cd") == 0 ){
				cType = 2;
			}else if ( strcmp(tmpc,"dv") == 0 ){
				cType = 3;
			}else {
				cType = 0;
			}		
// 			printf("Typ: %d<br>", cType);
			strtok2(NULL, "=");
			tmpc = strtok2(NULL, "&");
#ifdef DEBUG
			printf("<p> Titel: '%1s'", tmpc);
#endif
// 			printf("'%1s': char: '%1c', dezi: '%d'",tmpc,(tmpc)[0], (tmpc)[0]);				
			if (tmpc[0] != 0){	///- (Funktioniert wie bei Terminaleingabe: Es wird jeweils Speicher bereitgestellt und Angaben darin geschrieben.
				replaceSpecial(tmpc, cTitle);
#ifdef DEBUG
				printf(" / '%1s'</p>", cTitle);
#endif
// 				strcpy(cTitle, tmpc);
// 				printf("%s", fStr);
				strtok2(NULL, "=");
				tmpc = strtok2(NULL, "&");
#ifdef DEBUG
				printf("<p> Interpret/Autor: '%1s'", tmpc);
#endif
				if (tmpc[0] == 0){	
					tmpc = "";
				}
				replaceSpecial(tmpc, cAuthor);
#ifdef DEBUG
				printf(" / '%1s'</p>", cAuthor);
#endif
// 				strcpy(cAuthor, tmpc);
				strtok2(NULL, "=");
				tmpc = strtok2(NULL, "=");
#ifdef DEBUG
				printf("<p> Ausgeliehen an: '%1s'", tmpc);
#endif
				if (tmpc[0] != 0){
					replaceSpecial(tmpc, cLendee);
#ifdef DEBUG
					printf(" / '%1s'</p>", cLendee);
#endif
// 					strcpy(cLendee, tmpc);
					printf("<h2>Ausgeliehene Medien: </h2>\n");
// 					printf("%d, %s, %s, %s", cType, cTitle, cAuthor, cLendee);
					medium *newMedium = createItem(cType, cTitle, cAuthor, cLendee);
					insertItem( newMedium, &myLib );
					printItems();
				} else {
					printf("<h2>Ausgeliehene Medien: </h2>\n");
					libprint(error, "Keinen Ausleihenden eingegeben.");
					printItems();					
				}
			} else {
				printf("<h2>Ausgeliehene Medien: </h2>\n");
				libprint(error, "Keinen Titel eingegeben.");
				printItems();
			}
			break;
		case 'd':///- Löschen
// 			printf("<p>löschen</p>");
			;
			int dID = atoi(posted+4);
// 				printf("<p>ID: %d</p>", dID);
			deleteItem(atoi(posted+4), &myLib);
			printf("<h2>Ausgeliehene Medien: </h2>\n");
			printItems();
			break;
		default:
			printf("<h2>Ausgeliehene Medien: </h2>\n");
			printItems();
			break;
	}
}

/** @ingroup LendLibIn
 * @brief Ersetz '+' mit Leerzeichen und andere Sonderzeichen in HTML-Eingabe
 * @param inS Zeichenkette mit + und Sonderzeichen
 * @param outS Zeichenkette, in die die Eingabe mit Ersetzung geschrieben werder soll 
 */
void replaceSpecial(char *inS, char* outS){
	int i;
	char sp[3];
	sp[2] = 0;
	char* no;
// 	printf("<p>");
	for (i=0; *inS!=0; inS++, i++) {
// 		printf(" %d ", i);
		switch (*inS){
			case '+': 
				outS[i]=' '; 
				break;
			case '%': 
				inS++; 
				sp[0] = inS[0];
				sp[1] = inS[1];
				outS[i]=(char)strtol(inS,&no,16);
				inS += 2;
// 				printf("<br>%s (%c) | ", sp, outS[i]);
				inS--;
				break;
			default: 
				outS[i]=*inS; 
				break;
		}
	}
// 	printf(" (%d)</p>", i);
	outS[i]=0;
}



/**
 * @ingroup LendLibIn
 * @brief Eigenes strtok, welches auch leere Einträge ausgibt.
 * @param str Der String
 * @param delims Die Trennzeichen
 * Quelle: http://stackoverflow.com/a/8706031
 */
char* strtok2 (char * str, char const * delims){
  static char  * src = NULL;
  char  *  p,  * ret = 0;

  if (str != NULL)
    src = str;

  if (src == NULL)
    return NULL;

  if ((p = strpbrk (src, delims)) != NULL) {
    *p  = 0;
    ret = src;
    src = ++p;

  } else if (*src) {
    ret = src;
    src = NULL;
  }

  return ret;
}