# updated makefile for linked list implementation 

# CLIENT         = Safiniea
# ADT            = List
# SOURCE         = $(CLIENT).c
# OBJECT         = $(CLIENT).o
# ADT_SOURCE     = $(ADT).c
# ADT_OBJECT     = $(ADT).o
# ADT_HEADER     = $(ADT).h
# COMPILE        = gcc -c -std=c99 -Wall
# LINK           = gcc -o
# REMOVE         = rm -f
# MEMCHECK       = valgrind --leak-check=full

# $(CLIENT) : $(OBJECT) $(ADT_OBJECT) 
# 	$(LINK) $(CLIENT) $(OBJECT) $(ADT_OBJECT) 

# $(OBJECT) : $(SOURCE) $(ADT_HEADER) 
# 	$(COMPILE) $(SOURCE)

# $(ADT_OBJECT) : $(ADT_SOURCE) $(ADT_HEADER)
# 	$(COMPILE) $(ADT_SOURCE)

# clean :
# 	$(REMOVE) $(CLIENT) $(OBJECT) $(ADT_OBJECT) 

# memcheck : $(CLIENT)
# 	$(MEMCHECK) $(CLIENT)

do: 
	gcc -std=c99 -Wall safiniea.c mpc.c -ledit -lm -o prompt
