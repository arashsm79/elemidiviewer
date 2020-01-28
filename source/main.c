#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>
#include <sys/mman.h>
#include <signal.h> 
#include "guiconnection.h"
#include "midiparser.h"

// Make them global
//shared memory
GuiStatus *cacheStatus;
GuiStatus *playStatus;
GuiStatus *filePlayStatus;
int *isMainOpen;



//setting up the main window
GtkBuilder *main_builder;
GtkWidget *main_window;

//main windows widgets 
GtkWidget *main_play;
GtkWidget *main_pause;
GtkWidget *main_fileChooser;
GtkWidget *main_spinner;
GtkWidget *main_progressBar;
GtkWidget *main_eventGrid;


//Menu bar widgets
GtkWidget *menu_new;
GtkWidget *menu_quit;
GtkWidget *menu_about;

//child process that will play the music
pid_t pidFirst;

GtkWidget *main_gridButton[1024];
int	main_gridButtonCount = 0;


void on_destroy();
void sigquit();
void on_event_grid_row_clicked(GtkButton *);

int main(int argc, char *argv[]) {

	gtk_init(&argc, &argv); // init Gtk

//---------------------------------------------------------------------
// establish contact with xml code used to adjust widget settings
//---------------------------------------------------------------------
 
	main_builder = gtk_builder_new_from_file ("main.glade");
 
	main_window = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_window"));

	g_signal_connect(main_window, "destroy", G_CALLBACK(on_destroy), NULL);

    gtk_builder_connect_signals(main_builder, NULL);


	//initializing the widgets using heir IDs
	main_play = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_play"));
	main_pause = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_pause"));
	main_fileChooser = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_fileChooser"));
	main_spinner = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_spinner"));
	main_progressBar = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_progressBar"));
	menu_new = GTK_WIDGET(gtk_builder_get_object(main_builder, "menu_new"));
	menu_quit = GTK_WIDGET(gtk_builder_get_object(main_builder, "menu_quit"));
	menu_about = GTK_WIDGET(gtk_builder_get_object(main_builder, "menu_about"));
	main_eventGrid = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_eventGrid"));


	// setting up shared memories

	cacheStatus = mmap(NULL, sizeof(GuiStatus), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*cacheStatus = STATUS_PROCESSING;

	playStatus = mmap(NULL, sizeof(GuiStatus), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*playStatus = STATUS_PAUSE;

    filePlayStatus = mmap(NULL, sizeof(GuiStatus), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*filePlayStatus = STATUS_PROCESSING;

	isMainOpen = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*isMainOpen = 1;


    //setting up css
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, "button#errbtn { color: red; }", -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                               GTK_STYLE_PROVIDER(css_provider),
                               GTK_STYLE_PROVIDER_PRIORITY_USER);
//-----------------------------------

	gtk_widget_show_all(main_window);

	gtk_main();

	return EXIT_SUCCESS;
	}

//......................................................................................

//on program destroy
void on_destroy()
{
	//set the flag
	//*isMainOpen = 0;
    kill(pidFirst, SIGQUIT);
	gtk_main_quit();
}

//buttons
void on_main_play_clicked(GtkButton *b)
{
    //resume the music process
    *playStatus = STATUS_PLAY;

}

void on_main_pause_clicked(GtkButton *b)
{
    //pause the music process
    *playStatus = STATUS_PAUSE;
}

void on_event_grid_row_clicked(GtkButton *b)
{
	printf("You selected: %s\n", gtk_button_get_label (b));

}

//file chooser
//void on_main_fileChooser_file_set(GtkFileChooserButton *f)
void on_main_fileChooser_clicked(GtkButton *f)
{

    pidFirst = fork();
    if(pidFirst == 0)
    {
        //FIRST CHILD
        signal(SIGQUIT, sigquit);
        *cacheStatus = openMidiFileAndCreateEventCashe("believer.mid");
        *filePlayStatus = playMidiFile(playStatus);
        exit(0);
     
    }else if(pidFirst > 0)
    {
        //parent (main)
        while(1)
        { 
            if(*cacheStatus == STATUS_DONE)
            {

                FILE *f1 = fopen("cachedEvents.txt", "r");
                if (f1 == NULL ) 
                {
                    printf("File cachedEvents.txt not found\n");
                    //EXIT_FAILURE
                    return;
                }

                main_gridButtonCount = 0;
                char rowStr[1024];

                while (!feof(f1)) 
                {
                    fgets(rowStr, 1024, f1);
                    rowStr[strlen(rowStr)-1] = 0; // remove newline byte
                    gtk_grid_insert_row (GTK_GRID(main_eventGrid), main_gridButtonCount);

            //		A button can be freed by the function 'gtk_container_remove ()'
                    main_gridButton[main_gridButtonCount] = gtk_button_new_with_label (rowStr);
                    gtk_button_set_alignment (GTK_BUTTON(main_gridButton[main_gridButtonCount]), 0.0, 0.5); // hor left, ver center
                    
                    //events with error will have a red color
                    if(rowStr[14] != '>' && strstr(rowStr, "ERR") != NULL){
                        gtk_widget_set_name(main_gridButton[main_gridButtonCount], "errbtn");
                    }
                    gtk_grid_attach (GTK_GRID(main_eventGrid), main_gridButton[main_gridButtonCount], 1, main_gridButtonCount, 1, 1);
                    g_signal_connect(main_gridButton[main_gridButtonCount], "clicked", G_CALLBACK(on_event_grid_row_clicked), NULL);
                    main_gridButtonCount ++;

                }
                fclose(f1);
                gtk_widget_show_all(main_window);
                break;
            }else if(*cacheStatus == STATUS_FAILED)
            {
                    //EXIT_FAILURE
                break;
            }
             
        }
    }else
    {
        //EXIT_FAILURE
        return;
    }

    
}
//signals
void sigquit() 
{
    on_midiClosed(); 
    exit(0); 
} 
//menu buttons
void on_menu_new_activate(GtkMenuItem *m)
{

}

void on_menu_quit_activate(GtkMenuItem *m)
{

}

void on_menu_about_activate(GtkMenuItem *m)
{

}

//progress bar

	// gtk_progress_bar_set_fraction 
	// 		(GTK_PROGRESS_BAR(main_progressBar), (gdouble) 1.00 );

