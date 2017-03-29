// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// Dynamic buffer stuff
#ifndef dynabuf_H
#define dynabuf_H


#include "config.h"


// macros
#define DYNABUF_CLEAR(db)		do {db->size = 0;} while (0)
#define DYNABUF_APPEND(db, byte)	\
do {					\
	if (db->size == db->reserved)	\
		DynaBuf_enlarge(db);	\
	db->buffer[(db->size)++] = byte;\
} while (0)
// the next one is dangerous - the buffer location can change when a character
// is appended. So after calling this, don't change the buffer as long as you
// use the address.
#define GLOBALDYNABUF_CURRENT		(GlobalDynaBuf->buffer)


// dynamic buffer structure
struct dynabuf {
	char	*buffer;	// pointer to buffer
	int	size;		// size of buffer's used portion
	int	reserved;	// total size of buffer
};


// variables
extern struct dynabuf *GlobalDynaBuf;	// global dynamic buffer


// create global DynaBuf (call once on program startup)
extern void DynaBuf_init(void);
// create (private) DynaBuf
extern struct dynabuf *DynaBuf_create(int initial_size);
// call whenever buffer is too small
extern void DynaBuf_enlarge(struct dynabuf *db);
// return malloc'd copy of buffer contents
extern char *DynaBuf_get_copy(struct dynabuf *db);
// copy string to buffer (without terminator)
extern void DynaBuf_add_string(struct dynabuf *db, const char *);
// converts buffer contents to lower case
extern void DynaBuf_to_lower(struct dynabuf *target, struct dynabuf *source);
// add char to buffer
extern void DynaBuf_append(struct dynabuf *db, char);


#endif
