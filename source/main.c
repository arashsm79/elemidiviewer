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

// Make them global childStatus
//shared memory
GuiStatus *playStatus;
GuiStatus *childStatus;
double *progressBarFraction;
//process ids
pid_t pid_child;
pid_t *pid_child_pointer = NULL;
pid_t *pid_parent;



//setting up the main window
GtkBuilder *main_builder;
GtkWidget *main_window;

//main windows widgets 
GtkWidget *main_play;
GtkWidget *main_pause;
GtkWidget *main_fileChooser;
GtkWidget *main_spinner;
GtkWidget *main_progressBar;
GtkWidget *main_textEntry;
GtkWidget *main_eventGrid;


//Menu bar widgets
GtkWidget *menu_new;
GtkWidget *menu_quit;
GtkWidget *menu_about;



//an array of buttons for populating the grid
GtkWidget **main_gridButton;
int	main_gridButtonCount = 0;

//a flag for checking if the grid is empty
GuiStatus gridStatus = STATUS_CLEANED;


//signals
void sigquit_child();
void sigcorrupt();
void sigtype2();
void sigendoftrack();
void sigfilenotfound();
void siggeneralerror();
void signaldoneparsing();
void sigsetfraction();


void on_destroy();
void on_event_grid_row_clicked(GtkButton *);
void showDialog(char str[]);
void clearGrid();

int main(int argc, char *argv[]) {

	gtk_init(&argc, &argv); // init Gtk

//connect xml code 
 
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
	main_textEntry = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_textEntry"));
	main_eventGrid = GTK_WIDGET(gtk_builder_get_object(main_builder, "main_eventGrid"));


	// setting up shared memories
	playStatus = mmap(NULL, sizeof(GuiStatus *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*playStatus = STATUS_PAUSE;

    childStatus = mmap(NULL, sizeof(GuiStatus *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*childStatus = STATUS_CLOSE;

	pid_parent = mmap(NULL, sizeof(pid_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*pid_parent = 1;

    progressBarFraction = mmap(NULL, sizeof(pid_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        *progressBarFraction = 0;


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
    gtk_spinner_stop (GTK_SPINNER(main_spinner));

    if(*childStatus == STATUS_OPEN)
        kill(pid_child, SIGQUIT);

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
    showDialog(gtk_button_get_label (b));
}

//file chooser
void on_main_fileChooser_clicked(GtkButton *f)
{
    gtk_spinner_start (GTK_SPINNER(main_spinner));

    //clean up everything before opening up another file
    if(gridStatus == STATUS_UNCLEAN)
    { 
        clearGrid();
        gridStatus = STATUS_CLEANED;
        if(*childStatus == STATUS_OPEN)
        { 
            kill(pid_child, SIGQUIT);
        }
    }
    
    //set the spinner to 0
   	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(main_progressBar), (gdouble) 0 );
    
    
    //creating a child process for playing the midi
    pid_child = fork();

    // set the status to open
    *childStatus = STATUS_OPEN;

    if(pid_child == 0)
    {
        //FIRST CHILD
        signal(SIGQUIT, sigquit_child);
        sleep(1);
        openMidiFileAndCreateEventCashe(gtk_entry_get_text(GTK_ENTRY(main_textEntry)), pid_parent);

        //populate the grid
        kill(*pid_parent, DONEPARSING);

        //start playing the midi file
        playMidiFile(playStatus, pid_parent);

        *childStatus = STATUS_CLOSE;
        exit(EXIT_SUCCESS);   
     
    }else if(pid_child > 0)
    {
        //parent (main)
        *pid_parent = getpid();
        signal(CORRUPTMIDI, sigcorrupt);
        signal(ENDOFTRACKERROR, sigendoftrack);
        signal(TYPE2MIDI, sigtype2);
        signal(DONEPARSING, signaldoneparsing);
        signal(FILENOTFOUND, sigfilenotfound);
        signal(GENERALERROR, siggeneralerror);
        signal(SIGSETFRAC, sigsetfraction);
        return;
    }else
    {
        showDialog("An error occured while creating a new process!");
        gtk_spinner_stop (GTK_SPINNER(main_spinner));

        return;
    }

    
}
//signals
void sigquit_child() 
{

    gtk_spinner_stop (GTK_SPINNER(main_spinner));
    *childStatus = STATUS_CLOSE;
    on_midiClosed();
    exit(0); 
}

void signaldoneparsing()
{
    FILE *f1 = fopen("cachedEvents.txt", "r");
    if (f1 == NULL ) 
    {
        showDialog("File cachedEvents.txt not found!\n");
        if(*childStatus == STATUS_OPEN)
            kill(pid_child, SIGQUIT);
        return;
    }

    char rowStr[1024];
    unsigned int lineCount = 0;
    while (!feof(f1)) 
    {
        fgets(rowStr, 1024, f1);
        lineCount++;
    }
    fseek(f1, 0, SEEK_SET);

    main_gridButtonCount = 0;
    main_gridButton = calloc(lineCount, sizeof(GtkWidget *));
    gridStatus = STATUS_UNCLEAN;
    while (!feof(f1)) 
    {
        fgets(rowStr, 1024, f1);
        rowStr[strlen(rowStr)-1] = 0; // remove newline byte
        gtk_grid_insert_row (GTK_GRID(main_eventGrid), main_gridButtonCount);

        //A button can be freed by the function 'gtk_container_remove ()'
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
    gridStatus = STATUS_UNCLEAN;
    gtk_widget_show_all(main_window);
    gtk_spinner_stop (GTK_SPINNER(main_spinner));

    return;
}

void sigcorrupt() 
{
    if(*childStatus == STATUS_OPEN)
            kill(pid_child, SIGQUIT);
    showDialog("MIDI file is corrupted!");
    gtk_spinner_stop (GTK_SPINNER(main_spinner));

}

void sigtype2() 
{
    if(*childStatus == STATUS_OPEN)
            kill(pid_child, SIGQUIT);
    gtk_spinner_stop (GTK_SPINNER(main_spinner));
    showDialog("MIDI file is type 2 which is not supported!");
}

void sigendoftrack() 
{
    if(*childStatus == STATUS_OPEN)
            kill(pid_child, SIGQUIT);
    gtk_spinner_stop (GTK_SPINNER(main_spinner));
    showDialog("There seems to be a problem with the End of Track event!");
}
void sigfilenotfound() 
{
    if(*childStatus == STATUS_OPEN)
            kill(pid_child, SIGQUIT);
    gtk_spinner_stop (GTK_SPINNER(main_spinner));
    showDialog("Coudln't open the file!");
}

void siggeneralerror() 
{
    if(*childStatus == STATUS_OPEN)
            kill(pid_child, SIGQUIT);
    gtk_spinner_stop (GTK_SPINNER(main_spinner));
    showDialog("An error occured while parsing the file!");
}

void sigsetfraction()
{
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(main_progressBar), (gdouble) *progressBarFraction );
    
}

//menu buttons
void on_menu_new_activate(GtkMenuItem *m)
{

    if(*childStatus == STATUS_OPEN)
            kill(pid_child, SIGQUIT);


    if(gridStatus == STATUS_UNCLEAN)
    { 
        clearGrid();
        gridStatus = STATUS_CLEANED;
    }
    //set the spinner to 0
   	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(main_progressBar), (gdouble) 0 );

    gtk_entry_set_text(GTK_ENTRY(main_textEntry), "");
}

void on_menu_quit_activate(GtkMenuItem *m)
{
    if(*childStatus == STATUS_OPEN)
            kill(pid_child, SIGQUIT);
    exit(EXIT_SUCCESS);
}

void on_menu_about_activate(GtkMenuItem *m)
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(main_window),
                                 flags,
                                 GTK_MESSAGE_INFO,
                                 GTK_BUTTONS_CLOSE,
                                 "Ele Midi File Viewer\n\nDeveloped by ArashSM79.\nContact: @arashsm79 on LinkedIn, Instagram, \nFacebook, Telegram, Whatsapp\nEmail: arashsm79@yahoo.com");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

}

//////////////////

void showDialog(char str[])
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(main_window),
                                 flags,
                                 GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_CLOSE,
                                 "%s", str);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

    //set the spinner to 0
   	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(main_progressBar), (gdouble) 0 );

}

////////////////////

void clearGrid()
{
    for(int i = 0; i < main_gridButtonCount; i++)
    {
        gtk_container_remove(GTK_CONTAINER(main_eventGrid), main_gridButton[i]);            
    }
    free(main_gridButton);

}
//progress bar

	//gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(main_progressBar), (gdouble) 1.00 );

    // gtk_widget_set_sensitive(main_play, FALSE);
