/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2006 Benjamin Drieu (bdrieu@april.org)		      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

/**
 * \file gsb_currency_config.c
 * contains the part to set the configuration of the currencies
 */

#include "include.h"

/*START_INCLUDE*/
#include "gsb_currency_config.h"
#include "dialog.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils.h"
#include "utils_editables.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "structures.h"
#include "gsb_file_config.h"
#include "include.h"
#include "gsb_currency_config.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_currency_append_currency_to_list ( GtkListStore *model,
					    gint currency_number );
static gboolean gsb_currency_config_code_entry_changed ( GtkEditable *editable, gchar * text,
						  gint length, gpointer data );
static GtkWidget * gsb_currency_config_create_box_popup (void);
static gint gsb_currency_config_create_currency ( const gchar *currency_name,
					   const gchar *currency_code, 
					   const gchar *currency_isocode,
					   gint floating_point );
static GtkWidget *gsb_currency_config_create_list ();
static void gsb_currency_config_fill_popup_list ( GtkTreeView * tree_view,
					   gboolean include_obsolete );
static gboolean gsb_currency_config_floating_entry_changed ( GtkEditable *editable,
						      gchar * text,
						      gint length,
						      gpointer data );
static gint gsb_currency_config_get_selected ( GtkTreeView * tree_view );
static gboolean gsb_currency_config_isocode_entry_changed ( GtkEditable *editable, gchar * text,
						     gint length, gpointer data );
static gboolean gsb_currency_config_name_entry_changed ( GtkEditable *editable, gchar * text,
						  gint length, gpointer data );
static GtkWidget * gsb_currency_config_new_combobox ( gint * value, GCallback hook );
static void gsb_currency_config_remove_currency ( GtkWidget *button,
					   GtkWidget *tree_view );
static void gsb_currency_config_remove_selected_from_view ( GtkTreeView * tree_view );
static gboolean gsb_currency_config_select_currency ( GtkTreeSelection *selection, GtkTreeModel *model );
static gboolean gsb_currency_config_select_default ( GtkTreeModel * tree_model, GtkTreePath * path, 
					      GtkTreeIter * iter, GtkTreeView * tree_view );
static gboolean gsb_currency_config_set_int_from_combobox ( GtkWidget *combobox, gint * dummy);
static gboolean gsb_currency_config_update_list ( GtkWidget * checkbox,
					   GtkTreeView * tree_view );
/*END_STATIC*/

/*START_EXTERN*/
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GtkTreeSelection * selection;
extern GtkWidget *tree_view;
extern GtkWidget *window;
/*END_EXTERN*/


/* struct iso_4217_currency; */
struct iso_4217_currency iso_4217_currencies[] = {
    { N_("Africa"), N_("Algerian Dinar"), N_("Algeria"), "DZD", NULL, TRUE, "DZD.png", 2 },
    { N_("Africa"), N_("Botswana Pula"), N_("Botswana"), "BWP", NULL, TRUE, "BWP.png", 2 },
    { N_("Africa"), N_("Burundi Franc"), N_("Burundi"), "BIF", NULL, TRUE, "BIF.png", 2 },
    { N_("Africa"), N_("CFA Franc BCEAO"), N_("Niger"), "XOF", NULL, TRUE, "NIG.png", 2 },
    { N_("Africa"), N_("CFA Franc BCEAO"), N_("Senegal"), "XOF", NULL, TRUE, "SEN.png", 2 },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Cameroon"), "XAF", NULL, TRUE, "CAM.png", 2 },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Chad"), "XAF", NULL, TRUE, "CHA.png", 2 },
    { N_("Africa"), N_("CFA Franc BEAC"), N_("Congo"), "XAF", NULL, TRUE, "CON.png", 2 },
    { N_("Africa"), N_("Comoro Franc"), N_("Comoros"), "KMF", NULL, TRUE, "KMF.png", 2 },
    { N_("Africa"), N_("Egyptian Pound"), N_("Egypt"), "EGP", "£", TRUE, "EGP.png", 2 },
    { N_("Africa"), N_("Ethiopian Birr"), N_("Ethiopia"), "ETB", NULL, TRUE, "ETB.png", 2 },
    { N_("Africa"), N_("Gambian Dalasi"), N_("Gambia"), "GMD", NULL, TRUE, "GMD.png", 2 },
    { N_("Africa"), N_("Ghana Cedi"), N_("Ghana"), "GHC", NULL, TRUE, "GHC.png", 2 },
    { N_("Africa"), N_("Guinea-Bissau Peso"), N_("Guinea-Bissau"), "GWP", NULL, TRUE, "GWP.png", 2 },
    { N_("Africa"), N_("Kenyan Shilling"), N_("Kenya"), "KES", NULL, TRUE, "KES.png", 2 },
    { N_("Africa"), N_("Liberian Dollar"), N_("Liberia"), "LRD", "$", TRUE, "LRD.png", 2 },
    { N_("Africa"), N_("Libyan Dinar"), N_("Libyan Arab Jamahiriya"), "LYD", NULL, TRUE, "LYD.png", 2 },
    { N_("Africa"), N_("Malagasy Franc"), N_("Madagascar"), "MGF", NULL, TRUE, "MGF.png", 2 },
    { N_("Africa"), N_("Malawi Kwacha"), N_("Malawi"), "MWK", NULL, TRUE, "MWK.png", 2 },
    { N_("Africa"), N_("Mauritania Ouguiya"), N_("Mauritania"), "MRO", NULL, TRUE, "MRO.png", 2 },
    { N_("Africa"), N_("Moazambique Metical"), N_("Mozambique"), "MZM", NULL, TRUE, "MZM.png", 2 },
    { N_("Africa"), N_("Moroccan Dirham"), N_("Morocco"), "MAD", NULL, TRUE, "MAD.png", 2 },
    { N_("Africa"), N_("Nigerian Naira"), N_("Nigeria"), "NGN", "₦", TRUE, "NGN.png", 2 },
    { N_("Africa"), N_("Rwanda Franc"), N_("Rwanda"), "RWF", NULL, TRUE, "RWF.png", 2 },
    { N_("Africa"), N_("Sao Tome and Principe Dobra"), N_("Sao Tome and Principe"), "STD", NULL, TRUE, "STD.png", 2 },
    { N_("Africa"), N_("Seychelles Rupee"), N_("Seychelles"), "SCR", "₨", TRUE, "SCR.png", 2 },
    { N_("Africa"), N_("Sierra Leonean Leone"), N_("Sierra Leone"), "SLL", NULL, TRUE, "SLL.png", 2 },
    { N_("Africa"), N_("Somali Shilling"), N_("Somalia"), "SOS", NULL, TRUE, "SOS.png", 2 },
    { N_("Africa"), N_("South African Rand"), N_("Lesotho"), "ZAR", "R", TRUE, "LSL.png", 2 },
    { N_("Africa"), N_("South African Rand"), N_("Namibia"), "ZAR", "R", TRUE, "NAD.png", 2 },
    { N_("Africa"), N_("South African Rand"), N_("South Africa"), "ZAR", "R", TRUE, "ZAR.png", 2 },
    { N_("Africa"), N_("Swaziland Lilangeni"), N_("Swaziland"), "SZL", NULL, TRUE, "SZL.png", 2 },
    { N_("Africa"), N_("Tanzanian Shilling"), N_("United Republic of Tanzania"), "TZS", NULL, TRUE, "TZS.png", 2 },
    { N_("Africa"), N_("Tunisian Dinar"), N_("Tunisia"), "TND", NULL, TRUE, "TND.png", 2 },
    { N_("Africa"), N_("Zambian Kwacha"), N_("Zambia"), "ZMK", NULL, TRUE, "ZMK.png", 2 },
    { N_("Africa"), N_("Zimbabwe Dollar"), N_("Zimbabwe"), "ZWD", "$", TRUE, "ZWD.png", 2 },
    { N_("Asia"), N_("Afghani"), N_("Afghanistan"), "AFA", NULL, TRUE, "AFN.png", 2 },
    { N_("Asia"), N_("Bahraini Dinar"), N_("Bahrain"), "BHD", NULL, TRUE, "BHD.png", 2 },
    { N_("Asia"), N_("Bangladesh Taka"), N_("Bangladesh"), "BDT", NULL, TRUE, "BDT.png", 2 },
    { N_("Asia"), N_("Brunei Dollar"), N_("Brunei Darussalam"), "BND", "$", TRUE, "BND.png", 2 },
    { N_("Asia"), N_("Cambodian Riel"), N_("Cambodia"), "KHR", "៛", TRUE, "KHR.png", 2 },
    { N_("Asia"), N_("Cyprus Pound"), N_("Cyprus"), "CYP", "£", TRUE, "CYP.png", 2 },
    { N_("Asia"), N_("Hong Kong Dollar"), N_("Hong Kong"), "HKD", "$", TRUE, "HKD.png", 2 },
    { N_("Asia"), N_("Indian Rupee"), N_("Bhutan"), "INR", "₨", TRUE, "BHU.png", 2 },
    { N_("Asia"), N_("Indian Rupee"), N_("India"), "INR", "₨", TRUE, "INR.png", 2 },
    { N_("Asia"), N_("Indonesian Rupiah"), N_("Indonesia"), "IDR", NULL, TRUE, "IDR.png", 2 },
    { N_("Asia"), N_("Iranian Rial"), N_("Iran"), "IRR", "﷼", TRUE, "IRR.png", 2 },
    { N_("Asia"), N_("Iraqi Dinar"), N_("Iraq"), "IQD", NULL, TRUE, "IQD.png", 2 },
    { N_("Asia"), N_("Japanese Yen"), N_("Japan"), "JPY", "¥", TRUE, "JPY.png", 2 },
    { N_("Asia"), N_("Jordanian Dinar"), N_("Jordan"), "JOD", NULL, TRUE, "JOD.png", 2 },
    { N_("Asia"), N_("Kuwaiti Dinar"), N_("Kuwait"), "KWD", NULL, TRUE, "KWD.png", 2 },
    { N_("Asia"), N_("Lao Kip"), N_("Lao People's Democratic Republic"), "LAK", "₭", TRUE, "LAK.png", 2 },
    { N_("Asia"), N_("Lebanese Pound"), N_("Lebanon"), "LBP", "£", TRUE, "LBP.png", 2 },
    { N_("Asia"), N_("Macau Pataca"), N_("Macao"), "MOP", NULL, TRUE, "MOP.png", 2 },
    { N_("Asia"), N_("Malaysian Ringgit"), N_("Malaysia"), "MYR", NULL, TRUE, "MYR.png", 2 },
    { N_("Asia"), N_("Mongolian Tugrik"), N_("Mongolia"), "MNT", "₮", TRUE, "MNT.png", 2 },
    { N_("Asia"), N_("Nepalese Rupee"), N_("Nepal"), "NPR", "₨", TRUE, "NPR.png", 2 },
    { N_("Asia"), N_("New Israeli Shekel"), N_("Israel"), "ILS", "₪", TRUE, "ILS.png", 2 },
    { N_("Asia"), N_("New Taiwan Dollar"), N_("Taiwan, Province of China"), "TWD", "元", TRUE, "TWD.png", 2 },
    { N_("Asia"), N_("North Korean Won"), N_("Democratic People's Republic of Korea"), "KPW", "₩", TRUE, "KPW.png", 2 },
    { N_("Asia"), N_("Pakistan Rupee"), N_("Pakistan"), "PKR", "₨", TRUE, "PKR.png", 2 },
    { N_("Asia"), N_("Philippine peso"), N_("Philippines"), "PHP", "₱", TRUE, "PHP.png", 2 },
    { N_("Asia"), N_("Qatari Rial"), N_("Qatar"), "QAR", "﷼", TRUE, "QAR.png", 2 },
    { N_("Asia"), N_("Rial Omani"), N_("Oman"), "OMR", "﷼", TRUE, "OMR.png", 2 },
    { N_("Asia"), N_("Russian Ruble"), N_("Russia"), "RUR", "руб", TRUE, "RUB.png", 2 },
    { N_("Asia"), N_("Saudi Riyal"), N_("Saudi Arabia"), "SAR", "﷼", TRUE, "SAR.png", 2 },
    { N_("Asia"), N_("Singapore Dollar"), N_("Singapore"), "SGD", "$", TRUE, "SGD.png", 2 },
    { N_("Asia"), N_("South Korean Won"), N_("Republic of Korea"), "KRW", "₩", TRUE, "KRW.png", 2 },
    { N_("Asia"), N_("Sri Lanka Rupee"), N_("Sri Lanka"), "LKR", "௹", TRUE, "LKR.png", 2 },
    { N_("Asia"), N_("Syrian Pound"), N_("Syrian Arab Republic"), "SYP", "£", TRUE, "SYP.png", 2 },
    { N_("Asia"), N_("Thai Baht"), N_("Thailand"), "THB", "฿", TRUE, "THB.png", 2 },
    { N_("Asia"), N_("Turkish Lira"), N_("Turkey"), "TRL", "₤", TRUE, "TRL.png", 2 },
    { N_("Asia"), N_("United Arab Emirates Dirham"), N_("United Arab Emirates"), "AED", NULL, TRUE, "AED.png", 2 },
    { N_("Asia"), N_("Viet Nam Dong"), N_("Viet Nam"), "VND", "₫", TRUE, "VND.png", 2 },
    { N_("Asia"), N_("Yemeni Rial"), N_("Yemen"), "YER", "﷼", TRUE, "YER.png", 2 },
    { N_("Asia"), N_("Yuan Renminbi"), N_("China"), "CNY", "元", TRUE, "CNY.png", 2 },
    { N_("Central America"), N_("Belize Dollar"), N_("Belize"), "BZD", "$", TRUE, "BZD.png", 2 },
    { N_("Central America"), N_("Costa Rican Colon"), N_("Costa Rica"), "CRC", "₡", TRUE, "CRC.png", 2 },
    { N_("Central America"), N_("Guatemalan Quetzal"), N_("Guatemala"), "GTQ", NULL, TRUE, "GTQ.png", 2 },
    { N_("Central America"), N_("Honduran Lempira"), N_("Honduras"), "HNL", NULL, TRUE, "HNL.png", 2 },
    { N_("Central America"), N_("Mexican Peso"), N_("Mexico"), "MXP", "$", TRUE, "MXP.png", 2 },
    { N_("Central America"), N_("Panama Balboa"), N_("Panama"), "PAB", NULL, TRUE, "PAB.png", 2 },
    { N_("Europe"), N_("Albanian Lek"), N_("Albania"), "ALL", NULL, TRUE, "ALL.png", 2 },
    { N_("Europe"), N_("Austrian Schilling"), N_("Austria"), "ATS", NULL, FALSE, "ATS.png", 2 },
    { N_("Europe"), N_("Belgian Franc"), N_("Belgium"), "BEF", NULL, FALSE, "BEF.png", 2 },
    { N_("Europe"), N_("Bulgarian Lev"), N_("Bulgaria"), "BGL", NULL, FALSE, "BGN.png", 2 },
    { N_("Europe"), N_("Czech Koruna"), N_("Czech Republic"), "CZK", NULL, TRUE, "CZK.png", 2 },
    { N_("Europe"), N_("Danish Krone"), N_("Denmark"), "DKK", NULL, TRUE, "DKK.png", 2 },
    { N_("Europe"), N_("Deutsche Mark"), N_("Germany"), "DEM", NULL, FALSE, "DEM.png", 2 },
    { N_("Europe"), N_("Estonian Kroon"), N_("Estonia"), "EEK", NULL, TRUE, "EEK.png", 2 },
    { N_("Europe"), N_("Euro"), N_("CEE"), "EUR", "€", TRUE, "EUR.png", 2 },
    { N_("Europe"), N_("Finnish Markka"), N_("Finland"), "FIM", NULL, FALSE, "FIM.png", 2 },
    { N_("Europe"), N_("French Franc"), N_("France"), "FRF", "₣", FALSE, "FRF.png", 2 },
    { N_("Europe"), N_("Gibraltar Pound"), N_("Gibraltar"), "GIP", "£", TRUE, "GIP.png", 2 },
    { N_("Europe"), N_("Greek Drachma"), N_("Greece"), "GRD", "₯", FALSE, "GRD.png", 2 },
    { N_("Europe"), N_("Hungarian Forint"), N_("Hungary"), "HUF", NULL, TRUE, "HUF.png", 2 },
    { N_("Europe"), N_("Iceland Krona"), N_("Iceland"), "ISK", NULL, TRUE, "ISK.png", 2 },
    { N_("Europe"), N_("Irish Pound"), N_("Ireland"), "IEP", "£", FALSE, "IEP.png", 2 },
    { N_("Europe"), N_("Italian Lira"), N_("Holy See"), "ITL", "₤", FALSE, "VAT.png", 2 },
    { N_("Europe"), N_("Italian Lira"), N_("Italy"), "ITL", "₤", FALSE, "ITL.png", 2 },
    { N_("Europe"), N_("Italian Lira"), N_("San Marino"), "ITL", "₤", FALSE, "SAN.png", 2 },
    { N_("Europe"), N_("Latvian Lat"), N_("Latvia"), "LVL", NULL, TRUE, "LVL.png", 2 },
    { N_("Europe"), N_("Lithuanian Litas"), N_("Lietuva"), "LTL", NULL, TRUE, "LTL.png", 2 },
    { N_("Europe"), N_("Luxembourg Franc"), N_("Luxembourg"), "LUF", "₣", FALSE, "LUF.png", 2 },
    { N_("Europe"), N_("Netherlands Guilder"), N_("Netherlands"), "NLG", "ƒ", FALSE, "NLG.png", 2 },
    { N_("Europe"), N_("New Yugoslavian Dinar"), N_("Serbia and Montenegro"), "YUD", NULL, FALSE, "YUV.png", 2 },
    { N_("Europe"), N_("Norwegian Krone"), N_("Norway"), "NOK", NULL, TRUE, "NOK.png", 2 },
    { N_("Europe"), N_("Polish Zloty"), N_("Poland"), "PLZ", NULL, TRUE, "PLN.png", 2 },
    { N_("Europe"), N_("Portuguese Escudo"), N_("Portugal"), "PTE", NULL, FALSE, "PTE.png", 2 },
    { N_("Europe"), N_("Pound Sterling"), N_("United Kingdom"), "GBP", "£", TRUE, "GBP.png", 2 },
    { N_("Europe"), N_("Romanian Leu"), N_("Romania"), "ROL", NULL, TRUE, "ROL.png", 2 },
    { N_("Europe"), N_("Slovak Koruna"), N_("Slovakia"), "SKK", NULL, TRUE, "SKK.png", 2 },
    { N_("Europe"), N_("Slovene Tolar"), N_("Slovenia"), "SIT", NULL, TRUE, "SIT.png", 2 },
    { N_("Europe"), N_("Spanish Peseta"), N_("Spain"), "ESP", "₧", FALSE, "ESP.png", 2 },
    { N_("Europe"), N_("Swedish Krona"), N_("Sweden"), "SEK", "kr", TRUE, "SEK.png", 2 },
    { N_("Europe"), N_("Swiss Franc"), N_("Liechtenstein"), "CHF", NULL, TRUE, "LIE.png", 2 },
    { N_("Europe"), N_("Swiss Franc"), N_("Switzerland"), "CHF", NULL, TRUE, "CHF.png", 2 },
    { N_("Europe"), N_("Hryvnia"), N_("Ukraine"), "UAH", NULL, TRUE, "UAH.png", 2 },
    { N_("Northern America"), N_("Bahamian Dollar"), N_("Bahamas"), "BSD", "$", TRUE, "BSD.png", 2 },
    { N_("Northern America"), N_("Barbados Dollar"), N_("Barbados"), "BBD", "$", TRUE, "BBD.png", 2 },
    { N_("Northern America"), N_("Bermuda Dollar"), N_("Bermuda"), "BMD", "$", TRUE, "BMD.png", 2 },
    { N_("Northern America"), N_("Canadian Dollar"), N_("Canada"), "CAD", "$", TRUE, "CAD.png", 2 },
    { N_("Northern America"), N_("Cayman Islands Dollar"), N_("Cayman Islands"), "KYD", NULL, TRUE, "KYD.png", 2 },
    { N_("Northern America"), N_("Cuban Peso"), N_("Cuba"), "CUP", "₱ 	", TRUE, "CUP.png", 2 },
    { N_("Northern America"), N_("Dominican Peso"), N_("Dominican Republic"), "DOP", "₱", TRUE, "DOP.png", 2 },
    { N_("Northern America"), N_("East Caribbean Dollar"), N_("Grenada"), "XCD", "$", TRUE, "GRE.png", 2 },
    { N_("Northern America"), N_("East Caribbean Dollar"), N_("Saint Lucia"), "XCD", "$", TRUE, "SLC.png", 2 },
    { N_("Northern America"), N_("Haitian Gourde"), N_("Haiti"), "HTG", NULL, TRUE, "HTG.png", 2 },
    { N_("Northern America"), N_("Jamaican Dollar"), N_("Jamaica"), "JMD", "$", TRUE, "JMD.png", 2 },
    { N_("Northern America"), N_("Netherlands Antillian Guilder"), N_("Netherlands Antilles"), "ANG", "ƒ", TRUE, "ANG.png", 2 },
    { N_("Northern America"), N_("Trinidad and Tobago Dollar"), N_("Trinidad and Tobago"), "TTD", "$ 	", TRUE, "TTD.png", 2 },
    { N_("Northern America"), N_("United States Dollar"), N_("United States"), "USD", "$", TRUE, "USD.png", 2 },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Australia"), "AUD", "$", TRUE, "AUD.png", 2 },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Kiribati"), "AUD", "$", TRUE, "KIR.png", 2 },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Nauru"), "AUD", "$", TRUE, "NAU.png", 2 },
    { N_("Pacific Ocean"), N_("Australian Dollar"), N_("Tuvalu"), "AUD", "$", TRUE, "TUV.png", 2 },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("French Polynesia"), "XPF", NULL, TRUE, "FRF.png", 2 },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("New Caledonia"), "XPF", NULL, TRUE, "FRF.png", 2 },
    { N_("Pacific Ocean"), N_("CFP Franc"), N_("Wallis and Futuna"), "XPF", NULL, TRUE, "FRF.png", 2 },
    { N_("Pacific Ocean"), N_("Fiji Dollar"), N_("Fiji"), "FJD", "$", TRUE, "FJD.png", 2 },
    { N_("Pacific Ocean"), N_("New Zealand Dollar"), N_("Cook Islands"), "NZD", "$", TRUE, "COO.png", 2 },
    { N_("Pacific Ocean"), N_("New Zealand Dollar"), N_("New Zealand"), "NZD", NULL, TRUE, "NZD.png", 2 },
    { N_("Pacific Ocean"), N_("Papua New Guinea Kina"), N_("Papua New Guinea"), "PGK", NULL, TRUE, "PGK.png", 2 },
    { N_("Pacific Ocean"), N_("Samoa Tala"), N_("Samoa"), "WST", NULL, TRUE, "WST.png", 2 },
    { N_("Pacific Ocean"), N_("Solomon Islands Dollar"), N_("Solomon Islands"), "SBD", NULL, TRUE, "SBD.png", 2 },
    { N_("Pacific Ocean"), N_("Timor Escudo"), N_("Timor"), "TPE", NULL, TRUE, "TPE.png", 2 },
    { N_("Pacific Ocean"), N_("Tongan Pa'anga"), N_("Tonga"), "TOP", NULL, TRUE, "TOP.png", 2 },
    { N_("Pacific Ocean"), N_("United States Dollar"), N_("Panama"), "USD", "$", TRUE, "PAB.png", 2 },
    { N_("Pacific Ocean"), N_("Vanuatu Vatu"), N_("Vanuatu"), "VUV", NULL, TRUE, "VUV.png", 2 },
    { N_("Southern America"), N_("Peso"), N_("Argentina"), "ARP", "$", TRUE, "ARS.png", 2 },
    { N_("Southern America"), N_("Boliviano"), N_("Bolivia"), "BOB", "$", TRUE, "BOB.png", 2 },
    { N_("Southern America"), N_("Peso"), N_("Chile"), "CLP", "$", TRUE, "CLP.png", 2 },
    { N_("Southern America"), N_("Peso"), N_("Colombia"), "COP", "₱", TRUE, "COP.png", 2 },
    { N_("Southern America"), N_("Ecuador Sucre"), N_("Ecuador"), "ECS", NULL, FALSE, "ECS.png", 2 },
    { N_("Southern America"), N_("Guyana Dollar"), N_("Guyana"), "GYD", NULL, TRUE, "GYD.png", 2 },
    { N_("Southern America"), N_("Paraguay Guarani"), N_("Paraguay"), "PYG", NULL, TRUE, "PYG.png", 2 },
    { N_("Southern America"), N_("Nuevos Soles"), N_("Peru"), "PEN", "S/.", TRUE, "PEN.png", 2 },
    { N_("Southern America"), N_("Real"), N_("Brazil"), "BRL", "R$", TRUE, "BRL.png", 2 },
    { N_("Southern America"), N_("Suriname Guilder"), N_("Suriname"), "SRG", NULL, TRUE, "SRD.png", 2 },
    { N_("Southern America"), N_("Peso"), N_("Uruguay"), "UYU", "₱", TRUE, "UYU.png", 2 },
    { N_("Southern America"), N_("Venezuelan Bolivar"), N_("Venezuela"), "VEB", NULL, TRUE, "VEB.png", 2 },
    { NULL },
};

/** Columns numbers for currencies list  */
enum currency_list_column {
    CURRENCY_FLAG_COLUMN,
    CURRENCY_HAS_FLAG,
    COUNTRY_NAME_COLUMN,
    CURRENCY_NAME_COLUMN,
    CURRENCY_ISO_CODE_COLUMN,
    CURRENCY_NICKNAME_COLUMN,
    CURRENCY_FLOATING_COLUMN,
    CURRENCY_POINTER_COLUMN,
    NUM_CURRENCIES_COLUMNS,
};

/** Exchange rates cache, used by
 * gsb_currency_config_set_cached_exchange
 * and 
 * gsb_currency_config_get_cached_exchange */
GSList * cached_exchange_rates = NULL;


/**
 * Creates the currency list and associated form to configure them.
 *
 * \param
 *
 * \returns A newly created vbox
 */
GtkWidget *gsb_currency_config_create_page ( void )
{
    GtkWidget *vbox_pref, *label, *paddingbox, *hbox;
    GtkWidget *scrolled_window, *vbox, *table;
    GtkWidget *button;
    GtkTreeView *currency_list_view;
    GtkTreeModel *currency_tree_model;
    GtkWidget *entry;

    vbox_pref = new_vbox_with_title_and_icon ( _("Currencies"), "currencies.png" ); 
    paddingbox = new_paddingbox_with_title (vbox_pref, TRUE, _("Known currencies"));

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, TRUE, TRUE, 0);

    /* Currency list */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    /* Create it. */
    currency_list_view = GTK_TREE_VIEW ( gsb_currency_config_create_list () );
    currency_tree_model = gtk_tree_view_get_model ( currency_list_view );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), GTK_WIDGET(currency_list_view) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window, TRUE, TRUE, 0);
    g_signal_connect ( gtk_tree_view_get_selection (GTK_TREE_VIEW ( currency_list_view ) ), 
		       "changed", G_CALLBACK ( gsb_currency_config_select_currency ), 
		       currency_tree_model );

    /* if nothing opened, all is unsensitive */
    if ( !gsb_data_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    else
    {
	/* fill the list with the known currencies */

	GSList *tmp_list;

	tmp_list = gsb_data_currency_get_currency_list ();

	while ( tmp_list )
	{
	    gint currency_number;

	    currency_number = gsb_data_currency_get_no_currency (tmp_list -> data);

	    gsb_currency_append_currency_to_list ( GTK_LIST_STORE ( currency_tree_model ),
					       currency_number );
	    tmp_list = tmp_list -> next;
	}

    }

    /* Create Add/Remove buttons */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, FALSE, FALSE, 0 );

    /* Button "Add" */
    button = gtk_button_new_from_stock (GTK_STOCK_ADD);
    g_signal_connect ( G_OBJECT ( button ),
		       "clicked",
		       G_CALLBACK  ( gsb_currency_config_add_currency ),
		       currency_tree_model );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );

    /* Button "Remove" */
    button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    g_signal_connect ( G_OBJECT ( button ),
		       "clicked",
		       G_CALLBACK ( gsb_currency_config_remove_currency ),
		       currency_list_view );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );

    /* Input form for currencies */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Currency properties"));

    /* Create table */
    table = gtk_table_new ( 4, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 0 );

    /* Create currency name entry */
    label = gtk_label_new (COLON(_("Name")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry = new_text_entry ( NULL, (GCallback) gsb_currency_config_name_entry_changed, NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entry, 1, 2, 0, 1, 
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_tree_model), "entry_name", entry );
    g_object_set_data ( G_OBJECT(entry), "tree_view", currency_list_view );

    /* Create code entry */
    label = gtk_label_new (COLON(_("Sign")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry = new_text_entry ( NULL, (GCallback) gsb_currency_config_code_entry_changed,
			     NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entry, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_tree_model), "entry_code", entry );
    g_object_set_data ( G_OBJECT(entry), "tree_view", currency_list_view );

    /* Create code entry */
    label = gtk_label_new ( COLON(_("ISO code")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry = new_text_entry ( NULL, (GCallback) gsb_currency_config_isocode_entry_changed, 
			     NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entry, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_tree_model), "entry_iso_code", entry );
    g_object_set_data ( G_OBJECT(entry), "tree_view", currency_list_view );

    /* Create floating point entry */
    label = gtk_label_new ( COLON(_("Floating point")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 3, 4,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry = new_text_entry ( NULL, (GCallback) gsb_currency_config_floating_entry_changed, 
			     NULL );
    gtk_table_attach ( GTK_TABLE ( table ), entry, 1, 2, 3, 4,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(currency_tree_model), "entry_floating_point", entry );
    g_object_set_data ( G_OBJECT(entry), "tree_view", currency_list_view );

    return ( vbox_pref );
}


/**
 * create the tree wich contains the currency list
 * used both for the currency known list, and the currency to add list
 *
 * \param
 *
 * \return a GtkTreeView
 */
GtkWidget *gsb_currency_config_create_list ()
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkListStore * model;
    GtkWidget * treeview;
    gint col_offset;

    /* Create tree store
       CURRENCY_FLAG_COLUMN,
       CURRENCY_HAS_FLAG,
       COUNTRY_NAME_COLUMN,
       CURRENCY_NAME_COLUMN,
       CURRENCY_ISO_CODE_COLUMN,
       CURRENCY_NICKNAME_COLUMN,
       CURRENCY_FLOATING_COLUMN,
       CURRENCY_POINTER_COLUMN */
    model = gtk_list_store_new (NUM_CURRENCIES_COLUMNS,
				GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
				G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_INT, G_TYPE_INT );

    /* Create tree tree_view */
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

    /* Flag */
    cell = gtk_cell_renderer_pixbuf_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Country name"),
						     cell, "pixbuf",
						     CURRENCY_FLAG_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), cell,
				       "visible", CURRENCY_HAS_FLAG);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Country name */
    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), cell, TRUE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), cell, 
				       "text", COUNTRY_NAME_COLUMN);

    /* Currency name */
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    cell = gtk_cell_renderer_text_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Currency name"),
						     cell, "text",
						     CURRENCY_NAME_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    cell = gtk_cell_renderer_text_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("ISO Code"), 
						     cell, "text",
						     CURRENCY_ISO_CODE_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    cell = gtk_cell_renderer_text_new ();
    col_offset = 
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1, _("Sign"), 
						     cell, "text",
						     CURRENCY_NICKNAME_COLUMN,
						     NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model), 
					  COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

    return treeview;
}


/**
 * Pop up a dialog to create a new currency, do some sanity checks and
 * call the gsb_currency_config_create_currency() function to do the grunt work.
 * 
 * \param button	GtkButton that triggered event.
 * \param currency_tree_model the treemodel to add the new currency (can be NULL)
 * 
 * \return TRUE if currency has been created.
 */
gboolean gsb_currency_config_add_currency ( GtkWidget *button,
					    GtkTreeModel *currency_tree_model )
{
    GtkWidget *dialog, *label, *table, *list, *paddingbox;
    GtkTreeModel *model;
    const gchar *currency_name, *currency_code, *currency_isocode;
    gint floating_point;
    gint currency_number;
    gint result;
    GtkWidget *entry_name, *entry_code, *entry_isocode, *entry_floating_point;

    dialog = gtk_dialog_new_with_buttons ( _("Add a currency"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL,0,
					   GTK_STOCK_OK,1,
					   NULL );

    gtk_container_set_border_width ( GTK_CONTAINER ( dialog ), 12 );

    paddingbox = 
	new_paddingbox_with_title (GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ),
				   TRUE, _("ISO 4217 currencies"));

    /* Create list */
    list = gsb_currency_config_create_box_popup ();
    model = g_object_get_data ( G_OBJECT(list), "model" );

    gtk_box_pack_start ( GTK_BOX(paddingbox) , list, TRUE, TRUE, 5 );

    paddingbox = 
	new_paddingbox_with_title (GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ),
				   FALSE, _("Currency details"));

    /* Create table */
    table = gtk_table_new ( 4, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 0 );

    /* Currency name */
    label = gtk_label_new (COLON(_("Currency name")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry_name = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entry_name ), TRUE );
    gtk_table_attach ( GTK_TABLE ( table ), entry_name, 1, 2, 0, 1,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_name", entry_name );

    /* Currency ISO code */
    label = gtk_label_new (COLON(_("Currency ISO 4217 code")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry_isocode = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), entry_isocode, 1, 2, 1, 2,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_iso_code", entry_isocode );

    /* Currency usual sign */
    label = gtk_label_new (COLON(_("Currency sign")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry_code = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), entry_code, 1, 2, 2, 3,
		       GTK_EXPAND|GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_code", entry_code );

    /* Create floating point entry */
    label = gtk_label_new ( COLON(_("Floating point")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 3, 4,
		       GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    entry_floating_point = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), entry_floating_point, 1, 2, 3, 4,
		       GTK_EXPAND | GTK_FILL, 0, 0, 0 );
    g_object_set_data ( G_OBJECT(model), "entry_floating_point", entry_floating_point );

    /* Select default currency. */
    gtk_tree_model_foreach ( GTK_TREE_MODEL(model), 
			     (GtkTreeModelForeachFunc) gsb_currency_config_select_default, 
			     g_object_get_data ( G_OBJECT(list), "treeview" ) );
dialog_return:
    gtk_widget_show_all ( GTK_WIDGET ( dialog ) );
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( result )
    {
	case 1 :

	    currency_name = gtk_entry_get_text ( GTK_ENTRY ( entry_name ));
	    currency_code = gtk_entry_get_text ( GTK_ENTRY ( entry_code ));
	    currency_isocode = gtk_entry_get_text ( GTK_ENTRY ( entry_isocode ));
	    floating_point = utils_str_atoi (gtk_entry_get_text ( GTK_ENTRY ( entry_floating_point )));

	    if ( strlen ( currency_name ) && 
		 (strlen ( currency_code ) ||
		  strlen ( currency_isocode )))
	    {
		/* check if the currency exists */

		if ( gsb_data_currency_get_number_by_name ( currency_name )
		     ||
		     gsb_data_currency_get_number_by_code_iso4217 ( currency_isocode ))
		{
		    dialogue_error_hint ( _("Currency names or iso 4217 codes should be unique.  Please choose a new name for the currency."),
					  g_strdup_printf ( _("Currency '%s' already exists." ), currency_name ));
		    goto dialog_return;
		}

		currency_number = gsb_currency_config_create_currency ( currency_name, currency_code, currency_isocode, floating_point);

		if ( currency_tree_model )
		{
		    gsb_currency_append_currency_to_list ( GTK_LIST_STORE ( currency_tree_model ),
							   currency_number );
		    gsb_currency_update_currency_list ();
		    modification_fichier ( TRUE );
		    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
		    return TRUE;
		}
	    }
	    else
	    {
		dialogue_warning_hint ( _("Currency name and either currency ISO4217 code or currency nickname should be set."),
					_("All fields are not filled in") );
		goto dialog_return;
	    }
	    break;
    }
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
    return TRUE;
}


/**
 * append the given currency in the list of known currencies
 *
 * \param model the tree_model of the known currencies
 * \param currency_number
 *
 * \return
 *
 */
void gsb_currency_append_currency_to_list ( GtkListStore *model,
					    gint currency_number )
{
    GdkPixbuf * pixbuf;
    GtkTreeIter iter;
    gchar *string;

    string = g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
			  "flags", C_DIRECTORY_SEPARATOR,
			  gsb_data_currency_get_code_iso4217 (currency_number),
			  ".png", NULL );
    pixbuf = gdk_pixbuf_new_from_file ( string,
					NULL );	
    g_free (string);

    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter,
			 CURRENCY_FLAG_COLUMN, pixbuf,
			 COUNTRY_NAME_COLUMN, "",
			 CURRENCY_NAME_COLUMN, gsb_data_currency_get_name (currency_number),
			 CURRENCY_ISO_CODE_COLUMN, gsb_data_currency_get_code_iso4217 (currency_number),
			 CURRENCY_NICKNAME_COLUMN, gsb_data_currency_get_code (currency_number),
			 CURRENCY_FLOATING_COLUMN,  gsb_data_currency_get_floating_point (currency_number),
			 CURRENCY_POINTER_COLUMN, currency_number,
			 CURRENCY_HAS_FLAG, TRUE,
			 -1);
}


/**
 * create a new currency according to the param
 *
 * \param currency_name
 * \param currency_code
 * \param currency_isocode
 * \param floating_point
 *
 * \return the number of the new currency
 * */
gint gsb_currency_config_create_currency ( const gchar *currency_name,
					   const gchar *currency_code, 
					   const gchar *currency_isocode,
					   gint floating_point )
{
    gint currency_number;

    currency_number = gsb_data_currency_new (currency_name);
    gsb_data_currency_set_code ( currency_number,
				 currency_code );
    gsb_data_currency_set_code_iso4217 ( currency_number,
					 currency_isocode );
    gsb_data_currency_set_floating_point ( currency_number,
					   floating_point );
    return currency_number;
}


/**
 * look for a currency with its code name in the iso list
 * and create it
 *
 * \param currency_name the currency code we look for
 *
 * \return the number of the new currency or FALSE if not found
 *
 */
gint gsb_currency_config_create_currency_from_iso4217list ( gchar *currency_name )
{
    struct iso_4217_currency * currency = iso_4217_currencies;

    while ( currency -> country_name )
    {
	if ( !strcmp ( currency -> currency_code, currency_name ) )
	    return gsb_currency_config_create_currency ( currency -> currency_name, 
							 currency -> currency_nickname, 
							 currency -> currency_code,
							 currency -> floating_point );
	currency++;
    }

    return FALSE;
}


/**
 * Remove selected currency from currency list.  First, be sure there
 * is no use for it.
 *
 * \param button	Widget that triggered event.
 * \param tree_view		GtkTreeView that contains selected currency.
 */
void gsb_currency_config_remove_currency ( GtkWidget *button,
					   GtkWidget *tree_view )
{
    GSList *list_tmp;
    gint currency_number;

    currency_number = gsb_currency_config_get_selected ( GTK_TREE_VIEW ( tree_view ) );
    if ( !currency_number )
	return;

    /* we look for that currency in all the transactions,
     * if we find it, we cannot delete it */

    list_tmp = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp )
    {
	gint transaction_number;
	transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

	if ( gsb_data_transaction_get_currency_number (transaction_number) == currency_number )
	{
	    dialogue_error_hint ( g_strdup_printf ( _("Currency '%s' is used in current file.  Grisbi can't delete it."),
						    gsb_data_currency_get_name (currency_number)),
				  g_strdup_printf ( _("Impossible to remove currency '%s'"), 
						    gsb_data_currency_get_name (currency_number) ));
	    return;
	}
	else
	    list_tmp = list_tmp -> next;
    }

    /* check the currency in the scheduled transactions */
    list_tmp = gsb_data_scheduled_get_scheduled_list ();

    while ( list_tmp )
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp -> data);
	if ( gsb_data_scheduled_get_currency_number (scheduled_number) == currency_number )
	{
	    dialogue_error_hint ( g_strdup_printf ( _("Currency '%s' is used in current file.  Grisbi can't delete it."),
						    gsb_data_currency_get_name (currency_number)),
				  g_strdup_printf ( _("Impossible to remove currency '%s'"), 
						    gsb_data_currency_get_name (currency_number) ));
	    return;
	}
	else
	    list_tmp = list_tmp -> next;
    }

    gsb_currency_config_remove_selected_from_view ( GTK_TREE_VIEW(tree_view) );
    gsb_data_currency_remove (currency_number);
}


/**
 * Remove selected currency from tree.  In fact, this is a generic
 * function that could be used for any purpose (and could be then
 * renamed).
 *
 * \param tree_view	GtkTreeView to remove selected entry from.
 */
void gsb_currency_config_remove_selected_from_view ( GtkTreeView * tree_view )
{
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( tree_view );
    GtkTreeIter iter;
    GtkTreeModel * tree_model;

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return;

    gtk_list_store_remove ( GTK_LIST_STORE (tree_model), &iter );
}



/**
 * create the box wich contains the world currencies list for the
 * add currency popup
 *
 * \param
 *
 * \return a vbox
 *
 */
GtkWidget * gsb_currency_config_create_box_popup (void)
{ 
    GtkWidget * sw, * treeview, * vbox, * checkbox;
    GtkTreeModel * model;

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_ALWAYS);

    treeview = gsb_currency_config_create_list ();
    gtk_widget_set_usize ( treeview, FALSE, 200 );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW(treeview) );
    g_signal_connect ( gtk_tree_view_get_selection (GTK_TREE_VIEW ( treeview ) ), 
		       "changed", G_CALLBACK ( gsb_currency_config_select_currency ), 
		       model );

    gtk_container_add (GTK_CONTAINER (sw), treeview);
    gtk_container_set_resize_mode (GTK_CONTAINER (sw), GTK_RESIZE_PARENT);

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(vbox), sw,
			 TRUE, TRUE, 0 );

    checkbox = gtk_check_button_new_with_label ( _("Include obsolete currencies"));
    gtk_box_pack_start ( GTK_BOX(vbox), checkbox,
			 FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT(checkbox), "toggled",
		       (GCallback) gsb_currency_config_update_list, treeview );

    gsb_currency_config_fill_popup_list ( GTK_TREE_VIEW(treeview), FALSE );

    g_object_set_data ( G_OBJECT(vbox), "model", model );
    g_object_set_data ( G_OBJECT(vbox), "treeview", treeview );

    return vbox;
}


/**
 * fill the currency list in the popup
 * with all the world currencies
 *
 * \param tree_view the tree tree_view to fill in
 * \param include_obsolete TRUE to fill with the obsolete curencies
 *
 * \return
 */
void gsb_currency_config_fill_popup_list ( GtkTreeView * tree_view,
					   gboolean include_obsolete )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    struct iso_4217_currency * currency = iso_4217_currencies;

    model = gtk_tree_view_get_model ( tree_view );

    while (currency -> country_name )
    {
	if ( include_obsolete || currency -> active )
	{
	    GdkPixbuf * pixbuf;
	    gchar *string;

	    string = g_strconcat( PIXMAPS_DIR, 
				  C_DIRECTORY_SEPARATOR,
				  "flags", 
				  C_DIRECTORY_SEPARATOR,
				  currency -> flag_filename, 
				  NULL );
	    pixbuf = gdk_pixbuf_new_from_file ( string,
						NULL );	
	    g_free (string);

	    string = g_strconcat ( " ", 
				   _(currency -> country_name),
				   NULL );
	    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				CURRENCY_FLAG_COLUMN, pixbuf,
				COUNTRY_NAME_COLUMN, string,
				CURRENCY_NAME_COLUMN, _(currency -> currency_name),
				CURRENCY_ISO_CODE_COLUMN, _(currency -> currency_code),
				CURRENCY_NICKNAME_COLUMN, _(currency -> currency_nickname),
				CURRENCY_FLOATING_COLUMN, currency -> floating_point,
				CURRENCY_POINTER_COLUMN, currency,
				CURRENCY_HAS_FLAG, TRUE,
				-1);
	    g_free (string);
	}
	currency++;
    }
}



/**
 * called when toggle the button show/hide the obselete currencies
 * in the popup
 * fill the list according that button
 *
 * \param checkbox contains what the user want
 * \param tree_view
 *
 * \return FALSE
 *
 */
gboolean gsb_currency_config_update_list ( GtkWidget * checkbox,
					   GtkTreeView * tree_view )
{
    GtkTreeModel * model;

    model = gtk_tree_view_get_model ( tree_view );
    gtk_list_store_clear ( GTK_LIST_STORE (model) );
    gsb_currency_config_fill_popup_list ( tree_view, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) );
    return FALSE;
}


/**
 * called when select a currency in the whole world list in the add popup
 * fill the corresponding entries to append it
 *
 * \param selection
 * \param model the tree_model
 *
 */
gboolean gsb_currency_config_select_currency ( GtkTreeSelection *selection, GtkTreeModel *model )
{
    gchar * currency_name, * currency_iso_code, * currency_nickname;
    gint currency_floating;
    GtkWidget * entry_name, * entry_iso_code, * entry_code, * entry_floating_point;
    GtkTreeIter iter;

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    entry_name = g_object_get_data ( G_OBJECT(model), "entry_name" );
    entry_iso_code = g_object_get_data ( G_OBJECT(model), "entry_iso_code" );
    entry_code = g_object_get_data ( G_OBJECT(model), "entry_code" );
    entry_floating_point = g_object_get_data ( G_OBJECT(model), "entry_floating_point" );


    gtk_tree_model_get ( model, &iter, 
			 CURRENCY_NAME_COLUMN, &currency_name,
			 CURRENCY_ISO_CODE_COLUMN, &currency_iso_code,
			 CURRENCY_NICKNAME_COLUMN, &currency_nickname, 
			 CURRENCY_FLOATING_COLUMN, &currency_floating, 
			 -1 );

    if ( ! currency_name ) 
	currency_name = "";
    if ( ! currency_nickname ) 
	currency_nickname = "";
    if ( ! currency_iso_code ) 
	currency_iso_code = "";

    /* need to block the callback to avoid to modify the file even if we just select a currency */
    g_signal_handlers_block_by_func ( G_OBJECT (entry_name),
				     G_CALLBACK (gsb_currency_config_name_entry_changed),
				     NULL );
    gtk_entry_set_text ( GTK_ENTRY ( entry_name ), currency_name );
    g_signal_handlers_unblock_by_func ( G_OBJECT (entry_name),
				     G_CALLBACK (gsb_currency_config_name_entry_changed),
				     NULL );

    g_signal_handlers_block_by_func ( G_OBJECT (entry_iso_code),
				     G_CALLBACK (gsb_currency_config_isocode_entry_changed),
				     NULL );
    gtk_entry_set_text ( GTK_ENTRY ( entry_iso_code ), currency_iso_code );
    g_signal_handlers_unblock_by_func ( G_OBJECT (entry_iso_code),
				     G_CALLBACK (gsb_currency_config_isocode_entry_changed),
				     NULL );

    g_signal_handlers_block_by_func ( G_OBJECT (entry_code),
				     G_CALLBACK (gsb_currency_config_code_entry_changed),
				     NULL );
    gtk_entry_set_text ( GTK_ENTRY ( entry_code ), currency_nickname );
    g_signal_handlers_unblock_by_func ( G_OBJECT (entry_code),
				     G_CALLBACK (gsb_currency_config_code_entry_changed),
				     NULL );

    g_signal_handlers_block_by_func ( G_OBJECT (entry_floating_point),
				     G_CALLBACK (gsb_currency_config_floating_entry_changed),
				     NULL );
    gtk_entry_set_text ( GTK_ENTRY ( entry_floating_point ), utils_str_itoa (currency_floating));
    g_signal_handlers_unblock_by_func ( G_OBJECT (entry_floating_point),
				     G_CALLBACK (gsb_currency_config_floating_entry_changed),
				     NULL );
    return ( FALSE );
} 



/**
 * Obtain selected currency from currency tree.
 *
 * \param tree_view		GtkTreeView to remove currency from.
 * 
 * \return		the number of the selected currency.
 */
gint gsb_currency_config_get_selected ( GtkTreeView * tree_view )
{
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( tree_view );
    GtkTreeIter iter;
    GtkTreeModel * tree_model;
    gint currency_number;

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gtk_tree_model_get ( tree_model,
			 &iter, 
			 CURRENCY_POINTER_COLUMN, &currency_number,
			 -1 );
    return currency_number;
}



/**
 * called when user change something in the name entry
 * set the new name for the current selected currency
 *
 * \param editable the entry
 * \param text the new text - not used
 * \param length length of the new text
 * \param data not used
 *
 * \return FALSE
 */
gboolean gsb_currency_config_name_entry_changed ( GtkEditable *editable, gchar * text,
						  gint length, gpointer data )
{
    gint currency_number;
    GtkTreeSelection * selection;
    GtkTreeModel * tree_model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "tree_view" ) );
    currency_number = gsb_currency_config_get_selected ( g_object_get_data ( G_OBJECT (editable), "tree_view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gsb_data_currency_set_name ( currency_number,
				 gtk_entry_get_text ( GTK_ENTRY (editable)));
    gtk_list_store_set ( GTK_LIST_STORE ( tree_model ), &iter,
			 CURRENCY_NAME_COLUMN, gsb_data_currency_get_name (currency_number),
			 -1);
    gsb_currency_update_currency_list ();
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * called when user change something in the code entry
 * set the new code for the current selected currency
 *
 * \param editable the entry
 * \param text the new text - not used
 * \param length length of the new text
 * \param data not used
 *
 * \return FALSE
 */
gboolean gsb_currency_config_code_entry_changed ( GtkEditable *editable, gchar * text,
						  gint length, gpointer data )
{
    gint currency_number;
    GtkTreeModel * tree_model;
    GtkTreeSelection * selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "tree_view" ) );
    currency_number = gsb_currency_config_get_selected ( g_object_get_data ( G_OBJECT (editable), "tree_view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gsb_data_currency_set_code ( currency_number,
				 gtk_entry_get_text ( GTK_ENTRY (editable)));

    gtk_list_store_set ( GTK_LIST_STORE ( tree_model ), &iter,
			 CURRENCY_NICKNAME_COLUMN, gsb_data_currency_get_code (currency_number),
			 -1);
    gsb_currency_update_currency_list ();
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * called when user change something in the isocode entry
 * set the new isocode for the current selected currency
 *
 * \param editable the entry
 * \param text the new text - not used
 * \param length length of the new text
 * \param data not used
 *
 * \return FALSE
 */
gboolean gsb_currency_config_isocode_entry_changed ( GtkEditable *editable, gchar * text,
						     gint length, gpointer data )
{
    gint currency_number;
    GtkTreeModel * tree_model;
    GtkTreeSelection * selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "tree_view" ) );
    currency_number = gsb_currency_config_get_selected ( g_object_get_data ( G_OBJECT (editable), "tree_view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gsb_data_currency_set_code_iso4217 ( currency_number,
					 gtk_entry_get_text ( GTK_ENTRY (editable)));


    gtk_list_store_set ( GTK_LIST_STORE ( tree_model ), &iter,
			 CURRENCY_ISO_CODE_COLUMN, gsb_data_currency_get_code_iso4217 (currency_number),
			 -1);
    gsb_currency_update_currency_list ();
    modification_fichier ( TRUE );

    return FALSE;
}

/**
 * called when the user change the value in the floating point entry
 *
 * \param editable the entry
 * \param text
 * \param length
 * \param data not used
 *
 * \return FALSE
 * */
gboolean gsb_currency_config_floating_entry_changed ( GtkEditable *editable,
						      gchar * text,
						      gint length,
						      gpointer data )
{
    gint currency_number;
    GtkTreeModel *tree_model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection ( g_object_get_data ( G_OBJECT (editable), "tree_view" ) );
    currency_number = gsb_currency_config_get_selected ( g_object_get_data ( G_OBJECT (editable), "tree_view" ) );

    if ( !selection || ! gtk_tree_selection_get_selected (selection, &tree_model, &iter))
	return(FALSE);

    gsb_data_currency_set_floating_point ( currency_number,
					   utils_str_atoi (gtk_entry_get_text ( GTK_ENTRY (editable))));
    gtk_list_store_set ( GTK_LIST_STORE ( tree_model ), &iter,
			 CURRENCY_FLOATING_COLUMN, gsb_data_currency_get_floating_point (currency_number),
			 -1);
    modification_fichier ( TRUE );

    return FALSE;
}


/**
 * Find whether echange rate between two currencies is known.  If so,
 * returns a cached_exchange_rate structure with exchange rate
 * information.
 *
 * \param currency1 First currency
 * \param currency2 Second currency
 *
 * \return NULL on failure, a pointer to a cached_exchange_rate
 * structure on success.
 */
struct cached_exchange_rate *gsb_currency_config_get_cached_exchange ( gint currency1_number, 
								       gint currency2_number )
{
    GSList * tmp_list = cached_exchange_rates;

    while ( tmp_list )
    {
	struct cached_exchange_rate * tmp;

	tmp = tmp_list -> data;
	if ( currency1_number == tmp -> currency1_number && currency2_number == tmp -> currency2_number )
	    return tmp;

	tmp_list = tmp_list -> next;
    }
    return NULL;
}


/**
 * Update exchange rate cache according to arguments.
 *
 * \param currency1 First currency.
 * \param currency2 Second currency.
 * \param change    Exchange rate between two currencies.
 * \param fees      Fees of transaction.
 */
void gsb_currency_config_set_cached_exchange ( gint currency1_number, 
					       gint currency2_number,
					       gsb_real change, gsb_real fees )
{
    struct cached_exchange_rate * tmp;

    tmp = (struct cached_exchange_rate *) g_malloc(sizeof(struct cached_exchange_rate));

    tmp -> currency1_number = currency1_number;
    tmp -> currency2_number = currency2_number;
    tmp -> rate = change;
    tmp -> fees = fees;

    cached_exchange_rates = g_slist_append ( cached_exchange_rates, tmp );
}



/**
 * foreach function on the tree_view to find the default currency
 * according to the current locale
 *
 * \param tree_model
 * \param path
 * \param iter
 * \param tree_view
 *
 * \return TRUE if found
 */
gboolean gsb_currency_config_select_default ( GtkTreeModel * tree_model, GtkTreePath * path, 
					      GtkTreeIter * iter, GtkTreeView * tree_view )
{
    struct lconv * conv = localeconv();
    gchar * code, * symbol, * country;
    gboolean good = FALSE;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
			 CURRENCY_ISO_CODE_COLUMN, &code, 
			 COUNTRY_NAME_COLUMN, &country, 
			 -1 );
    if ( conv && conv -> int_curr_symbol && strlen ( conv -> int_curr_symbol ) )
    {
	symbol = g_strstrip ( my_strdup ( conv -> int_curr_symbol ));
	if ( ! strcmp ( code, symbol ) )
	{
	    good = TRUE;
	}
	free ( symbol );
    }
    else
    {
	symbol = g_strstrip ( my_strdup ( country ) );
	if ( ! strcmp ( symbol, _("United States") ) )
	{
	    good = TRUE;
	}
	free ( symbol );
    }

    if ( good )
    {
	gtk_tree_selection_select_path ( gtk_tree_view_get_selection ( tree_view ), path );
	gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (tree_view), path, NULL, TRUE, 0.5, 0 );
	return TRUE;
    }
    return FALSE;
}


/**
 * show combo_box with the currencies to choose the currency
 * for totals of lists (payee, categories, budgets
 *
 * \param
 *
 * \return the widget to set in the conf
 */
GtkWidget *gsb_currency_config_create_totals_page ( void )
{
    GtkWidget *vbox_pref, *combo_box, *table, *label;

    vbox_pref = new_vbox_with_title_and_icon ( _("Totals currencies"),
					       "currencies.png" );

    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );

    label = gtk_label_new (_("Currency for payees tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 
		       0, 1, 0, 1, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    combo_box = gsb_currency_config_new_combobox ( &no_devise_totaux_tiers, 
					       remplit_arbre_tiers );
    gtk_table_attach ( GTK_TABLE ( table ), combo_box,
		       1, 2, 0, 1, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    label = gtk_label_new (_("Currency for categories tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label,
		       0, 1, 1, 2, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    combo_box = gsb_currency_config_new_combobox ( &no_devise_totaux_categ, 
					       remplit_arbre_categ );
    gtk_table_attach ( GTK_TABLE ( table ), combo_box,
		       1, 2, 1, 2, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    label = gtk_label_new (_("Currency for budgetary lines tree"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_table_attach ( GTK_TABLE ( table ), label,
		       0, 1, 2, 3, GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    combo_box = gsb_currency_config_new_combobox ( &no_devise_totaux_ib, 
					       remplit_arbre_imputation );
    gtk_table_attach ( GTK_TABLE ( table ), combo_box,
		       1, 2, 2, 3, GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), table, TRUE, TRUE, 0);

    return ( vbox_pref );
}


/**
 * Create a new GtkComboBox with a pointer to an integer that will be
 * modified according to the entry's value.
 *
 * \param value A pointer to a gint wich contains the currency number
 * \param hook An optional function to execute as a handler
 *
 * \return A newly allocated option menu.
 */
GtkWidget * gsb_currency_config_new_combobox ( gint * value, GCallback hook )
{
    GtkWidget *combo_box;

    combo_box = gsb_currency_make_combobox (FALSE);

    if (value && *value)
	gsb_currency_set_combobox_history ( combo_box,
					    *value );

    g_signal_connect ( GTK_OBJECT (combo_box), "changed", (GCallback) gsb_currency_config_set_int_from_combobox, value );
    g_signal_connect ( GTK_OBJECT (combo_box), "changed", (GCallback) hook, value );
    g_object_set_data ( G_OBJECT ( combo_box ), "pointer", value);

    return combo_box;
}


/**
 * Set an integer to the value of a menu.  Normally called via a GTK
 * "changed" signal handler.
 * 
 * \param menu a pointer to a menu widget.
 * \param dummy unused
 */
gboolean gsb_currency_config_set_int_from_combobox ( GtkWidget *combobox, gint * dummy)
{
    gint *data;
    
    data = g_object_get_data ( G_OBJECT(combobox), "pointer" );

    if ( data )
    {
	*data = gsb_currency_get_currency_from_combobox (combobox);
    }

    /* Mark file as modified */
    modification_fichier ( TRUE );
    return (FALSE);
}


