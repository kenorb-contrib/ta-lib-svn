/* TA-LIB Copyright (c) 1999-2002, Mario Fortier
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
 *  MF       Mario Fortier
 *
 *
 * Change history:
 *
 *  MMDDYY BY   Description
 *  -------------------------------------------------------------------
 *  112400 MF   Template creation.
 *
 */

#include <string.h>

/**** START GENCODE SECTION 1 - DO NOT DELETE THIS LINE ****/
/* All code within this section is automatically
 * generated by gen_code. Any modification will be lost
 * next time gen_code is run.
 */

#ifndef TA_FUNC_H
   #include "ta_func.h"
#endif

#ifndef TA_UTILITY_H
   #include "ta_utility.h"
#endif

int TA_WMA_Lookback( TA_Integer    optInTimePeriod_0 )  /* From 2 to TA_INTEGER_MAX */

/**** END GENCODE SECTION 1 - DO NOT DELETE THIS LINE ****/
{
   /* insert lookback code here. */
   return optInTimePeriod_0 - 1;
}

/**** START GENCODE SECTION 2 - DO NOT DELETE THIS LINE ****/
/*
 * TA_WMA - Weighted Moving Average
 * 
 * Input  = TA_Real
 * Output = TA_Real
 * 
 * Optional Parameters
 * -------------------
 * optInTimePeriod_0:(From 2 to TA_INTEGER_MAX)
 *    Number of period
 * 
 * 
 */

TA_RetCode TA_WMA( TA_Integer    startIdx,
                   TA_Integer    endIdx,
                   const TA_Real inReal_0[],
                   TA_Integer    optInTimePeriod_0, /* From 2 to TA_INTEGER_MAX */
                   TA_Integer   *outBegIdx,
                   TA_Integer   *outNbElement,
                   TA_Real       outReal_0[] )
/**** END GENCODE SECTION 2 - DO NOT DELETE THIS LINE ****/
{
   /* Insert local variables here. */

/**** START GENCODE SECTION 3 - DO NOT DELETE THIS LINE ****/

#ifndef TA_FUNC_NO_RANGE_CHECK

   /* Validate the requested output range. */
   if( startIdx < 0 )
      return TA_OUT_OF_RANGE_START_INDEX;
   if( (endIdx < 0) || (endIdx < startIdx))
      return TA_OUT_OF_RANGE_END_INDEX;

   /* Validate the parameters. */
   if( !inReal_0 ) return TA_BAD_PARAM;
   /* min/max are checked for optInTimePeriod_0. */
   if( (TA_Integer)optInTimePeriod_0 == TA_INTEGER_DEFAULT )
      optInTimePeriod_0 = 30;
   else if( ((TA_Integer)optInTimePeriod_0 < 2) || ((TA_Integer)optInTimePeriod_0 > 2147483647) )
      return TA_BAD_PARAM;

   if( outReal_0 == NULL )
      return TA_BAD_PARAM;

#endif /* TA_FUNC_NO_RANGE_CHECK */

/**** END GENCODE SECTION 3 - DO NOT DELETE THIS LINE ****/

   /* Insert TA function code here. */

   return TA_INT_WMA( startIdx, endIdx,
                      inReal_0, optInTimePeriod_0,
                      outBegIdx, outNbElement, outReal_0 );
}

/* Calculate a Weighted Moving Average.
 * This is an internal version, parameters are assumed validated.
 * (startIdx and endIdx cannot be -1).
 */
TA_RetCode TA_INT_WMA( TA_Integer    startIdx,
                       TA_Integer    endIdx,
                       const TA_Real *inReal_0,
                       TA_Integer    optInTimePeriod_0, /* From 1 to TA_INTEGER_MAX */                       
                       TA_Integer   *outBegIdx,
                       TA_Integer   *outNbElement,
                       TA_Real      *outReal_0 )
{
   unsigned int inIdx, outIdx, i, trailingIdx, divider;
   TA_Real periodSum, periodSub, tempReal, trailingValue;
   unsigned int lookbackTotal;

   lookbackTotal = optInTimePeriod_0-1;

   /* Move up the start index if there is not
    * enough initial data.
    */
   if( (unsigned int)startIdx < lookbackTotal )
      startIdx = lookbackTotal;

   /* Make sure there is still something to evaluate. */
   if( startIdx > endIdx )
   {
      *outBegIdx    = 0;
      *outNbElement = 0;
      return TA_SUCCESS;
   }

   /* To make the rest more efficient, handle exception
    * case where the user is asking for a period of '1'.
    * In that case outputs equals inputs for the requested
    * range.
    */
   if( optInTimePeriod_0 == 1 ) 
   {      
      *outBegIdx    = startIdx;
      *outNbElement = endIdx-startIdx+1;
      memcpy( outReal_0, &inReal_0[startIdx], sizeof( TA_Real ) * (*outNbElement) );
      return TA_SUCCESS;
   }

   /* Calculate the divider (always an integer value).
    * By induction: 1+2+3+4+'n' = n(n+1)/2
    * '>>1' is usually faster than '/2' for unsigned.
    */
   divider = ((unsigned int)optInTimePeriod_0*((unsigned int)optInTimePeriod_0+1))>>1;

   /* The algo used here use a very basic property of
    * multiplication/addition: (x*2) = x+x
    *   
    * As an example, a 3 period weighted can be 
    * interpreted in two way:
    *  (x1*1)+(x2*2)+(x3*3)
    *      OR
    *  x1+x2+x2+x3+x3+x3 (this is the periodSum)
    *   
    * When you move forward in the time serie
    * you can quickly adjust the periodSum for the
    * period by substracting:
    *   x1+x2+x3 (This is the periodSub)
    * Making the new periodSum equals to:
    *   x2+x3+x3
    *
    * You can then add the new price bar
    * which is x4+x4+x4 giving:
    *   x2+x3+x3+x4+x4+x4
    *
    * At this point one iteration is completed and you can
    * see that we are back to the step 1 of this example.
    *
    * Why making it so un-intuitive? The number of memory
    * access and floating point operations are kept to a
    * minimum with this algo.
    */
   outIdx      = 0;
   trailingIdx = startIdx - lookbackTotal;

   /* Evaluate the initial periodSum/periodSub and trailingValue. */
   periodSum = periodSub = (TA_Real)0.0;
   inIdx=trailingIdx;
   i = 1;
   while( inIdx < (unsigned int)startIdx )
   {
      tempReal = inReal_0[inIdx++];
      periodSub += tempReal;
      periodSum += tempReal*i;
      i++;
   }
   trailingValue = 0.0;

   /* Tight loop for the requested range. */
   while( inIdx <= (unsigned int)endIdx )
   {
      /* Add the current price bar to the sum
       * who are carried through the iterations.
       */
      tempReal = inReal_0[inIdx++];
      periodSub += tempReal;
      periodSub -= trailingValue;
      periodSum += tempReal*optInTimePeriod_0;

      /* Save the trailing value for being substract at
       * the next iteration.
       * (must be saved here just in case outReal_0 and
       *  inReal_0 are the same buffer).
       */
      trailingValue = inReal_0[trailingIdx++];

      /* Calculate the WMA for this price bar. */
      outReal_0[outIdx++] = periodSum / divider;

      /* Prepare the periodSum for the next iteration. */
      periodSum -= periodSub;
   }

   /* Set output limits. */
   *outNbElement = outIdx;
   *outBegIdx    = startIdx;

   return TA_SUCCESS;
}
