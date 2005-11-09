/* TA-LIB Copyright (c) 1999-2005, Mario Fortier
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
 *  070401 MF   First version.
 *  050104 MF   Add TA_RegressionTest calls.
 *  080605 MF   Add tests for pseudo-random generator.
 *  091705 MF   Add tests for TA_AddTimeToTimestamp (Fix#1293953).
 */

/* Description:
 *         Regression testing of some internal utility like: 
 *            - collections: List/Stack/Circular buffer.
 *            - Memory allocation mechanism.
 *            etc...
 */

/**** Headers ****/
#include <stdio.h>
#include <string.h>

#include "ta_test_priv.h"
#include "ta_memory.h"
#include "ta_trace.h"
#include "ta_defs.h"
#include "mt19937ar.h"
#include "ta_common.h"


/**** External functions declarations. ****/
/* None */

/**** External variables declarations. ****/
/* None */

/**** Global variables definitions.    ****/
/* None */

/**** Local declarations.              ****/
/* None */

/**** Local functions declarations.    ****/
static ErrorNumber testFatalErrors( void );
static ErrorNumber testCircularBuffer( void );
static ErrorNumber testPseudoRandomGenerator( void );
static ErrorNumber testTimestamps( void );

static TA_RetCode circBufferFillFrom0ToSize( int size, int *buffer );


/**** Local variables definitions.     ****/
/* None */

/**** Global functions definitions.   ****/
/* None */

/**** Local functions definitions.     ****/
ErrorNumber test_internals( void )
{
   ErrorNumber retValue;

   printf( "Testing utility functions\n" );

   retValue = testCircularBuffer();
   if( retValue != TA_TEST_PASS )
   {
      printf( "\nFailed: Circular buffer tests (%d)\n", retValue );
      return retValue;
   }

   retValue = testFatalErrors();
   if( retValue != TA_TEST_PASS )
   {
      printf( "\nFailed: Fatal Errors Test (%d)\n", retValue );
      return retValue;
   }

   retValue = testPseudoRandomGenerator();
   if( retValue != TA_TEST_PASS )
   {
      printf( "\nFailed: Pseudo-random generator test (%d)\n", retValue );
      return retValue;
   }

   retValue = testTimestamps();
   if( retValue != TA_TEST_PASS )
   {
      printf( "\nFailed: Timestamps generator test (%d)\n", retValue );
      return retValue;
   }

   return TA_TEST_PASS; /* Success. */
}

static ErrorNumber testFatalErrors( void )
{
   TA_RetCode retCode;
   TA_UDBase *uDBase;
   ErrorNumber retValue;
   char *b;

   /* Initialize the library. */
   retValue = allocLib( &uDBase );
   if( retValue != TA_TEST_PASS )
   {
      printf( "\ntestFatalErrors Failed: Can't initialize the library\n" );
      return retValue;
   }

   TA_SetFatalErrorHandler( NULL );

   retCode = TA_RegressionTest(TA_REG_TEST_FATAL_ERROR);
   if( retCode != TA_FATAL_ERR )
      return TA_INTERNAL_FATAL_TST_FAIL;

   /* After a function returns a TA_FATAL_ERR, further
    * information can be obtained with TA_FatalReportToBuffer()
    * and/or TA_FatalReport().
    *
    * These functions returns the info only about the LAST
    * recorded fatal error. To more easily record all the fatal
    * error, you should install a fatal error handler and call
    * the TA_FatalReportXXXX() function from the handler.
    *
    * You can monitor what is getting in the buffer by 
    * un-commenting the printf.
    *      
    */
   b = (char *)TA_Malloc(TA_FATAL_ERROR_BUF_SIZE);
   if( b )
   {
      TA_FatalReportToBuffer( b, TA_FATAL_ERROR_BUF_SIZE );
      /* printf( b ); */
   }
   TA_Free(b);

   retValue = freeLib( uDBase );
   if( retValue != TA_TEST_PASS )
      return retValue;

   retValue = allocLib( &uDBase );
   if( retValue != TA_TEST_PASS )
   {
      printf( "\ntestFatalErrors Failed: Can't initialize the library\n" );
      return retValue;
   }

   TA_SetFatalErrorHandler( NULL );

   retCode = TA_RegressionTest(TA_REG_TEST_ASSERT_FAIL);
   if( retCode != TA_FATAL_ERR )
      return TA_INTERNAL_ASSERT_TST_FAIL;

   retValue = freeLib( uDBase );
   if( retValue != TA_TEST_PASS )
      return retValue;

   return TA_TEST_PASS; /* Success. */
}

static ErrorNumber testCircularBuffer( void )
{
   TA_RetCode retCode;
   int i; 
   int buffer[20];
   TA_UDBase *uDBase;
   ErrorNumber retValue;

   /* Initialize the library. */
   retValue = allocLib( &uDBase );
   if( retValue != TA_TEST_PASS )
   {
      printf( "\nFailed: Can't initialize the library\n" );
      return retValue;
   }

   /* The following function is supose to fill
    * the buffer with the value 0 to 8 sequentialy,
    * if somehow it is not 0 to 8, there is a bug!
    */
   memset( buffer, 0xFF, sizeof(buffer) );
   retCode = circBufferFillFrom0ToSize( 1, buffer );
   if( retCode != TA_SUCCESS )
   {
      printf( "\nFailed circular buffer test RetCode = %d\n", retCode );
      return TA_INTERNAL_CIRC_BUFF_FAIL_0;
   }
   for( i=0; i < (1+3); i++ )
   {
      if( buffer[i] != i )
      {
         printf( "\nFailed circular buffer test (%d != %d)\n", buffer[i], i );
         return TA_INTERNAL_CIRC_BUFF_FAIL_1;
      }
   }

   memset( buffer, 0xFF, sizeof(buffer) );
   retCode = circBufferFillFrom0ToSize( 2, buffer );
   if( retCode != TA_SUCCESS )
   {
      printf( "\nFailed circular buffer test RetCode = %d\n", retCode );
      return TA_INTERNAL_CIRC_BUFF_FAIL_0;
   }
   for( i=0; i < (2+3); i++ )
   {
      if( buffer[i] != i )
      {
         printf( "\nFailed circular buffer test (%d != %d)\n", buffer[i], i );
         return TA_INTERNAL_CIRC_BUFF_FAIL_2;
      }
   }

   memset( buffer, 0xFF, sizeof(buffer) );
   retCode = circBufferFillFrom0ToSize( 3, buffer );
   if( retCode != TA_SUCCESS )
   {
      printf( "\nFailed circular buffer test RetCode = %d\n", retCode );
      return TA_INTERNAL_CIRC_BUFF_FAIL_0;
   }
   for( i=0; i < (3+3); i++ )
   {
      if( buffer[i] != i )
      {
         printf( "\nFailed circular buffer test (%d != %d)\n", buffer[i], i );
         return TA_INTERNAL_CIRC_BUFF_FAIL_3;
      }
   }

   memset( buffer, 0xFF, sizeof(buffer) );
   retCode = circBufferFillFrom0ToSize( 4, buffer );
   if( retCode != TA_SUCCESS )
   {
      printf( "\nFailed circular buffer test RetCode = %d\n", retCode );
      return TA_INTERNAL_CIRC_BUFF_FAIL_0;
   }
   for( i=0; i < (4+3); i++ )
   {
      if( buffer[i] != i )
      {
         printf( "\nFailed circular buffer test (%d != %d)\n", buffer[i], i );
         return TA_INTERNAL_CIRC_BUFF_FAIL_4;
      }
   }

   memset( buffer, 0xFF, sizeof(buffer) );
   retCode = circBufferFillFrom0ToSize( 5, buffer );
   if( retCode != TA_SUCCESS )
   {
      printf( "\nFailed circular buffer test RetCode = %d\n", retCode );
      return TA_INTERNAL_CIRC_BUFF_FAIL_0;
   }
   for( i=0; i < (5+3); i++ )
   {
      if( buffer[i] != i )
      {
         printf( "\nFailed circular buffer test (%d != %d)\n", buffer[i], i );
         return TA_INTERNAL_CIRC_BUFF_FAIL_5;
      }
   }

   memset( buffer, 0xFF, sizeof(buffer) );
   retCode = circBufferFillFrom0ToSize( 6, buffer );
   if( retCode != TA_SUCCESS )
   {
      printf( "\nFailed circular buffer test RetCode = %d\n", retCode );
      return TA_INTERNAL_CIRC_BUFF_FAIL_0;
   }
   for( i=0; i < (6+3); i++ )
   {
      if( buffer[i] != i )
      {
         printf( "\nFailed circular buffer test (%d != %d)\n", buffer[i], i );
         return TA_INTERNAL_CIRC_BUFF_FAIL_6;
      }
   }

   retValue = freeLib( uDBase );
   if( retValue != TA_TEST_PASS )
      return retValue;

   return TA_TEST_PASS; /* Success. */
}

/* This function is suppose to fill the buffer
 * with values going from 0 to 'size'.
 * The filling is done using the CIRCBUF macros.
 */
static TA_RetCode circBufferFillFrom0ToSize( int size, int *buffer )
{
   CIRCBUF_PROLOG(MyBuf,int,4);
   int i, value;
   int outIdx;

   CIRCBUF_INIT(MyBuf,int,size);

   outIdx = 0;

   // 1st Loop: Fill MyBuf with initial values
   //           (must be done).
   value = 0;
   for( i=0; i < size; i++ )
   {
      MyBuf[MyBuf_Idx] = value++;
      CIRCBUF_NEXT(MyBuf);
   }

   // 2nd Loop: Get and Add subsequent values
   //           in MyBuf (optional)
   for( i=0; i < 3; i++ )
   {
      buffer[outIdx++] = MyBuf[MyBuf_Idx];
      MyBuf[MyBuf_Idx] = value++;
      CIRCBUF_NEXT(MyBuf);
   }

   // 3rd Loop: Empty MyBuf (optional)
   for( i=0; i < size; i++ )
   {
      buffer[outIdx++] = MyBuf[MyBuf_Idx];
      CIRCBUF_NEXT(MyBuf);
   }

   CIRCBUF_DESTROY(MyBuf);

   return TA_SUCCESS;
}

static UInt32 ExpectedRandomValuesUInt32[1000] =
{
1067595299U  ,955945823U  ,477289528U ,4107218783U ,4228976476U ,
3344332714U ,3355579695U  ,227628506U  ,810200273U ,2591290167U ,
2560260675U ,3242736208U  ,646746669U ,1479517882U ,4245472273U ,
1143372638U ,3863670494U ,3221021970U ,1773610557U ,1138697238U ,
1421897700U ,1269916527U ,2859934041U ,1764463362U ,3874892047U ,
3965319921U   ,72549643U ,2383988930U ,2600218693U ,3237492380U ,
2792901476U  ,725331109U  ,605841842U  ,271258942U  ,715137098U ,
3297999536U ,1322965544U ,4229579109U ,1395091102U ,3735697720U ,
2101727825U ,3730287744U ,2950434330U ,1661921839U ,2895579582U ,
2370511479U ,1004092106U ,2247096681U ,2111242379U ,3237345263U ,
4082424759U  ,219785033U ,2454039889U ,3709582971U  ,835606218U ,
2411949883U ,2735205030U  ,756421180U ,2175209704U ,1873865952U ,
2762534237U ,4161807854U ,3351099340U  ,181129879U ,3269891896U ,
 776029799U ,2218161979U ,3001745796U ,1866825872U ,2133627728U ,
  34862734U ,1191934573U ,3102311354U ,2916517763U ,1012402762U ,
2184831317U ,4257399449U ,2899497138U ,3818095062U ,3030756734U ,
1282161629U  ,420003642U ,2326421477U ,2741455717U ,1278020671U ,
3744179621U  ,271777016U ,2626330018U ,2560563991U ,3055977700U ,
4233527566U ,1228397661U ,3595579322U ,1077915006U ,2395931898U ,
1851927286U ,3013683506U ,1999971931U ,3006888962U ,1049781534U ,
1488758959U ,3491776230U  ,104418065U ,2448267297U ,3075614115U ,
3872332600U  ,891912190U ,3936547759U ,2269180963U ,2633455084U ,
1047636807U ,2604612377U ,2709305729U ,1952216715U  ,207593580U ,
2849898034U  ,670771757U ,2210471108U  ,467711165U  ,263046873U ,
3569667915U ,1042291111U ,3863517079U ,1464270005U ,2758321352U ,
3790799816U ,2301278724U ,3106281430U    ,7974801U ,2792461636U ,
 555991332U  ,621766759U ,1322453093U  ,853629228U  ,686962251U ,
1455120532U  ,957753161U, 1802033300U, 1021534190U, 3486047311U ,
1902128914U ,3701138056U ,4176424663U ,1795608698U  ,560858864U ,
3737752754U ,3141170998U ,1553553385U ,3367807274U  ,711546358U ,
2475125503U  ,262969859U  ,251416325U ,2980076994U ,1806565895U ,
 969527843U ,3529327173U ,2736343040U ,2987196734U ,1649016367U ,
2206175811U ,3048174801U ,3662503553U ,3138851612U ,2660143804U ,
1663017612U ,1816683231U  ,411916003U ,3887461314U ,2347044079U ,
1015311755U ,1203592432U ,2170947766U ,2569420716U  ,813872093U ,
1105387678U ,1431142475U  ,220570551U ,4243632715U ,4179591855U ,
2607469131U ,3090613241U  ,282341803U ,1734241730U ,1391822177U ,
1001254810U  ,827927915U ,1886687171U ,3935097347U ,2631788714U ,
3905163266U  ,110554195U ,2447955646U ,3717202975U ,3304793075U ,
3739614479U ,3059127468U  ,953919171U ,2590123714U ,1132511021U ,
3795593679U ,2788030429U  ,982155079U ,3472349556U  ,859942552U ,
2681007391U ,2299624053U  ,647443547U  ,233600422U  ,608168955U ,
3689327453U ,1849778220U ,1608438222U ,3968158357U ,2692977776U ,
2851872572U  ,246750393U ,3582818628U ,3329652309U ,4036366910U ,
1012970930U  ,950780808U ,3959768744U ,2538550045U  ,191422718U ,
2658142375U ,3276369011U ,2927737484U ,1234200027U ,1920815603U ,
3536074689U ,1535612501U ,2184142071U ,3276955054U  ,428488088U ,
2378411984U ,4059769550U ,3913744741U ,2732139246U   ,64369859U ,
3755670074U  ,842839565U ,2819894466U ,2414718973U ,1010060670U ,
1839715346U ,2410311136U  ,152774329U ,3485009480U ,4102101512U ,
2852724304U  ,879944024U ,1785007662U ,2748284463U ,1354768064U ,
3267784736U ,2269127717U ,3001240761U ,3179796763U  ,895723219U ,
 865924942U ,4291570937U   ,89355264U ,1471026971U ,4114180745U ,
3201939751U ,2867476999U ,2460866060U ,3603874571U ,2238880432U ,
3308416168U ,2072246611U ,2755653839U ,3773737248U ,1709066580U ,
4282731467U ,2746170170U ,2832568330U  ,433439009U ,3175778732U ,
  26248366U ,2551382801U  ,183214346U ,3893339516U ,1928168445U ,
1337157619U ,3429096554U ,3275170900U ,1782047316U ,4264403756U ,
1876594403U ,4289659572U ,3223834894U ,1728705513U ,4068244734U ,
2867840287U ,1147798696U  ,302879820U ,1730407747U ,1923824407U ,
1180597908U ,1569786639U  ,198796327U  ,560793173U ,2107345620U ,
2705990316U ,3448772106U ,3678374155U  ,758635715U  ,884524671U ,
 486356516U ,1774865603U ,3881226226U ,2635213607U ,1181121587U ,
1508809820U ,3178988241U ,1594193633U ,1235154121U  ,326117244U ,
2304031425U  ,937054774U ,2687415945U ,3192389340U ,2003740439U ,
1823766188U ,2759543402U   ,10067710U ,1533252662U ,4132494984U ,
  82378136U  ,420615890U ,3467563163U  ,541562091U ,3535949864U ,
2277319197U ,3330822853U ,3215654174U ,4113831979U ,4204996991U ,
2162248333U ,3255093522U ,2219088909U ,2978279037U  ,255818579U ,
2859348628U ,3097280311U ,2569721123U ,1861951120U ,2907080079U ,
2719467166U  ,998319094U ,2521935127U ,2404125338U  ,259456032U ,
2086860995U ,1839848496U ,1893547357U ,2527997525U ,1489393124U ,
2860855349U   ,76448234U ,2264934035U  ,744914583U ,2586791259U ,
1385380501U   ,66529922U ,1819103258U ,1899300332U ,2098173828U ,
1793831094U  ,276463159U  ,360132945U ,4178212058U  ,595015228U ,
 177071838U ,2800080290U ,1573557746U ,1548998935U  ,378454223U ,
1460534296U ,1116274283U ,3112385063U ,3709761796U  ,827999348U ,
3580042847U ,1913901014U  ,614021289U ,4278528023U ,1905177404U ,
  45407939U ,3298183234U ,1184848810U ,3644926330U ,3923635459U ,
1627046213U ,3677876759U  ,969772772U ,1160524753U ,1522441192U ,
 452369933U ,1527502551U  ,832490847U ,1003299676U ,1071381111U ,
2891255476U  ,973747308U ,4086897108U ,1847554542U ,3895651598U ,
2227820339U ,1621250941U ,2881344691U ,3583565821U ,3510404498U ,
 849362119U  ,862871471U  ,797858058U ,2867774932U ,2821282612U ,
3272403146U ,3997979905U  ,209178708U ,1805135652U    ,6783381U ,
2823361423U  ,792580494U ,4263749770U  ,776439581U ,3798193823U ,
2853444094U ,2729507474U ,1071873341U ,1329010206U ,1289336450U ,
3327680758U ,2011491779U   ,80157208U  ,922428856U ,1158943220U ,
1667230961U ,2461022820U ,2608845159U  ,387516115U ,3345351910U ,
1495629111U ,4098154157U ,3156649613U ,3525698599U ,4134908037U ,
 446713264U ,2137537399U ,3617403512U  ,813966752U ,1157943946U ,
3734692965U ,1680301658U ,3180398473U ,3509854711U ,2228114612U ,
1008102291U  ,486805123U  ,863791847U ,3189125290U ,1050308116U ,
3777341526U ,4291726501U  ,844061465U ,1347461791U ,2826481581U ,
 745465012U ,2055805750U ,4260209475U ,2386693097U ,2980646741U ,
 447229436U ,2077782664U ,1232942813U ,4023002732U ,1399011509U ,
3140569849U ,2579909222U ,3794857471U  ,900758066U ,2887199683U ,
1720257997U ,3367494931U ,2668921229U  ,955539029U ,3818726432U ,
1105704962U ,3889207255U ,2277369307U ,2746484505U ,1761846513U ,
2413916784U ,2685127085U ,4240257943U ,1166726899U ,4215215715U ,
3082092067U ,3960461946U ,1663304043U ,2087473241U ,4162589986U ,
2507310778U ,1579665506U  ,767234210U  ,970676017U  ,492207530U ,
1441679602U ,1314785090U ,3262202570U ,3417091742U ,1561989210U ,
3011406780U ,1146609202U ,3262321040U ,1374872171U ,1634688712U ,
1280458888U ,2230023982U  ,419323804U ,3262899800U   ,39783310U ,
1641619040U ,1700368658U ,2207946628U ,2571300939U ,2424079766U ,
 780290914U ,2715195096U ,3390957695U  ,163151474U ,2309534542U ,
1860018424U  ,555755123U  ,280320104U ,1604831083U ,2713022383U ,
1728987441U ,3639955502U  ,623065489U ,3828630947U ,4275479050U ,
3516347383U ,2343951195U ,2430677756U  ,635534992U ,3868699749U ,
 808442435U ,3070644069U ,4282166003U ,2093181383U ,2023555632U ,
1568662086U ,3422372620U ,4134522350U ,3016979543U ,3259320234U ,
2888030729U ,3185253876U ,4258779643U ,1267304371U ,1022517473U ,
 815943045U  ,929020012U ,2995251018U ,3371283296U ,3608029049U ,
2018485115U  ,122123397U ,2810669150U ,1411365618U ,1238391329U ,
1186786476U ,3155969091U ,2242941310U ,1765554882U  ,279121160U ,
4279838515U ,1641578514U ,3796324015U   ,13351065U  ,103516986U ,
1609694427U  ,551411743U ,2493771609U ,1316337047U ,3932650856U ,
4189700203U  ,463397996U ,2937735066U ,1855616529U ,2626847990U ,
  55091862U ,3823351211U  ,753448970U ,4045045500U ,1274127772U ,
1124182256U   ,92039808U ,2126345552U  ,425973257U  ,386287896U ,
2589870191U ,1987762798U ,4084826973U ,2172456685U ,3366583455U ,
3602966653U ,2378803535U ,2901764433U ,3716929006U ,3710159000U ,
2653449155U ,3469742630U ,3096444476U ,3932564653U ,2595257433U ,
 318974657U ,3146202484U  ,853571438U  ,144400272U ,3768408841U ,
 782634401U ,2161109003U  ,570039522U ,1886241521U   ,14249488U ,
2230804228U ,1604941699U ,3928713335U ,3921942509U ,2155806892U ,
 134366254U  ,430507376U ,1924011722U  ,276713377U  ,196481886U ,
3614810992U ,1610021185U ,1785757066U  ,851346168U ,3761148643U ,
2918835642U ,3364422385U ,3012284466U ,3735958851U ,2643153892U ,
3778608231U ,1164289832U  ,205853021U ,2876112231U ,3503398282U ,
3078397001U ,3472037921U ,1748894853U ,2740861475U  ,316056182U ,
1660426908U  ,168885906U  ,956005527U ,3984354789U  ,566521563U ,
1001109523U ,1216710575U ,2952284757U ,3834433081U ,3842608301U ,
2467352408U ,3974441264U ,3256601745U ,1409353924U ,1329904859U ,
2307560293U ,3125217879U ,3622920184U ,3832785684U ,3882365951U ,
2308537115U ,2659155028U ,1450441945U ,3532257603U ,3186324194U ,
1225603425U ,1124246549U  ,175808705U ,3009142319U ,2796710159U ,
3651990107U  ,160762750U ,1902254979U ,1698648476U ,1134980669U ,
 497144426U ,3302689335U ,4057485630U ,3603530763U ,4087252587U ,
 427812652U  ,286876201U  ,823134128U ,1627554964U ,3745564327U ,
2589226092U ,4202024494U   ,62878473U ,3275585894U ,3987124064U ,
2791777159U ,1916869511U ,2585861905U ,1375038919U ,1403421920U ,
  60249114U ,3811870450U ,3021498009U ,2612993202U  ,528933105U ,
2757361321U ,3341402964U ,2621861700U  ,273128190U ,4015252178U ,
3094781002U ,1621621288U ,2337611177U ,1796718448U ,1258965619U ,
4241913140U ,2138560392U ,3022190223U ,4174180924U  ,450094611U ,
3274724580U  ,617150026U ,2704660665U ,1469700689U ,1341616587U ,
 356715071U ,1188789960U ,2278869135U ,1766569160U ,2795896635U ,
  57824704U ,2893496380U ,1235723989U ,1630694347U ,3927960522U ,
 428891364U ,1814070806U ,2287999787U ,4125941184U ,3968103889U ,
3548724050U ,1025597707U ,1404281500U ,2002212197U   ,92429143U ,
2313943944U ,2403086080U ,3006180634U ,3561981764U ,1671860914U ,
1768520622U ,1803542985U  ,844848113U ,3006139921U ,1410888995U ,
1157749833U ,2125704913U ,1789979528U ,1799263423U  ,741157179U ,
2405862309U  ,767040434U ,2655241390U ,3663420179U ,2172009096U ,
2511931187U ,1680542666U  ,231857466U ,1154981000U  ,157168255U ,
1454112128U ,3505872099U ,1929775046U ,2309422350U ,2143329496U ,
2960716902U  ,407610648U ,2938108129U ,2581749599U  ,538837155U ,
2342628867U  ,430543915U  ,740188568U ,1937713272U ,3315215132U ,
2085587024U ,4030765687U  ,766054429U ,3517641839U  ,689721775U ,
1294158986U ,1753287754U ,4202601348U ,1974852792U   ,33459103U ,
3568087535U ,3144677435U ,1686130825U ,4134943013U ,3005738435U ,
3599293386U  ,426570142U  ,754104406U ,3660892564U ,1964545167U ,
 829466833U  ,821587464U ,1746693036U ,1006492428U ,1595312919U ,
1256599985U ,1024482560U ,1897312280U ,2902903201U  ,691790057U ,
1037515867U ,3176831208U ,1968401055U ,2173506824U ,1089055278U ,
1748401123U ,2941380082U  ,968412354U ,1818753861U ,2973200866U ,
3875951774U ,1119354008U ,3988604139U ,1647155589U ,2232450826U ,
3486058011U ,3655784043U ,3759258462U  ,847163678U ,1082052057U ,
 989516446U ,2871541755U ,3196311070U ,3929963078U  ,658187585U ,
3664944641U ,2175149170U ,2203709147U ,2756014689U ,2456473919U ,
3890267390U ,1293787864U ,2830347984U ,3059280931U ,4158802520U ,
1561677400U ,2586570938U  ,783570352U ,1355506163U   ,31495586U ,
3789437343U ,3340549429U ,2092501630U  ,896419368U  ,671715824U ,
3530450081U ,3603554138U ,1055991716U ,3442308219U ,1499434728U ,
3130288473U ,3639507000U   ,17769680U ,2259741420U  ,487032199U ,
4227143402U ,3693771256U ,1880482820U ,3924810796U  ,381462353U ,
4017855991U ,2452034943U ,2736680833U ,2209866385U ,2128986379U ,
 437874044U  ,595759426U  ,641721026U ,1636065708U ,3899136933U ,
 629879088U ,3591174506U  ,351984326U ,2638783544U ,2348444281U ,
2341604660U ,2123933692U  ,143443325U ,1525942256U  ,364660499U ,
 599149312U  ,939093251U ,1523003209U  ,106601097U  ,376589484U ,
1346282236U ,1297387043U  ,764598052U ,3741218111U  ,933457002U ,
1886424424U ,3219631016U  ,525405256U ,3014235619U  ,323149677U ,
2038881721U ,4100129043U ,2851715101U ,2984028078U ,1888574695U ,
2014194741U ,3515193880U ,4180573530U ,3461824363U ,2641995497U ,
3179230245U ,2902294983U ,2217320456U ,4040852155U ,1784656905U ,
3311906931U   ,87498458U ,2752971818U ,2635474297U ,2831215366U ,
3682231106U ,2920043893U ,3772929704U ,2816374944U  ,309949752U ,
2383758854U  ,154870719U  ,385111597U ,1191604312U ,1840700563U ,
 872191186U ,2925548701U ,1310412747U ,2102066999U ,1504727249U ,
3574298750U ,1191230036U ,3330575266U ,3180292097U ,3539347721U ,
 681369118U ,3305125752U ,3648233597U  ,950049240U ,4173257693U ,
1760124957U  ,512151405U  ,681175196U  ,580563018U ,1169662867U ,
4015033554U ,2687781101U  ,699691603U ,2673494188U ,1137221356U ,
 123599888U  ,472658308U ,1053598179U ,1012713758U ,3481064843U ,
3759461013U ,3981457956U ,3830587662U ,1877191791U ,3650996736U ,
 988064871U ,3515461600U ,4089077232U ,2225147448U ,1249609188U ,
2643151863U ,3896204135U ,2416995901U ,1397735321U ,3460025646U };


static double ExpectedRandomValuesDouble[1000] =
{
0.76275443 ,0.99000644 ,0.98670464 ,0.10143112 ,0.27933125 ,
0.69867227 ,0.94218740 ,0.03427201 ,0.78842173 ,0.28180608 ,
0.92179002 ,0.20785655 ,0.54534773 ,0.69644020 ,0.38107718 ,
0.23978165 ,0.65286910 ,0.07514568 ,0.22765211 ,0.94872929 ,
0.74557914 ,0.62664415 ,0.54708246 ,0.90959343 ,0.42043116 ,
0.86334511 ,0.19189126 ,0.14718544 ,0.70259889 ,0.63426346 ,
0.77408121 ,0.04531601 ,0.04605807 ,0.88595519 ,0.69398270 ,
0.05377184 ,0.61711170 ,0.05565708 ,0.10133577 ,0.41500776 ,
0.91810699 ,0.22320679 ,0.23353705 ,0.92871862 ,0.98897234 ,
0.19786706 ,0.80558809 ,0.06961067 ,0.55840445 ,0.90479405 ,
0.63288060 ,0.95009721 ,0.54948447 ,0.20645042 ,0.45000959 ,
0.87050869 ,0.70806991 ,0.19406895 ,0.79286390 ,0.49332866 ,
0.78483914 ,0.75145146 ,0.12341941 ,0.42030252 ,0.16728160 ,
0.59906494 ,0.37575460 ,0.97815160 ,0.39815952 ,0.43595080 ,
0.04952478 ,0.33917805 ,0.76509902 ,0.61034321 ,0.90654701 ,
0.92915732 ,0.85365931 ,0.18812377 ,0.65913428 ,0.28814566 ,
0.59476081 ,0.27835931 ,0.60722542 ,0.68310435 ,0.69387186 ,
0.03699800 ,0.65897714 ,0.17527003 ,0.02889304 ,0.86777366 ,
0.12352068 ,0.91439461 ,0.32022990 ,0.44445731 ,0.34903686 ,
0.74639273 ,0.65918367 ,0.92492794 ,0.31872642 ,0.77749724 ,
0.85413832 ,0.76385624 ,0.32744211 ,0.91326300 ,0.27458185 ,
0.22190155 ,0.19865383 ,0.31227402 ,0.85321225 ,0.84243342 ,
0.78544200 ,0.71854080 ,0.92503892 ,0.82703064 ,0.88306297 ,
0.47284073 ,0.70059042 ,0.48003761 ,0.38671694 ,0.60465770 ,
0.41747204 ,0.47163243 ,0.72750808 ,0.65830223 ,0.10955369 ,
0.64215401 ,0.23456345 ,0.95944940 ,0.72822249 ,0.40888451 ,
0.69980355 ,0.26677428 ,0.57333635 ,0.39791582 ,0.85377858 ,
0.76962816 ,0.72004885 ,0.90903087 ,0.51376506 ,0.37732665 ,
0.12691640 ,0.71249738 ,0.81217908 ,0.37037313 ,0.32772374 ,
0.14238259 ,0.05614811 ,0.74363008 ,0.39773267 ,0.94859135 ,
0.31452454 ,0.11730313 ,0.62962618 ,0.33334237 ,0.45547255 ,
0.10089665 ,0.56550662 ,0.60539371 ,0.16027624 ,0.13245301 ,
0.60959939 ,0.04671662 ,0.99356286 ,0.57660859 ,0.40269560 ,
0.45274629 ,0.06699735 ,0.85064246 ,0.87742744 ,0.54508392 ,
0.87242982 ,0.29321385 ,0.67660627 ,0.68230715 ,0.79052073 ,
0.48592054 ,0.25186266 ,0.93769755 ,0.28565487 ,0.47219067 ,
0.99054882 ,0.13155240 ,0.47110470 ,0.98556600 ,0.84397623 ,
0.12875246 ,0.90953202 ,0.49129015 ,0.23792727 ,0.79481194 ,
0.44337770 ,0.96564297 ,0.67749118 ,0.55684872 ,0.27286897 ,
0.79538393 ,0.61965356 ,0.22487929 ,0.02226018 ,0.49248200 ,
0.42247006 ,0.91797788 ,0.99250134 ,0.23449967 ,0.52531508 ,
0.10246337 ,0.78685622 ,0.34310922 ,0.89892996 ,0.40454552 ,
0.68608407 ,0.30752487 ,0.83601319 ,0.54956031 ,0.63777550 ,
0.82199797 ,0.24890696 ,0.48801123 ,0.48661910 ,0.51223987 ,
0.32969635 ,0.31075073 ,0.21393155 ,0.73453207 ,0.15565705 ,
0.58584522 ,0.28976728 ,0.97621478 ,0.61498701 ,0.23891470 ,
0.28518540 ,0.46809591 ,0.18371914 ,0.37597910 ,0.13492176 ,
0.66849449 ,0.82811466 ,0.56240330 ,0.37548956 ,0.27562998 ,
0.27521910 ,0.74096121 ,0.77176757 ,0.13748143 ,0.99747138 ,
0.92504502 ,0.09175241 ,0.21389176 ,0.21766512 ,0.31183245 ,
0.23271221 ,0.21207367 ,0.57903312 ,0.77523344 ,0.13242613 ,
0.31037988 ,0.01204835 ,0.71652949 ,0.84487594 ,0.14982178 ,
0.57423142 ,0.45677888 ,0.48420169 ,0.53465428 ,0.52667473 ,
0.46880526 ,0.49849733 ,0.05670710 ,0.79022476 ,0.03872047 ,
0.21697212 ,0.20443086 ,0.28949326 ,0.81678186 ,0.87629474 ,
0.92297064 ,0.27373097 ,0.84625273 ,0.51505586 ,0.00582792 ,
0.33295971 ,0.91848412 ,0.92537226 ,0.91760033 ,0.07541125 ,
0.71745848 ,0.61158698 ,0.00941650 ,0.03135554 ,0.71527471 ,
0.24821915 ,0.63636652 ,0.86159918 ,0.26450229 ,0.60160194 ,
0.35557725 ,0.24477500 ,0.07186456 ,0.51757096 ,0.62120362 ,
0.97981062 ,0.69954667 ,0.21065616 ,0.13382753 ,0.27693186 ,
0.59644095 ,0.71500764 ,0.04110751 ,0.95730081 ,0.91600724 ,
0.47704678 ,0.26183479 ,0.34706971 ,0.07545431 ,0.29398385 ,
0.93236070 ,0.60486023 ,0.48015011 ,0.08870451 ,0.45548581 ,
0.91872718 ,0.38142712 ,0.10668643 ,0.01397541 ,0.04520355 ,
0.93822273 ,0.18011940 ,0.57577277 ,0.91427606 ,0.30911399 ,
0.95853475 ,0.23611214 ,0.69619891 ,0.69601980 ,0.76765372 ,
0.58515930 ,0.49479057 ,0.11288752 ,0.97187699 ,0.32095365 ,
0.57563608 ,0.40760618 ,0.78703383 ,0.43261152 ,0.90877651 ,
0.84686346 ,0.10599030 ,0.72872803 ,0.19315490 ,0.66152912 ,
0.10210518 ,0.06257876 ,0.47950688 ,0.47062066 ,0.72701157 ,
0.48915116 ,0.66110261 ,0.60170685 ,0.24516994 ,0.12726050 ,
0.03451185 ,0.90864994 ,0.83494878 ,0.94800035 ,0.91035206 ,
0.14480751 ,0.88458997 ,0.53498312 ,0.15963215 ,0.55378627 ,
0.35171349 ,0.28719791 ,0.09097957 ,0.00667896 ,0.32309622 ,
0.87561479 ,0.42534520 ,0.91748977 ,0.73908457 ,0.41793223 ,
0.99279792 ,0.87908370 ,0.28458072 ,0.59132853 ,0.98672190 ,
0.28547393 ,0.09452165 ,0.89910674 ,0.53681109 ,0.37931425 ,
0.62683489 ,0.56609740 ,0.24801549 ,0.52948179 ,0.98328855 ,
0.66403523 ,0.55523786 ,0.75886666 ,0.84784685 ,0.86829981 ,
0.71448906 ,0.84670080 ,0.43922919 ,0.20771016 ,0.64157936 ,
0.25664246 ,0.73055695 ,0.86395782 ,0.65852932 ,0.99061803 ,
0.40280575 ,0.39146298 ,0.07291005 ,0.97200603 ,0.20555729 ,
0.59616495 ,0.08138254 ,0.45796388 ,0.33681125 ,0.33989127 ,
0.18717090 ,0.53545811 ,0.60550838 ,0.86520709 ,0.34290701 ,
0.72743276 ,0.73023855 ,0.34195926 ,0.65019733 ,0.02765254 ,
0.72575740 ,0.32709576 ,0.03420866 ,0.26061893 ,0.56997511 ,
0.28439072 ,0.84422744 ,0.77637570 ,0.55982168 ,0.06720327 ,
0.58449067 ,0.71657369 ,0.15819609 ,0.58042821 ,0.07947911 ,
0.40193792 ,0.11376012 ,0.88762938 ,0.67532159 ,0.71223735 ,
0.27829114 ,0.04806073 ,0.21144026 ,0.58830274 ,0.04140071 ,
0.43215628 ,0.12952729 ,0.94668759 ,0.87391019 ,0.98382450 ,
0.27750768 ,0.90849647 ,0.90962737 ,0.59269720 ,0.96102026 ,
0.49544979 ,0.32007095 ,0.62585546 ,0.03119821 ,0.85953001 ,
0.22017528 ,0.05834068 ,0.80731217 ,0.53799961 ,0.74166948 ,
0.77426600 ,0.43938444 ,0.54862081 ,0.58575513 ,0.15886492 ,
0.73214332 ,0.11649057 ,0.77463977 ,0.85788827 ,0.17061997 ,
0.66838056 ,0.96076133 ,0.07949296 ,0.68521946 ,0.89986254 ,
0.05667410 ,0.12741385 ,0.83470977 ,0.63969104 ,0.46612929 ,
0.10200126 ,0.01194925 ,0.10476340 ,0.90285217 ,0.31221221 ,
0.32980614 ,0.46041971 ,0.52024973 ,0.05425470 ,0.28330912 ,
0.60426543 ,0.00598243 ,0.97244013 ,0.21135841 ,0.78561597 ,
0.78428734 ,0.63422849 ,0.32909934 ,0.44771136 ,0.27380750 ,
0.14966697 ,0.18156268 ,0.65686758 ,0.28726350 ,0.97074787 ,
0.63676171 ,0.96649494 ,0.24526295 ,0.08297372 ,0.54257548 ,
0.03166785 ,0.33735355 ,0.15946671 ,0.02102971 ,0.46228045 ,
0.11892296 ,0.33408336 ,0.29875681 ,0.29847692 ,0.73767569 ,
0.02080745 ,0.62980060 ,0.08082293 ,0.22993106 ,0.25031439 ,
0.87787525 ,0.45150053 ,0.13673441 ,0.63407612 ,0.97907688 ,
0.52241942 ,0.50580158 ,0.06273902 ,0.05270283 ,0.77031811 ,
0.05113352 ,0.24393329 ,0.75036441 ,0.37436336 ,0.22877652 ,
0.59975358 ,0.85707591 ,0.88691457 ,0.85547165 ,0.36641027 ,
0.58720133 ,0.45462835 ,0.09243817 ,0.32981586 ,0.07820411 ,
0.25421519 ,0.36004706 ,0.60092307 ,0.46192412 ,0.36758683 ,
0.98424170 ,0.08019934 ,0.68594024 ,0.45826386 ,0.29962317 ,
0.79365413 ,0.89231296 ,0.49478547 ,0.87645944 ,0.23590734 ,
0.28106737 ,0.75026285 ,0.08136314 ,0.79582424 ,0.76010628 ,
0.82792971 ,0.27947652 ,0.72482861 ,0.82191216 ,0.46171689 ,
0.79189752 ,0.96043686 ,0.51609668 ,0.88995725 ,0.28998963 ,
0.55191845 ,0.03934737 ,0.83033700 ,0.49553013 ,0.98009549 ,
0.19017594 ,0.98347750 ,0.33452066 ,0.87144372 ,0.72106301 ,
0.71272114 ,0.71465963 ,0.88361677 ,0.85571283 ,0.73782329 ,
0.20920458 ,0.34855153 ,0.46766817 ,0.02780062 ,0.74898344 ,
0.03680650 ,0.44866557 ,0.77426312 ,0.91025891 ,0.25195236 ,
0.87319953 ,0.63265037 ,0.25552148 ,0.27422476 ,0.95217406 ,
0.39281839 ,0.66441573 ,0.09158900 ,0.94515992 ,0.07800798 ,
0.02507888 ,0.39901462 ,0.17382573 ,0.12141278 ,0.85502334 ,
0.19902911 ,0.02160210 ,0.44460522 ,0.14688742 ,0.68020336 ,
0.71323733 ,0.60922473 ,0.95400380 ,0.99611159 ,0.90897777 ,
0.41073520 ,0.66206647 ,0.32064685 ,0.62805003 ,0.50677209 ,
0.52690101 ,0.87473387 ,0.73918362 ,0.39826974 ,0.43683919 ,
0.80459118 ,0.32422684 ,0.01958019 ,0.95319576 ,0.98326137 ,
0.83931735 ,0.69060863 ,0.33671416 ,0.68062550 ,0.65152380 ,
0.33392969 ,0.03451730 ,0.95227244 ,0.68200635 ,0.85074171 ,
0.64721009 ,0.51234433 ,0.73402047 ,0.00969637 ,0.93835057 ,
0.80803854 ,0.31485260 ,0.20089527 ,0.01323282 ,0.59933780 ,
0.31584602 ,0.20209563 ,0.33754800 ,0.68604181 ,0.24443049 ,
0.19952227 ,0.78162632 ,0.10336988 ,0.11360736 ,0.23536740 ,
0.23262256 ,0.67803776 ,0.48749791 ,0.74658435 ,0.92156640 ,
0.56706407 ,0.36683221 ,0.99157136 ,0.23421374 ,0.45183767 ,
0.91609720 ,0.85573315 ,0.37706276 ,0.77042618 ,0.30891908 ,
0.40709595 ,0.06944866 ,0.61342849 ,0.88817388 ,0.58734506 ,
0.98711323 ,0.14744128 ,0.63242656 ,0.87704136 ,0.68347125 ,
0.84446569 ,0.43265239 ,0.25146321 ,0.04130111 ,0.34259839 ,
0.92697368 ,0.40878778 ,0.56990338 ,0.76204273 ,0.19820348 ,
0.66314909 ,0.02482844 ,0.06669207 ,0.50205581 ,0.26084093 ,
0.65139159 ,0.41650223 ,0.09733904 ,0.56344203 ,0.62651696 ,
0.67332139 ,0.58037374 ,0.47258086 ,0.21010758 ,0.05713135 ,
0.89390629 ,0.10781246 ,0.32037450 ,0.07628388 ,0.34227964 ,
0.42190597 ,0.58201860 ,0.77363549 ,0.49595133 ,0.86031236 ,
0.83906769 ,0.81098161 ,0.26694195 ,0.14215941 ,0.88210306 ,
0.53634237 ,0.12090720 ,0.82480459 ,0.75930318 ,0.31847147 ,
0.92768077 ,0.01037616 ,0.56201727 ,0.88107122 ,0.35925856 ,
0.85860762 ,0.61109408 ,0.70408301 ,0.58434977 ,0.92192494 ,
0.62667915 ,0.75988365 ,0.06858761 ,0.36156496 ,0.58057195 ,
0.13636150 ,0.57719713 ,0.59340255 ,0.63530602 ,0.22976282 ,
0.71915530 ,0.41162531 ,0.63979565 ,0.09931342 ,0.79344045 ,
0.10893790 ,0.84450224 ,0.23122236 ,0.99485593 ,0.73637397 ,
0.17276368 ,0.13357764 ,0.74965804 ,0.64991737 ,0.61990341 ,
0.41523170 ,0.05878239 ,0.05687301 ,0.05497131 ,0.42868366 ,
0.42571090 ,0.25810502 ,0.89642955 ,0.30439758 ,0.39310223 ,
0.11357431 ,0.04288255 ,0.23397550 ,0.11200634 ,0.85621396 ,
0.89733974 ,0.37508865 ,0.42077265 ,0.68597384 ,0.72781399 ,
0.19296476 ,0.61699087 ,0.31667128 ,0.67756410 ,0.00177323 ,
0.05725176 ,0.79474693 ,0.18885238 ,0.06724856 ,0.68193156 ,
0.42202167 ,0.22082041 ,0.28554673 ,0.64995708 ,0.87851940 ,
0.29124547 ,0.61009521 ,0.87374537 ,0.05743712 ,0.69902994 ,
0.81925115 ,0.45653873 ,0.37236821 ,0.31118709 ,0.52734307 ,
0.39672836 ,0.38185294 ,0.30163915 ,0.17374510 ,0.04913278 ,
0.90404879 ,0.25742801 ,0.58266467 ,0.97663209 ,0.79823377 ,
0.36437958 ,0.15206043 ,0.26529938 ,0.22690047 ,0.05839021 ,
0.84721160 ,0.18622435 ,0.37809403 ,0.55706977 ,0.49828704 ,
0.47659049 ,0.24289680 ,0.88477595 ,0.07807463 ,0.56245739 ,
0.73490635 ,0.21099431 ,0.13164942 ,0.75840044 ,0.66877037 ,
0.28988183 ,0.44046090 ,0.24967434 ,0.80048356 ,0.26029740 ,
0.30416821 ,0.64151867 ,0.52067892 ,0.12880774 ,0.85465381 ,
0.02690525 ,0.19149288 ,0.49630295 ,0.79682619 ,0.43566145 ,
0.00288078 ,0.81484193 ,0.03763639 ,0.68529083 ,0.01339574 ,
0.38405386 ,0.30537067 ,0.22994703 ,0.44000045 ,0.27217985 ,
0.53831243 ,0.02870435 ,0.86282045 ,0.61831306 ,0.09164956 ,
0.25609707 ,0.07445781 ,0.72185784 ,0.90058883 ,0.30070608 ,
0.94476583 ,0.56822213 ,0.21933909 ,0.96772793 ,0.80063440 ,
0.26307906 ,0.31183306 ,0.16501252 ,0.55436179 ,0.68562285 ,
0.23829083 ,0.86511559 ,0.57868991 ,0.81888344 ,0.20126869 ,
0.93172350 ,0.66028129 ,0.21786948 ,0.78515828 ,0.10262106 ,
0.35390326 ,0.79303876 ,0.63427924 ,0.90479631 ,0.31024934 ,
0.60635447 ,0.56198079 ,0.63573813 ,0.91854197 ,0.99701497 ,
0.83085849 ,0.31692291 ,0.01925964 ,0.97446405 ,0.98751283 ,
0.60944293 ,0.13751018 ,0.69519957 ,0.68956636 ,0.56969015 ,
0.46440193 ,0.88341765 ,0.36754434 ,0.89223647 ,0.39786427 ,
0.85055280 ,0.12749961 ,0.79452122 ,0.89449784 ,0.14567830 ,
0.45716830 ,0.74822309 ,0.28200437 ,0.42546044 ,0.17464886 ,
0.68308746 ,0.65496587 ,0.52935411 ,0.12736159 ,0.61523955 ,
0.81590528 ,0.63107864 ,0.39786553 ,0.20102294 ,0.53292914 ,
0.75485590 ,0.59847044 ,0.32861691 ,0.12125866 ,0.58917183 ,
0.07638293 ,0.86845380 ,0.29192617 ,0.03989733 ,0.52180460 ,
0.32503407 ,0.64071852 ,0.69516575 ,0.74254998 ,0.54587026 ,
0.48713246 ,0.32920155 ,0.08719954 ,0.63497059 ,0.54328459 ,
0.64178757 ,0.45583809 ,0.70694291 ,0.85212760 ,0.86074305 ,
0.33163422 ,0.85739792 ,0.59908488 ,0.74566046 ,0.72157152 };

static ErrorNumber testPseudoRandomGenerator( void )
{
    int i;
    UInt32 tempUInt32;
    double tempDouble;
    static UInt32 init[4]={0x123, 0x234, 0x345, 0x456};
    char buffer1[100];    
    char buffer2[100];    

    init_by_array(&init[0], (int)4 );

    for (i=0; i<1000; i++) {    
      tempUInt32 = genrand_int32();
      if( tempUInt32 != ExpectedRandomValuesUInt32[i] )
      {
         printf("%10u != %10u", tempUInt32, ExpectedRandomValuesUInt32[i] );
         return TA_INTERNAL_PSEUDORANDOM_UINT32;
      }
    }

    for (i=0; i<1000; i++) {
      tempDouble = genrand_real2();
      sprintf( buffer1, "%10.8f", tempDouble );
      sprintf( buffer2, "%10.8f", ExpectedRandomValuesDouble[i] );  
      if( strcmp(buffer1,buffer2) != 0 )
      {
         printf("%10.8f != %10.8f", tempDouble, ExpectedRandomValuesDouble[i] );
         return TA_INTERNAL_PSEUDORANDOM_DOUBLE;  
      }
    }
   
   return TA_TEST_PASS; /* Success */
}

static ErrorNumber testTimestamps( void )
{
  TA_Timestamp ts;

  /* TEst for fix #1293953. */
  TA_SetDate( 2001, 1, 1, &ts );
  TA_SetTime( 8, 0, 0, &ts );
  TA_AddTimeToTimestamp( &ts, &ts, TA_30MINS );

  if( (TA_GetYear( &ts ) != 2001) ||
      (TA_GetMonth( &ts ) != 1) ||
      (TA_GetDay( &ts ) != 1) ||
      (TA_GetHour( &ts ) != 8) ||
      (TA_GetMin( &ts ) != 30) ||
      (TA_GetSec( &ts ) != 0) )
  {
     printf( "Internal Error: Timestamp offset bug\n" );
     return TA_INTERNAL_TIMESTAMP_TEST_FAILED;
  }

  return TA_TEST_PASS; /* Success */
}

