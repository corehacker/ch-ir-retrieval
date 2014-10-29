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
 * \file   ch-ir-query-analyzer.c
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
#include <ch-utils/exp_list.h>
#include <ch-utils/exp_hashmap.h>
#include "ch-ir-common.h"
#include "ch-ir-tokenizer.h"
#include "ch-ir-query-parser.h"
#include "porter.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static CH_IR_RET_E ch_ir_qp_for_each_token_cbk(
   uint8_t *puc_token,
   uint8_t *puc_file_path,
   uint32_t ui_file_idx,
   uint32_t ui_cur_file_token_count,
   CH_IR_TOKENIZER_CBK_STATUS_E e_status,
   void *p_app_data);

/****************************** LOCAL FUNCTIONS *******************************/
CH_IR_RET_E ch_ir_qp_init(
   CH_IR_QP_INIT_PARAMS_X *px_init_params,
   CH_IR_QP_CTXT_X **ppx_qp_ctxt)
{
   CH_IR_RET_E e_ret_val = eCH_IR_RET_FAILURE;
   CH_IR_QP_CTXT_X *px_qp_ctxt = NULL;
   CH_IR_TOKENIZER_INIT_PARAMS_X x_tok_init_params = {0};

   if ((NULL == px_init_params) || (NULL == ppx_qp_ctxt))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_ret_val = eCH_IR_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if ((NULL == px_init_params->fn_ch_ir_for_each_query_cbk) ||
         (NULL == px_init_params->hl_stopwords_hm))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_ret_val = eCH_IR_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if ((0 == px_init_params->ui_max_filepath_len))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_ret_val = eCH_IR_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_qp_ctxt = pal_malloc (sizeof(CH_IR_QP_CTXT_X), NULL);
   if (NULL == px_qp_ctxt)
   {
      CH_IR_LOG_MED("pal_malloc failed");
      e_ret_val = eCH_IR_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   (void) pal_memmove(&(px_qp_ctxt->x_init_params), px_init_params,
      sizeof(px_qp_ctxt->x_init_params));

   x_tok_init_params.ui_max_token_len = 1024;
   x_tok_init_params.fn_ch_ir_for_each_token_cbk = ch_ir_qp_for_each_token_cbk;
   x_tok_init_params.p_app_data = px_qp_ctxt;
   e_ret_val = ch_ir_tokenizer_init (&x_tok_init_params,
      &(px_qp_ctxt->px_tokenizer_ctxt));
   if (eCH_IR_RET_SUCCESS != e_ret_val)
   {
      CH_IR_LOG_MED("ch_ir_tokenizer_init failed: %d", e_ret_val);
   }
   else
   {
      *ppx_qp_ctxt = px_qp_ctxt;
      e_ret_val = eCH_IR_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_ret_val;
}

CH_IR_RET_E ch_ir_qp_deinit(
   CH_IR_QP_CTXT_X *px_qp_ctxt)
{
   CH_IR_RET_E e_ret_val = eCH_IR_RET_FAILURE;

   if (NULL == px_qp_ctxt)
   {
      CH_IR_LOG_MED("Invalid Args");
      e_ret_val = eCH_IR_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   ch_ir_tokenizer_deinit(px_qp_ctxt->px_tokenizer_ctxt);
   px_qp_ctxt->px_tokenizer_ctxt = NULL;

   pal_free (px_qp_ctxt);
   px_qp_ctxt = NULL;
   e_ret_val = eCH_IR_RET_SUCCESS;

CLEAN_RETURN:
   return e_ret_val;
}

CH_IR_RET_E ch_ir_qp_parse (
   CH_IR_QP_CTXT_X *px_qp_ctxt,
   uint8_t *puc_query_file)
{
   CH_IR_RET_E e_ret_val = eCH_IR_RET_FAILURE;

   if ((NULL == px_qp_ctxt) || (NULL == puc_query_file))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_ret_val = eCH_IR_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_ret_val = ch_ir_tokenizer_parse_tokens_in_file (
      px_qp_ctxt->px_tokenizer_ctxt, puc_query_file, 0);
   if (eCH_IR_RET_SUCCESS != e_ret_val)
   {
      CH_IR_LOG_MED("ch_ir_tokenizer_parse_tokens_in_file failed: %d",
         e_ret_val);
   }
CLEAN_RETURN:
   return e_ret_val;
}

static CH_IR_RET_E ch_ir_qp_for_each_token_cbk(
   uint8_t *puc_token,
   uint8_t *puc_file_path,
   uint32_t ui_file_idx,
   uint32_t ui_cur_file_token_count,
   CH_IR_TOKENIZER_CBK_STATUS_E e_status,
   void *p_app_data)
{
   CH_IR_RET_E e_ret_val = eCH_IR_RET_FAILURE;
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   uint32_t ui_token_len = 0;
   LIST_INIT_PARAMS_X x_init_params = {0};
   LIST_NODE_DATA_X x_list_node_data = {NULL};
   HM_NODE_DATA_X x_hm_node_data = {eHM_KEY_TYPE_INVALID};
   CH_IR_QP_LIST_NODE_X *px_list_node = NULL;
   CH_IR_QP_CTXT_X *px_qp_ctxt = NULL;

   if (eCH_IR_TOKENIZER_CBK_STATUS_CONTINUE == e_status)
   {
      if ((NULL == puc_token) || (NULL == puc_file_path)
         || (NULL == p_app_data))
      {
         CH_IR_LOG_MED("Invalid Args");
         e_ret_val = eCH_IR_RET_INVALID_ARGS;
         goto CLEAN_RETURN;
      }
   }
   else
   {
      if (NULL == p_app_data)
      {
         CH_IR_LOG_MED("Invalid Args");
         e_ret_val = eCH_IR_RET_INVALID_ARGS;
         goto CLEAN_RETURN;
      }
      px_qp_ctxt = (CH_IR_QP_CTXT_X *) p_app_data;
      px_qp_ctxt->x_init_params.fn_ch_ir_for_each_query_cbk (
         px_qp_ctxt->hl_query_terms_hdl,
         px_qp_ctxt->hl_query_terms_original_hdl, px_qp_ctxt->uca_cur_query_id,
         px_qp_ctxt->x_init_params.p_app_data);

      list_delete (px_qp_ctxt->hl_query_terms_hdl);
      px_qp_ctxt->hl_query_terms_hdl = NULL;

      list_delete (px_qp_ctxt->hl_query_terms_original_hdl);
      px_qp_ctxt->hl_query_terms_original_hdl = NULL;

      pal_memset (px_qp_ctxt->uca_cur_query_id, 0x00,
         sizeof(px_qp_ctxt->uca_cur_query_id));

      e_ret_val = eCH_IR_RET_SUCCESS;
      goto CLEAN_RETURN;
   }

   px_qp_ctxt = (CH_IR_QP_CTXT_X *) p_app_data;

   ch_ir_indexer_truncate_trailing_whitespace(puc_token);

   ui_token_len = pal_strlen (puc_token);

   if (0 != ui_token_len)
   {
      if (('q' == puc_token [0]) && (':' == puc_token [ui_token_len - 1]))
      {
         if (NULL != px_qp_ctxt->hl_query_terms_hdl)
         {
            px_qp_ctxt->x_init_params.fn_ch_ir_for_each_query_cbk (
               px_qp_ctxt->hl_query_terms_hdl,
               px_qp_ctxt->hl_query_terms_original_hdl,
               px_qp_ctxt->uca_cur_query_id,
               px_qp_ctxt->x_init_params.p_app_data);

            list_delete (px_qp_ctxt->hl_query_terms_hdl);
            px_qp_ctxt->hl_query_terms_hdl = NULL;

            list_delete (px_qp_ctxt->hl_query_terms_original_hdl);
            px_qp_ctxt->hl_query_terms_original_hdl = NULL;

            pal_memset (px_qp_ctxt->uca_cur_query_id, 0x00,
               sizeof(px_qp_ctxt->uca_cur_query_id));
         }

         pal_strncpy (px_qp_ctxt->uca_cur_query_id, puc_token,
            sizeof(px_qp_ctxt->uca_cur_query_id));
         x_init_params.ui_list_max_elements = 1000;
         list_create (&(px_qp_ctxt->hl_query_terms_hdl), &x_init_params);
         list_create (&(px_qp_ctxt->hl_query_terms_original_hdl),
            &x_init_params);
         e_ret_val = eCH_IR_RET_SUCCESS;
      }
      else
      {
         px_list_node = pal_malloc(sizeof(CH_IR_QP_LIST_NODE_X), NULL);
         pal_strncpy (px_list_node->uca_query_term, puc_token,
            sizeof(px_list_node->uca_query_term));

         (void) pal_memset (&x_list_node_data, 0x00, sizeof(x_list_node_data));
         x_list_node_data.p_data = px_list_node;
         x_list_node_data.ui_data_size = sizeof(*px_list_node);
         list_node_append (px_qp_ctxt->hl_query_terms_original_hdl,
            &x_list_node_data);

         (void) pal_memset (&x_list_node_data, 0x00, sizeof(x_list_node_data));
         x_hm_node_data.e_hm_key_type = eHM_KEY_TYPE_STRING;
         x_hm_node_data.u_hm_key.puc_str_key = puc_token;
         e_hm_ret = hm_search_node (px_qp_ctxt->x_init_params.hl_stopwords_hm,
            &x_hm_node_data);
         if (eHM_RET_HM_NODE_FOUND == e_hm_ret)
         {
            e_ret_val = eCH_IR_RET_SUCCESS;
         }
         else
         {
            porter_stem((char *) puc_token);
            px_list_node = pal_malloc(sizeof(CH_IR_QP_LIST_NODE_X), NULL);
            pal_strncpy (px_list_node->uca_query_term, puc_token,
               sizeof(px_list_node->uca_query_term));

            (void) pal_memset (&x_list_node_data, 0x00,
               sizeof(x_list_node_data));
            x_list_node_data.p_data = px_list_node;
            x_list_node_data.ui_data_size = sizeof(*px_list_node);
            list_node_append (px_qp_ctxt->hl_query_terms_hdl,
               &x_list_node_data);
            e_ret_val = eCH_IR_RET_SUCCESS;
         }
      }
   }
CLEAN_RETURN:
   return e_ret_val;
}
