/* TA-LIB Copyright (c) 1999-2003, Mario Fortier
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither name of author nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* List of contributors:
 *
 *  Initial  Name/description
 *  -------------------------------------------------------------------
 *  PK       Pawel Konieczny
 *
 *
 * Change history:
 *
 *  MMDDYY BY   Description
 *  -------------------------------------------------------------------
 *  101703 PK   First version.
 *  110103 PK   Minidriver architecture
 *
 */

/* Description:
 *    This is the entry points of the data source driver for SQL database.
 *    It depend on the mysql++ library
 *
 *    It provides ALL the functions needed by the "TA_DataSourceDriver"
 *    structure (see ta_source.h).
 */

/**** Headers ****/
#include <ctype.h>
#include <string.h>
#include "ta_source.h"
#include "ta_common.h"
#include "ta_trace.h"
#include "ta_memory.h"
#include "ta_list.h"
#include "ta_global.h"
#include "ta_data.h"
#include "ta_system.h"
#include "ta_sql.h"
#include "ta_sql_handle.h"
#include "ta_sql_local.h"

/**** External functions declarations. ****/
/* None */

/**** External variables declarations. ****/
/* None */

/**** Global variables definitions.    ****/
/* None */

/**** Local declarations.              ****/
/* None */

/**** Local functions declarations.    ****/

static TA_RetCode executeDataQuery( TA_PrivateSQLHandle *privateHandle, 
                                    const char          *queryStr, 
                                    TA_Period            period,
                                    TA_Field             fieldToAlloc,
                                    TA_ParamForAddData  *paramForAddData );

static const char *formatISODate(const TA_Timestamp *ts);
static const char *formatISOTime(const TA_Timestamp *ts);


/**** Local variables definitions.     ****/
TA_FILE_INFO;

#if !defined( TA_SINGLE_THREAD )
TA_Sema mod_sema;
#endif

/**** Global functions definitions.   ****/

TA_RetCode TA_SQL_InitializeSourceDriver( void )
{
   TA_PROLOG
   TA_RetCode retCode = TA_SUCCESS;

   TA_TRACE_BEGIN(  TA_SQL_InitializeSourceDriver );

#if !defined( TA_SINGLE_THREAD )
   retCode = TA_SemaInit( &mod_sema, 1);
#endif

   TA_TRACE_RETURN( retCode );
}



TA_RetCode TA_SQL_ShutdownSourceDriver( void )
{
   TA_PROLOG
   TA_RetCode retCode = TA_SUCCESS;

   TA_TRACE_BEGIN(  TA_SQL_ShutdownSourceDriver );

#if !defined( TA_SINGLE_THREAD )
   retCode = TA_SemaDestroy( &mod_sema );
#endif

   TA_TRACE_RETURN( retCode );
}



TA_RetCode TA_SQL_GetParameters( TA_DataSourceParameters *param )
{
   TA_PROLOG
   TA_TRACE_BEGIN( TA_SQL_GetParameters );

   memset( param, 0, sizeof( TA_DataSourceParameters ) );

   /* Parameters supported by TA_SQL */
   param->flags = TA_REPLACE_ZERO_PRICE_BAR;

   TA_TRACE_RETURN( TA_SUCCESS );
}



TA_RetCode TA_SQL_OpenSource( const TA_AddDataSourceParamPriv *param,
                              TA_DataSourceHandle **handle )
{
   TA_PROLOG
   TA_DataSourceHandle *tmpHandle;
   TA_PrivateSQLHandle *privData;
   char *scheme, *host, *dbase;
   unsigned port;
   TA_RetCode retCode;

   *handle = NULL;

   TA_TRACE_BEGIN( TA_SQL_OpenSource );

   /* 'info' and 'location' are mandatory. */
   if( (!param->info) || (!param->location) )
   {
      TA_TRACE_RETURN( TA_BAD_PARAM );
   }

   /* get parameters for SQL connection */
   scheme = (char*)TA_Malloc(strlen(TA_StringToChar(param->location)));
   if (scheme == NULL)
   {
      TA_TRACE_RETURN( TA_ALLOC_ERR );
   }
   host = (char*)TA_Malloc(strlen(TA_StringToChar(param->location)));
   if (host == NULL)
   {
      TA_Free(scheme);
      TA_TRACE_RETURN( TA_ALLOC_ERR );
   }
   dbase = (char*)TA_Malloc(strlen(TA_StringToChar(param->location)));
   if (dbase == NULL)
   {
      TA_Free(scheme);
      TA_Free(host);
      TA_TRACE_RETURN( TA_ALLOC_ERR );
   }
   retCode = TA_SQL_ParseLocation(TA_StringToChar(param->location), scheme, host, &port, dbase);
   if( retCode != TA_SUCCESS )
   {
      TA_Free(scheme);
      TA_Free(host);
      TA_Free(dbase);
      TA_TRACE_RETURN( retCode );
   }

   /* Allocate and initialize the handle. This function will also allocate the
    * private handle (opaque data).
    */
   tmpHandle = TA_SQL_DataSourceHandleAlloc(param);

   if( tmpHandle == NULL )
   {
      TA_Free(scheme);
      TA_Free(host);
      TA_Free(dbase);
      TA_TRACE_RETURN( TA_ALLOC_ERR );
   }

   privData = (TA_PrivateSQLHandle *)(tmpHandle->opaqueData);

   /* Determine the minidriver to use */
   privData->minidriver = TA_SQL_IdentifyMinidriver( scheme );

   /* Establish the connection with the SQL server */
   if(  privData->minidriver < TA_SQL_NUM_OF_MINIDRIVERS 
     && TA_gSQLMinidriverTable[privData->minidriver].openConnection )
   {
      retCode = (*TA_gSQLMinidriverTable[privData->minidriver].openConnection)(
         dbase, 
         host, 
         TA_StringToChar(param->username), 
         TA_StringToChar(param->password), 
         port,
         &privData->connection);
   }
   else
   {    /* minidriver not recognized */
      retCode = TA_NOT_SUPPORTED;
   }

   if( retCode != TA_SUCCESS )
   {
      TA_Free(scheme);
      TA_Free(host);
      TA_Free(dbase);
      TA_SQL_DataSourceHandleFree( tmpHandle );
      TA_TRACE_RETURN( retCode );
   }

   /* Database name is a fallback symbol name when not available from parameters */
   privData->database = TA_StringAlloc(TA_GetGlobalStringCache(), dbase);
   TA_Free(scheme);
   TA_Free(host);
   TA_Free(dbase);
   if( !privData->database )
   {
      TA_SQL_DataSourceHandleFree( tmpHandle );
      TA_TRACE_RETURN( TA_ALLOC_ERR );
   }

   /* Now build the symbols index. */
   retCode = TA_SQL_BuildSymbolsIndex( tmpHandle );

   if( retCode != TA_SUCCESS )
   {
      TA_SQL_DataSourceHandleFree( tmpHandle );
      TA_TRACE_RETURN( retCode );
   }

   /* Set the total number of distinct categories. */
   tmpHandle->nbCategory = TA_ListSize(privData->theCategoryIndex);

   *handle = tmpHandle;

   TA_TRACE_RETURN( TA_SUCCESS );
}



TA_RetCode TA_SQL_CloseSource( TA_DataSourceHandle *handle )
{
   TA_PROLOG

   TA_TRACE_BEGIN(  TA_SQL_CloseSource );

   /* Free all ressource used by this handle. */
   if( handle )
      TA_SQL_DataSourceHandleFree( handle );

   TA_TRACE_RETURN( TA_SUCCESS );
}



TA_RetCode TA_SQL_GetFirstCategoryHandle( TA_DataSourceHandle *handle,
                                          TA_CategoryHandle   *categoryHandle )
{
   TA_PROLOG
   TA_PrivateSQLHandle *privData;
   TA_List               *categoryIndex;
   TA_SQLCategoryNode  *categoryNode;

   TA_TRACE_BEGIN(  TA_SQL_GetFirstCategoryHandle );

   if( (handle == NULL) || (categoryHandle == NULL) )
   {
      TA_TRACE_RETURN( TA_BAD_PARAM );
   }

   privData = (TA_PrivateSQLHandle *)(handle->opaqueData);

   if( !privData )
   {
      TA_TRACE_RETURN( (TA_RetCode)TA_INTERNAL_ERROR(49) );
   }

   categoryIndex = privData->theCategoryIndex;

   if( !categoryIndex )
   {
      TA_TRACE_RETURN( (TA_RetCode)TA_INTERNAL_ERROR(50) );
   }

   /* Get the first category from the category index */
   categoryNode = (TA_SQLCategoryNode*)TA_ListAccessHead(categoryIndex);

   if( !categoryNode || !categoryNode->category )
   {
      TA_TRACE_RETURN( (TA_RetCode)TA_INTERNAL_ERROR(51) ); /* At least one category must exist. */
   }

   /* Set the categoryHandle. */
   categoryHandle->string = categoryNode->category;
   categoryHandle->nbSymbol = TA_ListSize(categoryNode->theSymbols);
   categoryHandle->opaqueData = categoryNode->theSymbols;

   TA_TRACE_RETURN( TA_SUCCESS );
}



TA_RetCode TA_SQL_GetNextCategoryHandle( TA_DataSourceHandle *handle,
                                         TA_CategoryHandle   *categoryHandle,
                                         unsigned int         index )
{
   TA_PROLOG
   TA_PrivateSQLHandle *privData;
   TA_List               *categoryIndex;
   TA_SQLCategoryNode  *categoryNode;

   TA_TRACE_BEGIN(  TA_SQL_GetNextCategoryHandle );

   (void)index; /* Get rid of compiler warnings. */

   if( (handle == NULL) || (categoryHandle == NULL) )
   {
      TA_TRACE_RETURN( TA_BAD_PARAM );
   }

   privData = (TA_PrivateSQLHandle *)(handle->opaqueData);

   if( !privData )
   {
      TA_TRACE_RETURN( (TA_RetCode)TA_INTERNAL_ERROR(52) );
   }

   categoryIndex = privData->theCategoryIndex;

   if( !categoryIndex )
   {
      TA_TRACE_RETURN( (TA_RetCode)TA_INTERNAL_ERROR(50) );
   }

   /* Get the next category from the category index */
   categoryNode = (TA_SQLCategoryNode*)TA_ListAccessNext(categoryIndex);

   if( !categoryNode )
   {
      TA_TRACE_RETURN( TA_END_OF_INDEX );
   }

   /* Set the categoryHandle. */
   categoryHandle->string = categoryNode->category;
   categoryHandle->nbSymbol = TA_ListSize(categoryNode->theSymbols);
   categoryHandle->opaqueData = categoryNode->theSymbols;

   TA_TRACE_RETURN( TA_SUCCESS );
}

TA_RetCode TA_SQL_GetFirstSymbolHandle( TA_DataSourceHandle *handle,
                                          TA_CategoryHandle   *categoryHandle,
                                          TA_SymbolHandle     *symbolHandle )
{
   TA_PROLOG
   TA_PrivateSQLHandle *privData;
   TA_List               *symbolsIndex;
   TA_String             *symbol;

   TA_TRACE_BEGIN(  TA_SQL_GetFirstSymbolHandle );

   if( (handle == NULL) || (categoryHandle == NULL) || (symbolHandle == NULL) )
   {
      TA_TRACE_RETURN( TA_BAD_PARAM );
   }

   privData = (TA_PrivateSQLHandle *)(handle->opaqueData);

   if( !privData )
   {
      TA_TRACE_RETURN( (TA_RetCode)TA_INTERNAL_ERROR(49) );
   }

   symbolsIndex = (TA_List*)categoryHandle->opaqueData;

   /* Get the first symbol in this category. */
   symbol = (TA_String*)TA_ListAccessHead(symbolsIndex);

   if( !symbol )
   {
      TA_TRACE_RETURN( TA_END_OF_INDEX );
   }

   /* Set the symbolHandle. */
   symbolHandle->string = symbol;
   symbolHandle->opaqueData = NULL;  /* not needed */

   TA_TRACE_RETURN( TA_SUCCESS );
}



TA_RetCode TA_SQL_GetNextSymbolHandle( TA_DataSourceHandle *handle,
                                       TA_CategoryHandle   *categoryHandle,
                                       TA_SymbolHandle     *symbolHandle,
                                       unsigned int         index )
{
   TA_PROLOG
   TA_PrivateSQLHandle *privData;
   TA_List               *symbolsIndex;
   TA_String             *symbol;

   TA_TRACE_BEGIN(  TA_SQL_GetNextSymbolHandle );

   (void)index; /* Get rid of compiler warnings. */

   if( (handle == NULL) || (categoryHandle == NULL) || (symbolHandle == NULL) )
   {
      TA_TRACE_RETURN( TA_BAD_PARAM );
   }

   privData = (TA_PrivateSQLHandle *)(handle->opaqueData);

   if( !privData )
   {
      TA_TRACE_RETURN( (TA_RetCode)TA_INTERNAL_ERROR(57) );
   }

   symbolsIndex = (TA_List*)categoryHandle->opaqueData;

   /* Get the first symbol in this category. */
   symbol = (TA_String*)TA_ListAccessNext(symbolsIndex);

   if( !symbol )
   {
      TA_TRACE_RETURN( TA_END_OF_INDEX );
   }

   /* Set the symbolHandle. */
   symbolHandle->string = symbol;
   symbolHandle->opaqueData = NULL;  /* not needed */

   TA_TRACE_RETURN( TA_SUCCESS );
}




TA_RetCode TA_SQL_GetHistoryData( TA_DataSourceHandle *handle,
                                  TA_CategoryHandle   *categoryHandle,
                                  TA_SymbolHandle     *symbolHandle,
                                  TA_Period            period,
                                  const TA_Timestamp  *start,
                                  const TA_Timestamp  *end,
                                  TA_Field             fieldToAlloc,
                                  TA_ParamForAddData  *paramForAddData )
{
   TA_PROLOG
   TA_RetCode retCode = TA_SUCCESS;
   TA_PrivateSQLHandle *privateHandle;
   char *queryStr, *tempStr;
   TA_Timestamp trueEnd;


   TA_TRACE_BEGIN(  TA_SQL_GetHistoryData );

   TA_ASSERT( handle != NULL );

   if (  (start && TA_TimestampValidate(start) != TA_SUCCESS)
      || (end && TA_TimestampValidate(end) != TA_SUCCESS))
       return TA_BAD_PARAM;

   privateHandle = (TA_PrivateSQLHandle *)handle->opaqueData;
   TA_ASSERT( privateHandle != NULL );

   TA_ASSERT( paramForAddData != NULL );
   TA_ASSERT( categoryHandle != NULL );
   TA_ASSERT( symbolHandle != NULL );


   /* verify whether this source can deliver data usable for the requested period */
   if( privateHandle->param->period > 0)
   {
      /* this check can be done only when a period is known for this source */
      if( period < privateHandle->param->period )
         return TA_SUCCESS;  /* no usable data, but also no error */

      period = privateHandle->param->period;  /* to be passed to ta_history */
   }
   else
   {
      /* just assume that this source has usable data for the requested period */
   }
   
   /* we need a valid end timestamp for placeholder expansion;
    * if end not defined, take maximal value
    */
   if ( end )
   {
      trueEnd = *end;
   }
   else /* default value 0 means no upper bound */
   {
      TA_SetDate(9999,12,31,&trueEnd);  /* need some high value to substitute in SQL */
      TA_SetTime(23,59,59,&trueEnd);
   }

   /* Replace all relevant placeholders */
#if !defined( TA_SINGLE_THREAD )
   retCode = TA_SemaWait( &mod_sema );
   if( retCode != TA_SUCCESS )
   {
      TA_TRACE_RETURN(retCode);
   }
#endif
   queryStr = TA_SQL_ExpandPlaceholders(TA_StringToChar(privateHandle->param->info),
                                        TA_SQL_CATEGORY_PLACEHOLDER,
                                        TA_StringToChar(categoryHandle->string));

   tempStr = queryStr;
   queryStr = TA_SQL_ExpandPlaceholders(tempStr,
                                        TA_SQL_SYMBOL_PLACEHOLDER,
                                        TA_StringToChar(symbolHandle->string));
   if ( tempStr)
      TA_Free( tempStr );

   tempStr = queryStr;
   queryStr = TA_SQL_ExpandPlaceholders(tempStr,
                                        TA_SQL_START_DATE_PLACEHOLDER,
                                        formatISODate(start));

   if ( tempStr)
      TA_Free( tempStr );
   tempStr = queryStr;
   queryStr = TA_SQL_ExpandPlaceholders(tempStr,
                                        TA_SQL_END_DATE_PLACEHOLDER,
                                        formatISODate(&trueEnd));

   if ( tempStr)
      TA_Free( tempStr );

   tempStr = queryStr;
   queryStr = TA_SQL_ExpandPlaceholders(tempStr,
                                        TA_SQL_START_TIME_PLACEHOLDER,
                                        formatISOTime(start));

   if ( tempStr)
      TA_Free( tempStr );

   tempStr = queryStr;
   queryStr = TA_SQL_ExpandPlaceholders(tempStr,
                                        TA_SQL_END_TIME_PLACEHOLDER,
                                        formatISOTime(&trueEnd));

   if ( tempStr)
      TA_Free( tempStr );

#if !defined( TA_SINGLE_THREAD )
   retCode = TA_SemaPost( &mod_sema );
   if( retCode != TA_SUCCESS )
   {
      if ( queryStr )
         TA_Free( queryStr );
      TA_TRACE_RETURN(retCode);
   }
#endif
   if( !queryStr )
   {
       TA_TRACE_RETURN( TA_ALLOC_ERR );
   }

   /* Now the SQL query */
   retCode = executeDataQuery( privateHandle, queryStr, period, fieldToAlloc, paramForAddData );

   TA_Free( queryStr );

   TA_TRACE_RETURN( retCode );
}



/**** Local functions definitions.     ****/

static TA_RetCode executeDataQuery( TA_PrivateSQLHandle *privateHandle, 
                                    const char          *queryStr, 
                                    TA_Period            period,
                                    TA_Field             fieldToAlloc,
                                    TA_ParamForAddData  *paramForAddData )
{
   TA_PROLOG
   TA_RetCode retCode = TA_SUCCESS;
   unsigned int timeRequired = (period < TA_DAILY);      /* Boolean */

   /* result vectors */
   TA_Timestamp *timestampVec = NULL;
   TA_Real *openVec = NULL;
   TA_Real *highVec = NULL;
   TA_Real *lowVec = NULL;
   TA_Real *closeVec = NULL;
   TA_Integer *volumeVec = NULL;
   TA_Integer *oiVec = NULL;

   /* recognized columns */
   int dateCol, timeCol, openCol, highCol, lowCol, closeCol, volumeCol, oiCol;

   void *queryResult;
   int resColumns, resRows = -1;
   int colNum, rowNum, barNum;
   const char *strval = NULL;


   TA_TRACE_BEGIN( executeDataQuery );

   /* Now the SQL query */
   retCode = (*TA_gSQLMinidriverTable[privateHandle->minidriver].executeQuery)(
               privateHandle->connection,
               queryStr,
               &queryResult);
   if( retCode != TA_SUCCESS )
   {
      TA_TRACE_RETURN( retCode );
   }
   
   /* from now on: the query result has to be released upon premature return 
    * later, also allocated vectors have to be released
    */
   #define RETURN_ON_ERROR( rc )                            \
         if( rc != TA_SUCCESS )                             \
         {                                                  \
            retCode = rc;                                   \
            goto executeDataQuery_cleanup;                  \
         }


   /* find recognized columns */
   dateCol = timeCol = openCol = highCol = lowCol = closeCol = volumeCol = oiCol = -1;

   retCode = (*TA_gSQLMinidriverTable[privateHandle->minidriver].getNumColumns)(
                  queryResult,
                  &resColumns );
   RETURN_ON_ERROR( retCode );

   retCode = (*TA_gSQLMinidriverTable[privateHandle->minidriver].getNumRows)(
                  queryResult,
                  &resRows );
   if( retCode != TA_SUCCESS || resRows <= 0 )
   {
      /* not all minidrivers support reporting the number of rows in a query in advance
       * it is not a disaster, just less efficient
       * data will be collected in fixed size chunks
       */
      resRows = 500;
   }

   for( colNum = 0; colNum < resColumns; colNum++ ) 
   { 
      const char *name;
      retCode = (*TA_gSQLMinidriverTable[privateHandle->minidriver].getColumnName)(
                           queryResult,
                           colNum,
                           &name );
      RETURN_ON_ERROR( retCode );

      if( stricmp(name, TA_SQL_DATE_COLUMN) == 0 )
         dateCol = colNum;
      else
      if( stricmp(name, TA_SQL_TIME_COLUMN) == 0 )
         timeCol = colNum;
      else
      if( stricmp(name, TA_SQL_OPEN_COLUMN) == 0 )
         openCol = colNum;
      else
      if( stricmp(name, TA_SQL_HIGH_COLUMN) == 0 )
         highCol = colNum;
      else
      if( stricmp(name, TA_SQL_LOW_COLUMN) == 0 )
         lowCol = colNum;
      else
      if( stricmp(name, TA_SQL_CLOSE_COLUMN) == 0 )
         closeCol = colNum;
      else
      if( stricmp(name, TA_SQL_VOLUME_COLUMN) == 0 )
         volumeCol = colNum;
      else
      if( stricmp(name, TA_SQL_OI_COLUMN) == 0 )
         oiCol = colNum;
   } 

   /* timestamp is always needed */
   if( fieldToAlloc != TA_ALL )
      fieldToAlloc |= TA_TIMESTAMP;   

   /* When the TA_REPLACE_ZERO_PRICE_BAR flag is set, we must
    * always process the close.
    */
   if( fieldToAlloc != TA_ALL && privateHandle->param->flags & TA_REPLACE_ZERO_PRICE_BAR )
   {   
      fieldToAlloc |= TA_CLOSE;
   }

   /* verify whether all required columns are present */
   if( timeRequired && timeCol < 0 )
   {
      /* we cannot deliver data for the requested period, thus exit gracefully */
      retCode = (*TA_gSQLMinidriverTable[privateHandle->minidriver].releaseQuery)(queryResult);
      TA_TRACE_RETURN( retCode );
   }
   if(  dateCol < 0
     || (fieldToAlloc & TA_OPEN   && openCol   < 0)
     || (fieldToAlloc & TA_HIGH   && highCol   < 0)
     || (fieldToAlloc & TA_LOW    && lowCol    < 0)
     || (fieldToAlloc & TA_CLOSE  && closeCol  < 0)
     || (fieldToAlloc & TA_VOLUME && volumeCol < 0)
     || (fieldToAlloc & TA_OPENINTEREST && oiCol < 0) )
   {
      /* required column not found, so cannot deliver data */
      retCode = (*TA_gSQLMinidriverTable[privateHandle->minidriver].releaseQuery)(queryResult);
      TA_TRACE_RETURN( retCode );
   }
      
   /* iterate through the result set */
   for( rowNum = 0, barNum = 0;  
        (retCode = 
            (*TA_gSQLMinidriverTable[privateHandle->minidriver].getRowString)(
                              queryResult,
                              rowNum, 
                              dateCol,
                              &strval )
        ) != TA_END_OF_INDEX;
        rowNum++ ) 
   { 
      unsigned int u1, u2, u3;

      RETURN_ON_ERROR( retCode );  /* retCode from the for-condition */

      if( timestampVec == NULL )
      {
         /* Preallocate vectors memory */
         if ( !(timestampVec = (TA_Timestamp*)TA_Malloc( resRows * sizeof(TA_Timestamp))))
            RETURN_ON_ERROR( TA_ALLOC_ERR );
         memset(timestampVec, 0, resRows * sizeof(TA_Timestamp));
         
         #define TA_SQL_ALLOC_VEC( col_num, field_flag, type, vec )           \
               if( col_num >= 0                                               \
                  && (fieldToAlloc & field_flag || fieldToAlloc == TA_ALL)    \
                  && !(vec = (type*)TA_Malloc( resRows * sizeof(type) )))     \
               {                                                              \
                  RETURN_ON_ERROR( TA_ALLOC_ERR);                             \
               }
         
         TA_SQL_ALLOC_VEC( openCol,   TA_OPEN,         TA_Real,    openVec   )
         TA_SQL_ALLOC_VEC( highCol,   TA_HIGH,         TA_Real,    highVec   )
         TA_SQL_ALLOC_VEC( lowCol,    TA_LOW,          TA_Real,    lowVec    )
         TA_SQL_ALLOC_VEC( closeCol,  TA_CLOSE,        TA_Real,    closeVec  )
         TA_SQL_ALLOC_VEC( volumeCol, TA_VOLUME,       TA_Integer, volumeVec )
         TA_SQL_ALLOC_VEC( oiCol,     TA_OPENINTEREST, TA_Integer, oiVec     )
            
         #undef TA_SQL_ALLOC_VEC
            
      }

      /* date must be always present */
      if ( sscanf(strval, "%4u-%2u-%2u", &u1, &u2, &u3) != 3 )
      {
         RETURN_ON_ERROR( TA_BAD_QUERY );  /* other error code? */
      }

      retCode = TA_SetDate(u1, u2, u3, &timestampVec[barNum]);
      RETURN_ON_ERROR( retCode );

      if (timeCol >= 0)
      {
         strval = NULL;
         retCode = (*TA_gSQLMinidriverTable[privateHandle->minidriver].getRowString)(
                              queryResult,
                              rowNum, 
                              timeCol,
                              &strval );
         RETURN_ON_ERROR( retCode );
         
         if ( strval && *strval ) 
         {  
            if (sscanf(strval, "%2u:%2u:%2u", &u1, &u2, &u3) != 3 )
            {
               RETURN_ON_ERROR( TA_BAD_QUERY );
            }

            retCode = TA_SetTime(u1, u2, u3, &timestampVec[barNum]);
            RETURN_ON_ERROR( retCode );
         }
         else /* ignore NULL fields */
            continue;
      }

      #define TA_SQL_STORE_VALUE( type, getRow, vec, col, flag )                    \
         if (vec)                                                                   \
         {                                                                          \
            retCode = (*TA_gSQLMinidriverTable[privateHandle->minidriver].getRow )( \
                                 queryResult,                                       \
                                 rowNum,                                            \
                                 col,                                               \
                                 &vec[barNum] );                                    \
            RETURN_ON_ERROR( retCode );                                             \
                                                                                    \
            if (vec[barNum] == 0 && (privateHandle->param->flags & flag) )          \
               vec[barNum] = (type) ( (barNum > 0)? closeVec[barNum-1] : 0 );       \
                                                                                    \
            if (vec[barNum] == 0)                                                   \
               continue;                                                            \
         }
      
      TA_SQL_STORE_VALUE(TA_Real,    getRowReal,    openVec,   openCol,   TA_REPLACE_ZERO_PRICE_BAR)
      TA_SQL_STORE_VALUE(TA_Real,    getRowReal,    highVec,   highCol,   TA_REPLACE_ZERO_PRICE_BAR)
      TA_SQL_STORE_VALUE(TA_Real,    getRowReal,    lowVec,    lowCol,    TA_REPLACE_ZERO_PRICE_BAR)
      TA_SQL_STORE_VALUE(TA_Real,    getRowReal,    closeVec,  closeCol,  TA_REPLACE_ZERO_PRICE_BAR)
      TA_SQL_STORE_VALUE(TA_Integer, getRowInteger, volumeVec, volumeCol, TA_NO_FLAGS)
      TA_SQL_STORE_VALUE(TA_Integer, getRowInteger, oiVec,     oiCol,     TA_NO_FLAGS)

      #undef TA_SQL_STORE_VALUE

      barNum++;  /* bar stored */

      if( barNum == resRows )   /* vectors filled up completely, pass to ta_history */
      {
         retCode = TA_HistoryAddData(  paramForAddData,
                                       barNum,
                                       period,
                                       timestampVec,
                                       openVec,
                                       highVec,
                                       lowVec,
                                       closeVec,
                                       volumeVec,
                                       oiVec );
         
         /* whatever happened, the vectors are not belonging to us anymore */
         timestampVec = NULL;
         openVec = highVec = lowVec = closeVec = NULL;
         volumeVec = oiVec = NULL;
         barNum = 0;
         
         if( retCode == TA_ENOUGH_DATA )
            break;

         RETURN_ON_ERROR( retCode );
      }
   }

   /* now pass remaining collected data to ta_history module */
   if( barNum > 0 )
   {
      retCode = TA_HistoryAddData(  paramForAddData,
                                    barNum,
                                    period,
                                    timestampVec,
                                    openVec,
                                    highVec,
                                    lowVec,
                                    closeVec,
                                    volumeVec,
                                    oiVec );
      
      /* whatever happened, the vectors are not belonging to us anymore */
      timestampVec = NULL;
      openVec = highVec = lowVec = closeVec = NULL;
      volumeVec = oiVec = NULL;
      
      if( retCode != TA_ENOUGH_DATA )
         RETURN_ON_ERROR( retCode );
   }

   if( retCode == TA_ENOUGH_DATA || retCode == TA_END_OF_INDEX )
      retCode = TA_SUCCESS;
   
   /* cleanup */
   #undef RETURN_ON_ERROR

executeDataQuery_cleanup:

   /* retCode is set to the exit reason, so do not overwrite it here */
   (*TA_gSQLMinidriverTable[privateHandle->minidriver].releaseQuery)(queryResult);

   if ( timestampVec ) TA_Free( timestampVec );
   if ( openVec      ) TA_Free( openVec      );
   if ( highVec      ) TA_Free( highVec      );
   if ( lowVec       ) TA_Free( lowVec       );
   if ( closeVec     ) TA_Free( closeVec     );
   if ( volumeVec    ) TA_Free( volumeVec    );
   if ( oiVec        ) TA_Free( oiVec        );

   TA_TRACE_RETURN( retCode );
}



/* Format TA_Timestamp to ISO date as used by SQL
 * Not multithread-safe ;-)
 */
static const char *formatISODate(const TA_Timestamp *ts)
{
   static char str[11];  /* CCYY-MM-DD\0 */

   sprintf(str, "%04u-%02u-%02u", TA_GetYear(ts), TA_GetMonth(ts), TA_GetDay(ts));
   return str;
}

static const char *formatISOTime(const TA_Timestamp *ts)
{
   static char str[9];  /* hh:mm:ss\0 */

   sprintf(str, "%02u:%02u:%02u", TA_GetHour(ts), TA_GetMin(ts), TA_GetSec(ts));
   return str;
}
