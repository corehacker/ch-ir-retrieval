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
 * \date   Apr 13, 2014
 *
 * \brief  
 *
 ******************************************************************************/

#ifndef __CH_IR_QUERY_ANALYZER_H__
#define __CH_IR_QUERY_ANALYZER_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
CH_IR_RET_E ch_ir_indexer_analyze_query_with_index (
   CH_IR_INDEXER_CTXT_X *px_indexer_ctxt,
   uint8_t *puc_query_filepath);

#ifdef   __cplusplus
}
#endif

#endif /* __CH_IR_QUERY_ANALYZER_H__ */
