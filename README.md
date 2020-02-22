# Memory-Allocator
  A simple memory allocator in C. The allocation principle is based on an ’arena’ and it simulates mechanisms such as malloc, free or hexdump.
  
  Pentru gestionarea arenei am folosit 2 pointeri globali de tip char,
unul pentru a pastra adresa de inceput a arenei, iar celalalt pentru a
pastra adresa de inceput al primului bloc alocat in arena. De asemenea
dimensiunea arenei este tot o variabila globala.
	In implementarea temei am folosit urmatoarele functii:
		- init		  - primeste ca parametru dimensiunea arenei
					      - aloca memoria pentru arena si o initializeaza pe 0
		- finalize	- elibereaza memoria folosita pentru arena
					      - seteaza pointerii de arena si al primului bloc pe NULL
		- dump		  - afiseaza continutul arenei in formatul cerut
					      - la fiecare iteratie se afiseaza un rand (16 octeti sau
						      dimensiunea ramasa, daca aceasta este mai mica)
		- alloc		  - primeste ca parametru dimensiunea blocului ce se doreste
						      a fi alocat
					      - algoritmul implementat este urmatorul:
						1) se verifica daca nu este alocat nici un bloc in arena,
							daca nu este si de asemenea arena are spatiu suficient
							se va aloca blocul la inceputul arenei
						2) altfel se verifica daca exista spatiu intre inceputul
							arenei si primul bloc alocat, daca da se va aloca
							noul bloc incepand cu pozitia 0
						3) altfel se parcurg blocurile si se cauta un spatiu
							suficient intre 2 blocuri consecutive, daca este
							gasit, blocul nou se va aloca intre cele 2 blocuri
							intre care exista spatiu
						4) altfel se verifica daca exista spatiu dupa ultimul bloc,
							in caz afirmatic aici se va aloca noul bloc
		- Free		  - primeste ca parametru indexul de la care incepe zona de
						      date, a blocului ce se doreste a fi sters
					      - se cauta blocul precedent si urmator ale blocului de sters
					      - daca exista si precedent si urmator, se va reface legatura
						      intre ele
					      - daca exista doar blocul precedent, acesta se va marca ca
						      neavand bloc urmator
					      - daca exista doar bloc urmator, acesta se va marca ca neavand
						      predecesor
					      - daca blocul de sters este primul din arena, pointerul ce
						      indica primul bloc din arena va fi actualizat sa indice
						      blocul urmator
					      - se itereaza pe zona blocului si se seteaza pe 0
		- fill		  - primeste ca parametrii indexul la care incepe zona de date
						      a blocului pe care se doreste fill, numarul de octeti si
						      valoarea cu care se face fill
					      - se seteaza un numar de octeti (minimul dintre dimensiunea de date
						      a blocului si dimensiunea de octeti dorita pentru fill)
					      - daca dimensiunea dorita este mai mare decat cea a blocului
						      si exista un bloc urmator, se va apela functia recursiv
						      cu indexul zonei de date al blocului urmator
		- myrealloc	- primeste ca parametrii indexul zonei de date al blocului
						      ce se doreste a fi realocat si noua dimensiune a acestuia
					      - se copiaza zona de date a blocului (numarul minim de octeti
						      dintre dimensiunea deja existenta si noua dimensiune)
						      intr-o zona temporala
					      - se elibereaza blocul, folosind functia myfree
					      - se incearca alocarea unui nou bloc de dimensiunea primita,
						      folosind functia alloc, iar daca alocarea a avut succes
						      se copiaza datele din zona temporala in blocul nou alocat
