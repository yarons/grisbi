 /* void importer_fichier_qif ( void );
void selection_fichier_qif ( void );
void fichier_choisi_importation_qif ( GtkWidget *fenetre );
void traitement_donnees_brutes ( void );
void exporter_fichier_qif ( void );
void click_compte_export_qif ( GtkWidget *bouton,
			       GtkWidget *entree ); */
 

gboolean recuperation_donnees_qif ( FILE *fichier );
void fichier_choisi_importation_qif ( GtkWidget *fenetre );
void exporter_fichier_qif ( void );
void click_compte_export_qif ( GtkWidget *bouton,
			       GtkWidget *entree );


