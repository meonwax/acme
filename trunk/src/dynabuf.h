// ACME - a crossassembler for producing 6502/65c02/65816 code.
// Copyright (C) 1998-2009 Marco Baye
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
struct dynabuf_t {
	char	*buffer;	// pointer to buffer
	int	size;		// size of buffer's used portion
	int	reserved;	// total size of buffer
};


// variables
extern struct dynabuf_t *GlobalDynaBuf;	// global dynamic buffer


// create global DynaBuf (call once on program startup)
extern void DynaBuf_init(void);
// create (private) DynaBuf
extern struct dynabuf_t *DynaBuf_create(int initial_size);
// call whenever buffer is too small
extern void DynaBuf_enlarge(struct dynabuf_t *db);
// return malloc'd copy of buffer contents
extern char *DynaBuf_get_copy(struct dynabuf_t *db);
// copy string to buffer (without terminator)
extern void DynaBuf_add_string(struct dynabuf_t *db, const char *);
// add string version of int to buffer (without terminator)
extern void DynaBuf_add_signed_long(struct dynabuf_t *db, signed long value);
// add string version of float to buffer (without terminator)
extern void DynaBuf_add_double(struct dynabuf_t *db, double value);
// converts buffer contents to lower case
extern void DynaBuf_to_lower(struct dynabuf_t *target, struct dynabuf_t *source);
// add char to buffer
extern void DynaBuf_append(struct dynabuf_t *db, char);


#endif
