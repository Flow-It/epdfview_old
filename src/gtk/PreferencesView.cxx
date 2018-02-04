// ePDFView - A lightweight PDF Viewer.
// Copyright (C) 2006, 2007, 2009 Emma's Software.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <gtk/gtk.h>
#include <epdfview.h>
#include "PreferencesView.h"

using namespace ePDFView;

// Callbacks
static void preferences_view_browser_command_changed (GtkEntry *, gpointer);
static void preferences_view_backsearch_command_changed (GtkEntry *, gpointer);

PreferencesView::PreferencesView (GtkWindow *parent):
    IPreferencesView ()
{
    m_PreferencesDialog = gtk_dialog_new_with_buttons (_("Preferences"),
													   parent, GTK_DIALOG_MODAL,
													   // GTK_STOCK_OK,    GTK_RESPONSE_ACCEPT,
													   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
            NULL);
    // Utility dialog should not show itself in the window list.
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (m_PreferencesDialog), TRUE);

    GtkWidget *notebook = gtk_notebook_new ();
    gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (
				    GTK_DIALOG (m_PreferencesDialog))),
                       notebook);
    gtk_container_set_border_width (GTK_CONTAINER (notebook), 6);

    // Add the notebook's tabs.
    {
        GtkWidget *label = gtk_label_new (_("External Commands"));
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                                  createExternalCommandsTab (), label);
    }
    gtk_widget_show_all (notebook);
}

PreferencesView::~PreferencesView ()
{
    gtk_widget_destroy (m_PreferencesDialog);
}

void
PreferencesView::setPresenter (PreferencesPter *pter)
{
    IPreferencesView::setPresenter (pter);

    // Connect the signals.
    g_signal_connect (G_OBJECT (m_BrowserCommandLine), "changed",
                      G_CALLBACK (preferences_view_browser_command_changed),
                      pter);
    g_signal_connect (G_OBJECT (m_BacksearchCommandLine), "changed",
                      G_CALLBACK (preferences_view_backsearch_command_changed),
                      pter);
    // Run the dialog.
    gtk_dialog_run (GTK_DIALOG (m_PreferencesDialog));
    // The close must have been activated.
    pter->closeActivated ();
}

const gchar *
PreferencesView::getBrowserCommandLine ()
{
    return gtk_entry_get_text (GTK_ENTRY (m_BrowserCommandLine));
}

const gchar *
PreferencesView::getBacksearchCommandLine ()
{
    return gtk_entry_get_text (GTK_ENTRY (m_BacksearchCommandLine));
}


////////////////////////////////////////////////////////////////
// Tab Creators
////////////////////////////////////////////////////////////////

GtkWidget *
PreferencesView::createExternalCommandsTab ()
{
    // The grid to add all the controls.
    GtkWidget *grid = gtk_grid_new ();
    gtk_widget_set_halign (grid, GTK_ALIGN_END);
    gtk_widget_set_valign (grid, GTK_ALIGN_END);
    gtk_widget_set_margin_start (grid, 6);
    gtk_widget_set_margin_end (grid, 6);
    gtk_widget_set_margin_top (grid, 6);
    gtk_widget_set_margin_bottom (grid, 6);
    gtk_grid_set_row_spacing (GTK_GRID (grid), 12);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 3);

    m_BrowserCommandLine = gtk_entry_new ();
    GtkWidget *webBrowserLabel = gtk_label_new (_("Web_Browser:"));
    gtk_label_set_xalign (GTK_LABEL (webBrowserLabel), 1.0f);
    gtk_label_set_use_markup (GTK_LABEL (webBrowserLabel), TRUE);
    gtk_label_set_use_underline (GTK_LABEL (webBrowserLabel), TRUE);
    gtk_label_set_mnemonic_widget (GTK_LABEL (webBrowserLabel),
                                   m_BrowserCommandLine);
    gtk_grid_attach (GTK_GRID (grid), webBrowserLabel, 0, 0, 1, 1);

    // The web browser.
    gchar *browserCommandLine =
        Config::getConfig ().getExternalBrowserCommandLine ();
    gtk_entry_set_text (GTK_ENTRY (m_BrowserCommandLine), browserCommandLine);
    g_free (browserCommandLine);
    gtk_grid_attach (GTK_GRID (grid), m_BrowserCommandLine, 1, 0, 1, 1);

    // The %s note.
    GtkWidget *note = gtk_label_new (_("Note: <i>%s</i> will be replaced by the URI."));
    gtk_label_set_xalign (GTK_LABEL (note), 0.0f);
    gtk_label_set_use_markup (GTK_LABEL (note), TRUE);
    gtk_grid_attach (GTK_GRID (grid), note, 0, 1, 2, 1);

    // Back-searching (through SyncTeX)
    m_BacksearchCommandLine = gtk_entry_new ();
    GtkWidget *backsearchLabel = gtk_label_new (_("SyncTeX script:"));
    gtk_label_set_xalign (GTK_LABEL (backsearchLabel), 1.0f);
    gtk_label_set_use_markup (GTK_LABEL (backsearchLabel), TRUE);
    gtk_label_set_use_underline (GTK_LABEL (backsearchLabel), TRUE);
    gtk_label_set_mnemonic_widget (GTK_LABEL (backsearchLabel),
                                   m_BacksearchCommandLine);
    gtk_grid_attach (GTK_GRID (grid), backsearchLabel, 0, 2, 1, 1);
    gchar *backsearchCommandLine =
        Config::getConfig ().getExternalBacksearchCommandLine ();
    gtk_entry_set_text (GTK_ENTRY (m_BacksearchCommandLine), backsearchCommandLine);
    g_free (backsearchCommandLine);
    gtk_grid_attach (GTK_GRID (grid), m_BacksearchCommandLine, 1, 2, 1, 1);


    // The meaning of %p %x %y %f
    GtkWidget *note2 = gtk_label_new (_("Note: <i>%p</i>=page <i>%x,%y</i>=coordinates <i>%f</i>=PDF file"));
    gtk_label_set_xalign (GTK_LABEL (note2), 0.0f);
    gtk_label_set_use_markup (GTK_LABEL (note2), TRUE);
    gtk_grid_attach (GTK_GRID (grid), note2, 0, 3, 2, 1);


    return grid;
}

////////////////////////////////////////////////////////////////
// Callbacks
////////////////////////////////////////////////////////////////
void
preferences_view_browser_command_changed (GtkEntry *entry, gpointer data)
{
    g_assert (NULL != data && "The data parameter is NULL.");

    PreferencesPter *pter = (PreferencesPter *)data;
    pter->browserCommandLineChanged ();
}

void
preferences_view_backsearch_command_changed (GtkEntry *entry, gpointer data)
{
    g_assert (NULL != data && "The data parameter is NULL.");

    PreferencesPter *pter = (PreferencesPter *)data;
    pter->backsearchCommandLineChanged ();
}
