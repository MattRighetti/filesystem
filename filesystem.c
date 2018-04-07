/**
*	ALGORITHMS AND DATA STRUCTURES PROJECT
*	
*	Software developed by
*	MATTIA RIGHETTI, student at Politecnico Di Milano
*	
*	
*													
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENTRIES 1024
#define MAX_NAME 255
#define MAX_PATH 255

#define TYPE_FILE 1
#define TYPE_DIR 2

#define D(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

struct treeNode {
    char* stringa;
    struct treeNode* right;
    struct treeNode* left;
};

struct entry {
	int type;
	void *ptr;
	char name[MAX_NAME];
	struct entry *next;
};

static struct directory {
	int nitems;	
	struct entry * entries[MAX_ENTRIES];
} root;

/**
 * Prints in lexicographical order what's in the tree
 *
 * @param pointer to root of tree
 */
void in_order(struct treeNode *root) {
    if (root != NULL) {
        in_order(root->left);
        printf("ok %s\n", root->stringa);
        in_order(root->right);
    }
}

/**
 * Destroys the tree
 *
 * @param pointer to root of tree
 */
void delete_tree(struct treeNode** leaf) {
    if (*leaf != NULL) {
        delete_tree(&(*leaf)->left);
        delete_tree(&(*leaf)->right);
		free((*leaf)->stringa);
		(*leaf)->stringa = NULL;
		free((*leaf));
		*leaf = NULL;
    }
}

/**
 * Inserts path in the tree
 *
 * @param double pointer to tree
 * @param string path
 */
void insert_in_tree(char* key, struct treeNode** leaf) {
    int res;
    if (*leaf == NULL) {
        *leaf = (struct treeNode*) malloc(sizeof(struct treeNode));
        (*leaf)->stringa = malloc(strlen(key)+1);
        strcpy ((*leaf)->stringa, key);
        (*leaf)->left = NULL;
        (*leaf)->right = NULL;
    } else {
        res = strcmp(key, (*leaf)->stringa);
        if( res < 0)
            insert_in_tree( key, &(*leaf)->left);
        else if( res > 0)
            insert_in_tree( key, &(*leaf)->right);
    }
}

/**
 * Calcola il codice hash della stringa passata come parametro (algoritmo java String)
 * 
 * @param s stringa di cui calcolare l'hash code
 * @return codice hash
 */
unsigned int hashcode_string(char *s) {
	unsigned int count = 0;
	while (*s)
		count = count * 31 + *s++;
	return count;
}

/**
 * Seaches for a child in the given directory
 * 
 * @param dir directory in which to search 
 * @param name to be found
 * @return pointer to found elem if found, NULL otherwise
 */
struct entry * find_entry(struct directory *dir, char *name) {
	unsigned int hash = hashcode_string(name) % MAX_ENTRIES;
	
	struct entry *ptr = dir->entries[hash]; 

	while (ptr) {
		if (!strcmp(ptr->name, name)) // found
			return ptr;
		ptr = ptr->next; // next list position
	}
	
	return NULL;
}

/**
 * Adds elem to specified directory
 * 
 * @param dir directory where to add
 * @param type of elem to add
 * @param name of the elem to add
 * @param ptr to data held by elem
 * @return true if success, false otherwise
 */
int add_entry(struct directory *dir, int type, char *name, void *ptr) {

	if (dir->nitems == MAX_ENTRIES) {
		if (ptr) {
			free(ptr);
			ptr = NULL;
		}
		return 0;
	}

	unsigned int hash = hashcode_string(name) % MAX_ENTRIES;

	struct entry **entry = &(dir->entries[hash]);

	while (*entry) {
		if (!strcmp((*entry)->name, name)) {
			if (ptr) {
				free(ptr);	
				ptr = NULL;
			}
			return 0;
		}
		entry = &((*entry)->next);
	}

	struct entry *tmp = calloc(1, sizeof(struct entry));
	strcpy(tmp->name, name);
	tmp->type = type;
	tmp->ptr = ptr;	
	ptr = NULL;
	tmp->next = NULL;
	*entry = tmp;
	tmp = NULL;
	dir->nitems++;
	return 1;
	
}

/** 
 * Removes entry from directory
 * 
 * @param dir from where to remove
 * @param name entry to remove
 * @return true if deleted, false if it didn't exist
 */
int remove_entry(struct directory *dir, char *name) {
	unsigned int hash = hashcode_string(name) % MAX_ENTRIES;
	
	struct entry **ptr = &(dir->entries[hash]);

	while (*ptr && strcmp(name, (*ptr)->name)) {    //Se *ptr esiste e se i nomi coincidono esci dal while
		ptr = &((*ptr)->next);
	}

	if (!*ptr) // dato non presente
		return 0;

	// *ptr è il dato da rimuovere
	struct entry *tmp = *ptr;
	*ptr = tmp->next;
	if (tmp->ptr != NULL) {
		free(tmp->ptr);
		tmp->ptr = NULL;		
	}
	free(tmp);
	dir->nitems--;
	return 1;
}

/**
 * Tokenizes path in single folder name strings
 *
 * @param path path to tokenize
 * @return pointers to arrays, NULL if path invalid
 */
char ** tokenize_path(char *path) {
	
	static char *result[MAX_PATH];
	int i;

	if (*path != '/') // path non inizia con /
		return NULL;
	
	result[0] = strtok(path, "/");

	for (i = 1; i < MAX_PATH && result[i-1]; i++) {
		result[i] = strtok(NULL, "/");
	}

	return result;
}

/**
 * Searches dir in a specified subdir
 *
 * @param dir directory in which to search
 * @param name dir to find
 * @return directory if found, NULL otherwise
 */
struct directory * find_directory(struct directory *dir, char *name) {
	struct entry *entry;

	entry = find_entry(dir, name);

	if (entry && entry->type == TYPE_DIR)
		return entry->ptr;
	else
		return NULL;
}

/**
 * Crea una nuova risorsa e la aggiunge al filesystem
 * 
 * @param path path della risorsa da creare
 * @param type tipo di risorsa da creare
 * @return true in caso di successo, false altrimenti
 */
int create_resource(char *path, int type) {
	char **p = tokenize_path(path);
	int i;
	struct directory *dir = &root;

	for (i = 0; i < MAX_PATH && p[i+1]; i++) {
		dir = find_directory(dir, p[i]);
		if (!dir)
			return 0;
	}

	if (type == TYPE_FILE)
		return add_entry(dir, TYPE_FILE, p[i], NULL);
	
	void* ptr = calloc(1, sizeof(struct directory));
	return add_entry(dir, TYPE_DIR, p[i], ptr);
}

/**
 * Ottiene una entry dal suo path
 *
 * @param path path dove cercare
 * @return l'entry in caso di successo, NULL altrimenti
 */
struct entry * get_resource(char *path) {
	char **p = tokenize_path(path);
	int i;
    struct directory *dir = &root;
    if (*path != '/') return NULL;

	for (i = 0; i < MAX_PATH && p[i+1]; i++) { // va a cercare se esistono tutte le directory
		dir = find_directory(dir, p[i]);
		if (!dir)
			return NULL;
	}

	return find_entry(dir, p[i]);
}

/**
 * mostra il contenuto di una directory
 *
 * @param path path della directory
 */
void ls(char *path) {
	struct directory *dir;
	struct entry *entry;

	if (!strcmp(path, "/")) {
		dir = &root;
	} else {
		entry = get_resource(path);

		if (!entry)
			return;

		if (entry->type != TYPE_DIR) {
			puts("Not a directory");
			return;
		}

		dir = entry->ptr;
	}

	for (int i = 0; i < MAX_ENTRIES; i++) {
		struct entry *e = dir->entries[i];

		while (e) {
			printf("%s %s\n", e->type == TYPE_FILE ? "F" : "D", e->name);
			e = e->next;
		}
	}
}

/**
 * Crea un nuovo file 
 * 
 * @param path path del file da creare
 */
void create(char *path) {
	if (create_resource(path, TYPE_FILE))
		puts("ok");
	else 
		puts("no");
}

/**
 * Crea una nuova directory
 *
 * @param path della directory da creare
 */
void create_dir(char *path) {
	if (create_resource(path, TYPE_DIR))
		puts("ok");
	else 
		puts("no");
}

/**
 * Stampa il contenuto del file
 *
 * @param path path del contenuto da stampare
 */
void read(char *path) {
	struct entry *entry = get_resource(path);

	if (!entry || entry->type != TYPE_FILE) {
		puts("no");
		return;
	}

	if (!entry->ptr)
		puts("contenuto ");
	else
		printf("contenuto %s\n", (char*) entry->ptr);
}

/**
 * Scrive il contenuto nel file
 *
 * @param path path del file
 * @param content contenuto da scrivere
 */
void write(char *content, char *path) {
    
    if (content == NULL) {
        puts("no");
        return;
    }

    struct entry *entry = get_resource(path);
    char *c = strrchr(content, '\"');
    
    if (*content != '\"' || !c || c == content ||!entry || entry->type != TYPE_FILE) {
        puts("no");
        return;
    }
    
    *c = '\0';
    content++;
	
	if (entry->ptr)
		free(entry->ptr);

	entry->ptr = (char*) malloc(strlen(content)+1);
	strcpy(entry->ptr, content);
	printf("ok %lu\n", strlen(content));
}

/**
 * Elimina un file dal filesystem
 *
 * @param path path del file da eliminare
 */
void delete(char *path) {
	char **p = tokenize_path(path);
	int i;
	struct directory *dir = &root;

	for (i = 0; i < MAX_PATH && p[i+1]; i++) {
		dir = find_directory(dir, p[i]);
		if (!dir)
			goto error;
	}

	struct entry *entry = find_entry(dir, p[i]);

	if (!entry)
		goto error;

	if (entry->type == TYPE_DIR) {
		struct directory *d = entry->ptr;
		if (d->nitems != 0) 
			goto error;
	}

	if (remove_entry(dir, p[i])) {
		puts("ok");
		return;
	}

error:
	puts("no");
}

/**
 * Elimina ricorsivamente entry da directory
 *
 * d directory da cui eliminare l'entry
 * entry entry da eliminare da d
 */
int delete_rec(struct directory *d, struct entry *entry) {
	int status = 0;
	struct directory *dir;

	if (entry == NULL)
		return 0;

	if (entry->type == TYPE_FILE)
		return remove_entry(d, entry->name);
	
	dir = entry->ptr;

	if (dir->nitems != 0) {
		// itera su tutti gli elementi della directory
		for (int i = 0; i < MAX_ENTRIES; i++) {
			struct entry *e = dir->entries[i];	//Scorre tutti gli elem di /foo (bar e baz)
			while (e) {
				status += delete_rec(dir, e);
				e = dir->entries[i];			
			}					
		}
	}
	status += remove_entry(d, entry->name);

	return status;
}

/**
 * Elimina un file dal filesystem ricorsivametne, 
 * eliminando anche i figli nel caso di directory
 *
 * @param path path del file da eliminare
 */
void delete_r(char *path) {
	char **p = tokenize_path(path);
	int i;
	struct directory *dir = &root;

	for (i = 0; i < MAX_PATH && p[i+1]; i++) { 
		dir = find_directory(dir, p[i]);
		if (!dir)
			goto error;
	}

	struct entry *entry = find_entry(dir, p[i]);

	if (!entry)
		goto error;

	if (delete_rec(dir, entry))
		puts("ok");
	else
error:
		puts("no");
}

/**
 * Costruisce una stringa della path 
 * 
 * @param path array dei path
 * @param level numero di parti della path
 * @param name nome del file finale
 * @return stringa nuova allocata
 */
void build_path(char *path[], int level, char *name, struct treeNode** rootOfTree) {
	char tmp[MAX_PATH*MAX_NAME];
	
	strcpy(tmp, "/");
	for (int i = 0; i < level; i++) {
		strcat(tmp, path[i]);
		strcat(tmp, "/");
	}
	strcat(tmp, name);
	insert_in_tree(tmp, rootOfTree);
}

/**
 * Cerca ricorsivamente 
 *
 * @param dir directory in cui cercare
 * @param path array dei path 
 * @param level livello di ricorsione 
 * @param name nome da cercare 
 * @param founds array dei path cercati
 * @param index numero di file trovati
 */
void find_rec(struct directory *dir, char *path[], int level, char *name, int *index, struct treeNode** rootOfTree) {
	for (int i = 0; i < MAX_ENTRIES; i++) {
		struct entry *e = dir->entries[i];

		while (e) {
			if (!strcmp(e->name, name)) {
				(*index)++;
				build_path(path, level, name, rootOfTree);
			}

			if (e->type == TYPE_DIR) {
				path[level] = e->name;
				find_rec(e->ptr, path, level+1, name, index, rootOfTree);
			}
			e = e->next;
		}
	}
}

/**
 * Cerca il nome nel filesystem
 *
 * @param name nome da cercare
 */
void find(char *name) {
	struct treeNode* rootOfTree = NULL;
	char *path[MAX_PATH];

	int nfound = 0;

	find_rec(&root, path, 0, name, &nfound, &rootOfTree);

	if (nfound == 0) {
		puts("no");
		return;
	}

	if (rootOfTree != NULL) {
        in_order(rootOfTree);
        delete_tree(&rootOfTree);
    }
}

#define CMD(name, ...) 		\
if (!strcmp(cmd, #name)) {	\
    name(__VA_ARGS__);		\
    continue;				\
}

int main(int argc, char *argv[]) {
	char line[MAX_NAME*MAX_NAME+255];
	char* readF;
    while (1) {
        readF = fgets(line, sizeof(line), stdin);
		if (*line != '\n') {
			char *message = strchr(line, '"');
       		char *cmd = strtok(strtok(line, "\n"), " ");
			char *ARG = strtok(NULL, " ");
			if (cmd != NULL && ARG != NULL && *ARG=='/' && message == NULL) { 
				CMD(create, ARG)
        		CMD(create_dir, ARG)
        		CMD(read, ARG)
        		CMD(delete, ARG)
        		CMD(delete_r, ARG)
        		puts("no");
			} else if ((!strcmp(cmd, "find")) && ARG != NULL) {
				CMD(find, ARG)
			} else if (cmd != NULL && (!strcmp(cmd, "write")) && message != NULL && ARG != NULL && *message == '"' && *ARG == '/') {
				CMD(write, message, ARG)	
			} else if (cmd != NULL && !strcmp(cmd, "exit")) {
				CMD(exit, 0)
			} else { 
				puts("no"); 
				continue; 
			}
		} else { puts("no"); continue; }
    }
}