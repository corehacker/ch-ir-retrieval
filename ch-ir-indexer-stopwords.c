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
 * \file   ch-ir-indexer-stopwords.c
 *
 * \author sandeepprakash
 *
 * \date   Apr 13, 2014
 *
 * \brief  
 *
 ******************************************************************************/

/********************************** INCLUDES **********************************/
#include <ch-pal/exp_pal.h>
#include <ch-utils/exp_list.h>
#include <ch-utils/exp_hashmap.h>
#include "ch-ir-common.h"
#include "ch-ir-dir-parser.h"
#include "ch-ir-tokenizer.h"
#include "ch-ir-query-parser.h"
#include "ch-ir-indexer.h"
#include "ch-ir-indexer-stopwords.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/

/****************************** LOCAL FUNCTIONS *******************************/
void ch_ir_indexer_create_stopwords_cache_hm (
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt,
   uint8_t *puc_stopwords_filepath)
{
   PAL_FILE_HDL hl_file_hdl = NULL;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint32_t ui_line_count = 0;
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   HM_INIT_PARAMS_X x_hm_init_params = { 0 };
   uint32_t ui_line_len = 0;
   uint32_t ui_actual_line_len = 0;
   uint8_t *puc_line = NULL;
   uint32_t ui_max_token_len = 0;
   HM_NODE_DATA_X x_node_data = { eHM_KEY_TYPE_INVALID };

   e_pal_ret = pal_fopen (&hl_file_hdl, puc_stopwords_filepath,
      (const uint8_t *) "r");
   if ((ePAL_RET_SUCCESS == e_pal_ret) && (NULL != hl_file_hdl))
   {
      e_pal_ret = pal_fget_line_count (hl_file_hdl, &ui_line_count);
      if ((ePAL_RET_SUCCESS == e_pal_ret) && (0 != ui_line_count))
      {
         x_hm_init_params.ui_hm_table_size = ui_line_count;
      }
      else
      {
         x_hm_init_params.ui_hm_table_size =
            DEFAULT_STOPWORDS_HASHMAP_TABLE_SIZE;
      }
   }
   else
   {
      x_hm_init_params.ui_hm_table_size = DEFAULT_STOPWORDS_HASHMAP_TABLE_SIZE;
   }

   x_hm_init_params.e_hm_key_type = eHM_KEY_TYPE_STRING;
   e_hm_ret = hm_create (&(px_indexer_ctxt->hl_stopword_hm), &x_hm_init_params);
   if (eHM_RET_SUCCESS != e_hm_ret)
   {
      printf ("hm_create failed: %d\n", e_hm_ret);
      goto LBL_CLEANUP;
   }

   e_pal_ret = pal_frewind(hl_file_hdl);
   if (eHM_RET_SUCCESS != e_hm_ret)
   {
      printf ("pal_frewind failed: %d\n", e_pal_ret);
      goto LBL_CLEANUP;
   }

   ui_max_token_len = px_indexer_ctxt->x_init_params.ui_max_token_len;
   puc_line = pal_malloc (ui_max_token_len, NULL);

   while (1)
   {
      (void) pal_memset(puc_line, 0x00, ui_max_token_len);
      e_pal_ret = pal_freadline_v2 (hl_file_hdl, puc_line, ui_max_token_len,
         &ui_line_len, &ui_actual_line_len);
      if (ePAL_RET_FILE_EOF_REACHED == e_pal_ret)
      {
         e_pal_ret = ePAL_RET_SUCCESS;
         break;
      }
      else
      {
         if (ePAL_RET_FILE_READ_BUF_OVERFLOW == e_pal_ret)
         {
            pal_free (puc_line);
            puc_line = NULL;
            ui_max_token_len = ui_actual_line_len;
            puc_line = pal_malloc (ui_max_token_len, NULL);
            e_pal_ret = pal_frewind(hl_file_hdl);
            if (eHM_RET_SUCCESS != e_hm_ret)
            {
               break;
            }
            else
            {
               continue;
            }
         }
         else if (ePAL_RET_SUCCESS != e_pal_ret)
         {
            PAL_LOG_HIGH("pal_freadline failed: %d", e_pal_ret);
            break;
         }
         else
         {
            ch_ir_indexer_truncate_trailing_whitespace(puc_line);
            (void) pal_memset (&x_node_data, 0x00, sizeof(x_node_data));
            x_node_data.e_hm_key_type = eHM_KEY_TYPE_STRING;
            x_node_data.u_hm_key.puc_str_key = puc_line;
            e_hm_ret = hm_search_node (px_indexer_ctxt->hl_stopword_hm,
               &x_node_data);
            if (eHM_RET_HM_NODE_FOUND != e_hm_ret)
            {
               (void) pal_memset (&x_node_data, 0x00, sizeof(x_node_data));
               x_node_data.e_hm_key_type = eHM_KEY_TYPE_STRING;
               x_node_data.u_hm_key.puc_str_key = puc_line;
               x_node_data.p_data = NULL;
               x_node_data.ui_data_size = 0;
               e_hm_ret = hm_add_node (px_indexer_ctxt->hl_stopword_hm,
                  &x_node_data);
               if (eHM_RET_SUCCESS == e_hm_ret)
               {
                  //printf ("Key \"%s\" Added: %p\n", puc_line,
                  //   x_node_data.p_data);
               }
               else
               {
                  printf ("Key \"%s\" Add failed: %p\n", puc_line,
                     x_node_data.p_data);
                  break;
               }
            }
         }
      }
   }
LBL_CLEANUP:
   if (NULL != puc_line)
   {
      pal_free (puc_line);
      puc_line = NULL;
   }
   if (NULL != hl_file_hdl)
   {
      (void) pal_fclose(hl_file_hdl);
      hl_file_hdl = NULL;
   }
   return;
}

HM_RET_E ch_ir_indexer_stopwords_hm_for_each_delete_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data)
{
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   CH_IR_POSTING_X *px_posting = NULL;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;

   px_indexer_ctxt = (CH_IR_INDEXER_CTXT_X *) p_app_data;
   px_posting = (CH_IR_POSTING_X *) px_curr_node_data->p_data;

   if (NULL != px_curr_node_data->p_data)
   {
      pal_free (px_curr_node_data->p_data);
      px_curr_node_data->p_data = NULL;
   }

   e_hm_ret = eHM_RET_SUCCESS;
LBL_CLEANUP:
   return e_hm_ret;
}
