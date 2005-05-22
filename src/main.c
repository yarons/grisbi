/* *******************************************************************************/
/*                                 GRISBI                                        */
/* Programme de gestion financière personnelle                                   */
/*           	  license : GPL                                                  */
/*                                                                               */
/*     Copyright (C)    2000-2003 Cédric Auger (cedric@grisbi.org)               */
/*                      2003 Benjamin Drieu (bdrieu@april.org)                   */
/*                      http://www.grisbi.org                                    */
/*      Version : 0.5.5                                                           */
/*                                                                               */
/* *******************************************************************************/

/* *******************************************************************************/
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/



/* Fichier de base contenant la procédure main */


#include "include.h"
#include "structures.h"
#include "variables.c"
#include "main.h"


#include "erreur.h"
#include "etats_calculs.h"
#include "etats_config.h"
#include "etats_onglet.h"
#include "fichier_configuration.h"
#include "fichiers_gestion.h"
#include "menu.h"
#include "parametres.h"
#include "traitement_variables.h"
#include "utils.h"

#ifdef _WIN32
// Special include for Windows only
#include "win32utils.h"
#include "win32gtkutils.h"
#endif




/* vbox ajoutée dans la fenetre de base, contient le menu et la fenetre d'utilisation */

GtkWidget *window_vbox_principale;
GtkWidget *menu_general;
GtkItemFactory *item_factory_menu_general;


/***********************************************************************************************
 ** Début de la procédure principale **
 ***********************************************************************************************/

/**                                                                              
 * Main function
 *
 * @param argc number of arguments
 * @param argv arguments
 *
 * @return Nothing
 */
int main (int argc, char *argv[])
{
#ifndef _WIN32
    struct sigaction sig_sev;
#endif
    gint demande_page;

    /* on ajoute la possibilité de mettre l'option --onglet dans la ligne de commande */
    /* Permet d'ouvrir le fichier demandé sur l'onglet désiré  */
    /* w=-1 : fenêtre de configuration */
    /*    x=numéro de l'onglet de configuration (0-9) */
    /* w=0 : onglet d'accueil (identique sans argument) */
    /* w=1 : onglet des opérations */
    /* w=2 : onglet de l'échéancier */
    /* w=3 : onglet des comptes */
    /* w=4 : onglet des tiers */
    /* w=5 : onglet des catégories */
    /* w=6 : onglet des imputations budgétaires */
    /* w=7 : onglet des états */
    /*   x=numéro de l'état à afficher */
    /*   y=absent pour rester sur l'état, numéro de l'onglet principal de personnalisation sinon */
    /*   z=numéro de l'onglet secondaire de la personnalisation */

    /*   Exemples : */
    /* grisbi --onglet=3 mes_comptes */
    /* place grisbi directement sur l'échéancier du fichier mes_comptes */
    /* grisbi --onglet=-1,3 mes_comptes */
    /* affiche la configuration de grisbi et la place sur Affichage */
    /* grisbi --onglet=7,2 mes_comptes */
    /* affiche le 3ème état */
    /* grisbi --onglet=7,2,2,2 mes_comptes */
    /* affiche l'onglet opération de l'onglet Affichage des données du 3ème état */

#ifdef _WIN32
    /* we store the path of the running file to use it for pixmaps, help and locales .... */
    win32_set_app_path(argv[0]);
    /* needed to be able to use the "common" installation of GTK libraries */
    win32_make_sure_the_gtk2_dlls_path_is_in_PATH(); 
    /* work aroung LANG defintiion issues */
    win32_set_locale();
#endif

	/*   setlocale (LC_ALL, ""); */
	bindtextdomain (PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);

	/* FIXME : tous les arguments du gnome_init... */
	/* 	gnome_init_with_popt_table ("Grisbi", VERSION, argc, argv, options, 0, &ctx); */

	gtk_init ( &argc, &argv );

	/* on commence par détourner le signal SIGSEGV */
#ifndef _WIN32
    /* sauf sous Windows*/
	memset ( &sig_sev, 0, sizeof ( struct sigaction ));
	sig_sev.sa_handler = traitement_sigsegv;
	sig_sev.sa_flags = 0;
	sigemptyset (&(sig_sev.sa_mask));

	if ( sigaction ( SIGSEGV, &sig_sev, NULL ))
	    printf (_("Error on sigaction: SIGSEGV won't be trapped\n"));
#endif

	/*  Création de la fenêtre principale */

	window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );

	gtk_signal_connect ( GTK_OBJECT (window),
			     "delete_event",
			     GTK_SIGNAL_FUNC ( fermeture_grisbi ),
			     NULL );

	gtk_signal_connect ( GTK_OBJECT ( window ),
			     "size-allocate",
			     GTK_SIGNAL_FUNC ( tente_modif_taille ),
			     NULL );
	gtk_window_set_policy ( GTK_WINDOW ( window ),
				TRUE,
				TRUE,
				FALSE );

	/* 	création de la pixmap du logiciel */

	gtk_window_set_default_icon_from_file ( g_strconcat(PIXMAPS_DIR,
                                                            C_DIRECTORY_SEPARATOR,
							    "euro.gif",
							    NULL),
						NULL );

	window_vbox_principale = gtk_vbox_new ( FALSE,
						5 );
	gtk_container_add ( GTK_CONTAINER ( window ),
			    window_vbox_principale );
	gtk_widget_show ( window_vbox_principale );

	/*   création des menus */

	menu_general = init_menus ( window_vbox_principale );
	gtk_box_pack_start ( GTK_BOX ( window_vbox_principale ),
			     menu_general,
			     FALSE,
			     FALSE,
			     0 );

	/* on grise les fonctions inutiles au départ */

	init_variables ( FALSE );
	style_label_nom_compte = NULL;


	charge_configuration();

	/* on met dans le menu les derniers fichiers ouverts */

	affiche_derniers_fichiers_ouverts ();

	/*   si la taille avait déjà été sauvée, on remet l'ancienne taille à
	     la fenetre */
	if ( largeur_window && hauteur_window )
	    gtk_window_set_default_size ( GTK_WINDOW ( window ),
					  largeur_window,
					  hauteur_window );
	else
	    gtk_window_set_default_size ( GTK_WINDOW ( window ),
					  640, 480 );


	gtk_widget_show ( window );

	/*   on applique la fonte aux listes si nécessaire */

	initialisation_couleurs_listes ();

	if ( etat.utilise_fonte_listes )
	{
	    /* 	on recharge la fonte */

	    GdkFont *font;

	    font = gdk_font_load ( fonte_liste );

	    /* Use font */
	    style_couleur[0] -> font_desc = pango_font_description_from_string(fonte_liste);
	    style_couleur[1] -> font_desc = pango_font_description_from_string(fonte_liste);
	    style_rouge_couleur[0] -> font_desc = pango_font_description_from_string(fonte_liste);
	    style_rouge_couleur[1] -> font_desc = pango_font_description_from_string(fonte_liste);
	    style_gris -> font_desc = pango_font_description_from_string(fonte_liste);
	    gtk_style_set_font (style_couleur [0], NULL);
	    gtk_style_set_font (style_couleur [1], NULL);
	    gtk_style_set_font (style_rouge_couleur [0], NULL);
	    gtk_style_set_font (style_rouge_couleur [1], NULL);
	    gtk_style_set_font (style_gris, NULL);
	}

	/* on vérifie les arguments de ligne de commande */

	demande_page = 0;

	switch ( argc )
	{
	    case 1:
		/* il n'y a aucun argument */

		/* ouvre le dernier fichier s'il existe et si c'est demandé */

		if ( etat.dernier_fichier_auto
		     &&
		     nom_fichier_comptes
		     &&
		     strlen ( nom_fichier_comptes ) )
		    ouverture_confirmee();
		break;

	    case 2:
		/* l'argument peut être soit --onglet, soit le fichier à ouvrir */

		if ( !strncmp ( argv[1],
				"--",
				2 ))
		{
		    demande_page = 1;

		    /* ouvre le dernier fichier s'il existe et si c'est demandé */

		    if ( etat.dernier_fichier_auto
			 &&
			 nom_fichier_comptes
			 &&
			 strlen ( nom_fichier_comptes ) )
			ouverture_confirmee();
		}
		else
		{
		    nom_fichier_comptes = argv[1];
		    ouverture_confirmee();
		}
		break;

	    case 3:
		/* il y a --onglet et un nom de fichier */
		/*       il faut que argv[1] commence par -- sinon on considère que c'est le nom de fichier */
		/* et on oublie le 2ème argument */

		if ( !strncmp ( argv[1],
				"--",
				2 ))
		{
		    demande_page = 1;

		    /* ouvre le fichier demandé */

		    nom_fichier_comptes = argv[2];
		    ouverture_confirmee();
		}
		else
		{
		    nom_fichier_comptes = argv[1];
		    ouverture_confirmee();
		}
		break;
	}

	/*   à ce niveau, le fichier doit être chargé, on met sur l'onglet demandé si nécessaire */

	if ( nb_comptes
	     &&
	     demande_page )
	{
	    gchar **split_argument;

	    split_argument = g_strsplit ( argv[1],
					  "=",
					  2 );

	    /*       si le 2ème argument retourné est null, c'est qu'on avait marqué --onglet= */
	    /* et dans ce cas on fait rien */

	    if ( split_argument[1] )
	    {
		gchar **split_chiffres;

		split_chiffres = g_strsplit ( split_argument[1],
					      ",",
					      0 );

		/* 	  comme split_argument[1] existait, split_chiffres[0] existe forcemment */

		switch ( my_atoi ( split_chiffres[0] ))
		{
		    case -1:
			/* on demande l'onglet de configuration */

			/* on affiche l'onglet du 2ème argument s'il existe */

			if ( split_chiffres[1] )
			    preferences ( my_atoi ( split_chiffres[1] ));
			else
			    preferences ( NOT_A_PAGE );

			break;

		    case 0:
		    case 1:
		    case 2:
		    case 3:
		    case 4:
		    case 5:
		    case 6:

			gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
						my_atoi ( split_chiffres[0] ));
			break;

		    case 7:
			/* on demande l'onglet des états  */

			gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
						my_atoi ( split_chiffres[0] ));

			/* s'il y a un chiffre ensuite, on ouvre l'état correspondant à ce chiffre */

			if ( split_chiffres[1]
			     &&
			     liste_struct_etats )
			{
			    GSList *liste_tmp;

			    liste_tmp = g_slist_nth ( liste_struct_etats,
						      my_atoi ( split_chiffres[1] ));

			    /* si on a sélectionné un état qui n'existait pas, on ouvre le 1er */

			    if ( !liste_tmp )
				liste_tmp = liste_struct_etats;

			    etat_courant = liste_tmp -> data;

			    remplissage_liste_etats ();

			    gtk_widget_set_sensitive ( bouton_personnaliser_etat,
						       TRUE );
			    gtk_widget_set_sensitive ( bouton_exporter_etat,
						       TRUE );
			    gtk_widget_set_sensitive ( bouton_dupliquer_etat,
						       TRUE );
			    gtk_widget_set_sensitive ( bouton_effacer_etat,
						       TRUE );

			    gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
						 etat_courant -> nom_etat );


			    rafraichissement_etat ( etat_courant );

			    /* s'il y a une suite dans la demande en ligne de commande, on ouvre la personnalisation */

			    if ( split_chiffres[2] )
			    {
				personnalisation_etat ();

				/* le 1er chiffre correspond aux 1ers onglets */

				gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_config_etat ),
							my_atoi ( split_chiffres[2] ));

				/* s'il y a encore un chiffre, c'est pour le sous onglet */

				if ( split_chiffres[3] )
				{
				    switch ( my_atoi ( split_chiffres[2] ))
				    {
					case 0:

					    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_selection ),
								    my_atoi ( split_chiffres[3] ));
					    break;
					case 2:
					    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_aff_donnees ),
								    my_atoi ( split_chiffres[3] ));
					    break;
				    }
				}
			    }
			}
			break;
		    }
		g_strfreev( split_chiffres );
	    }
	    g_strfreev( split_argument );
	}


	gtk_main ();

	exit(0);
}
/************************************************************************************************/



/************************************************************************************************/
gint tente_modif_taille ( GtkWidget *win,
			  GtkAllocation *requisition,
			  gpointer null )
{

    if ( requisition -> width < 800 )
	gtk_signal_emit_stop_by_name ( GTK_OBJECT ( window ),
				       "size-allocate" );
    /*     gtk_widget_set_usize ( window, */
    /* 			   850, */
    /* 			   requisition -> height ); */

    return ( TRUE );

}
/************************************************************************************************/
