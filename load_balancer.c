/* Copyright 2021 Iordache Madalina Gabriela 313CA */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "server.h"
#include "load_balancer.h"
#define MAX 100000


struct load_balancer {
    int nr_servers;
    struct server_memory **servers;

    /* sorted circular array */
    int *hashring;
    int nr_hashring;
};

/* hash function */
unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

/*
 * function to create load balancer which distributes the work
 * allocate memory for the load balancer
 * allocate memory for the servers stored in load balancer
 * allocate memory for the hashring
 * initialize number of elements in hashring
 * this function returns the load balancer
 */
load_balancer* init_load_balancer() {
    load_balancer *load_b = NULL;
    load_b = calloc(1, sizeof(struct load_balancer));
    load_b->servers = malloc(MAX * sizeof(struct server_memory*));

    for (int i = 0; i < MAX; i++) {
        load_b->servers[i] = NULL;
    }

    load_b->hashring = malloc(MAX * sizeof(int));
    load_b->nr_servers = 0;
    load_b->nr_hashring = 0;

    return load_b;
}

/*
 * function to store a product (the key-value pair) inside the system
 * function will return the chosen server ID which stores the object
 */
void loader_store(load_balancer* main, char* key,
                  char* value, int* server_id) {
    /*
     * apply a hashing algorithm to a data item, known as the hashing key,
     * to create a hash value
     */
    unsigned int hash = hash_function_key(key);
    int id = -1;

    /* go through the hashring */
    for (int i = 0; i < main->nr_hashring; i++) {
        int val = main->hashring[i];

        /* compare hash values */
        if (hash <= hash_function_servers(&val)) {
            id = val;
            break;
        }
    }
    if (id == -1)
        id = main->hashring[0];

    id %= MAX;
    /* store the given product */
    server_store(main->servers[id], key, value);
    *server_id = id;
}

/*
 * function to calculate on which server is stored a product(the key-value
 * pair) inside the system
 * return the server ID which stores the value
 */
char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
    /*
     * apply a hashing algorithm to a data item, known as the hashing key,
     * to create a hash value
     */
    unsigned int hash = hash_function_key(key);
    int id = -1;

    /* go through the hashring */
    for (int i = 0; i < main->nr_hashring; i++) {
        int val = main->hashring[i];

        /* compare hash values */
        if (hash <= hash_function_servers(&val)) {
            id = main->hashring[i];
            break;
        }
    }

    if (id == -1)
        id = main->hashring[0];

    id %= MAX;
    *server_id = id;

    /* return the given server id*/
    return server_retrieve(main->servers[id], key);
}

/*
 * function to insert an element at a specific position in an array
 */
void insert_specific_position(int position, int elem,
                                load_balancer *main) {
    /* insert element at the beginning*/
    if (position == 0) {
        /* increase number of servers distributed on the hashring */
        main->nr_hashring++;

        /* go through the hashring */
        for (int i = main->nr_hashring - 1; i > position; i--) {
            /* shift elements forward */
            main->hashring[i] = main->hashring[i - 1];
        }
        /* insert an element at given position */
        main->hashring[position] = elem;

    /* insert element at the ending */
    } else if (position == main->nr_hashring) {
        main->nr_hashring++;
        main->hashring[position] = elem;

    /* insert element arbitrary*/
    } else {
        /* increase number of servers distributed on the hashring */
        main->nr_hashring++;

        /* go through the hashring */
        for (int i = main->nr_hashring - 1; i >= position; i--) {
            /* shift elements forward */
            main->hashring[i] = main->hashring[i - 1];
        }
        main->hashring[position] = elem;
    }
}

/*
 * function to randomly map the objects and the servers to a unit circle
 * if the number of servers changes, keys need to be remapped
 * (objects to be reassigned and moved to new servers)
 * each object is then assigned to the next server that appears
 * on the circle in clockwise order
 * this provides an even distribution of objects to servers
 */
void balancer(load_balancer *main, int position, int sv_id) {
    int neigh_right = -1;

    /* stop redistribution if there is only one server on the hashring*/
    if (main->nr_hashring == 1)
        return;
    else
        /* right neighbour to a given server */
        neigh_right = main->hashring[(position + 1) % main->nr_hashring] % MAX;

    if (sv_id % MAX == neigh_right)
        return;

    /* go through the hashring */
    for (int i = 0; i < HMAX; i++) {
        if (main->servers[neigh_right]->elements[i] != NULL) {
            /* head pointer to traverse the list of objects*/
            ll_node_t* aux = main->servers[neigh_right]->elements[i]->head;

            while (aux != NULL) {
                /* get the key-value pair for an object */
                char *key = ((struct buckets*)aux->data)->key;
                char *value = ((struct buckets*)aux->data)->value;

                /* redistribute data */
                server_store(main->servers[sv_id % MAX], key, value);
                aux = aux->next;
            }
        }
    }
}

/*
 * function to insert a server on the hashring
 */
void insert_in_hashring(load_balancer *main, int label,
                        unsigned int hash_label) {
    int index = -1;

    /* go through the hashring */
    for (int i = 0; i < main->nr_hashring; i++) {
        int val = main->hashring[i];

        /* compare hash values */
        if (hash_label < hash_function_servers(&val)) {
            index = i;
            break;
        }
    }
    if (index == -1 && main->nr_hashring == 0)
        index = 0;
    else if (index == -1)
        index = main->nr_hashring;

    index %= MAX;

    /*insert server at a specific position and redistribute data */
    insert_specific_position(index, label, main);
    balancer(main, index, label);
}

/*
 * function to add a new server to the system
 * when a server fails, the objects assigned to each of its replicas on
 * the unit circle will get reassigned to a different server in clockwise
 * order, thus redistributing the objects more evenly
 */
void loader_add_server(load_balancer* main, int server_id) {
    main->servers[server_id] = init_server_memory();

    /* increase number of servers stored on the hashring */
    main->nr_servers++;

    /* generate 3 replica TAGs */
    int label1 = server_id;
    int label2 = MAX + label1;
    int label3 = MAX + label2;

    unsigned int hash_label1 = hash_function_servers(&label1);
    unsigned int hash_label2 = hash_function_servers(&label2);
    unsigned int hash_label3 = hash_function_servers(&label3);

    insert_in_hashring(main, label1, hash_label1);
    insert_in_hashring(main, label2, hash_label2);
    insert_in_hashring(main, label3, hash_label3);
}

/*
 * function to delete an element at a specific position in an array
 */
void delete_specific_position(int position,
                              load_balancer *main) {
    /* delete server from beginning */
    if (position == 0) {
        for (int i = position; i < main->nr_hashring - 1; i++) {
            main->hashring[i] = main->hashring[i + 1];
        }
        main->nr_hashring--;

    /* delete server from ending */
    } else if (position == main->nr_hashring) {
        main->nr_hashring--;

    /* delete server arbitrary */
    } else {
        for (int i = position; i < main->nr_hashring - 1; i++) {
            main->hashring[i] = main->hashring[i + 1];
        }
        main->nr_hashring--;
    }
}

/*
 * function to remove a server from hashring
 */
void delete_in_hashring(load_balancer *main,
                        unsigned int hash_label) {
    int index = -1;

    /* go through the hashring */
    for (int i = 0; i < main->nr_hashring; i++) {
        int val = main->hashring[i];

        /* compare hash values */
        if (hash_label <= hash_function_servers(&val)) {
            index = i;
            break;
        }
    }
    if (index == -1 && main->nr_hashring == 0)
        index = 0;
    else if (index == -1)
        index = main->nr_hashring;

    index %= MAX;

    delete_specific_position(index, main);
}

/*
 * function to remove a server from the system
 * when a server is removed, the objects assigned to each of its replicas on
 * the unit circle will get reassigned to a different server in clockwise
 */
void loader_remove_server(load_balancer* main, int server_id) {
    /* replica TAGs */
    int label1 = server_id;
    int label2 = MAX + label1;
    int label3 = MAX + label2;

    int hash_label1 = hash_function_servers(&label1);
    int hash_label2 = hash_function_servers(&label2);
    int hash_label3 = hash_function_servers(&label3);

    /* delete all replicas from the hashring */
    delete_in_hashring(main, hash_label1);
    delete_in_hashring(main, hash_label2);
    delete_in_hashring(main, hash_label3);

    for (int i = 0; i < HMAX; i++) {
        if (main->servers[server_id]->elements[i] != NULL) {
            ll_node_t* curr = main->servers[server_id]->elements[i]->head;

            while (curr != NULL) {
                int sv_aux = server_id;

                /* distribute all objects stored on the removed server */
                loader_store(main, ((buckets*)curr->data)->key,
                            ((buckets*)curr->data)->value, &sv_aux);
                curr = curr->next;
            }
        }
    }
    /* deallocate memory for removed server and its replicas*/
    free_server_memory(main->servers[server_id]);
    main->servers[server_id] = NULL;
}

/*
 * function to deallocate memory for the load balancer
 */
void free_load_balancer(load_balancer* main) {
    for (int i = 0; i < MAX; i++) {
        if (main->servers[i] != NULL) {
            free_server_memory(main->servers[i]);
        }
    }
    free(main->servers);
    free(main->hashring);
    free(main);
}
