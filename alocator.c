/* Mirciu Andrei-Constantin */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MIN(a, b) (a < b ? (a) : (b))
#define ADM_AREA_SIZE (3 * sizeof(uint32_t))
char *first_block = NULL;
char *arena = NULL;
unsigned int arena_size;

void init(unsigned int size) {
    // alocare memorie pentru arena si setarea pe 0 folosind calloc
    arena = (char *) calloc(size, sizeof(char));
    // salvarea dimensiunii arenei in variabila globala
    arena_size = size;
}

void finalize() {
    // eliberarea memoriei arenei
    free(arena);
    // marcarea pointerului ce indica adresa de inceput a arenei cu
    //  o adresa invalida
    arena = NULL;
    // marcarea pointerului ce indica adresa de inceput a primului bloc din
    //  arena cu o adresa invalida
    first_block = NULL;
}

void dump() {
    uint32_t ind = 0;
    uint32_t i;

    // cat timp mai sunt randuri a cate 16 octeti de afisat
    while (ind < arena_size) {
        
        // indexul primului octet din rand in format hexa
        fprintf(stdout, "%08X\t", ind);

        // se parcurg 16 octeti sau restul randului daca nu mai
        //  exista 16 octeti in arena
        for (i = ind; i < MIN(ind + 16, arena_size); ++i) {
            // octetul in format hexa, cu 2 cifre
            fprintf(stdout, "%02X ", *(arena + i) & 0xff);
            // daca s-au afisat 8 octeti din randul curent se afiseaza un spatiu
            if (i - ind == 7)
                fprintf(stdout, " ");
        }

        // se trece la un rand nou
        fprintf(stdout, "\n");

        // se incrementeaza indexul cu dimensiunea randului
        ind += 16;
    }
}

int alloc(unsigned int size) {
    int result = 0;

    // daca nu exista nici un bloc alocat
    if (first_block == NULL) {
        // daca este spatiu pentru alocarea noului bloc
        if (arena_size >= size + ADM_AREA_SIZE) {
            // blocul nou alocat va fi primul din arena, nu va avea bloc
            //  precedent sau urmator
            first_block = arena;
            *((int*) first_block) = 0;
            *((int*) first_block + 1) = 0;
            // este marcata dimensiunea de date a blocului
            *((int*) first_block + 2) = size;   
            // rezultatul va fi indexul de inceput al zonei de date
            result = ADM_AREA_SIZE;
        }
    } else {    // exista deja blocuri alocate in arena

        // adresa blocului curent, intial identica cu a primului bloc
        int *p = (int *) first_block;

        // daca exista spatiu inaintea primului bloc alocarea se va face
        //  asemenea cazului in care arena este libera, cu mentiunea ca
        //  se modifica zonele de gestiune a blocului nou alocat si a celui
        //  de dupa el, pentru a marca legatura
        if ( (uint32_t) ((char *)p - arena) >= ADM_AREA_SIZE + size) {
            *((int*) arena) = first_block - arena;
            *((int*) arena + 1) = 0;
            *((int*) arena + 2) = size;
            *((int*) first_block + 1) = 0;
            first_block = arena;
            result = ADM_AREA_SIZE;
        } else {
            int block_len;
            char *new_block;

            // cat timp exista bloc urmator
            while (*p != 0) {
                // dimensiunea totala a blocului curent
                block_len = ADM_AREA_SIZE + *(p + 2);
                // adresa de inceput a blocului urmator
                char *next_block = ((char*) arena) + *p;

                // daca este spatiu intre blocul curent si cel urmator pentru alocarea ceruta
                if ( (uint32_t) (next_block - (char*) p - block_len) >= ADM_AREA_SIZE + size ) {

                    // noul bloc va incepe imediat dupa cel curent
                    new_block = (char*) p + block_len;

                    // blocul curent va avea ca bloc urmator pe cel nou alocat
                    *((int*) p) = new_block - arena;
                    // blocul nou alocat va avea ca bloc urmator blocul urmatoe celui curent
                    *((int*) new_block) = next_block - arena;
                    // blocul nou alocat va avea ca bloc precedent blocul curent
                    *((int*) new_block + 1) = (char*) p - arena;
                    // dimensiunea de date a blocului nou alocat
                    *((int*) new_block + 2) = size;
                    // blocul urmator blocului curent va avea ca bloc precedent blocul nou alocat
                    *((int*) next_block + 1) = new_block - arena;

                    // calcularea indexului din arena la care incepe zona de date a blocului
                    //  nou alocat; oprirea cautarii
                    result = new_block - arena + ADM_AREA_SIZE;
                    break;
                } else {
                    p = (int*) next_block;
                }
            }

            // daca nu a fost gasit spatiu suficient intre blocurile existente sau
            //  daca exista un singur bloc in arena => se aloca la finalul arenei
            if (0 == result) {
                
                // dimensiunea totala a blocului curent
                block_len = ADM_AREA_SIZE + *(p + 2);

                // daca exista spatiu in arena dupa blocul curent
                if ((uint32_t) (arena_size - ((char*) p - arena) - block_len) >= ADM_AREA_SIZE + size) {
                    // noul bloc va incepe imediat dupa cel curent
                    new_block = (char*) p + block_len;

                    // blocul curent va avea ca bloc urmator pe cel nou alocat
                    *((int*) p) = new_block - arena;
                    // blocul nou alocat nu va avea bloc urmator
                    *((int*) new_block) = 0;
                    // blocul nou alocat va avea ca bloc precedent blocul curent
                    *((int*) new_block + 1) = (char*) p - arena;
                    // dimensiunea de date a blocului nou alocat
                    *((int*) new_block + 2) = size;

                    // calcularea indexului din arena la care incepe zona de
                    //  date a blocului nou alocat
                    result = new_block - arena + ADM_AREA_SIZE;
                }
            }

        }

    }

    return result;
}

void Free(uint32_t index) {
    int i;
    // adresa de memorie a zonei de gestiune
    int *start = (int*) (arena + index - ADM_AREA_SIZE);
    // dimensiunea in octeti a intregului bloc
    int dim = *(start + 2) + ADM_AREA_SIZE;

    // adresa de memorie a blocului precedent
    char *prev = NULL;
    // adresa de memorie a blocului urmator
    char *next = NULL;   

    // daca exista un bloc urmator se memoreaza adresa acesutia
    if (*start != 0) {
        next = arena + *start;
    }

    // daca exista un bloc precedent se memoreaza adresa acesutia
    if (*(start + 1) != 0 || (char*) start != first_block ) {
        prev = arena + *(start + 1);
    }

    // daca blocul curent este primul, cel urmator va deveni primuk
    if ((char*) start == first_block) {
        first_block = next;
    }

    // daca exista doar bloc precedent, acesta va fi marcat ca neavand
    //  bloc urmator
    if (prev != NULL && NULL == next) {
        *((int*) prev) = 0;
    }

    // daca exista doar bloc urmator, acesta va fi marcat ca neavand
    //  bloc precedent
    if (NULL == prev && next != NULL) {
        *((int*) next + 1) = 0;
    }

    // daca exista si bloc urmator si precedent, precedentulului ii este marcat
    //  blocul urmator ca al 2-lea bloc dupa el, iar blocului urmator ii este
    //  marcat blocul precedent ca blocul precedent al blocului curent
    if (prev != NULL && next != NULL) {
        *((int*) prev) = (char*) next - arena;
        *((int*) next + 1) = (char*) prev - arena;
    }

    // setarea pe 0 a zonei de memorie a blocului (inclusiv zona de gestiune)
    for (i = 0; i < dim; ++i)
        *((char*)start + i) = 0;
}

void fill(uint32_t index, unsigned int size, char value) {
    uint32_t i;
    // adresa de memorie la care incepe zona de date
    char *data = arena + index;
    // adresa de memorie la care incepe zona de gestiune
    int *administration = (int*)data - 3;
    // dimensiunea zonei de date a blocului
    unsigned int dim = *(administration + 2);

    // setarea zonei de date
    // se seteaza ori dim octeti, ori size-ul cerut, daca acesta este mai mic
    //  decat dimensiunea de date a blocului
    for (i = 0; i < MIN(dim, size); ++i)
        *(data + i) = value;

    // numarul de octeti ce au ramas de setat in urmatoarele blocuri
    int new_size = size - MIN(dim, size);

    // daca exista bloc urmator si mai trebuie setati octeti
    if (new_size > 0 && *administration != 0) 
        // se apleleaza recursiv functia, actualizand indexul blocului
        //  ce trebuie setat
        fill(*administration + ADM_AREA_SIZE, new_size, value);
}


int myrealloc(unsigned int index, unsigned int size) {
    // lungimea zonei de date a blocului care se doreste a fi realocat
    unsigned int data_len = *( (int*) (arena + index) - 1);
    // numarul de octeti de date ce vor fi copiati
    unsigned int new_data_len = MIN(data_len, size);
    // copierea zonei de date intr-o zona de memorie temporala
    char *tmp = malloc(new_data_len);
    memcpy(tmp, arena + index, new_data_len);
    // eliberarea blocului
    Free(index);
    // alocarea unui nou bloc cu dimensiunea ceruta
    int new_index = alloc(size);
    if (new_index != 0)
        // daca alocarea a reusit, se copiaza datele din zona temporala in noul bloc
        memcpy(arena + new_index, tmp, new_data_len);
    // eliberarea zonei temporale si intoarcerea indexului de date al blocului realocat
    free(tmp);
    return new_index;
}

void parse_command(char* cmd)
{
    const char* delims = " \n";

    char* cmd_name = strtok(cmd, delims);
    if (!cmd_name) {
        goto invalid_command;
    }

    if (strcmp(cmd_name, "INITIALIZE") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        init(size);

    } else if (strcmp(cmd_name, "FINALIZE") == 0) {
        finalize();

    } else if (strcmp(cmd_name, "DUMP") == 0) {
        dump();

    } else if (strcmp(cmd_name, "ALLOC") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        fprintf(stdout, "%d\n", alloc(size));

    } else if (strcmp(cmd_name, "FREE") == 0) {
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        Free(index);

    } else if (strcmp(cmd_name, "FILL") == 0) {
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        char* value_str = strtok(NULL, delims);
        if (!value_str) {
            goto invalid_command;
        }
        uint32_t value = atoi(value_str);
        fill(index, size, value);

    } else if (strcmp(cmd_name, "ALLOCALIGNED") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        ///uint32_t size = atoi(size_str);
        char* align_str = strtok(NULL, delims);
        if (!align_str) {
            goto invalid_command;
        }
        ///uint32_t align = atoi(align_str);
        // TODO - ALLOCALIGNED
        goto invalid_command;

    } else if (strcmp(cmd_name, "REALLOC") == 0) {
        printf("Found cmd REALLOC\n");
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        myrealloc(index, size);
        
    } else {
        goto invalid_command;
    }

    return;

invalid_command:
    printf("Invalid command: %s\n", cmd);
    exit(1);
}

int main(void)
{
    ssize_t read;
    char* line = NULL;
    size_t len;

    /* parse input line by line */
    while ((read = getline(&line, &len, stdin)) != -1) {
        /* print every command to stdout */
        printf("%s", line);

        parse_command(line);
    }

    free(line);

    return 0;
}
