#ifndef _GSB_CURRENCY_CONFIG_H
#define _GSB_CURRENCY_CONFIG_H (1)

/** This structure holds informations needed for exchange rates
  cache. */
struct cached_exchange_rate {
    gint currency1_number;	/** First currency */
    gint currency2_number;	/** Second currency */
    gsb_real rate;				/** Exchange rate betweend
						  currency1 and currency 2 */
    gsb_real fees;				/** Fees associated with
						  exchange rate */
};

/** ISO 4217 currency.  Not specific to Grisbi. */
struct iso_4217_currency 
{
    gchar * continent;
    gchar * currency_name;
    gchar * country_name;
    gchar * currency_code;
    gchar * currency_nickname;
    gboolean active;
    gchar *flag_filename;
    gint floating_point;  /** number of digit after the point => 2 : 0.00 / 1 : 0.0 */
};


/* START_INCLUDE_H */
#include "gsb_currency_config.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_currency_config_add_currency ( GtkWidget *button,
					    GtkTreeModel *currency_tree_model );
gint gsb_currency_config_create_currency_from_iso4217list ( gchar *currency_name );
GtkWidget *gsb_currency_config_create_page ( void );
GtkWidget *gsb_currency_config_create_totals_page ( void );
struct cached_exchange_rate *gsb_currency_config_get_cached_exchange ( gint currency1_number, 
								       gint currency2_number );
void gsb_currency_config_set_cached_exchange ( gint currency1_number, 
					       gint currency2_number,
					       gsb_real change, gsb_real fees );
/* END_DECLARATION */
#endif
