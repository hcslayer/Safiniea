/* List.h 
 * Henry Slayer 
 * CMPS101 Fall 2019 
 * CruzID: hslayer SSID: 1641932 
 * 
 * Header file for the integer list ADT. The following specifies publicly 
 * available subroutines and ADT operations. The list itself is a 
 * bidirectional integer queue, with a cursor function. 
 * The exported reference type is 'List'. 
 */ 

#include<stdio.h>
#include<stdlib.h>
#ifndef LIST_ADT_HEAD
#define LIST_ADT_HEAD

typedef struct ListObj* List; 

// constructors and destructors  

List newList(void); 				// creates a new empty list 
void freeList(List* pL); 		// deallocates all heap memory associated 
										// with *pL, and sets *pL to null 

// accessors 

int length(List L); 			// returns number of elements in L 
int idx(List L);			// returns cursor position. -1 if undefined 
void* front(List L); 			// returns first element of L. Pre: length() > 0 
void* back(List L);				// returns back element of L. Pre: length() > 0 
void* get(List L); 				// returns cursor element. Pre: length() > 0, idx() >= 0 
 

// manipulation subroutines

void clear(List L); 			// restores L to an empty state 
void moveFront(List L); 	// sets cursor to the front of the list. if empty, does nothing 
void moveBack(List L);		// sets cursor to the back element. if empty, does nothing
void movePrev(List L); 		// if L is nonempty, the cursor is moved one idx towards the front
void moveNext(List L); 		// if L is nonempty, the cursor is moved one idx towards the back 
									// NOTE: for any cursor manipulation, the cursor becomes undefined 
									// 		if it 'falls off' the front or back of the list. 

void prepend(List L, void* data); 		// Inserts new element at the front of the list. 
void append(List L, void* data);		// Inserts new element at the back of the list. 
void insertBefore(List L, void* data); // Inserts new element before cursor element. 
void insertAfter(List L, void* data);  // Inserts new elment after the cursor element. 
												 // NOTE: insertBefore/After have preconditions, 
												 //		 Pre: length()>0 idx()>=1 

void deleteFront(List L);		// deletes the first element. Pre: length() > 0  
void deleteBack(List L);	  		// deletes the back element. Pre: length() > 0 
void delete(List L); 				// deletes the cursor element, cursor becomes undef. 
										// Pre: length() > 0, idx() >= 0



#endif


