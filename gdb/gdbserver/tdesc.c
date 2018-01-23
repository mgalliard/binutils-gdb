/* Copyright (C) 2012-2018 Free Software Foundation, Inc.

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

#include "server.h"
#include "tdesc.h"
#include "regdef.h"

#ifndef IN_PROCESS_AGENT

target_desc::~target_desc ()
{
  int i;

  xfree ((char *) arch);
  xfree ((char *) osabi);

  char *f;

  for (i = 0; VEC_iterate (char_ptr, features, i, f); i++)
    xfree (f);
  VEC_free (char_ptr, features);
}

bool target_desc::operator== (const target_desc &other) const
{
  if (reg_defs.size () != other.reg_defs.size ())
    return false;

  for (int i = 0; i < reg_defs.size (); ++i)
    {
      struct reg reg = reg_defs[i];
      struct reg reg2 = other.reg_defs[i];

      if (&reg != &reg2 && reg != reg2)
	return false;
    }

  /* Compare expedite_regs.  */
  int i = 0;
  for (; expedite_regs[i] != NULL; i++)
    {
      if (strcmp (expedite_regs[i], other.expedite_regs[i]) != 0)
	return false;
    }
  if (other.expedite_regs[i] != NULL)
    return false;

  return true;
}

#endif

void
init_target_desc (struct target_desc *tdesc)
{
  int offset = 0;

  /* Go through all the features and populate reg_defs.  */
  for (const tdesc_reg_up &treg : tdesc->registers)
    {
      int current_size = tdesc->reg_defs.size ();

      /* Register number will either increase (possibly with gaps) or be
	 zero.  */
      gdb_assert (treg->target_regnum == 0
		  || treg->target_regnum >= current_size);

      tdesc->reg_defs.resize (treg->target_regnum != 0
				? treg->target_regnum + 1
				: current_size + 1);

      /* Fill in any blank spaces.  */
      while (current_size < treg->target_regnum)
	{
	  struct reg *reg = &tdesc->reg_defs[current_size];
	  reg->name = "";
	  reg->size = 0;
	  reg->offset = offset;
	  current_size++;
	}

      struct reg *reg = &tdesc->reg_defs.back ();
      reg->name = treg->name.c_str ();
      reg->size = treg->bitsize;
      reg->offset = offset;
      offset += reg->size;
    }

  tdesc->registers_size = offset / 8;

  /* Make sure PBUFSIZ is large enough to hold a full register
     packet.  */
  gdb_assert (2 * tdesc->registers_size + 32 <= PBUFSIZ);
}

struct target_desc *
allocate_target_description (void)
{
  return new target_desc ();
}

#ifndef IN_PROCESS_AGENT

static const struct target_desc default_description {};

void
copy_target_description (struct target_desc *dest,
			 const struct target_desc *src)
{
  dest->reg_defs = src->reg_defs;
  dest->expedite_regs = src->expedite_regs;
  dest->registers_size = src->registers_size;
  dest->xmltarget = src->xmltarget;
}

const struct target_desc *
current_target_desc (void)
{
  if (current_thread == NULL)
    return &default_description;

  return current_process ()->tdesc;
}

/* See common/tdesc.h.  */

void
set_tdesc_architecture (struct target_desc *target_desc,
			const char *name)
{
  target_desc->arch = xstrdup (name);
}

/* See common/tdesc.h.  */

void
set_tdesc_osabi (struct target_desc *target_desc, const char *name)
{
  target_desc->osabi = xstrdup (name);
}

/* Return a string which is of XML format, including XML target
   description to be sent to GDB.  */

const char *
tdesc_get_features_xml (target_desc *tdesc)
{
  /* Either .xmltarget or .features is not NULL.  */
  gdb_assert (tdesc->xmltarget != NULL
	      || (tdesc->features != NULL
		  && tdesc->arch != NULL));

  if (tdesc->xmltarget == NULL)
    {
      std::string buffer ("@<?xml version=\"1.0\"?>");

      buffer += "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">";
      buffer += "<target>";
      buffer += "<architecture>";
      buffer += tdesc->arch;
      buffer += "</architecture>";

      if (tdesc->osabi != nullptr)
	{
	  buffer += "<osabi>";
	  buffer += tdesc->osabi;
	  buffer += "</osabi>";
	}

      char *xml;

      for (int i = 0; VEC_iterate (char_ptr, tdesc->features, i, xml); i++)
	{
	  buffer += "<xi:include href=\"";
	  buffer += xml;
	  buffer += "\"/>";
	}

      buffer += "</target>";

      tdesc->xmltarget = xstrdup (buffer.c_str ());
    }

  return tdesc->xmltarget;
}
#endif

struct tdesc_type
{};

/* See common/tdesc.h.  */

struct tdesc_feature *
tdesc_create_feature (struct target_desc *tdesc, const char *name,
		      const char *xml)
{
#ifndef IN_PROCESS_AGENT
  VEC_safe_push (char_ptr, tdesc->features, xstrdup (xml));
#endif
  return tdesc;
}

/* See common/tdesc.h.  */

tdesc_type_with_fields *
tdesc_create_flags (struct tdesc_feature *feature, const char *name,
		    int size)
{
  return NULL;
}

/* See common/tdesc.h.  */

void
tdesc_add_flag (tdesc_type_with_fields *type, int start,
		const char *flag_name)
{}

/* See common/tdesc.h.  */

struct tdesc_type *
tdesc_named_type (const struct tdesc_feature *feature, const char *id)
{
  return NULL;
}

/* See common/tdesc.h.  */

tdesc_type_with_fields *
tdesc_create_union (struct tdesc_feature *feature, const char *id)
{
  return NULL;
}

/* See common/tdesc.h.  */

tdesc_type_with_fields *
tdesc_create_struct (struct tdesc_feature *feature, const char *id)
{
  return NULL;
}

/* See common/tdesc.h.  */

void
tdesc_create_reg (struct tdesc_feature *feature, const char *name,
		  int regnum, int save_restore, const char *group,
		  int bitsize, const char *type)
{
  tdesc_reg *reg = new tdesc_reg (feature, name, regnum, save_restore,
				  group, bitsize, type);

  tdesc->registers.emplace_back (reg);
}

/* See common/tdesc.h.  */

struct tdesc_type *
tdesc_create_vector (struct tdesc_feature *feature, const char *name,
		     struct tdesc_type *field_type, int count)
{
  return NULL;
}

void
tdesc_add_bitfield (tdesc_type_with_fields *type, const char *field_name,
		    int start, int end)
{}

/* See common/tdesc.h.  */

void
tdesc_add_field (tdesc_type_with_fields *type, const char *field_name,
		 struct tdesc_type *field_type)
{}

/* See common/tdesc.h.  */

void
tdesc_set_struct_size (tdesc_type_with_fields *type, int size)
{
}
