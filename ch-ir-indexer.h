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
 * \file   ch-ir-indexer.h
 *
 * \author sandeepprakash
 *
 * \date   Mar 11, 2014
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __CH_IR_INDEXER_H__
#define __CH_IR_INDEXER_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct _CH_IR_INDEXER_INIT_PARAMS_X
{
   uint32_t ui_token_hm_table_size;

   uint32_t ui_postings_hm_table_size;

   uint32_t ui_max_token_len;

   uint32_t ui_max_filepath_len;

   uint8_t uca_stopwords_filepath[2048];

   bool b_ignore_stopwords;
} CH_IR_INDEXER_INIT_PARAMS_X;

typedef struct _CH_IR_INDEXER_STATS_X
{
   CH_IR_TOKENIZER_STATS_X x_tokenizer_stats;

   uint32_t ui_num_unique_tokens;

   uint32_t ui_num_tokens_occuring_once;

   uint32_t ui_num_tokens_ignored;

   uint32_t ui_tokenization_time_ms;

   uint32_t ui_token_sort_time_ms;
} CH_IR_INDEXER_STATS_X;

typedef struct _CH_IR_DOCS_CTXT_X
{
   HM_HDL hl_doc_hm;
   /*!< #_CH_IR_DOC_HM_ENTRY_X */

   uint32_t ui_doc_count;

   uint32_t ui_all_docs_token_count;

   uint32_t ui_average_token_count;

   uint32_t ui_temp_counter;
} CH_IR_DOCS_CTXT_X;

typedef struct _CH_IR_DOC_HM_ENTRY_X
{
   uint8_t uca_doc_path[2048];

   uint32_t ui_doc_token_count;

   double d_max_term_freq;

   double d_cumulative_w1;

   double d_cumulative_w2;
} CH_IR_DOC_HM_ENTRY_X;

typedef struct _CH_IR_INDEXER_CTXT_X
{
   CH_IR_INDEXER_INIT_PARAMS_X x_init_params;

   CH_IR_INDEXER_STATS_X x_stats;

   CH_IR_TOKENIZER_CTXT_X *px_tokenizer_ctxt;

   CH_IR_DIR_PARSER_CTXT_X *px_dir_parser_ctxt;

   CH_IR_QP_CTXT_X *px_qp_ctxt;

   HM_HDL hl_stopword_hm;

   HM_HDL hl_token_hm;

   CH_IR_DOCS_CTXT_X x_docs;
} CH_IR_INDEXER_CTXT_X;

typedef struct _CH_IR_POSTING_X
{
   uint32_t ui_doc_id;

   uint32_t ui_term_freq;

   uint32_t ui_gap;

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
} CH_IR_POSTING_X;

typedef struct _CH_IR_POSTINGS_X
{
   HM_HDL hl_posting_hm;

   uint32_t ui_doc_freq;
} CH_IR_POSTINGS_X;

typedef struct _CH_IR_TOKEN_HM_ENTRY_X
{
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt;

   uint8_t *puc_token;

   uint32_t ui_num_occurances;

   CH_IR_POSTINGS_X x_postings;
} CH_IR_TOKEN_HM_ENTRY_X;

/***************************** FUNCTION PROTOTYPES ****************************/
CH_IR_RET_E ch_ir_indexer_init(
   CH_IR_INDEXER_INIT_PARAMS_X *px_init_params,
   CH_IR_INDEXER_CTXT_X **ppx_indexer_ctxt);

CH_IR_RET_E ch_ir_indexer_deinit(
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt);

CH_IR_RET_E ch_ir_indexer_build_index (
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt,
   uint8_t *puc_dir_path);

CH_IR_RET_E ch_ir_indexer_print_stats (
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt,
   uint8_t *puc_token);

#ifdef   __cplusplus
}
#endif

#endif /* __CH_IR_INDEXER_H__ */
