#ifndef TA_COUNTRY_INFO_H
#define TA_COUNTRY_INFO_H

/* This module provides a table of constant allowing to
 * cross-reference information identifying countries.
 *
 * Updated using ISO3166.
 */

typedef enum {

   /* Important: 
    *   This enum must stay in synch with
    *   the TA_CountryInfoTable define
    *   in ta_country_info.c
    */
   TA_Country_ID_INVALID=0,
   TA_Country_ID_US,
   TA_Country_ID_CA,
   TA_Country_ID_FR,
   TA_Country_ID_DE,
   TA_Country_ID_UK,
   TA_Country_ID_IT,
   TA_Country_ID_MX,
   TA_Country_ID_JP,
   TA_Country_ID_AF,
   TA_Country_ID_AL,
   TA_Country_ID_DZ,
   TA_Country_ID_AS,
   TA_Country_ID_AD,
   TA_Country_ID_AO,
   TA_Country_ID_AI,
   TA_Country_ID_AQ,
   TA_Country_ID_AG,
   TA_Country_ID_AR,
   TA_Country_ID_AM,
   TA_Country_ID_AW,
   TA_Country_ID_AU,
   TA_Country_ID_AT,
   TA_Country_ID_AZ,
   TA_Country_ID_BS,
   TA_Country_ID_BH,
   TA_Country_ID_BD,
   TA_Country_ID_BB,
   TA_Country_ID_BY,
   TA_Country_ID_BE,
   TA_Country_ID_BZ,
   TA_Country_ID_BJ,
   TA_Country_ID_BM,
   TA_Country_ID_BT,
   TA_Country_ID_BO,
   TA_Country_ID_BA,
   TA_Country_ID_BW,
   TA_Country_ID_BV,
   TA_Country_ID_BR,
   TA_Country_ID_IO,
   TA_Country_ID_BN,
   TA_Country_ID_BG,
   TA_Country_ID_BF,
   TA_Country_ID_BI,
   TA_Country_ID_KH,
   TA_Country_ID_CM,
   TA_Country_ID_CV,
   TA_Country_ID_KY,
   TA_Country_ID_CF,
   TA_Country_ID_TD,
   TA_Country_ID_CL,
   TA_Country_ID_CN,
   TA_Country_ID_CX,
   TA_Country_ID_CC,
   TA_Country_ID_CO,
   TA_Country_ID_KM,
   TA_Country_ID_CG,
   TA_Country_ID_CK,
   TA_Country_ID_CR,
   TA_Country_ID_CI,
   TA_Country_ID_HR,
   TA_Country_ID_CU,
   TA_Country_ID_CY,
   TA_Country_ID_CZ,
   TA_Country_ID_DK,
   TA_Country_ID_DJ,
   TA_Country_ID_DM,
   TA_Country_ID_DO,
   TA_Country_ID_TP,
   TA_Country_ID_EC,
   TA_Country_ID_EG,
   TA_Country_ID_SV,
   TA_Country_ID_GQ,
   TA_Country_ID_ER,
   TA_Country_ID_EE,
   TA_Country_ID_ET,
   TA_Country_ID_FK,
   TA_Country_ID_FO,
   TA_Country_ID_FJ,
   TA_Country_ID_FI,
   TA_Country_ID_FX,
   TA_Country_ID_GF,
   TA_Country_ID_PF,
   TA_Country_ID_TF,
   TA_Country_ID_GA,
   TA_Country_ID_GM,
   TA_Country_ID_GE,
   TA_Country_ID_GH,
   TA_Country_ID_GI,
   TA_Country_ID_GR,
   TA_Country_ID_GL,
   TA_Country_ID_GD,
   TA_Country_ID_GP,
   TA_Country_ID_GU,
   TA_Country_ID_GT,
   TA_Country_ID_GN,
   TA_Country_ID_GW,
   TA_Country_ID_GY,
   TA_Country_ID_HT,
   TA_Country_ID_HM,
   TA_Country_ID_HN,
   TA_Country_ID_HK,
   TA_Country_ID_HU,
   TA_Country_ID_IS,
   TA_Country_ID_IN,
   TA_Country_ID_ID,
   TA_Country_ID_IR,
   TA_Country_ID_IQ,
   TA_Country_ID_IE,
   TA_Country_ID_IL,
   TA_Country_ID_JM,
   TA_Country_ID_JO,
   TA_Country_ID_KZ,
   TA_Country_ID_KE,
   TA_Country_ID_KI,
   TA_Country_ID_KP,
   TA_Country_ID_KR,
   TA_Country_ID_KW,
   TA_Country_ID_KG,
   TA_Country_ID_LA,
   TA_Country_ID_LV,
   TA_Country_ID_LB,
   TA_Country_ID_LS,
   TA_Country_ID_LR,
   TA_Country_ID_LY,
   TA_Country_ID_LI,
   TA_Country_ID_LT,
   TA_Country_ID_LU,
   TA_Country_ID_MO,
   TA_Country_ID_MK,
   TA_Country_ID_MG,
   TA_Country_ID_MW,
   TA_Country_ID_MY,
   TA_Country_ID_MV,
   TA_Country_ID_ML,
   TA_Country_ID_MT,
   TA_Country_ID_MH,
   TA_Country_ID_MQ,
   TA_Country_ID_MR,
   TA_Country_ID_MU,
   TA_Country_ID_YT,
   TA_Country_ID_FM,
   TA_Country_ID_MD,
   TA_Country_ID_MC,
   TA_Country_ID_MN,
   TA_Country_ID_MS,
   TA_Country_ID_MA,
   TA_Country_ID_MZ,
   TA_Country_ID_MM,
   TA_Country_ID_NA,
   TA_Country_ID_NR,
   TA_Country_ID_NP,
   TA_Country_ID_NL,
   TA_Country_ID_AN,
   TA_Country_ID_NC,
   TA_Country_ID_NZ,
   TA_Country_ID_NI,
   TA_Country_ID_NE,
   TA_Country_ID_NG,
   TA_Country_ID_NU,
   TA_Country_ID_NF,
   TA_Country_ID_MP,
   TA_Country_ID_NO,
   TA_Country_ID_OM,
   TA_Country_ID_PK,
   TA_Country_ID_PW,
   TA_Country_ID_PA,
   TA_Country_ID_PG,
   TA_Country_ID_PY,
   TA_Country_ID_PE,
   TA_Country_ID_PH,
   TA_Country_ID_PN,
   TA_Country_ID_PL,
   TA_Country_ID_PT,
   TA_Country_ID_PR,
   TA_Country_ID_QA,
   TA_Country_ID_RE,
   TA_Country_ID_RO,
   TA_Country_ID_RU,
   TA_Country_ID_RW,
   TA_Country_ID_KN,
   TA_Country_ID_LC,
   TA_Country_ID_VC,
   TA_Country_ID_WS,
   TA_Country_ID_SM,
   TA_Country_ID_ST,
   TA_Country_ID_SA,
   TA_Country_ID_SN,
   TA_Country_ID_SC,
   TA_Country_ID_SL,
   TA_Country_ID_SG,
   TA_Country_ID_SK,
   TA_Country_ID_SI,
   TA_Country_ID_SB,
   TA_Country_ID_SO,
   TA_Country_ID_ZA,
   TA_Country_ID_GS,
   TA_Country_ID_ES,
   TA_Country_ID_LK,
   TA_Country_ID_SH,
   TA_Country_ID_PM,
   TA_Country_ID_SD,
   TA_Country_ID_SR,
   TA_Country_ID_SJ,
   TA_Country_ID_SZ,
   TA_Country_ID_SE,
   TA_Country_ID_CH,
   TA_Country_ID_SY,
   TA_Country_ID_TW,
   TA_Country_ID_TJ,
   TA_Country_ID_TZ,
   TA_Country_ID_TH,
   TA_Country_ID_TG,
   TA_Country_ID_TK,
   TA_Country_ID_TO,
   TA_Country_ID_TT,
   TA_Country_ID_TN,
   TA_Country_ID_TR,
   TA_Country_ID_TM,
   TA_Country_ID_TC,
   TA_Country_ID_TV,
   TA_Country_ID_UG,
   TA_Country_ID_UA,
   TA_Country_ID_AE,
   TA_Country_ID_UM,
   TA_Country_ID_UY,
   TA_Country_ID_UZ,
   TA_Country_ID_VU,
   TA_Country_ID_VA,
   TA_Country_ID_VE,
   TA_Country_ID_VN,
   TA_Country_ID_VG,
   TA_Country_ID_VI,
   TA_Country_ID_WF,
   TA_Country_ID_EH,
   TA_Country_ID_YE,
   TA_Country_ID_YU,
   TA_Country_ID_ZR,
   TA_Country_ID_ZM,
   TA_Country_ID_ZW
} TA_CountryId;

typedef struct
{
   const char *name; /* Country name */

   /* Standard 2 letters abbreviation. Example: "CA" for Canada. */
   char abbrev[2];

} TA_CountryInfo;

/* Nb of country in the table. */
#define TA_COUNTRY_INFO_TABLE_SIZE 239

/* A table of TA_CountryInfo. */
extern const TA_CountryInfo TA_CountryInfoTable[TA_COUNTRY_INFO_TABLE_SIZE];

/* Conversion/search function. */

/* Return NULL or 0 if not found. */
const char   *TA_CountryIdToAbbrev( TA_CountryId countryId );
TA_CountryId  TA_CountryAbbrevToId( const char *abbrev );
const char   *TA_CountryIdToName( TA_CountryId countryId );

/* Validate that the TA_CountryId is valid. 
 * Return 1 if valid.
 */
unsigned int TA_CountryIdIsValid( TA_CountryId countryId );

#endif