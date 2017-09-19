/* DWARF 2 debugging format utils for GDB.

   Copyright (C) 2017 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
#ifndef DWARF2UTILS_H
#define DWARF2UTILS_H

/* See documentation in the utils.c file.  */
extern int producer_is_gcc_ge_4 (const char *producer);

/* See documentation in the utils.c file.  */
extern int producer_is_gcc (const char *producer, int *major, int *minor);

/* See documentation in the utils.c file.  */
extern bool producer_is_icc (const char *producer, int *major, int *minor);

#endif
