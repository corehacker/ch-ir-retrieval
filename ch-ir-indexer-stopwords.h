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
 * \file   ch-ir-indexer-stopwords.h
 *
 * \author sandeepprakash
 *
 * \date   Apr 13, 2014
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __CH_IR_INDEXER_STOPWORDS_H__
#define __CH_IR_INDEXER_STOPWORDS_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define DEFAULT_STOPWORDS_HASHMAP_TABLE_SIZE       (100)

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
void ch_ir_indexer_create_stopwords_cache_hm (
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt,
   uint8_t *puc_stopwords_filepath);

HM_RET_E ch_ir_indexer_stopwords_hm_for_each_delete_cbk(
   HM_NODE_DATA_X *px_curr_node_data,
   void *p_app_data);

#ifdef   __cplusplus
}
#endif

#endif /* __CH_IR_INDEXER_STOPWORDS_H__ */
