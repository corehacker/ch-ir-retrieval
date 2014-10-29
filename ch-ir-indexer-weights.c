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
 * \file   ch-ir-indexer-weights.c
 *
 * \author sandeepprakash
 *
 * \date   Apr 13, 2014
 *
 * \brief  
 *
 ******************************************************************************/

/********************************** INCLUDES **********************************/
#include <math.h>
#include <ch-pal/exp_pal.h>
#include <ch-utils/exp_list.h>
#include <ch-utils/exp_hashmap.h>
#include "ch-ir-common.h"
#include "ch-ir-dir-parser.h"
#include "ch-ir-tokenizer.h"
#include "ch-ir-query-parser.h"
#include "ch-ir-indexer.h"
#include "ch-ir-indexer-weights.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static HM_RET_E ch_ir_indexer_token_hm_for_each_calculate_weights_cbk (
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data);

static HM_RET_E ch_ir_indexer_postings_hm_for_each_calculate_weights_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data);

/****************************** LOCAL FUNCTIONS *******************************/
static HM_RET_E ch_ir_indexer_postings_hm_for_each_calculate_weights_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data)
{
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   CH_IR_POSTING_X *px_posting = NULL;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   CH_IR_TOKEN_HM_ENTRY_X *px_token_hm_entry = NULL;
   HM_NODE_DATA_X x_node_data = { eHM_KEY_TYPE_INVALID };
   CH_IR_DOC_HM_ENTRY_X *px_doc_hm_entry = NULL;
   double d_log1 = 0.0;
   double d_log2 = 0.0;
   double d_log3 = 0.0;
   double d_log4 = 0.0;

   double d_term1 = 0.0;
   double d_term2 = 0.0;
   double d_w1;
   /*
    * W1 = (0.4 + 0.6 * log (tf + 0.5) / log (maxtf + 1.0))
    *  * (log (collectionsize / df)/ log (collectionsize))
    */

   double d_w2;
   /*
    * W2 = (0.4 + 0.6 * (tf / (tf + 0.5 + 1.5 *
    * (doclen / avgdoclen))) * log (collectionsize / df)/
    * log (collectionsize))
    */

   if ((NULL == px_curr_node_data) || (NULL == p_app_data))
   {
      CH_IR_LOG_MED("Invalid Args");
      e_hm_ret = eHM_RET_INVALID_ARGS;
      goto LBL_CLEANUP;
   }

   px_token_hm_entry = (CH_IR_TOKEN_HM_ENTRY_X *) p_app_data;

   if (NULL == px_token_hm_entry->px_indexer_ctxt)
   {
      CH_IR_LOG_MED("Invalid Args");
      e_hm_ret = eHM_RET_INVALID_ARGS;
      goto LBL_CLEANUP;
   }

   px_indexer_ctxt = px_token_hm_entry->px_indexer_ctxt;
   px_posting = (CH_IR_POSTING_X *) px_curr_node_data->p_data;

   if (NULL != px_indexer_ctxt->x_docs.hl_doc_hm)
   {
      (void) pal_memset (&x_node_data, 0x00, sizeof(x_node_data));
      x_node_data.e_hm_key_type = eHM_KEY_TYPE_INT;
      x_node_data.u_hm_key.ui_uint_key = px_posting->ui_doc_id;
      e_hm_ret = hm_search_node (px_indexer_ctxt->x_docs.hl_doc_hm,
         &x_node_data);
      if (eHM_RET_HM_NODE_FOUND == e_hm_ret)
      {
         px_doc_hm_entry = (CH_IR_DOC_HM_ENTRY_X *) x_node_data.p_data;
      }
   }


   /*
    * W1 = (0.4 + 0.6 * log (tf + 0.5) / log (maxtf + 1.0))
    *  * (log (collectionsize / df)/ log (collectionsize))
    */
   d_log1 = log10 ((double) px_posting->ui_term_freq + 0.5);
   d_log2 = log10 (px_doc_hm_entry->d_max_term_freq + 1.0); // Add Max Term Frequency
   d_log3 = log10 ((double) px_indexer_ctxt->x_docs.ui_doc_count
      / (double) px_token_hm_entry->x_postings.ui_doc_freq);
   d_log4 = log10 ((double) px_indexer_ctxt->x_docs.ui_doc_count);

   d_w1 = (0.4 + 0.6 * d_log1 / d_log2) * (d_log3 / d_log4);


   /*
    * W2 = (0.4 + 0.6 * (tf / (tf + 0.5 + 1.5 *
    * (doclen / avgdoclen))) * log (collectionsize / df)/
    * log (collectionsize))
    */
   d_log1 = log10 ((double) px_indexer_ctxt->x_docs.ui_doc_count
         / (double) px_token_hm_entry->x_postings.ui_doc_freq);
   d_log2 = log10 ((double) px_indexer_ctxt->x_docs.ui_doc_count);

   d_term1 = ((double) px_doc_hm_entry->ui_doc_token_count
      / (double) px_indexer_ctxt->x_docs.ui_average_token_count);
   d_term2 = ((double) px_posting->ui_term_freq
      / ((double) px_posting->ui_term_freq + 0.5 + 1.5 * d_term1));
   d_w2 = (0.4 + 0.6 * d_term2 * d_log1 / d_log2);

   px_posting->d_w1 = d_w1;
   px_posting->d_w2 = d_w2;

   // printf ("W1: %lf :: W2: %lf\n", d_w1, d_w2);
   e_hm_ret = eHM_RET_SUCCESS;
LBL_CLEANUP:
   return e_hm_ret;
}

static HM_RET_E ch_ir_indexer_token_hm_for_each_calculate_weights_cbk (
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data)
{
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   CH_IR_TOKEN_HM_ENTRY_X *px_token_hm_entry = NULL;
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt = NULL;
   HM_FOR_EACH_PARAMS_X x_for_each_param = {eHM_FOR_EACH_DIRECTION_INVALID};

   px_indexer_ctxt = (CH_IR_INDEXER_CTXT_X *) p_app_data;
   px_token_hm_entry = px_curr_node_data->p_data;

   x_for_each_param.e_data_structure = eHM_DATA_STRUCT_LINKED_LIST;
   x_for_each_param.e_direction = eHM_FOR_EACH_DIRECTION_FORWARD;
   e_hm_ret = hm_for_each_v2 (px_token_hm_entry->x_postings.hl_posting_hm,
      &x_for_each_param,
      ch_ir_indexer_postings_hm_for_each_calculate_weights_cbk,
      px_token_hm_entry);

   e_hm_ret = eHM_RET_SUCCESS;
LBL_CLEANUP:
   return e_hm_ret;
}

CH_IR_RET_E ch_ir_indexer_calculate_weights (
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt)
{
   CH_IR_RET_E e_ret_val = eCH_IR_RET_FAILURE;
   HM_FOR_EACH_PARAMS_X x_for_each_param = {eHM_DATA_STRUCT_INVALID};
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;

   if (NULL == px_indexer_ctxt)
   {
      CH_IR_LOG_MED("Invalid Args");
      e_ret_val = eCH_IR_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   x_for_each_param.e_data_structure = eHM_DATA_STRUCT_LINKED_LIST;
   x_for_each_param.e_direction = eHM_FOR_EACH_DIRECTION_FORWARD;
   e_hm_ret = hm_for_each_v2 (px_indexer_ctxt->hl_token_hm,
      &x_for_each_param, ch_ir_indexer_token_hm_for_each_calculate_weights_cbk,
      px_indexer_ctxt);

   e_ret_val = eCH_IR_RET_SUCCESS;
CLEAN_RETURN:
   return e_ret_val;
}
