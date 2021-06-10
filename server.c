/* Copyright 2021 Iordache Madalina Gabriela 313CA */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "server.h"

/* hash function */
unsigned int hash_function_key(void *a) {
	unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}
/*
 * function to create a server
 * allocate memory for a server
 * allocate memory for elements in a server
 * initialize number of elements and max size for a server
 * this function returns a server
 */
server_memory* init_server_memory() {
	server_memory* server = malloc(sizeof(server_memory));
	server->elements = malloc(HMAX * sizeof(linked_list_t));

	for (int i = 0; i < HMAX; i++) {
		server->elements[i] = NULL;
	}

	server->nr_of_elem = 0;
	server->max_size = HMAX;
	return server;
}

/*
 * function to store a key-value pair to the server
 */
void server_store(server_memory* server, char* key, char* value) {
	int i;
	/* uniform distribution of data */
	i = hash_function_key(key) % server->max_size;

	/* create a linkedlist for every servers's buckets */
	if (server->elements[i] == NULL)
		server->elements[i] = ll_create(sizeof(buckets));

	/* take pointer to head to go through the list */
	ll_node_t* curr = server->elements[i]->head;

	while (curr != NULL) {
		if (strcmp(((buckets*)(curr->data))->key, key) == 0) {
			strncpy(((buckets*)(curr->data))->value, value, strlen(value));
			return;
		}
		curr = curr->next;
	}

	/* store the key-value pair to the server */
	buckets info;
	strcpy(info.key, key);
	strcpy(info.value, value);

	ll_add_nth_node(server->elements[i], 0, &info);
}

/*
 * function to remove a key-pair value from the server
 */
void server_remove(server_memory* server, char* key) {
	int i;
	/* uniform distribution of data */
	i = hash_function_key(key) % server->max_size;

	/* take pointer to head to go through the list */
	ll_node_t* curr = server->elements[i]->head;
	int pos = 0;
	while (curr != NULL) {
		if (strcmp(((buckets*)(curr->data))->key, key) == 0) {
			/*
			 * remove the key-pair value from the server
			 * deallocate memory
			 */
			ll_node_t* rmv = ll_remove_nth_node(server->elements[i], pos);

			free(rmv->data);
			free(rmv);
			rmv = NULL;

			return;
		}
		curr = curr->next;
		pos++;
	}
}

/*
 * function to get the value associated with the key
 */
char* server_retrieve(server_memory* server, char* key) {
	int i;
	i = hash_function_key(key) % server->max_size;

	if (server->elements[i] == NULL)
		return NULL;
	/* take pointer to head to go through the list */
	ll_node_t* curr = server->elements[i]->head;

	/* traverse the linkedlist */
	while (curr != NULL) {
		if (strcmp(((buckets*)(curr->data))->key, key) == 0) {
			/* return the requested value */
			return ((buckets*)(curr->data))->value;
		}
		curr = curr->next;
	}

	/* return NULL in case the key does not exist */
	return NULL;
}

/*
 * function to deallocate memory for a server
 */
void free_server_memory(server_memory* server) {
	for (int i = 0; i < HMAX; i++) {
		if (server->elements[i] != NULL)
			ll_free(&server->elements[i]);
	}

	free(server->elements);
	free(server);
}
