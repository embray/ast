/*
 *  Minimal stub for the Starlink MERS (message/error reporting) header.
 *  Provides no-op implementations of the EMS functions used by the
 *  AST test suite, sufficient for tests that don't rely on full error
 *  context stacking.
 */
#ifndef MERS_H_INCLUDED
#define MERS_H_INCLUDED

#include "ast.h"

static inline void errMark( void ) {}
static inline void errRlse( void ) {}

static inline void errStat( int *status ) {
   *status = astOK ? 0 : 1;
}

static inline void errAnnul( int *status ) {
   astClearStatus;
   *status = 0;
}

static inline void errRep( const char *token, const char *text, int *status ) {
   (void)token;
   (void)text;
   (void)status;
}

#endif
