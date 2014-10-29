/*******************************************************************************
 *  Repository for C modules.
 *  Copyright (C) 2014 Sandeep Prakash
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ******************************************************************************/

/*******************************************************************************
 * Copyright (c) 2014, Sandeep Prakash <123sandy@gmail.com>
 *
 * \file   ch-ir_common.c
 *
 * \author sandeepprakash
 *
 * \date   Apr 12, 2014
 *
 * \brief  
 *
 ******************************************************************************/

/********************************** INCLUDES **********************************/
#include <ch-pal/exp_pal.h>
#include "ch-ir-common.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/

/****************************** LOCAL FUNCTIONS *******************************/
void ch_ir_indexer_truncate_trailing_whitespace(
   uint8_t *puc_string)
{
   uint32_t ui_len = 0;
   uint32_t ui_i = 0;

   ui_len = pal_strlen (puc_string);

   if (1 == ui_len)
   {
      if (' ' == puc_string [0] || '\t' == puc_string [0]
         || '\n' == puc_string [0] || '\r' == puc_string [0])
      {
         puc_string [0] = '\0';
      }
      return;
   }

   while (ui_i < ui_len)
   {
      if (' ' == puc_string [ui_i] || '\t' == puc_string [ui_i]
         || '\n' == puc_string [ui_i] || '\r' == puc_string [ui_i])
      {
         ui_i++;
         continue;
      }
      else
      {
         (void) pal_memmove (&(puc_string [0]), &(puc_string [ui_i]),
            (ui_len - ui_i + 1));
         break;
      }
   }

   ui_len = ui_len - ui_i - 1;
   while (ui_len >= 0)
   {
      if (' ' == puc_string [ui_len] || '\t' == puc_string [ui_len]
         || '\n' == puc_string [ui_len] || '\r' == puc_string [ui_len])
      {
         ui_len--;
         continue;
      }
      else
      {
         puc_string [ui_len + 1] = '\0';
         break;
      }
   }
}
