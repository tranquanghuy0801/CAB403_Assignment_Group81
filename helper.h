/** CAB403 - Systems Programming 
 *  Group 81 - DS Assignment 
 * 	Quang Huy Tran - n10069275 
 *  Tuan Minh Nguyen - 
 *  Ho Fong Law - 
 * */
#include <stdbool.h> // for bool type
#define C_NAME_LEN 256

typedef struct client_info
{
	char m_name[C_NAME_LEN + 1];
	in_addr_t address;
	in_port_t port;
} client_info_t; // temporary struct that saves the client info in at after doing MSG_WHO

typedef struct message message_t;

// temporary struct that saves new message on each channel 
struct message
{
	char *text;
	int messIndex;
	message_t *next;
};

typedef struct node node_t;
struct node
{
	int channelID;
	int messIndex;
	int countMess; // count messages since the start of the server
	int countRead; // count messages that have been read by client
	int countUnread ;// count message that have not yet been read
	message_t *message;
	node_t *next;
};



void sig_handler(int);

typedef struct item item_t;
struct item {
	char *key;
	node_t* subChannel;
	item_t *next;
};


// A hash table mapping a string to an integer. 
typedef struct htab htab_t;
struct htab {
	item_t **buckets;
	size_t size;
};

// Add another bucket
// Add n more bucket
bool htab_add_bucket(htab_t *h, size_t n){
	h->size = h->size+n;
	h->buckets = realloc(h->buckets, sizeof(node_t*)*h->size);
	if(h->buckets == NULL){
		perror("Reallocation failed \n");
		return 0;
	}
	else return h->buckets;
}

// Initialise a new hash table with n buckets. 
// pre: true
// post: (return == false AND allocation of table failed)
//       OR (all buckets are null pointers)
bool htab_init(htab_t *h, size_t n) {
	h->size = n;
	h->buckets = (item_t **)calloc(n, sizeof(item_t *));
	return h->buckets != 0;
}

// The Bernstein hash function.
// A very fast hash function that works well in practice.
size_t djb_hash(char *s) {
	size_t hash = 5381;
	int c;
	while ((c = *s++) != '\0') {
		// hash = hash * 33 + c
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

// Calculate the offset for the bucket for key in hash table.
size_t htab_index(htab_t *h, char* key) {

	return djb_hash(key) % h->size;
}

// Find pointer to head of list for key in hash table.
item_t * htab_bucket(htab_t *h, char* key) {
	return h->buckets[htab_index(h, key)];
}

// Find an item for key in hash table.
// pre: true
// post: (return == NULL AND item not found)
//       OR (strcmp(return->key, key) == 0)
item_t * htab_find(htab_t *h, char* key) {
	for (item_t *i = htab_bucket(h, key); i != NULL; i = i->next) {
		if (strcmp(i->key, key) == 0) { // found the key
			return i;
		}
	}
	return NULL;
}

node_t *node_find_channel(htab_t* h, char* key, int channelID)
{
	for (item_t *i = htab_bucket(h, key); i != NULL; i = i->next) {
		if (strcmp(i->key, key) == 0) { // found the key
			node_t *head = i->subChannel;
			for (; head != NULL; head = head->next)
			{
				if (channelID == head->channelID)
				{
					return head;
				}
			}
			return NULL;
		}
	}
	return NULL;
}

// Add a key with value to the hash table.
// pre: htab_find(h, key) == NULL
// post: (return == false AND allocation of new item failed)
//       OR (htab_find(h, key) != NULL)
bool htab_add_node(htab_t *h, char* hChannel[255][10], char* key, int channelID) {
	// hash key and place item in appropriate bucket
	size_t bucket = htab_index(h, key);
	//If client already exist
	if(htab_find(h,key)!=NULL){
		node_t *new = (node_t *)malloc(sizeof(node_t));
		if (new == NULL)
		{
			return NULL;
		}

	// insert new node
		node_t *head = h->buckets[bucket]->subChannel;
		new->channelID = (int)channelID;
		new->next = head;
		head = new;
		h->buckets[bucket]->subChannel = head;
	}
	else{
		// allocate new item
		item_t *newhead = (item_t *)malloc(sizeof(item_t));
		if (newhead == NULL) {
			return false;
		}
		node_t *new = (node_t *)malloc(sizeof(node_t));
		if (new == NULL)
		{
			return false;
		}

		//Insert new item in bucket
		newhead->key = key;
		newhead->next = h->buckets[bucket];
		h->buckets[bucket] = newhead;
		//Insert new node in item
		new->channelID = (int)channelID;
		new->next = NULL;
		newhead->subChannel = new;
	}
	int i = 0;
	if(hChannel[channelID][i] == NULL){
		h->buckets[bucket]->subChannel->messIndex = i;
	} else{
		do{i++;}
		while(hChannel[channelID][i] != NULL);
		h->buckets[bucket]->subChannel->messIndex = i;
	}
	return true;
}

// Delete an item with key from the hash table.
// pre: htab_find(h, key) != NULL 
// post: htab_find(h, key) == NULL 
void htab_delete(htab_t *h, char *key) {
	item_t *head = htab_bucket(h, key);
	item_t *current = head; 
	item_t *previous = NULL;
	while (current != NULL) {
		if (strcmp(current->key, key) == 0) {
			if (previous == NULL) { // first item in list
				h->buckets[htab_index(h, key)] = current->next;
			} else {
				previous->next = current->next;
			}
			free(current);
			break;
		}
		previous = current;
		current = current->next;
	}
}

//Remove channel from a client hash table
void htab_delete_node(htab_t *h, char* key, int channelID){
	node_t *head = htab_bucket(h,key)->subChannel;
	node_t *previous = NULL;
	node_t *current = head;
	while (current != NULL)
	{
		if (channelID == current->channelID)
		{
			node_t *newhead = head;
			if (previous == NULL) // first item in list
				newhead = current->next;
			else
				previous->next = current->next;
			free(current);
			htab_bucket(h,key)->subChannel = newhead;
		}
		previous = current;
		current = current->next;
	}

	// name not found
	//return head;

}
// Destroy an initialised hash table.
// pre: htab_init(h)
// post: all memory for hash table is released
void htab_destroy(htab_t *h) {
	// free linked lists
	for (size_t i = 0; i < h->size; ++i) {
		item_t *bucket = h->buckets[i];
		while (bucket != NULL) {
			item_t *next = bucket->next;
			free(bucket);
			bucket = next;
		}
	}

	// free buckets array
	free(h->buckets);
	h->buckets = NULL;
	h->size = 0;
}

// message_t* message_add(int channelID, char *text){

//}

void message_add(char *text, char* channel_mess[255][10],int channelID){
	int i;
	for(i = 0; i < 10;i++){
		if(channel_mess[channelID][i] == NULL){
			channel_mess[channelID][i] = text;
			printf("%s",channel_mess[channelID][i]);
			break;
		}
	}
}



node_t *node_delete(node_t *head, int channelID)
{
	node_t *previous = NULL;
	node_t *current = head;
	while (current != NULL)
	{
		if (channelID == current->channelID)
		{
			node_t *newhead = head;
			if (previous == NULL) // first item in list
				newhead = current->next;
			else
				previous->next = current->next;
			free(current);
			return newhead;
		}
		previous = current;
		current = current->next;
	}

	// name not found
	return head;
}

