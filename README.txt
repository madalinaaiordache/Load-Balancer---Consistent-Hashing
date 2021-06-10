T2 SD - Madalina Gabriela IORDACHE 313CA
===============================================================================
Implementation of Consistent Hashing-a distributed hashing scheme that operates
independently of the number of servers or objects in a distributed hash table
by assigning them a position on an abstract circle or hashring. This allows
servers and objects to scale without affecting the overall system.


Mandatory functions implemented:

I. In server.c:

    1. init_server_memory:
- function to create and allocate memory for a server and its elements
- init_server_memory function returns a server

    2. server_store:
- this function stores a key-value pair in the server
- uniform distribution of data using the modulo operator
- create a linkedlist for every servers's elements and traverse it to store the
given key-value pair in specific element

    3. server_remove:
- this function removes a key-value pair from the server
- we take pointer to head of the linkedlist to traverse it till we find the
product (key-value pair) to be removed
- deallocate memory for the removed node

    4. server_retrieve:
- get the value associated with the key
- we take pointer to head of the linkedlist to traverse it till we find the
product (key-value pair) whose key is needed
- it returns the string value associated with the key or NULL (in case the key
does not exist)

    5. free_server_memory:
- deallocate memory for the server and its elements

II. In load_balancer.c:

    1. loader_store:
- function to store a product (the key-value pair) inside the system;
this function will return the chosen server ID which stores the object
- we apply a hashing algorithm to a data item, known as the hashing key,
to create a hash value, go through the hashring, compare the hash values and
store the given product defined by the key-value pair

    2. loader_retrieve:
- function to search for the server which should posess the value associated to
the key inside the system; we apply a hashing algorithm to a data item, the
hashing key, to create a hash value, go through the hashring, compare the hash
values and store the server defined by the key-value pair
- it returns the server ID which stores the value and the server will return
NULL in case the key does NOT exist in the system.

    3. loader_add_server:
- the load balancer will generate 3 replica TAGs and it will place them inside
the hash ring. The neighbor servers will distribute some the objects to the
added server
- function to add a new server to the system when a server fails, the objects
assigned to each of its replicas on the unit circle will get reassigned to a 
different server in clockwise order,thus redistributing the objects more evenly
    Auxiliary functions:
    a. insert_specific_position
        - function to insert an element at a specific position in an array
        - it goes through the hashring and shifts elements forward to add a new
        element
    b. balancer
        - function to randomly map the objects and the servers to a unit circle
        - if the number of servers changes, keys need to be remapped, objects
        to be reassigned and moved to new servers);each object is then assigned
        to the next server that appearson the circle in clockwise order; this
        provides an even distribution of objects to servers
    c.insert_in_hashring
        - function to compare hash values and insert a server on the hashring

    4. loader_remove_server
- function to remove a server from the system
when a server is removed, the objects assigned to each of its replicas on the
unit circle will get reassigned to a different server in clockwise order
    Auxiliary functions:
    a. delete_specific_position
        - function to delete an element at a specific position in an array
        - it goes through the hashring and shifts elements to remove one
    c. delete_in_hashring
        - function to compare hash values and delete a server from the hashring

    5. free_load_balancer
- function to deallocate memory for the load balancer