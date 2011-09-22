/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2011 Pierre Biava (grisbi@pierre.biava.name)             */
/*                   2011 Guillaume Verger                                    */
/*          http://www.grisbi.org                                             */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdk.h>
#include <glib/gi18n.h>
#include <strings.h>

#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>

/*START_INCLUDE*/
#include "bet_graph.h"
#include "bet_data.h"
#include "bet_hist.h"
#include "dialog.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_fyear.h"
#include "gsb_dirs.h"
#include "navigation.h"
#include "structures.h"
#include "utils_gtkbuilder.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


typedef struct _struct_bet_graph_data   struct_bet_graph_data;
typedef struct _struct_bet_graph_button struct_bet_graph_button;


struct _struct_bet_graph_data
{
    GtkTreeView *tree_view;
    gint account_number;

    /* Données pour le graphique */
    GtkWidget *widget;
    gchar *title;
    gchar *service_id;          /* définit le type de graphique */
    gboolean is_legend;

    /* données communes */
    gint nbre_elemnts;

    /* données pour les histogrammes et XY */
    gdouble tab_X[MAX_POINTS_GRAPHIQUE];
    gdouble tab_Y[MAX_POINTS_GRAPHIQUE];

    /* données pour les camemberts */
    gint type_infos;    /* 0 type crédit ou < 0, 1 type débit ou >= 0, -1 tous types */
    gchar tab_libelle[MAX_POINTS_GRAPHIQUE][TAILLE_MAX_LIBELLE];
    gchar **tab_vue_libelle;

};


struct _struct_bet_graph_button
{
    gchar *name;
    gchar *filename;
    gchar *service_id;          /* définit le type de graphique */
    GCallback callback;         /* fonction de callback */
    gboolean is_visible;        /* TRUE si le bouton est visible dans la barre d'outils */
    GtkWidget *box;
    GtkWidget *button;
    GtkWidget *tree_view;
};


/*START_STATIC*/
static gboolean bet_graph_affiche_camemberts ( struct_bet_graph_data *self );
static gboolean bet_graph_affiche_XY_line ( struct_bet_graph_data *self );
static void bet_graph_create_graph_widget  ( struct_bet_graph_data *self );
static void bet_graph_create_pie_canvas  ( struct_bet_graph_data *self );
static GtkWidget *bet_graph_get_canvas  ( void );
static gboolean bet_graph_initialise_builder ( void );
static gboolean bet_graph_populate_lines_by_forecast_data ( struct_bet_graph_data *self );
static gboolean bet_graph_populate_sectors_by_hist_data ( struct_bet_graph_data *self );
static void bet_graph_popup_choix_graph_activate ( GtkMenuItem *menuitem,
                        struct_bet_graph_button *self );
static gboolean bet_graph_popup_choix_graph_menu ( GtkWidget *button,
                        GdkEventButton *event,
                        GList *liste );
static void struct_free_bet_graph_button ( struct_bet_graph_button *self );
static struct_bet_graph_button *struct_initialise_bet_graph_button ( void );
static void struct_free_bet_graph_data ( struct_bet_graph_data *self );
static struct_bet_graph_data *struct_initialise_bet_graph_data ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static GtkBuilder *bet_graph_builder = NULL;


/**
 * Crée un builder et récupère les widgets du fichier bet_graph.ui
 *
 *
 *
 * */
gboolean bet_graph_initialise_builder ( void )
{
    /* Creation d'un nouveau GtkBuilder */
    bet_graph_builder = gtk_builder_new ( );
    if ( bet_graph_builder == NULL )
        return FALSE;

    /* récupère les widgets */
    if ( utils_gtkbuilder_merge_ui_data_in_builder ( bet_graph_builder, "bet_graph.ui" ) )
        return TRUE;
    else
        return FALSE;

}


/**
 *
 *
 *
 *
 * */
void bet_graph_create_graph_widget  ( struct_bet_graph_data *self )
{
	GtkWidget *w;
	GogChart *chart;
	GogGraph *graph;
	GogLabel *label;
	GogPlot *plot;
	GOStyle *style;
	GOData *data;
	PangoFontDescription *desc;

	/* Set the graph widget */
	w = self->widget;

	/* Get the embedded graph */
	graph = go_graph_widget_get_graph ( GO_GRAPH_WIDGET ( w ) );

    /* set the title */
    if ( self->title && strlen ( self->title ) )
    {
        label = (GogLabel *) g_object_new ( GOG_TYPE_LABEL, NULL );
        data = go_data_scalar_str_new ( self->title, FALSE );
        gog_dataset_set_dim ( GOG_DATASET ( label ), 0, data, NULL );
        gog_object_add_by_name ( GOG_OBJECT ( graph ), "Title", GOG_OBJECT ( label ) );

        /* Change the title font */
        style = go_styled_object_get_style ( GO_STYLED_OBJECT ( label ) );
        desc = pango_font_description_from_string ( "Sans bold 12" );
        go_style_set_font_desc ( style, desc );
    }

	/* Get the chart created by the widget initialization */
	chart = go_graph_widget_get_chart ( GO_GRAPH_WIDGET ( w ) );

	/* Create a pie plot and add it to the chart */
	plot = ( GogPlot *) gog_plot_new_by_name ( self->service_id );
	gog_object_add_by_name ( GOG_OBJECT ( chart ), "Plot", GOG_OBJECT ( plot ) );

	/* Add a legend to the chart */
    if ( self->is_legend )
	    gog_object_add_by_name ( GOG_OBJECT ( chart ), "Legend", NULL);
}


/**
 *
 *
 *
 *
 * */
void bet_graph_sectors_graph_new ( GtkWidget *button, GtkTreeView *tree_view )
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *sw_canvas;
    GtkWidget *box_pie;
    gchar *title;
    gint result;
    gint account_number;
    struct_bet_graph_data *self_credit;
    struct_bet_graph_data *self_debit;

    devel_debug (NULL);

    /* Initialisation d'un nouveau GtkBuilder */
    if ( !bet_graph_initialise_builder ( ) )
        return;

    /* Création de la fenêtre de dialogue pour le graph */
    dialog = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "bet_graph_dialog" ) );
    gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), GTK_WINDOW ( run.window ) );
    gtk_widget_set_usize ( dialog, PAGE_WIDTH+30, PAGE_HEIGHT+70 );
    gtk_signal_connect ( GTK_OBJECT ( dialog ),
                        "destroy",
		                GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
                        NULL);

    sw_canvas = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "sw_canvas" ) );
    box_pie = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "box_pie" ) );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw_canvas ), box_pie );

    /* set the title */
    label = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( tree_view ), "label_title") );
    title = make_pango_attribut ( "weight=\"bold\" size=\"x-large\"",
                    gtk_label_get_text ( GTK_LABEL ( label ) ) );

    label = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "label_canvas" ) );
    gtk_label_set_markup ( GTK_LABEL ( label ), title );
    g_free ( title );

    /* initialisation des structures de données */
    account_number = gsb_gui_navigation_get_current_account ( );

    /* Set the graph for credit */
    self_credit = struct_initialise_bet_graph_data ( );
    self_credit->tree_view = tree_view;
    self_credit->account_number = account_number;
    self_credit->type_infos = 0;
    self_credit->title = g_strdup ( _("Credit") );
    self_credit->service_id = g_strdup ( "GogPiePlot" );
    self_credit->is_legend = TRUE;

    self_credit->widget = go_graph_widget_new ( NULL );
    bet_graph_create_graph_widget ( self_credit );
    gtk_box_pack_start ( GTK_BOX ( box_pie ), self_credit->widget, TRUE, TRUE, 0 );

    /* Set the graph for debit */
    self_debit = struct_initialise_bet_graph_data ( );
    self_debit->tree_view = tree_view;
    self_debit->account_number = account_number;
    self_debit->type_infos = 1;
    self_debit->title = g_strdup ( _("Debit") );
    self_debit->service_id = g_strdup ( "GogPiePlot" );
    self_debit->is_legend = TRUE;

    self_debit->widget = go_graph_widget_new ( NULL );
    bet_graph_create_graph_widget ( self_debit );
    gtk_box_pack_start ( GTK_BOX ( box_pie ), self_debit->widget, TRUE, TRUE, 0 );

    /* populate and show the data for credit */
    result = bet_graph_populate_sectors_by_hist_data ( self_credit );
    if ( ! result )
        return;
    result = bet_graph_affiche_camemberts ( self_credit );
    if ( ! result )
        return;

    /* populate and show the data for debit */
    result = bet_graph_populate_sectors_by_hist_data ( self_debit );
    if ( ! result )
        return;
   result = bet_graph_affiche_camemberts ( self_debit );
    if ( ! result )
        return;

    gtk_widget_show_all ( dialog );

    result = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    /* free the data */
    struct_free_bet_graph_data ( self_debit );
    struct_free_bet_graph_data ( self_credit );
    g_object_unref ( G_OBJECT ( bet_graph_builder ) );
    gtk_widget_destroy ( dialog );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_populate_sectors_by_hist_data ( struct_bet_graph_data *self )
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( self -> tree_view ) );
    if ( model == NULL )
        return FALSE;

    if ( gtk_tree_model_get_iter_first ( model, &iter ) )
    {
        gint account_number;
        gchar *libelle_division = self -> tab_libelle[0];
        gchar **tab_libelle_division;
        gdouble *tab_montant_division = self -> tab_Y;

        tab_libelle_division = &libelle_division;

        /* test du numero de compte */
        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
                        -1 );
        if ( account_number != self -> account_number )
            return FALSE;
        do
        {
            gchar *desc;
            gchar *amount;
            gint div;
            gint type_infos;

            gtk_tree_model_get ( GTK_TREE_MODEL( model ),
                        &iter,
                        SPP_HISTORICAL_DESC_COLUMN, &desc,
                        SPP_HISTORICAL_RETAINED_AMOUNT, &amount,
                        SPP_HISTORICAL_DIV_NUMBER, &div,
                        -1 );

            type_infos = bet_data_get_div_type ( div );
            if ( self -> type_infos == -1 || type_infos == self -> type_infos )
            {
                strncpy ( &libelle_division[self -> nbre_elemnts * TAILLE_MAX_LIBELLE], desc, TAILLE_MAX_LIBELLE );
                tab_montant_division[self -> nbre_elemnts] = utils_str_strtod ( ( amount == NULL) ? "0" : amount, NULL );

                self -> nbre_elemnts++;
            }

            if ( self -> nbre_elemnts >= MAX_SEGMENT_CAMEMBERT )
                break;

        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );

        return TRUE;
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_affiche_camemberts ( struct_bet_graph_data *self )
{
    GogChart *chart;
    GogObject *cur_plot;
    GogSeries *series;
    GOData *data;
    GError * error = NULL;

    /* Afficher les montants des divisions de type débit dans le camembert */
    chart = go_graph_widget_get_chart ( GO_GRAPH_WIDGET ( self -> widget ) );
    cur_plot = gog_object_get_child_by_name ( GOG_OBJECT ( chart ), "Plot" );

    gog_plot_clear_series ( GOG_PLOT ( cur_plot ) );
    series = GOG_SERIES ( gog_plot_new_series ( GOG_PLOT ( cur_plot ) ) );

    data = go_data_vector_str_new ( (const char * const*) self -> tab_vue_libelle, self -> nbre_elemnts, NULL );
    gog_series_set_dim ( series, 0, data, &error );
    if ( error != NULL )
    {
        printf ("tab_vue_libelle_division : erreur = %s\n", error -> message);
        g_error_free ( error );
        error = NULL;
        return FALSE;
    }

    data = go_data_vector_val_new ( self -> tab_Y, self -> nbre_elemnts, NULL);
	gog_series_set_dim (series, 1, data, &error);
    if ( error != NULL )
    {
        printf ("tab_montant_division : erreur = %s\n", error -> message);
        g_error_free ( error );
        error = NULL;
        return FALSE;
    }

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
void bet_graph_line_graph_new ( GtkWidget *button, GtkTreeView *tree_view )
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *sw_canvas;
    GtkWidget *box_line;
    gchar *title;
    gchar *service_id;
    gint result;
    gint account_number;
    struct_bet_graph_data *self;

    devel_debug (NULL);

    /* Initialisation d'un nouveau GtkBuilder */
    if ( !bet_graph_initialise_builder ( ) )
        return;

    /* initialisation des structures de données */
    account_number = gsb_gui_navigation_get_current_account ( );
    service_id = g_object_get_data ( G_OBJECT ( button ), "service_id" );

    self = struct_initialise_bet_graph_data ( );
    self->tree_view = tree_view;
    self->account_number = account_number;
    self->service_id = g_strdup ( service_id );

    /* Création de la fenêtre de dialogue pour le graph */
    dialog = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "bet_graph_dialog" ) );
    gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), GTK_WINDOW ( run.window ) );
    gtk_widget_set_usize ( dialog, PAGE_WIDTH+30, PAGE_HEIGHT+70 );
    gtk_signal_connect ( GTK_OBJECT ( dialog ),
                        "destroy",
		                GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
                        NULL);

    sw_canvas = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "sw_canvas" ) );
    box_line = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "box_line" ) );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw_canvas ), box_line );

    /* set the title */
    label = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( tree_view ), "label_title") );
    title = make_pango_attribut ( "weight=\"bold\" size=\"x-large\"",
                    gtk_label_get_text ( GTK_LABEL ( label ) ) );

    label = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "label_canvas" ) );
    gtk_label_set_markup ( GTK_LABEL ( label ), title );
    g_free ( title );

    /* Set the graph for credit */
    self->widget = go_graph_widget_new ( NULL );
    bet_graph_create_graph_widget ( self );
    gtk_box_pack_start ( GTK_BOX ( box_line ), self->widget, TRUE, TRUE, 0 );

    bet_graph_populate_lines_by_forecast_data ( self );
    bet_graph_affiche_XY_line ( self );
    gtk_widget_show_all ( dialog );

    result = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    /* free the data */
    struct_free_bet_graph_data ( self );
    g_object_unref ( G_OBJECT ( bet_graph_builder ) );
    gtk_widget_destroy ( dialog );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_populate_lines_by_forecast_data ( struct_bet_graph_data *self )
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( self -> tree_view ) );
    if ( model == NULL )
        return FALSE;

    if ( gtk_tree_model_get_iter_first ( model, &iter ) )
    {
        gchar *libelle_axe_x = self -> tab_libelle[0];
        gchar **tab_libelle_axe_x;
/*         gdouble *tab_X = self -> tab_X;  */
        gdouble *tab_Y = self -> tab_Y;
        gdouble montant = 0.;
        GDateMonth mois_courant;

/*         gint x_axis = 0;  */

        tab_libelle_axe_x = &libelle_axe_x;

        do
        {
            gchar *amount;
            gchar *str_date;
            GValue date_value = {0,};
            GDate *date;
/*             GDateMonth mois;  */

            gtk_tree_model_get_value ( model,
                        &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value );

            gtk_tree_model_get ( GTK_TREE_MODEL( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &amount,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, &str_date,
                        -1 );
            date = g_value_get_boxed ( &date_value );

            montant += (gdouble) utils_str_strtod ( ( amount == NULL) ? "0" : amount, NULL );
            tab_Y[self->nbre_elemnts] = montant;

            if ( self -> nbre_elemnts == 0 )
            {
                g_date_add_days ( date, 1 );
                mois_courant = g_date_get_month ( date );
                strncpy ( &libelle_axe_x[self -> nbre_elemnts * TAILLE_MAX_LIBELLE], str_date, TAILLE_MAX_LIBELLE );
            }
            else
            {
/*                 mois = g_date_get_month ( date );
 *                 if ( mois == mois_courant )
 *                 {
 *                     strncpy ( &libelle_axe_x[self -> nbre_elemnts * TAILLE_MAX_LIBELLE], "", TAILLE_MAX_LIBELLE );
 *                 }
 *                 else
 *                 {
 */
                    strncpy ( &libelle_axe_x[self -> nbre_elemnts * TAILLE_MAX_LIBELLE], str_date, TAILLE_MAX_LIBELLE );
                    mois_courant = g_date_get_month ( date );
/* printf ("self -> nbre_elemnts = %d\tdate_value = %s date_texte = %s x = %d y = %.2f\n",
 *     self -> nbre_elemnts, gsb_format_gdate (date), str_date, (gint) tab_X[self->nbre_elemnts], tab_Y[self->nbre_elemnts]);
 */

/*                 }                  */
            }

            self -> nbre_elemnts++;

            if ( self -> nbre_elemnts >= MAX_POINTS_GRAPHIQUE )
                break;
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );

        return TRUE;
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_affiche_XY_line ( struct_bet_graph_data *self )
{
    GogChart *chart;
    GogObject *cur_plot;
    GogSeries *series;
    GOData *data;
    GError * error = NULL;

    /* Afficher données dans le graphique */
    chart = go_graph_widget_get_chart ( GO_GRAPH_WIDGET ( self->widget ) );
    cur_plot = gog_object_get_child_by_name ( GOG_OBJECT ( chart ), "Plot" );

    series = GOG_SERIES ( gog_plot_new_series ( GOG_PLOT ( cur_plot ) ) );

    data = go_data_vector_str_new ( (const char * const*) self -> tab_vue_libelle, self -> nbre_elemnts, NULL );
	gog_series_set_dim (series, 0, data, &error);
    data = go_data_vector_val_new ( self->tab_Y, self->nbre_elemnts, NULL);
	gog_series_set_dim (series, 1, data, &error);

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_graph_button_menu_new ( GsbButtonStyle style,
					    const gchar *type_graph,
					    GCallback callback,
                        GtkWidget *tree_view )
{
    GtkWidget *arrow_button = NULL;
    GtkWidget *box;
    GtkWidget *box_button;
    GList *liste = NULL;
    struct_bet_graph_button *self;

    /* Initialisation d'un nouveau GtkBuilder */
    if ( !bet_graph_initialise_builder ( ) )
        return NULL;

    /* boite qui contiendra les deux boutons */
    box = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "button_menu_box" ) );

    /* boite qui contiendra le bouton actif */
    box_button = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "box_button" ) );

    if ( strcmp ( type_graph, "forecast_graph" ) == 0 )
    {
        self = struct_initialise_bet_graph_button ( );

        self->name = g_strdup ( _("Column") );
        self->filename = g_strdup ( "graph-histo.png" );
        self->service_id = g_strdup ( "GogBarColPlot" );
        self->callback = callback;
        self->box = box_button;
        self->tree_view = tree_view;
        self->is_visible = TRUE;

        self->button = gsb_automem_imagefile_button_new ( style,
                        self->name,
                        self->filename,
                        NULL,
                        NULL );

        g_object_set_data ( G_OBJECT ( self->button ), "service_id", self->service_id );
        g_signal_connect ( G_OBJECT ( self->button ),
                        "clicked",
                        self->callback,
                        self->tree_view );
        gtk_box_pack_start ( GTK_BOX ( self->box ), self->button, TRUE, TRUE, 0 );
        liste = g_list_append ( liste, self );

        self = struct_initialise_bet_graph_button ( );

        self->name = g_strdup ( _("Line") );
        self->filename = g_strdup ( "graph-line.png" );
        self->service_id = g_strdup ( "GogLinePlot" );
        self->callback = callback;
        self->box = box_button;
        self->tree_view = tree_view;
        self->is_visible = FALSE;

        liste = g_list_append ( liste, self );
    }

    /* on attache la liste des boutons servira  plus tard */
    g_object_set_data ( G_OBJECT ( box ), "button_list", liste );

    /* bouton qui ouvre le menu de choix du bouton actif */
    arrow_button = GTK_WIDGET ( gtk_builder_get_object ( bet_graph_builder, "arrow_button" ) );
    g_signal_connect ( G_OBJECT ( arrow_button ),
                        "button-press-event",
			            G_CALLBACK ( bet_graph_popup_choix_graph_menu ),
                        liste );

    return box;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_graph_popup_choix_graph_menu ( GtkWidget *button,
                        GdkEventButton *event,
                        GList *liste )
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    GtkWidget *icon;
    GList *tmp_list;

    menu = gtk_menu_new ();

    tmp_list = liste;
    
    while (tmp_list)
    {
        struct_bet_graph_button *self;

        self = tmp_list -> data;

        icon = gtk_image_new_from_file ( g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), self->filename, NULL ) );
        menu_item = gtk_image_menu_item_new_with_label  ( self->name );
        gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), icon );
        g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
				        G_CALLBACK ( bet_graph_popup_choix_graph_activate ),
                        self );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        /* on rend insensible le bouton visible */
        gtk_widget_set_sensitive ( menu_item, !self->is_visible );

        tmp_list = tmp_list -> next;
    }

    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU ( menu ),
                        NULL, button,
                        set_popup_position,
                        button,
                        1,
		                gtk_get_current_event_time ( ) );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
void bet_graph_popup_choix_graph_activate ( GtkMenuItem *menuitem,
                        struct_bet_graph_button *self )
{
    GtkWidget *parent;
    GList *tmp_list;

    parent = gtk_widget_get_parent ( self->box );
    tmp_list = g_object_get_data ( G_OBJECT ( parent ), "button_list" );
    
    while ( tmp_list )
    {
        struct_bet_graph_button *self;

        self = tmp_list -> data;

        if ( self->is_visible == TRUE )
        {
            self->is_visible = FALSE;
            break;
        }

        tmp_list = tmp_list -> next;
    }

    gtk_container_foreach ( GTK_CONTAINER ( self->box ), ( GtkCallback ) gtk_widget_destroy, NULL );

    self->is_visible = TRUE;
    self->button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
                        self->name,
                        self->filename,
                        NULL,
                        NULL );

    g_object_set_data ( G_OBJECT ( self->button ), "service_id", self->service_id );
    g_signal_connect ( G_OBJECT ( self->button ),
                        "clicked",
                        self->callback,
                        self->tree_view );

    gtk_box_pack_start ( GTK_BOX ( self->box ), self->button, TRUE, TRUE, 0 );
}


/**
 *
 *
 *
 *
 * */
struct_bet_graph_button *struct_initialise_bet_graph_button ( void )
{
    struct_bet_graph_button *self;

    self = g_new0 ( struct_bet_graph_button, 1 );

   return self;
}


void struct_free_bet_graph_button ( struct_bet_graph_button *self )
{
    g_free ( self->name );
    g_free ( self->filename );
    g_free ( self->service_id );

    g_free ( self );
}


/**
 *
 *
 *
 *
 * */
struct_bet_graph_data *struct_initialise_bet_graph_data ( void )
{
    struct_bet_graph_data *self;
    gint i;

    self = g_new0 ( struct_bet_graph_data, 1 );

    self -> tab_vue_libelle = g_malloc ( MAX_POINTS_GRAPHIQUE * sizeof ( gchar* ) );

    for ( i = 0; i < MAX_POINTS_GRAPHIQUE; i++ )
    {
        self -> tab_vue_libelle[i] = self -> tab_libelle[i];
    }

   return self;
}


void struct_free_bet_graph_data ( struct_bet_graph_data *self )
{
    g_free ( self->title );
    g_free ( self->service_id );
    g_free ( self->tab_vue_libelle );

    g_free ( self );
}


/**
 *
 *
 *
 *
 * */
void bet_graph_free_builder ( void )
{
    g_object_unref ( G_OBJECT ( bet_graph_builder ) );
}


#endif /* HAVE_GOFFICE */

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
