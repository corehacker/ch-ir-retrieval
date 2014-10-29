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
 * \file   ch-ir-query-analyzer.h
 *
 * \author sandeepprakash
 *
 * \date   Apr 12, 2014
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __CH_IR_QUERY_PARSER_H__
#define __CH_IR_QUERY_PARSER_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define CH_IR_QP_MAX_FILENAME_LEN                           (16384)

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct _CH_IR_QP_LIST_NODE_X
{
   uint8_t uca_query_term[1024];
} CH_IR_QP_LIST_NODE_X;

typedef CH_IR_RET_E (*pfn_ch_ir_for_each_query_cbk)(
   LIST_HDL hl_query_terms_hdl,
   LIST_HDL hl_query_terms_original_hdl,
   uint8_t *puc_query_id,
   void *p_app_data);

typedef struct _CH_IR_QP_INIT_PARAMS_X
{
   uint32_t ui_max_filepath_len;

   pfn_ch_ir_for_each_query_cbk fn_ch_ir_for_each_query_cbk;

   void *p_app_data;

   HM_HDL hl_stopwords_hm;
} CH_IR_QP_INIT_PARAMS_X;

typedef struct _CH_IR_QP_CTXT_X
{
   CH_IR_QP_INIT_PARAMS_X x_init_params;

   CH_IR_TOKENIZER_CTXT_X *px_tokenizer_ctxt;

   uint8_t uca_cur_query_id[1024];

   LIST_HDL hl_query_terms_hdl;

   LIST_HDL hl_query_terms_original_hdl;
} CH_IR_QP_CTXT_X;

/***************************** FUNCTION PROTOTYPES ****************************/
CH_IR_RET_E ch_ir_qp_init(
   CH_IR_QP_INIT_PARAMS_X *px_init_params,
   CH_IR_QP_CTXT_X **ppx_qp_ctxt);

CH_IR_RET_E ch_ir_qp_deinit(
   CH_IR_QP_CTXT_X *px_qp_ctxt);

CH_IR_RET_E ch_ir_qp_parse (
   CH_IR_QP_CTXT_X *px_qp_ctxt,
   uint8_t *puc_query_file);

#ifdef   __cplusplus
}
#endif

#endif /* __CH_IR_QUERY_PARSER_H__ */
