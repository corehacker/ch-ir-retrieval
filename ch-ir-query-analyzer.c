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
#include "ch-ir-query-analyzer.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static HM_RET_E ch_ir_qa_postings_hm_for_each_analyze_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data);

static LIST_RET_E ch_ir_qa_list_for_all_query_term_cbk(
   LIST_NODE_DATA_X *px_curr_node_data,
   void *p_app_data);

static LIST_RET_E ch_ir_qa_list_for_all_query_term_print_cbk (
      LIST_NODE_DATA_X *px_curr_node_data,
      void *p_app_data);

static HM_RET_E ch_ir_qa_docs_hm_for_each_reset_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data);

static HM_RET_E ch_ir_qa_docs_hm_compare_w1_cbk(
   HM_NODE_DATA_X *px_node_a_data,
   HM_NODE_DATA_X *px_node_b_data,
   void *p_app_data);

static HM_RET_E ch_ir_qa_docs_hm_compare_w2_cbk(
   HM_NODE_DATA_X *px_node_a_data,
   HM_NODE_DATA_X *px_node_b_data,
   void *p_app_data);

static HM_RET_E ch_ir_qa_docs_hm_for_each_top_10_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data);

static CH_IR_RET_E ch_ir_qa_for_each_query_cbk(
   LIST_HDL hl_query_terms_hdl,
   LIST_HDL hl_query_terms_original_hdl,
   uint8_t *puc_query_id,
   void *p_app_data);

/****************************** LOCAL FUNCTIONS *******************************/
static HM_RET_E ch_ir_qa_postings_hm_for_each_analyze_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data)
{
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   CH_IR_POSTING_X *px_posting = NULL;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   HM_NODE_DATA_X x_node_data = { eHM_KEY_TYPE_INVALID };
   CH_IR_TOKEN_HM_ENTRY_X *px_token_hm_entry = NULL;
   CH_IR_DOC_HM_ENTRY_X *px_doc_hm_entry = NULL;

   if ((NULL == px_curr_node_data) || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_hm_ret = eHM_RET_INVALID_ARGS;
      goto LBL_CLEANUP;
   }

   px_token_hm_entry = (CH_IR_TOKEN_HM_ENTRY_X *) p_app_data;
   px_indexer_ctxt = px_token_hm_entry->px_indexer_ctxt;
   px_posting = (CH_IR_POSTING_X *) px_curr_node_data->p_data;

   (void) pal_memset (&x_node_data, 0x00, sizeof(x_node_data));
   x_node_data.e_hm_key_type = eHM_KEY_TYPE_INT;
   x_node_data.u_hm_key.ui_uint_key = px_posting->ui_doc_id;
   e_hm_ret = hm_search_node (px_indexer_ctxt->x_docs.hl_doc_hm,
      &x_node_data);
   if (eHM_RET_HM_NODE_FOUND == e_hm_ret)
   {
      px_doc_hm_entry = (CH_IR_DOC_HM_ENTRY_X *) x_node_data.p_data;
      px_doc_hm_entry->d_cumulative_w1 += px_posting->d_w1;
      px_doc_hm_entry->d_cumulative_w2 += px_posting->d_w2;
   }

   e_hm_ret = eHM_RET_SUCCESS;
LBL_CLEANUP:
   return e_hm_ret;
}

static LIST_RET_E ch_ir_qa_list_for_all_query_term_cbk (
      LIST_NODE_DATA_X *px_curr_node_data,
      void *p_app_data)
{
   LIST_RET_E e_list_ret = eLIST_RET_FAILURE;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   CH_IR_QP_LIST_NODE_X *px_qp_list_node = NULL;
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   HM_NODE_DATA_X x_hm_node_data = {eHM_KEY_TYPE_INVALID};
   CH_IR_TOKEN_HM_ENTRY_X *px_token_hm_entry = NULL;
   HM_FOR_EACH_PARAMS_X x_for_each_params = {eHM_DATA_STRUCT_INVALID};

   if ((NULL == px_curr_node_data) || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_list_ret = eLIST_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_indexer_ctxt = (CH_IR_INDEXER_CTXT_X *) p_app_data;
   px_qp_list_node = (CH_IR_QP_LIST_NODE_X *) px_curr_node_data->p_data;

   // printf ("\t Term: %s", px_qp_list_node->uca_query_term);

   x_hm_node_data.e_hm_key_type = eHM_KEY_TYPE_STRING;
   x_hm_node_data.u_hm_key.puc_str_key = px_qp_list_node->uca_query_term;
   e_hm_ret = hm_search_node(px_indexer_ctxt->hl_token_hm, &x_hm_node_data);
   if (eHM_RET_HM_NODE_FOUND == e_hm_ret)
   {
      //printf ("... Found");
      px_token_hm_entry = (CH_IR_TOKEN_HM_ENTRY_X *) x_hm_node_data.p_data;

      x_for_each_params.e_data_structure = eHM_DATA_STRUCT_LINKED_LIST;
      x_for_each_params.e_direction = eHM_FOR_EACH_DIRECTION_FORWARD;
      e_hm_ret = hm_for_each_v2 (px_token_hm_entry->x_postings.hl_posting_hm,
         &x_for_each_params, ch_ir_qa_postings_hm_for_each_analyze_cbk,
         px_token_hm_entry);
   }
   else
   {
      //printf ("... Not Found");
   }
   //printf ("\n");
   e_list_ret = eLIST_RET_SUCCESS;
CLEAN_RETURN:
   return e_list_ret;
}

static LIST_RET_E ch_ir_qa_list_for_all_query_term_print_cbk (
      LIST_NODE_DATA_X *px_curr_node_data,
      void *p_app_data)
{
   LIST_RET_E e_list_ret = eLIST_RET_FAILURE;
   CH_IR_QP_LIST_NODE_X *px_qp_list_node = NULL;

   if ((NULL == px_curr_node_data) || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_list_ret = eLIST_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_qp_list_node = (CH_IR_QP_LIST_NODE_X *) px_curr_node_data->p_data;

    printf (" %s", px_qp_list_node->uca_query_term);
   e_list_ret = eLIST_RET_SUCCESS;
CLEAN_RETURN:
   return e_list_ret;
}

static HM_RET_E ch_ir_qa_docs_hm_for_each_reset_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data)
{
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   CH_IR_DOC_HM_ENTRY_X *px_doc_hm_entry = NULL;

   if ((NULL == px_curr_node_data) || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_hm_ret = eHM_RET_INVALID_ARGS;
      goto LBL_CLEANUP;
   }

   px_doc_hm_entry = (CH_IR_DOC_HM_ENTRY_X *) px_curr_node_data->p_data;
   px_indexer_ctxt = (CH_IR_INDEXER_CTXT_X *) p_app_data;

   px_doc_hm_entry->d_cumulative_w1 = 0.0;
   px_doc_hm_entry->d_cumulative_w2 = 0.0;

   e_hm_ret = eHM_RET_SUCCESS;
LBL_CLEANUP:
   return e_hm_ret;
}

static HM_RET_E ch_ir_qa_docs_hm_compare_w1_cbk(
   HM_NODE_DATA_X *px_node_a_data,
   HM_NODE_DATA_X *px_node_b_data,
   void *p_app_data)
{
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   CH_IR_DOC_HM_ENTRY_X *px_doc_hm_entry_a = NULL;
   CH_IR_DOC_HM_ENTRY_X *px_doc_hm_entry_b = NULL;

   if ((NULL == px_node_a_data) || (NULL == px_node_b_data)
      || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_hm_ret = eHM_RET_INVALID_ARGS;
      goto LBL_CLEANUP;
   }

   px_doc_hm_entry_a = (CH_IR_DOC_HM_ENTRY_X *) px_node_a_data->p_data;
   px_doc_hm_entry_b = (CH_IR_DOC_HM_ENTRY_X *) px_node_b_data->p_data;

   if (px_doc_hm_entry_a->d_cumulative_w1 > px_doc_hm_entry_b->d_cumulative_w1)
   {
      e_hm_ret = eHM_RET_CMP_LESSER;
   }
   else if (px_doc_hm_entry_a->d_cumulative_w1 < px_doc_hm_entry_b->d_cumulative_w1)
   {
      e_hm_ret = eHM_RET_CMP_GREATER;
   }
   else
   {
      e_hm_ret = eHM_RET_CMP_EQUAL;
   }
LBL_CLEANUP:
   return e_hm_ret;
}

static HM_RET_E ch_ir_qa_docs_hm_compare_w2_cbk(
   HM_NODE_DATA_X *px_node_a_data,
   HM_NODE_DATA_X *px_node_b_data,
   void *p_app_data)
{
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   CH_IR_DOC_HM_ENTRY_X *px_doc_hm_entry_a = NULL;
   CH_IR_DOC_HM_ENTRY_X *px_doc_hm_entry_b = NULL;

   if ((NULL == px_node_a_data) || (NULL == px_node_b_data)
      || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_hm_ret = eHM_RET_INVALID_ARGS;
      goto LBL_CLEANUP;
   }

   px_doc_hm_entry_a = (CH_IR_DOC_HM_ENTRY_X *) px_node_a_data->p_data;
   px_doc_hm_entry_b = (CH_IR_DOC_HM_ENTRY_X *) px_node_b_data->p_data;

   if (px_doc_hm_entry_a->d_cumulative_w2 > px_doc_hm_entry_b->d_cumulative_w2)
   {
      e_hm_ret = eHM_RET_CMP_LESSER;
   }
   else if (px_doc_hm_entry_a->d_cumulative_w2 < px_doc_hm_entry_b->d_cumulative_w2)
   {
      e_hm_ret = eHM_RET_CMP_GREATER;
   }
   else
   {
      e_hm_ret = eHM_RET_CMP_EQUAL;
   }
LBL_CLEANUP:
   return e_hm_ret;
}

static void ch_ir_qa_get_doc_title (
   uint8_t *puc_doc_path,
   uint8_t **ppuc_doc_title,
   uint32_t *pui_doc_title_len)
{
   PAL_FILE_HDL hl_doc_hdl = NULL;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   uint8_t uca_line [MAX_LINE_SIZE] = { 0 };
   uint32_t ui_line_len = 0;
   uint8_t *puc_doc_title = NULL;
   uint32_t ui_cur_doc_title_len = 0;
   uint32_t ui_future_doc_title_len = 0;
   uint32_t ui_allocated_doc_title_len = 0;
   bool b_parsing_title = false;
   bool b_parsing_title_done = false;

   if ((NULL == puc_doc_path) || (NULL == ppuc_doc_title)
      || (NULL == pui_doc_title_len))
   {
      CH_IR_LOG_MED("Invalid Args");
      goto CLEAN_RETURN;
   }

   e_pal_ret = pal_fopen (&hl_doc_hdl, puc_doc_path, (const uint8_t *) "r");
   if (ePAL_RET_SUCCESS != e_pal_ret)
   {
      CH_IR_LOG_MED("pal_fopen failed: %d", e_pal_ret);
      goto CLEAN_RETURN;
   }

   b_parsing_title = false;
   while (1)
   {
      (void) pal_memset(uca_line, 0x00, sizeof(uca_line));
      e_pal_ret = pal_freadline (hl_doc_hdl, uca_line, sizeof(uca_line),
         &ui_line_len);
      if (ePAL_RET_FILE_EOF_REACHED == e_pal_ret)
      {
         break;
      }

      if (0 == pal_strncmp(uca_line, "<TITLE>", sizeof(uca_line)))
      {
         b_parsing_title = true;
         continue;
      }

      if (0 == pal_strncmp(uca_line, "</TITLE>", sizeof(uca_line)))
      {
         if (true == b_parsing_title)
         {
            b_parsing_title = false;
            break;
         }
      }

      if (true == b_parsing_title)
      {
         ui_future_doc_title_len = ui_cur_doc_title_len
                        + pal_strlen (uca_line);
         if (NULL == puc_doc_title)
         {
            ui_allocated_doc_title_len = ui_future_doc_title_len * 2;
            puc_doc_title = pal_malloc(ui_allocated_doc_title_len, NULL);
         }
         else
         {
            ui_future_doc_title_len = ui_cur_doc_title_len
               + pal_strlen (uca_line);
            if (ui_future_doc_title_len >= ui_allocated_doc_title_len)
            {
               ui_allocated_doc_title_len = ui_future_doc_title_len * 2;
               puc_doc_title = realloc (puc_doc_title,
                  ui_allocated_doc_title_len);
            }
         }

         pal_strncpy (puc_doc_title + ui_cur_doc_title_len,
            uca_line,
            (ui_allocated_doc_title_len - ui_cur_doc_title_len));
         ui_cur_doc_title_len += pal_strlen (uca_line);

         pal_strncpy (puc_doc_title + ui_cur_doc_title_len, " ",
            (ui_allocated_doc_title_len - ui_cur_doc_title_len));
         ui_cur_doc_title_len += 1;

         continue;
      }
   }

   e_pal_ret = pal_fclose (hl_doc_hdl);

   *ppuc_doc_title = puc_doc_title;
   *pui_doc_title_len = ui_cur_doc_title_len;
CLEAN_RETURN:
   return;
}

static HM_RET_E ch_ir_qa_docs_hm_for_each_top_10_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data)
{
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   CH_IR_DOC_HM_ENTRY_X *px_doc_hm_entry = NULL;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   uint8_t *puc_doc_title = NULL;
   uint32_t ui_doc_title_len = 0;

   if ((NULL == px_curr_node_data) || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_hm_ret = eHM_RET_INVALID_ARGS;
      goto LBL_CLEANUP;
   }

   px_indexer_ctxt = (CH_IR_INDEXER_CTXT_X *) p_app_data;
   px_doc_hm_entry = (CH_IR_DOC_HM_ENTRY_X *) px_curr_node_data->p_data;

   px_indexer_ctxt->x_docs.ui_temp_counter++;

   if (11 == px_indexer_ctxt->x_docs.ui_temp_counter)
   {
      e_hm_ret = eHM_RET_FAILURE;
   }
   else
   {
      ch_ir_qa_get_doc_title (px_doc_hm_entry->uca_doc_path, &puc_doc_title,
         &ui_doc_title_len);

      printf ("    Rank: %2d | W1: %5.3lf | W2: %5.3lf | Path: %s\n      "
            "Title: %s\n\n",
         px_indexer_ctxt->x_docs.ui_temp_counter,
         px_doc_hm_entry->d_cumulative_w1, px_doc_hm_entry->d_cumulative_w2,
         px_doc_hm_entry->uca_doc_path, puc_doc_title);

      e_hm_ret = eHM_RET_SUCCESS;
   }
LBL_CLEANUP:
   return e_hm_ret;
}

static CH_IR_RET_E ch_ir_qa_for_each_query_cbk (
      LIST_HDL hl_query_terms_hdl,
      LIST_HDL hl_query_terms_original_hdl,
      uint8_t *puc_query_id,
      void *p_app_data)
{
   CH_IR_RET_E e_ret_val = eCH_IR_RET_FAILURE;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   HM_FOR_EACH_PARAMS_X x_for_each_params = {eHM_DATA_STRUCT_INVALID};

   if ((NULL == hl_query_terms_hdl) || (NULL == puc_query_id)
      || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_ret_val = eCH_IR_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }
   px_indexer_ctxt = (CH_IR_INDEXER_CTXT_X *) p_app_data;

   printf ("\n***************************************************************\n");
   printf ("Query: %s", puc_query_id);
   list_for_all_nodes (hl_query_terms_original_hdl,
      ch_ir_qa_list_for_all_query_term_print_cbk, px_indexer_ctxt);
   printf ("\n");
   printf ("Indexed:");
   list_for_all_nodes (hl_query_terms_hdl,
      ch_ir_qa_list_for_all_query_term_print_cbk, px_indexer_ctxt);
   printf ("\n");
   printf ("***************************************************************\n");




   x_for_each_params.e_data_structure = eHM_DATA_STRUCT_LINKED_LIST;
   x_for_each_params.e_direction = eHM_FOR_EACH_DIRECTION_FORWARD;
   e_hm_ret = hm_for_each_v2 (px_indexer_ctxt->x_docs.hl_doc_hm,
      &x_for_each_params, ch_ir_qa_docs_hm_for_each_reset_cbk, px_indexer_ctxt);

   list_for_all_nodes (hl_query_terms_hdl, ch_ir_qa_list_for_all_query_term_cbk,
      px_indexer_ctxt);

   e_hm_ret = hm_linked_list_sort (px_indexer_ctxt->x_docs.hl_doc_hm,
      ch_ir_qa_docs_hm_compare_w1_cbk, px_indexer_ctxt);

   printf ("  W1 Based:\n");
   px_indexer_ctxt->x_docs.ui_temp_counter = 0;
   x_for_each_params.e_data_structure = eHM_DATA_STRUCT_LINKED_LIST;
   x_for_each_params.e_direction = eHM_FOR_EACH_DIRECTION_FORWARD;
   e_hm_ret = hm_for_each_v2 (px_indexer_ctxt->x_docs.hl_doc_hm,
      &x_for_each_params, ch_ir_qa_docs_hm_for_each_top_10_cbk,
      px_indexer_ctxt);

   e_hm_ret = hm_linked_list_sort (px_indexer_ctxt->x_docs.hl_doc_hm,
      ch_ir_qa_docs_hm_compare_w2_cbk, px_indexer_ctxt);

   printf ("  W2 Based:\n");
   px_indexer_ctxt->x_docs.ui_temp_counter = 0;
   x_for_each_params.e_data_structure = eHM_DATA_STRUCT_LINKED_LIST;
   x_for_each_params.e_direction = eHM_FOR_EACH_DIRECTION_FORWARD;
   e_hm_ret = hm_for_each_v2 (px_indexer_ctxt->x_docs.hl_doc_hm,
      &x_for_each_params, ch_ir_qa_docs_hm_for_each_top_10_cbk,
      px_indexer_ctxt);

   e_ret_val = eCH_IR_RET_SUCCESS;
CLEAN_RETURN:
   return e_ret_val;
}

CH_IR_RET_E ch_ir_indexer_analyze_query_with_index (
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt,
   uint8_t *puc_query_filepath)
{
   CH_IR_RET_E e_ret_val = eCH_IR_RET_FAILURE;
   CH_IR_QP_INIT_PARAMS_X x_qp_init_params = {0};

   if ((NULL == px_indexer_ctxt) || (NULL == puc_query_filepath))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_ret_val = eCH_IR_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   x_qp_init_params.ui_max_filepath_len = 2048;
   x_qp_init_params.fn_ch_ir_for_each_query_cbk = ch_ir_qa_for_each_query_cbk;
   x_qp_init_params.p_app_data = px_indexer_ctxt;
   x_qp_init_params.hl_stopwords_hm = px_indexer_ctxt->hl_stopword_hm;
   e_ret_val = ch_ir_qp_init (&x_qp_init_params,
      &(px_indexer_ctxt->px_qp_ctxt));
   if (eCH_IR_RET_SUCCESS != e_ret_val)
   {
      CH_IR_LOG_LOW("ch_ir_qp_init failed: %d", e_ret_val);
      goto CLEAN_RETURN;
   }

   e_ret_val = ch_ir_qp_parse (px_indexer_ctxt->px_qp_ctxt,
      puc_query_filepath);
   if (eCH_IR_RET_SUCCESS != e_ret_val)
   {
      CH_IR_LOG_MED("ch_ir_qa_analyze failed: %d", e_ret_val);
   }
CLEAN_RETURN:
   if (NULL != px_indexer_ctxt)
   {
      if (NULL != px_indexer_ctxt->px_qp_ctxt)
      {
         ch_ir_qp_deinit(px_indexer_ctxt->px_qp_ctxt);
         px_indexer_ctxt->px_qp_ctxt = NULL;
      }
   }
   return e_ret_val;
}
