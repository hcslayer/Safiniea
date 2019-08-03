/* List.c 
 * Henry Slayer 
 * CMPS101 Fall 2019 
 * CruzID: hslayer SSID: 1641932 
 * 
 * Re-implementation of the List ADT for the Matrix.c data structure. 
 * Integer values have been replaced with generic pointers, though structurally
 * the list is the same: doubly-linked, with a cursor function. 
 * Print functions have been delegated to the client module. 
 * Comparison functions have been delegated to the client module. 
 * Generalized variable names have been used. 
 */ 

#include"List.h"
#include<assert.h>
#include<math.h>

// structure definitions and reference types -----------------------------------

// inner node struct 
typedef struct NodeObj {
	void* value; 
	struct NodeObj* next; 
	struct NodeObj* prev; 
} NodeObj; 

// reference for Node pointers 
typedef struct NodeObj* Node;

// structure definition 
typedef struct ListObj {
	int length; 
	Node cursor; 
	Node head; 
	Node tail; 
} ListObj;

// helper functions ------------------------------------------------------------

// locates cursor idx
int getIdx(List L) {
	Node N; 
	int idx = 0; 
	assert(L!=NULL); 
	if (L->cursor) {
		for (N = L->head; N != NULL; N = N->next, idx++) {
			if (N == L->cursor) {
				return idx;
			}
		}
	}
	return -1; 
}


// constructors and destructors ------------------------------------------------

// list constructor 
List newList(void) {
	List L = malloc(sizeof(ListObj)); 
	assert(L!=NULL); 
	L->length = 0; 
	L->head = L->tail = L->cursor = NULL; 
	return L; 
}

// node constructor, sets value to n 
Node newNode(void* n) {
	Node N = malloc(sizeof(NodeObj)); 
	assert(N!=NULL); 
	N->value = n; 
	N->next = N->prev = NULL; 
	return N; 
}

void freeNode(Node* pN) {
	if (pN != NULL && *pN != NULL) {
		free(*pN);
		*pN = NULL; 
	}
}				

void freeList(List* pL) {
	if (*pL != NULL && pL != NULL) {
		clear(*pL);  
		free(*pL); 
		*pL = NULL; 
	}
} 		

// accessors -------------------------------------------------------------------

// returns the length of the list 
int length(List L) {
	assert(L!=NULL); 
	return L->length;  
} 

// returns the cursor idx, if defined 
int idx(List L) {
	return getIdx(L); 
	// huh... why don't I just erase that function and put it here? 
}	

// front() 
// returns the front element of the list
void* front(List L) {
	if (length(L) > 0) {
		return L->head->value;
	}
	else {
		printf("List Error: Cannot access contents of empty list.\n");
		return NULL; 
	} 
} 			 

//back()
// returns the back element of the list 
void* back(List L) {
	if (length(L) > 0) {
		return L->tail->value; 
	}
	else {
		printf("List Error: Cannot access contents of empty list.\n");
		return NULL;
	}
	
}	

// get() 
// returns the cursor element, if defined 
void* get(List L) {
	if (idx(L) >= 0) {
		return L->cursor->value;
	}
	else {
		printf("List Error: Undefined cursor.\n");
		return NULL;
	}
} 


// manipulation subroutines ----------------------------------------------------

void clear(List L) {
	assert(L!=NULL); 
	while (length(L) > 0) {
		deleteFront(L); 
	}
	L->cursor = NULL;
}

void moveFront(List L) {
	if (length(L) > 0) {
		L->cursor = L->head;
	}
	else {
		printf("List Error: Cannot define cursor for empty list.\n"); 
	}
}

void moveBack(List L) {
	if (length(L) > 0) {
		L->cursor = L->tail;
	}
	else {
		printf("List Error: Cannot define cursor for empty list.\n"); 
	}
}

void movePrev(List L) {
	if (idx(L) >= 0) {
		L->cursor = L->cursor->prev;
	}
}

void moveNext(List L) {
	if (idx(L) >= 0) {
		L->cursor = L->cursor->next;
	}
}

// prepend() 
// adds a new node to the front of the list 		
void prepend(List L, void* data) {
	assert(L!=NULL);
	Node N = newNode(data); 
	if (length(L) == 0) { L->head = L->tail = N; L->length++; }
	else { 
		N->next = L->head; 
		L->head->prev = N; 
		L->head = N; 
		L->length++; 
	}
} 

// append() 
// add new node to the back of the list 
void append(List L, void* data) {
	assert(L!=NULL); 
	Node N = newNode(data); 
	if (length(L) == 0) { L->tail = L->head = N; L->length++;}
	else {
		N->prev = L->tail; 
		L->tail->next = N; 
		L->tail = N; 
		L->length++;
	}
}	

void insertBefore(List L, void* data) {
	assert(L!=NULL); 
	if (idx(L) >= 0) {
		if (L->cursor == L->head) {
			prepend(L, data);
			return; 
		}
		Node N = newNode(data); 
		L->cursor->prev->next = N;
		N->prev = L->cursor->prev;
		N->next = L->cursor;
		L->cursor->prev = N; 
		L->length++;
		return;
	}
	else {
		printf("List error: Undefined cursor\n");
		printf("Could not create node\n"); 
		return;
	}
} 

void insertAfter(List L, void* data) {
	assert(L!=NULL); 
	if (idx(L) >= 0) {
		if (L->cursor == L->tail) {
			append(L, data);
			return; 
		}
		Node N = newNode(data); 
		L->cursor->next->prev = N;
		N->next = L->cursor->next;
		N->prev = L->cursor;
		L->cursor->next = N; 
		L->length++;
		return;
	}
	else {
		printf("List error: Undefined cursor\n");
		printf("Could not create node\n"); 
		return;
	}
}  

void deleteFront(List L) {
	assert(L!=NULL); 
	if (length(L) > 0) {
		if (length(L) == 1) {
			freeNode(&L->head); 
			L->head = L->tail = L->cursor = NULL;
			L->length = 0; 
			return;
		}
		Node N = L->head->next; 
		N->prev = NULL; 
		freeNode(&L->head); 
		L->head = N; 
		L->length--;
	}
}		  

void deleteBack(List L) {
	assert(L!=NULL); 
	if (length(L) > 0) {
		if (length(L) == 1) {
			freeNode(&L->head); 
			L->head = L->tail = L->cursor = NULL;
			L->length = 0; 
			return;
		}
		Node N = L->tail->prev; 
		N->next = NULL; 
		freeNode(&L->tail); 
		L->tail = N; 
		L->length--;
	}
}  		

void delete(List L) {
	assert(L!=NULL); 
	if (idx(L) >= 0) {
		if (L->cursor == L->head) {
			deleteFront(L);
			L->cursor = NULL;
			return;
		}
		if (L->cursor == L->tail) {
			deleteBack(L); 
			L->cursor = NULL;
			return;
		}
		// link the prior and adjacent nodes 
		Node N = L->cursor->prev;
		N->next = L->cursor->next; 
		L->cursor->next->prev = N;  
		freeNode(&L->cursor); 
		L->cursor = NULL;
		L->length--;
		return;
	}
}			

// other operations ------------------------------------------------------------

/* SUSPENDED void printList(FILE* out, List L) {
	assert(out!=NULL); 
	assert(L!=NULL); 
	Node N; 
	if (length(L) > 0) {
		for(N = L->head; N != NULL; N = N->next) {
			fprintf(out, "%g ", N->value); 
		}
	}
	fprintf(out, "\n"); 
}	*/ 
					
