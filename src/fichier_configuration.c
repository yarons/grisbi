/* ce fichier se charge de toutes les op�rations relatives � la
   configuration sauvegard�e */

/*     Copyright (C)	2000-2003 C�dric Auger (cedric@grisbi.org) */
/*			2004 Axel Rousseau (axel584@axel584.org) */
/*			2004 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "fichier_configuration.h"



#include "accueil.h"
#include "categories_onglet.h"
#include "comptes_traitements.h"
#include "devises.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "fichiers_io.h"
#include "operations_comptes.h"
#include "operations_liste.h"
#include "patienter.h"
#include "type_operations.h"
#include "utils.h"
#include "utils_files.h"
#include "constants.h"





extern gint decalage_echeance;  
extern GtkWidget *paned_onglet_operations;
extern GtkWidget *paned_onglet_echeancier;
extern GtkWidget *paned_onglet_comptes;
extern GtkWidget *paned_onglet_etats;



/* ***************************************************************************************************** */
void charge_configuration ( void )
{
    xmlDocPtr doc;
    xmlNodePtr node, root;
    struct stat buffer_stat;

    raz_configuration ();

    if ( utf8_stat ( g_strconcat ( my_get_grisbirc_dir(), C_GRISBIRC, NULL ),&buffer_stat ) == -1 )
    {
#ifndef _WIN32 /* No old configuration under Windows */
	if (  utf8_stat ( g_strconcat ( my_get_grisbirc_dir(), "/.gnome/Grisbi", NULL ),&buffer_stat ) != -1 )
	    charge_configuration_ancien();
#endif
	return;
    }

    doc = utf8_xmlParseFile ( g_strconcat ( my_get_grisbirc_dir(), C_GRISBIRC, NULL ) );

    /* v�rifications d'usage */
    root = xmlDocGetRootElement(doc);

    if ( !root
	 ||
	 !root->name
	 ||
	 g_strcasecmp ( root->name,
			"Configuration" ))
    {
	xmlFreeDoc ( doc );
	return;
    }

    /* On se place sur Generalite*/
    node = root -> children;

    while (node) {
	if ( !strcmp ( node -> name, "Geometry" ) )
	{
	    xmlNodePtr node_geometry;
	    node_geometry = node -> children;
	    while (node_geometry) {
		if ( !strcmp ( node_geometry -> name, "Width" ) ) {
		    largeur_window = my_atoi(xmlNodeGetContent ( node_geometry));
		}
		if ( !strcmp ( node_geometry -> name, "Height" ) ) {
		    hauteur_window = my_atoi(xmlNodeGetContent ( node_geometry));
		}
		node_geometry = node_geometry->next;
	    }
	}
	if ( !strcmp ( node -> name, "General" ) )
	{
	    xmlNodePtr node_general;
	    node_general = node -> children;
	    while (node_general) {
		if ( !strcmp ( node_general -> name, "Modification_operations_rapprochees" ) ) {
		    etat.r_modifiable = my_atoi(xmlNodeGetContent ( node_general));
		}
		if ( !strcmp ( node_general -> name, "Dernier_chemin_de_travail" ) ) {
		    dernier_chemin_de_travail = xmlNodeGetContent ( node_general);
		    if ( !dernier_chemin_de_travail )
			dernier_chemin_de_travail = g_strconcat ( my_get_gsb_file_default_dir(), C_DIRECTORY_SEPARATOR,NULL );
		}
		if ( !strcmp ( node_general -> name, "Affichage_alerte_permission" ) ) {
		    etat.alerte_permission = my_atoi(xmlNodeGetContent ( node_general));
		}
		if ( !strcmp ( node_general -> name, "Force_enregistrement" ) ) {
		    etat.force_enregistrement = my_atoi(xmlNodeGetContent ( node_general));
		}
		if ( !strcmp ( node_general -> name, "Fonction_touche_entree" ) ) {
		    etat.entree = my_atoi(xmlNodeGetContent ( node_general));
		}
		if ( !strcmp ( node_general -> name, "Affichage_messages_alertes" ) ) {
		    etat.alerte_mini = my_atoi(xmlNodeGetContent ( node_general));
		}

		if ( !strcmp ( node_general -> name, "Utilise_fonte_des_listes" ) ) {
		    etat.utilise_fonte_listes = my_atoi (xmlNodeGetContent ( node_general));
		}

		if ( !strcmp ( node_general -> name, "Fonte_des_listes" ) ) {
		    fonte_liste = latin2utf8 (xmlNodeGetContent ( node_general));
		}

		if ( !strcmp ( node_general -> name, "Navigateur_web" ) ) {
		    etat.browser_command = my_strdelimit ( xmlNodeGetContent ( node_general),
							  "\\e",
							  "&" );
		}

		if ( !strcmp ( node_general -> name, "Latex_command" ) ) {
		    etat.latex_command = xmlNodeGetContent ( node_general);
		}
		if ( !strcmp ( node_general -> name, "Dvips_command" ) ) {
		    etat.dvips_command = xmlNodeGetContent ( node_general);
		}

		if ( !strcmp ( node_general -> name, "Animation_attente" ) ) {
		    etat.fichier_animation_attente = xmlNodeGetContent ( node_general);
		}
		if ( !strcmp ( node_general -> name, "Largeur_colonne_comptes_operation" ) ) {
		    etat.largeur_colonne_comptes_operation = my_atoi(xmlNodeGetContent ( node_general));
		}
		if ( !strcmp ( node_general -> name, "Largeur_colonne_echeancier" ) ) {
		    etat.largeur_colonne_echeancier = my_atoi(xmlNodeGetContent ( node_general));
		}
		if ( !strcmp ( node_general -> name, "Largeur_colonne_comptes_comptes" ) ) {
		    etat.largeur_colonne_comptes_comptes = my_atoi(xmlNodeGetContent ( node_general));
		}
		if ( !strcmp ( node_general -> name, "Largeur_colonne_etats" ) ) {
		    etat.largeur_colonne_etat = my_atoi(xmlNodeGetContent ( node_general));
		}


		node_general = node_general->next;
	    }
	}
	if ( fonte_liste && !strlen( fonte_liste ) ) {
	    fonte_liste = NULL;
	}

	if ( !strcmp ( node -> name, "IO" ) )
	{
	    xmlNodePtr node_io;
	    node_io = node -> children;
	    while (node_io) {
		if ( !strcmp ( node_io -> name, "Chargement_auto_dernier_fichier" ) ) {
		    etat.dernier_fichier_auto = my_atoi(xmlNodeGetContent ( node_io));
		}
		if ( !strcmp ( node_io -> name, "Nom_dernier_fichier" ) ) {
		    nom_fichier_comptes = xmlNodeGetContent ( node_io);
		}
		if ( !strcmp ( node_io -> name, "Enregistrement_automatique" ) ) {
		    etat.sauvegarde_auto = my_atoi(xmlNodeGetContent ( node_io));
		}
		if ( !strcmp ( node_io -> name, "Enregistrement_au_demarrage" ) ) {
		    etat.sauvegarde_demarrage = my_atoi(xmlNodeGetContent ( node_io));
		}
		if ( !strcmp ( node_io -> name, "Nb_max_derniers_fichiers_ouverts" ) ) {
		    nb_max_derniers_fichiers_ouverts = my_atoi(xmlNodeGetContent ( node_io));
		}
		if ( !strcmp ( node_io -> name, "Compression_fichier" ) ) {
		    compression_fichier = my_atoi(xmlNodeGetContent ( node_io));
		    xmlSetCompressMode ( compression_fichier );
		}
		if ( !strcmp ( node_io -> name, "Compression_backup" ) ) {
		    compression_backup = my_atoi(xmlNodeGetContent ( node_io));
		}
		if ( !strcmp ( node_io->name, "Liste_noms_derniers_fichiers_ouverts" ) ) {
		    xmlNodePtr node_filename = node_io -> children;
		    nb_derniers_fichiers_ouverts = 0;
		    tab_noms_derniers_fichiers_ouverts = malloc ( nb_max_derniers_fichiers_ouverts * sizeof(gchar *) );
		    while ( node_filename ) {
			if ( !strcmp ( node_filename -> name, "fichier" ) ) {
			    tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts] = xmlNodeGetContent ( node_filename );
			    nb_derniers_fichiers_ouverts++;
			}
			node_filename = node_filename->next;
		    }
		}

		node_io = node_io->next;
	    }
	}
	if ( !strcmp ( node -> name, "Echeances" ) )
	{
	    xmlNodePtr node_echeances;
	    node_echeances = node -> children;
	    while (node_echeances) {
		if ( !strcmp ( node_echeances -> name, "Delai_rappel_echeances" ) ) {
		    decalage_echeance = my_atoi(xmlNodeGetContent ( node_echeances));
		}
		node_echeances = node_echeances->next;
	    }
	}
	/*if ( !strcmp ( node -> name, "Applet" ) )
	  {
	  xmlNodePtr node_io;
	  node_io = node -> children;
	  while (node_io) {
	  if ( !strcmp ( node_io -> name, "Chargement_auto_dernier_fichier" ) ) {
	  etat.dernier_fichier_auto = my_atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Nom_dernier_fichier" ) ) {
	  nom_fichier_comptes = xmlNodeGetContent ( node_io);
	  }
	  if ( !strcmp ( node_io -> name, "Enregistrement_automatique" ) ) {
	  etat.sauvegarde_auto = my_atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Enregistrement_au_demarrage" ) ) {
	  etat.sauvegarde_demarrage = my_atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Nb_max_derniers_fichiers_ouverts" ) ) {
	  nb_max_derniers_fichiers_ouverts = my_atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Compression_fichier" ) ) {
	  compression_fichier = my_atoi(xmlNodeGetContent ( node_io));
	  }
	  if ( !strcmp ( node_io -> name, "Compression_backup" ) ) {
	  variable = xmlNodeGetContent ( node_io);
	  }
	// boucler pour avoir la liste des derniers fichiers.
	node_io = node_io->next;
	}
	}*/
	if ( !strcmp ( node -> name, "Affichage" ) )
	{
	    xmlNodePtr node_affichage;
	    node_affichage = node -> children;
	    while (node_affichage) {
		if ( !strcmp ( node_affichage -> name, "Affichage_formulaire" ) ) {
		    etat.formulaire_toujours_affiche = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Affichage_formulaire_echeancier" ) ) {
		    etat.formulaire_echeancier_toujours_affiche = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Affichage_tous_types" ) ) {
		    etat.affiche_tous_les_types = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Affiche_no_operation" ) ) {
		    etat.affiche_no_operation = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Affiche_date_bancaire" ) ) {
		    etat.affiche_date_bancaire = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Tri_par_date" ) ) {
		    etat.classement_par_date = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Regrouper_rp" ) ) {
		    etat.classement_rp = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Affiche_boutons_valider_annuler" ) ) {
		    etat.affiche_boutons_valider_annuler = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Largeur_auto_colonnes" ) ) {
		    etat.largeur_auto_colonnes = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Caracteristiques_par_compte" ) ) {
		    etat.retient_affichage_par_compte = my_atoi( xmlNodeGetContent ( node_affichage));
		}
		// boucler pour avoir les tailles des diff�rentes colonnes
		if ( !strcmp ( node_affichage -> name, "taille_largeur_colonne" ) ) {
		    //int numero_colonne;
		    //int largeur_colonne;
		    int numero_colonne = my_atoi(xmlGetProp ( node_affichage, "No"));
		    int largeur_colonne = my_atoi(xmlNodeGetContent ( node_affichage));
		    taille_largeur_colonnes[numero_colonne] = largeur_colonne;
		}
		if ( !strcmp ( node_affichage -> name, "Affichage_exercice_automatique" ) ) {
		    etat.affichage_exercice_automatique = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		if ( !strcmp ( node_affichage -> name, "Affichage_nb_ecritures" ) ) {
		    etat.affiche_nb_ecritures_listes = my_atoi(xmlNodeGetContent ( node_affichage));
		}
		node_affichage = node_affichage->next;
	    }
	}
	if ( !strcmp ( node -> name, "Exercice" ) )
	{
	    xmlNodePtr node_exercice;
	    node_exercice = node -> children;
	    while (node_exercice) {
		node_exercice = node_exercice->next;
	    }
	}
	if ( !strcmp ( node -> name, "Messages" ) )
	{
	    xmlNodePtr node_messages;
	    node_messages = node -> children;
	    while (node_messages) {
		if ( !strcmp ( node_messages -> name, "display_message_lock_active" ) ) {
		    etat.display_message_lock_active = my_atoi(xmlNodeGetContent ( node_messages));
		}
    /* On Windows, the chmod feature does not work: FAT does not have right access permission notions , 
     * on NTFS it to complicated to implement => the feature is removed from the Windows version :
     * for that the corresponding parameter check box is not displayed and the paramater is forced to not display msg */
#ifndef _WIN32
		if ( !strcmp ( node_messages -> name, "display_message_file_readable" ) ) {
		    etat.display_message_file_readable = my_atoi(xmlNodeGetContent ( node_messages));
		}
#else
                    etat.display_message_file_readable = 1;
#endif
		if ( !strcmp ( node_messages -> name, "display_message_minimum_alert" ) ) {
		    etat.display_message_minimum_alert = my_atoi(xmlNodeGetContent ( node_messages));
		}
		node_messages = node_messages->next;
	    }
	}
	if ( !strcmp ( node -> name, "Print_config" ) )
	{
	    xmlNodePtr node_print;

	    node_print = node -> children;
	    while (node_print) {
		if ( !strcmp ( node_print -> name, "printer" ) ) {
		    etat.print_config.printer = my_atoi(xmlNodeGetContent (node_print));
		}
		if ( !strcmp ( node_print -> name, "printer_name" ) ) {
		    etat.print_config.printer_name = xmlNodeGetContent (node_print);
		}
		if ( !strcmp ( node_print -> name, "printer_filename" ) ) {
		    etat.print_config.printer_filename = xmlNodeGetContent (node_print);
		}
		if ( !strcmp ( node_print -> name, "filetype" ) ) {
		    etat.print_config.filetype = my_atoi(xmlNodeGetContent (node_print));
		}
		if ( !strcmp ( node_print -> name, "orientation" ) ) {
		    etat.print_config.orientation = my_atoi(xmlNodeGetContent (node_print));
		}
		if ( !strcmp ( node_print -> name, "paper_config" ) ) {
		    etat.print_config.paper_config.width = my_atoi(xmlGetProp (node_print, "width"));
		    etat.print_config.paper_config.height = my_atoi(xmlGetProp (node_print, "height"));
		    etat.print_config.paper_config.name = xmlGetProp (node_print, "name");
		}

		node_print = node_print->next;
	    }
	}

	node = node -> next;
    }
}
/* ***************************************************************************************************** */


/******************************************************************************************************************/
/* Fonction charge_configuration */
/* appel�e � l'ouverture de grisbi, charge les pr�f�rences */
/******************************************************************************************************************/

void charge_configuration_ancien ( void )
{
    FILE *fichier;
    gchar *fichier_conf;
    gchar temp[100];

    etat.fichier_animation_attente = g_strdup ( ANIM_PATH );

    /* modif -> vire gnome, donc fait tout � la main */

    fichier_conf = g_strconcat ( my_get_grisbirc_dir(),
				 "/.gnome/Grisbi",
				 NULL );

    fichier = utf8_fopen ( fichier_conf,
		      "ro" );
    if ( !fichier )
    {
	dialogue_error_hint ( g_strconcat ( latin2utf8 ( strerror ( errno )),
					    "\n",
					    _("File : "),
					    fichier_conf,
					    NULL ),
			      _("Error opening config"));
	raz_configuration ();
	return;
    }

    fonte_liste = NULL;

    /*     on lit ligne par ligne... */

    while ( fgets ( temp,
		    100,
		    fichier ))
    {
	sscanf ( temp,
		 "Width=%d",
		 &largeur_window );
	sscanf ( temp,
		 "Height=%d",
		 &hauteur_window );
	sscanf ( temp,
		 "Modification_operations_rapprochees=%d",
		 &etat.r_modifiable );
	sscanf ( temp,
		 "Dernier_chemin_de_travail=%as",
		 &dernier_chemin_de_travail );
	sscanf ( temp,
		 "Fonction_touche_entree=%d",
		 &etat.entree );
	sscanf ( temp,
		 "Fonte_des_listes=%as",
		 &fonte_liste );
	sscanf ( temp,
		 "Force_enregistrement=%d",
		 &etat.force_enregistrement );
	sscanf ( temp,
		 "Chargement_auto_dernier_fichier=%d",
		 &etat.dernier_fichier_auto);
	sscanf ( temp,
		 "Nom_dernier_fichier=%as",
		 &nom_fichier_comptes );
	sscanf ( temp,
		 "Enregistrement_automatique=%d",
		 &etat.sauvegarde_auto );
	sscanf ( temp,
		 "Enregistrement_au_demarrage=%d",
		 &etat.sauvegarde_demarrage );
	sscanf ( temp,
		 "Nb_max_derniers_fichiers_ouverts=%d",
		 &nb_max_derniers_fichiers_ouverts );
	sscanf ( temp,
		 "Compression_fichier=%d",
		 &compression_fichier );
	sscanf ( temp,
		 "Compression_backup=%d",
		 &compression_backup  );
	sscanf ( temp,
		 "Delai_rappel_echeances=%d",
		 &decalage_echeance );
	sscanf ( temp,
		 "Affichage_formulaire=%d",
		 &etat.formulaire_toujours_affiche );
	sscanf ( temp,
		 "Affichage_formulaire_echeancier=%d",
		 &etat.formulaire_echeancier_toujours_affiche  );
	sscanf ( temp,
		 "Affichage_tous_types=%d",
		 &etat.affiche_tous_les_types );
	sscanf ( temp,
		 "Affiche_no_operation=%d",
		 &etat.affiche_no_operation );
	sscanf ( temp,
		 "Affiche_date_bancaire=%d",
		 &etat.affiche_date_bancaire );
	sscanf ( temp,
		 "Tri_par_date=%d",
		 &etat.classement_par_date );
	sscanf ( temp,
		 "Affiche_boutons_valider_annuler=%d",
		 & etat.affiche_boutons_valider_annuler);
	sscanf ( temp,
		 "Largeur_auto_colonnes=%d",
		 &etat.largeur_auto_colonnes );
	sscanf ( temp,
		 "Caracteristiques_par_compte=%d",
		 &etat.retient_affichage_par_compte );
	sscanf ( temp,
		 "Affichage_exercice_automatique=%d",
		 &etat.affichage_exercice_automatique );
	sscanf ( temp,
		 "Affichage_nb_ecritures=%d",
		 &etat.affiche_nb_ecritures_listes );
	sscanf ( temp,
		 "taille_largeur_colonne0=%d",
		 &taille_largeur_colonnes[0] );
	sscanf ( temp,
		 "taille_largeur_colonne1=%d",
		 &taille_largeur_colonnes[1] );
	sscanf ( temp,
		 "taille_largeur_colonne2=%d",
		 &taille_largeur_colonnes[2] );
	sscanf ( temp,
		 "taille_largeur_colonne3=%d",
		 &taille_largeur_colonnes[3] );
	sscanf ( temp,
		 "taille_largeur_colonne4=%d",
		 &taille_largeur_colonnes[4] );
	sscanf ( temp,
		 "taille_largeur_colonne5=%d",
		 &taille_largeur_colonnes[5] );
	sscanf ( temp,
		 "taille_largeur_colonne6=%d",
		 &taille_largeur_colonnes[6] );
	sscanf ( temp,
		 "display_message_lock_active=%d",
		 &etat.display_message_lock_active );
	sscanf ( temp,
		 "display_message_file_readable=%d",
		 &etat.display_message_file_readable );
	sscanf ( temp,
		 "display_message_minimum_alert=%d",
		 &etat.display_message_minimum_alert );
    }

    if ( !dernier_chemin_de_travail )
	dernier_chemin_de_travail = g_strconcat ( my_get_gsb_file_default_dir(),
						  C_DIRECTORY_SEPARATOR,
						  NULL );

    if ( fonte_liste && !strlen( fonte_liste ) )
	fonte_liste = NULL;

    xmlSetCompressMode ( compression_fichier );

    tab_noms_derniers_fichiers_ouverts = NULL;
    nb_derniers_fichiers_ouverts = 0;

    /* on transforme les chaines en utf8 */

    dernier_chemin_de_travail = latin2utf8 ( dernier_chemin_de_travail );
    fonte_liste= latin2utf8 ( fonte_liste );
    nom_fichier_comptes = latin2utf8 ( nom_fichier_comptes );
}
/*************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction RAZ_configuration */
/* ***************************************************************************************************** */

void raz_configuration ( void )
{

    largeur_window = 0;
    hauteur_window = 0;

    etat.r_modifiable = 0;       /* on ne peux modifier les op� relev�es */
    etat.dernier_fichier_auto = 1;   /*  on n'ouvre pas directement le dernier fichier */
    buffer_dernier_fichier = g_strdup ( "" );
    etat.sauvegarde_auto = 0;    /* on ne sauvegarde pas automatiquement */
    etat.entree = 1;    /* la touche entree provoque l'enregistrement de l'op�ration */
    decalage_echeance = 3;     /* nb de jours avant l'�ch�ance pour pr�venir */
    etat.formulaire_toujours_affiche = 0;       /* le formulaire ne s'affiche que lors de l'edition d'1 op� */
    etat.formulaire_echeancier_toujours_affiche = 0;       /* le formulaire ne s'affiche que lors de l'edition d'1 op� */
    etat.affichage_exercice_automatique = 1;        /* l'exercice est choisi en fonction de la date */
    fonte_liste = NULL;
    etat.force_enregistrement = 0;     /* par d�faut, on ne force pas l'enregistrement */
    etat.affiche_tous_les_types = 0;   /* par d�faut, on n'affiche ds le formulaire que les types du d�bit ou cr�dit */
    etat.classement_par_date = 1;  /* par d�faut, on tri la liste des op�s par les dates */
    etat.affiche_boutons_valider_annuler = 1;
    etat.classement_par_date = 1;
    dernier_chemin_de_travail = g_strconcat ( my_get_gsb_file_default_dir(),
					      C_DIRECTORY_SEPARATOR,
					      NULL );
    nb_derniers_fichiers_ouverts = 0;
    nb_max_derniers_fichiers_ouverts = 3;
    tab_noms_derniers_fichiers_ouverts = NULL;
    compression_fichier = 0;     /* pas de compression par d�faut */
    compression_backup = 0;
    etat.largeur_auto_colonnes = 1;
    etat.retient_affichage_par_compte = 0;
    etat.fichier_animation_attente = g_strdup ( ANIM_PATH );

	/* Messages */
	etat.display_message_lock_active = 0;
    /* On Windows, the chmod feature does not work: FAT does not have right access permission notions , 
     * on NTFS it to complicated to implement => the feature is removed from the Windows version :
     * for that the corresponding parameter check box is not displayed and the paramater is forced to not display msg */
#ifndef _WIN32        
    etat.display_message_file_readable = 0;
#else
    etat.display_message_file_readable = 1;
#endif
    etat.display_message_minimum_alert = 0;

    /* Commands */
    etat.latex_command = "latex";
    etat.dvips_command = "dvips";
    etat.browser_command = ETAT_WWW_BROWSER;

    /* Print */
    etat.print_config.printer = 0;
#ifndef _WIN32
    etat.print_config.printer_name = "lpr";
#else
    etat.print_config.printer_name = "gsprint";
#endif
    etat.print_config.printer_filename = "";
    etat.print_config.filetype = POSTSCRIPT_FILE;
    etat.print_config.paper_config.name = _("A4");
    etat.print_config.paper_config.width = 21;
    etat.print_config.paper_config.height = 29.7;
    etat.print_config.orientation = LANDSCAPE;
}
/* ***************************************************************************************************** */

/* ***************************************************************************************************** */
/* Fonction sauve_configurationXML */
/* Appel�e � chaque changement de configuration */
/* ***************************************************************************************************** */

void sauve_configuration(void)
{
    gint i;
    // document XML (voir fichiers_io.c l.4000)
    xmlDocPtr doc;
    xmlNodePtr node;
    xmlNodePtr node_1;
    xmlNodePtr node_2;
    // resultat de la sauvegarde
    gint resultat;


    /* creation de l'arbre xml en memoire */

    doc = xmlNewDoc("1.0");

    /* la racine est "configuration" */

    doc->children = xmlNewDocNode ( doc,NULL, "Configuration",NULL );

    /*   sauvegarde de la g�om�trie */
    if ( GTK_WIDGET ( window) -> window ) 
    {
	gtk_window_get_size (GTK_WINDOW ( window ),
			     &largeur_window,&hauteur_window); // gtk2 ???
    } 
    else 
    {
	largeur_window = 0;
	hauteur_window = 0;
    }

    node = xmlNewChild ( doc->children,NULL, "Geometry",NULL );
    xmlNewChild ( node,NULL, "Width",
		  itoa(largeur_window));
    xmlNewChild ( node,NULL, "Height",
		  itoa(hauteur_window));

    /* sauvegarde de l'onglet g�n�ral */
    node = xmlNewChild ( doc->children,NULL, "General",NULL );
    xmlNewChild ( node,NULL, "Modification_operations_rapprochees",
		  itoa(etat.r_modifiable));
    xmlNewChild ( node,NULL, "Dernier_chemin_de_travail",dernier_chemin_de_travail);
    xmlNewChild ( node,NULL, "Affichage_alerte_permission",
		  itoa(etat.alerte_permission));
    xmlNewChild ( node,NULL, "Force_enregistrement",
		  itoa(etat.force_enregistrement));
    xmlNewChild ( node,NULL, "Fonction_touche_entree",
		  itoa(etat.entree));
    xmlNewChild ( node,NULL, "Affichage_messages_alertes",
		  itoa(etat.alerte_mini));
    xmlNewChild ( node,NULL, "Utilise_fonte_des_listes",itoa (etat.utilise_fonte_listes));

    xmlNewChild ( node,NULL, "Fonte_des_listes",fonte_liste);
    xmlNewChild ( node,NULL, "Animation_attente",etat.fichier_animation_attente);

/*     on modifie la chaine si �a contient &, il semblerait que la libxml n'appr�cie pas... */
    
    xmlNewChild ( node,NULL, "Navigateur_web",my_strdelimit ( etat.browser_command,
							      "&",
							      "\\e" ));

/*     on ne fait la sauvegarde que si les colonnes existent (compte non ferm�) */
	
    if ( nb_comptes )
    {
	xmlNewChild ( node,NULL, "Largeur_colonne_comptes_operation",
		      itoa(gtk_paned_get_position (GTK_PANED (paned_onglet_operations))));
	xmlNewChild ( node,NULL, "Largeur_colonne_echeancier",
		      itoa(gtk_paned_get_position (GTK_PANED (paned_onglet_echeancier))));
	xmlNewChild ( node,NULL, "Largeur_colonne_comptes_comptes",
		      itoa(gtk_paned_get_position (GTK_PANED (paned_onglet_comptes))));
	xmlNewChild ( node,NULL, "Largeur_colonne_etats",
		      itoa(gtk_paned_get_position (GTK_PANED (paned_onglet_etats))));
    }

    /* sauvegarde de l'onglet I/O */
    node = xmlNewChild ( doc->children,NULL, "IO",NULL );
    xmlNewChild ( node,NULL, "Chargement_auto_dernier_fichier",
		  itoa(etat.dernier_fichier_auto));
    xmlNewChild ( node,NULL, "Nom_dernier_fichier",nom_fichier_comptes);
    xmlNewChild ( node,NULL, "Enregistrement_automatique",
		  itoa(etat.sauvegarde_auto));
    xmlNewChild ( node,NULL, "Enregistrement_au_demarrage",
		  itoa(etat.sauvegarde_demarrage));
    xmlNewChild ( node,NULL, "Nb_max_derniers_fichiers_ouverts",
		  itoa(nb_max_derniers_fichiers_ouverts));
    xmlNewChild ( node,NULL, "Compression_fichier",
		  itoa(compression_fichier));
    xmlNewChild ( node,NULL, "Compression_backup",
		  itoa(compression_backup));
    node_1 = xmlNewChild ( node,NULL, "Liste_noms_derniers_fichiers_ouverts",NULL);
    for (i=0;i<nb_derniers_fichiers_ouverts;i++) {
	// ajout des noeuds de la forme fichier1,fichier2,fichier3...
	//sprintf(buff, "fichier%i",i);
	node_2 = xmlNewChild ( node_1,NULL, "fichier",tab_noms_derniers_fichiers_ouverts[i]);
	xmlSetProp ( node_2, "No", itoa (i));
    }

    /* sauvegarde de l'onglet �ch�ances */
    node = xmlNewChild ( doc->children,NULL, "Echeances",NULL );
    xmlNewChild ( node,NULL, "Delai_rappel_echeances",
		  itoa(decalage_echeance));

    /* sauvegarde de l'onglet affichage */
    node = xmlNewChild ( doc->children,NULL, "Affichage",NULL );
    xmlNewChild ( node,NULL, "Affichage_formulaire",
		  itoa(etat.formulaire_toujours_affiche));
    xmlNewChild ( node,NULL, "Affichage_formulaire_echeancier",
		  itoa(etat.formulaire_echeancier_toujours_affiche));
    xmlNewChild ( node,NULL, "Affichage_tous_types",
		  itoa(etat.affiche_tous_les_types));
    xmlNewChild ( node,NULL, "Affiche_no_operation",
		  itoa(etat.affiche_no_operation));
    xmlNewChild ( node,NULL, "Affiche_date_bancaire",
		  itoa(etat.affiche_date_bancaire));
    xmlNewChild ( node,NULL, "Tri_par_date",
		  itoa(etat.classement_par_date));
    xmlNewChild ( node,NULL, "Regrouper_rp",
		  itoa(etat.classement_rp));
    xmlNewChild ( node,NULL, "Affiche_boutons_valider_annuler",
		  itoa(etat.affiche_boutons_valider_annuler));
    xmlNewChild ( node,NULL, "Largeur_auto_colonnes",
		  itoa(etat.largeur_auto_colonnes));
    xmlNewChild ( node,NULL, "Caracteristiques_par_compte",
		  itoa(etat.retient_affichage_par_compte));
    for ( i=0 ; i<7 ; i++ ) {
	node_2 = xmlNewChild ( node,NULL, "taille_largeur_colonne",
			       itoa(taille_largeur_colonnes[i]));
	xmlSetProp ( node_2, "No", itoa (i));
    }
    xmlNewChild ( node,NULL, "Affichage_nb_ecritures",
		  itoa(etat.affichage_exercice_automatique));
    xmlNewChild ( node,NULL, "Affichage_exercice_automatique",
		  itoa(etat.affichage_exercice_automatique));

    /*   sauvegarde de l'onglet d'exercice */
    node = xmlNewChild ( doc->children,NULL, "Exercice",NULL );

    /* sauvegarde des messages */
    node = xmlNewChild ( doc->children,NULL, "Messages",NULL );
    xmlNewChild ( node,NULL, "display_message_lock_active",
		  itoa(etat.display_message_lock_active));
    xmlNewChild ( node,NULL, "display_message_file_readable",
		  itoa(etat.display_message_file_readable));
    xmlNewChild ( node,NULL, "display_message_minimum_alert",
		  itoa(etat.display_message_minimum_alert));

    /* sauvegarde des messages */
    node = xmlNewChild ( doc->children, NULL, "Print_config", NULL );
    xmlNewChild ( node, NULL, "printer", itoa(etat.print_config.printer));
    xmlNewChild ( node, NULL, "printer_name", etat.print_config.printer_name);
    xmlNewChild ( node, NULL, "printer_filename", etat.print_config.printer_filename);
    xmlNewChild ( node, NULL, "filetype", itoa(etat.print_config.filetype));
    xmlNewChild ( node, NULL, "orientation", itoa(etat.print_config.orientation));
    node = xmlNewChild ( node, NULL, "paper_config", NULL);
    xmlSetProp ( node, "name", etat.print_config.paper_config.name);
    xmlSetProp ( node, "width", itoa(etat.print_config.paper_config.width));
    xmlSetProp ( node, "height", itoa(etat.print_config.paper_config.height));


    /* Enregistre dans le ~/.grisbirc */
    resultat = xmlSaveFormatFile ( g_strconcat ( my_get_grisbirc_dir(), C_GRISBIRC,
						 NULL), doc, 1 );

    /* on lib�re la memoire */
    xmlFreeDoc ( doc );
    if ( resultat == -1 ) 
    {
	dialogue_error ( g_strdup_printf ( _("Error saving file '%s': %s"), 
					   g_strconcat ( my_get_grisbirc_dir(), C_GRISBIRC, NULL), 
					   latin2utf8(strerror(errno)) ));
    }
}
/* ***************************************************************************************************** */



