#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <gtk/gtk.h>

#include "unix_serial.h"

#define APP_ID          "me.n0dere.changer"
#define UI_FILENAME     "changer.glade"
#define INITIAL_INPUT   "0"

typedef struct {
    GObject *pWindow;
    GObject *pSerialOpen;
    GObject *pSerialDevices;
    GObject *pNumberInput;
    Serial *pSerial;
} ChangerWindow;

static void showErrorDialog(GtkWindow *pParent, const char *pError)
{
    GtkWidget *pDialog = NULL;

    pDialog = gtk_message_dialog_new(pParent, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_OK, pError);

    gtk_dialog_run(GTK_DIALOG(pDialog));
    gtk_widget_destroy(pDialog);
}

static void setInitialInput(ChangerWindow *pData)
{
    GtkEntryBuffer *pEntryBuffer = NULL;
    pEntryBuffer = gtk_entry_get_buffer(GTK_ENTRY(pData->pNumberInput));
    gtk_entry_buffer_set_text(pEntryBuffer, INITIAL_INPUT, 1);
}

static void onInsertText(GtkEditable *pEditable, const gchar *pText,
                         gint length, gint *pPos, gpointer pUserData)
{
    size_t i;

    /* https://stackoverflow.com/a/55893352 */
    for (i = 0; i < length; i++) {
        if (isdigit(pText[i]))
            continue;

        g_signal_stop_emission_by_name(G_OBJECT(pEditable), "insert-text");
        return;
    }
}

static void onTextChanged(GtkEditable *pEditable, gpointer pUserData)
{
    ChangerWindow *pData = (ChangerWindow*) pUserData;
    GtkEntryBuffer *pEntryBuffer = NULL;
    const char *pText = NULL;
    uint16_t value = 0;

    pEntryBuffer = gtk_entry_get_buffer(GTK_ENTRY(pEditable));

    pText = gtk_entry_buffer_get_text(pEntryBuffer);

    value = g_ascii_strtoll(pText, NULL, 10);

    serialWrite(pData->pSerial, (uint8_t*) &value, 2);
}

static void onSerialOpenClick(GtkWidget *pButton, gpointer pUserData)
{
    ChangerWindow *pData = (ChangerWindow*) pUserData;
    GtkComboBoxText *pBox = GTK_COMBO_BOX_TEXT(pData->pSerialDevices);
    GtkEntryBuffer *pEntryBuffer = NULL;
    char *pSelected = NULL;
    char *pPort = NULL;

    pSelected = gtk_combo_box_text_get_active_text(pBox);

    if (pSelected == NULL) {
        showErrorDialog(GTK_WINDOW(pData->pWindow), "tty not selected");
        return;
    }

    pPort = g_strdup_printf("/dev/%s", pSelected);

    if (pPort == NULL) {
        showErrorDialog(GTK_WINDOW(pData->pWindow), "alloc error");
        g_free(pSelected);
        return;
    }
    
    pData->pSerial = serialOpenPort(pPort, B9600);

    if (pData->pSerial == NULL) {
        showErrorDialog(GTK_WINDOW(pData->pWindow), "failed to open port");
        g_free(pPort);
        g_free(pSelected);
        return;
    }

    gtk_widget_set_sensitive(GTK_WIDGET(pData->pSerialOpen), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(pData->pSerialDevices), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(pData->pNumberInput), TRUE);

    setInitialInput(pData);

    g_free(pPort);
    g_free(pSelected);
}

static gboolean onWindowDelete(GtkWidget *widget, GdkEvent *event,
                               gpointer pUserData)
{
    serialClose(((ChangerWindow*) pUserData)->pSerial);
    return FALSE;
}

static void addTtyDevicesToComboBox(GtkComboBoxText *pBox)
{
    GDir *pDir = g_dir_open("/sys/class/tty/", 0, NULL);
    const gchar *pFileName = NULL;

    while ((pFileName = g_dir_read_name(pDir)) != NULL) {
        if (g_str_has_prefix(pFileName, "tty") == TRUE)
            gtk_combo_box_text_append_text(pBox, pFileName);
    }

    g_dir_close(pDir);
}

static void activate(GtkApplication *pApp, gpointer pUserData)
{
    GError *pError = NULL;
    GtkBuilder *pBuilder = gtk_builder_new();
    ChangerWindow *pData = (ChangerWindow*) pUserData;

    if (!gtk_builder_add_from_file(pBuilder, UI_FILENAME, &pError)) {
        showErrorDialog(NULL, pError->message);
        g_clear_error(&pError);
        return;
    }

    pData->pWindow = gtk_builder_get_object(pBuilder, "window");

    gtk_window_set_application(GTK_WINDOW(pData->pWindow), pApp);
    gtk_widget_set_visible(GTK_WIDGET(pData->pWindow), TRUE);

    g_signal_connect(pData->pWindow, "delete-event",
                     G_CALLBACK(onWindowDelete), pData);

    pData->pSerialOpen = gtk_builder_get_object(pBuilder, "serial-open");    

    g_signal_connect(GTK_BUTTON(pData->pSerialOpen), "clicked",
                     G_CALLBACK(onSerialOpenClick), pData);

    pData->pSerialDevices = gtk_builder_get_object(pBuilder, "serial-dev");

    addTtyDevicesToComboBox(GTK_COMBO_BOX_TEXT(pData->pSerialDevices));

    gtk_combo_box_set_active(GTK_COMBO_BOX(pData->pSerialDevices), 0);

    pData->pNumberInput = gtk_builder_get_object(pBuilder, "number");

    g_signal_connect(G_OBJECT(pData->pNumberInput), "insert-text",
                     G_CALLBACK(onInsertText), pData);
    
    g_signal_connect(G_OBJECT(pData->pNumberInput), "changed",
                     G_CALLBACK(onTextChanged), pData);

    g_object_unref(pBuilder);
}

int main(int argc, char **argv)
{
    ChangerWindow data = {0};
    GtkApplication *pApp = NULL;
    int status;

    pApp = gtk_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(pApp, "activate", G_CALLBACK(activate), &data);

    status = g_application_run(G_APPLICATION(pApp), argc, argv);
    
    g_object_unref(pApp);
    return status;
}