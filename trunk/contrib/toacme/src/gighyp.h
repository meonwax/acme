// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// stuff needed for both "Hypra-Ass" and "Giga-Ass"
//
#ifndef gigahypra_H
#define gigahypra_H


// Includes
//
#include "config.h"


// Constants
//
#define FLAGS_NOSPACE		(1u << 0)
#define FLAGS_ADDLEFTBRACE	(1u << 1)
#define FLAGS_STRIPOPENING	(1u << 2)
#define FLAGS_STRIPCLOSING	(1u << 3)


// Prototypes
//
extern void	gigahypra_ConvComment(void);
extern void	gigahypra_Operator(void);
extern void	gigahypra_Indent(int indent);
extern void	gigahypra_Opcode(int Flags);
extern int	gigahypra_LabelDef(void);


#endif
