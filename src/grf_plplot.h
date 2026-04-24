/*
*  Name:
*     grf_plplot.h

*  Purpose:
*     Declare public utility functions specific to the PLplot graphics
*     system.

*  Description:
*     The PLplot GRF module (grf_plplot.c) implements the standard GRF
*     graphics interface against the PLplot backend. Most of its functions
*     are exported through the common graphics interface provided by grf.h
*     but this header also provides utility functions useful for plotting
*     with PLplot specifically.

*  Licence:
*     This program is free software: you can redistribute it and/or
*     modify it under the terms of the GNU Lesser General Public
*     License as published by the Free Software Foundation, either
*     version 3 of the License, or (at your option) any later
*     version.
*
*     This program is distributed in the hope that it will be useful,
*     but WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*     GNU Lesser General Public License for more details.
*
*     You should have received a copy of the GNU Lesser General
*     License along with this program.  If not, see
*     <http://www.gnu.org/licenses/>.
*/

#ifndef GRF_PLPLOT_H_INCLUDED
#define GRF_PLPLOT_H_INCLUDED

/* Function prototypes. */
/* ==================== */
const char *astPlSetupDevice( const char * );

#endif /* GRF_PLPLOT_H_INCLUDED */
