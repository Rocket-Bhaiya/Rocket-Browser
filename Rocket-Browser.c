#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <glib.h>
#include <stdio.h>

static gboolean is_valid_url(const gchar* url) {
    // Check if the input starts with "http://" or "https://"
    return g_str_has_prefix(url, "http://") || g_str_has_prefix(url, "https://");
}

static gboolean is_localhost(const gchar* input) {
    // Check if the input is localhost with a port
    return g_str_has_prefix(input, "localhost:") || g_str_has_prefix(input, "127.0.0.1:");
}

static gboolean is_domain(const gchar* input) {
    // Check if the input contains a dot (.) and doesn't start with "http://"
    return g_str_has_suffix(input, ".com") || g_str_has_suffix(input, ".org") ||
           g_str_has_suffix(input, ".net") || g_str_has_suffix(input, ".edu") ||
           g_str_has_suffix(input, ".gov") || g_str_has_suffix(input, ".co") ||
           g_str_has_suffix(input, ".io");
}

static void destroy_window(GtkWidget* widget, GtkWidget* window) {
    gtk_main_quit();
}

static void on_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event, gpointer user_data) {
    switch (load_event) {
        case WEBKIT_LOAD_STARTED:
            printf("Load started\n");
            break;
        case WEBKIT_LOAD_COMMITTED:
            printf("Load committed\n");
            break;
        case WEBKIT_LOAD_FINISHED:
            printf("Load finished\n");
            break;
        default:
            break;
    }
}

static void on_url_entry_activate(GtkEntry* entry, gpointer data) {
    const gchar* text = gtk_entry_get_text(entry);
    WebKitWebView* webview = WEBKIT_WEB_VIEW(data);

    // Check if input is a valid URL or domain
    if (is_valid_url(text)) {
        // If it's a valid URL, load it
        webkit_web_view_load_uri(webview, text);
    } else if (is_localhost(text)) {
        // If it's localhost with a port, treat it as a valid URL (add "http://")
        gchar* url_with_prefix = g_strdup_printf("http://%s", text);
        webkit_web_view_load_uri(webview, url_with_prefix);
        g_free(url_with_prefix);
    } else if (is_domain(text)) {
        // If it's a domain, treat it as a URL (add "http://")
        gchar* url_with_prefix = g_strdup_printf("http://%s", text);
        webkit_web_view_load_uri(webview, url_with_prefix);
        g_free(url_with_prefix);
    } else {
        // Treat it as a search query
        GtkComboBoxText* search_engine_combo = GTK_COMBO_BOX_TEXT(g_object_get_data(G_OBJECT(entry), "search_engine_combo"));
        const gchar* selected_engine = gtk_combo_box_text_get_active_text(search_engine_combo);
        gchar* search_url = NULL;

        if (g_strcmp0(selected_engine, "Google") == 0) {
            search_url = g_strdup_printf("https://www.google.com/search?q=%s", text);
        } else if (g_strcmp0(selected_engine, "Bing") == 0) {
            search_url = g_strdup_printf("https://www.bing.com/search?q=%s", text);
        } else if (g_strcmp0(selected_engine, "DuckDuckGo") == 0) {
            search_url = g_strdup_printf("https://www.duckduckgo.com/?q=%s", text);
        } else if (g_strcmp0(selected_engine, "Yahoo") == 0) {
            search_url = g_strdup_printf("https://search.yahoo.com/search?p=%s", text);
        }

        if (search_url) {
            webkit_web_view_load_uri(webview, search_url);
            g_free(search_url);
        }
    }
}

static void on_search_button_clicked(GtkButton* button, gpointer data) {
    GtkWidget* entry = GTK_WIDGET(data);
    WebKitWebView* webview = g_object_get_data(G_OBJECT(entry), "webview");
    on_url_entry_activate(GTK_ENTRY(entry), webview);  // Explicitly call the same function as when activating the entry
}

static void on_back_button_clicked(GtkButton* button, gpointer data) {
    WebKitWebView* webview = WEBKIT_WEB_VIEW(data);
    if (webkit_web_view_can_go_back(webview)) {
        webkit_web_view_go_back(webview);
    }
}

static void on_forward_button_clicked(GtkButton* button, gpointer data) {
    WebKitWebView* webview = WEBKIT_WEB_VIEW(data);
    if (webkit_web_view_can_go_forward(webview)) {
        webkit_web_view_go_forward(webview);
    }
}

static void on_home_button_clicked(GtkButton* button, gpointer data) {
    WebKitWebView* webview = WEBKIT_WEB_VIEW(data);
    // Set a default home page (e.g., Google)
    webkit_web_view_load_uri(webview, "https://www.google.com");
}

int main(int argc, char* argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
    gtk_window_set_title(GTK_WINDOW(window), "Rocket Browser");
    g_signal_connect(window, "destroy", G_CALLBACK(destroy_window), NULL);

    // Create a vertical box for layout
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a horizontal box for URL entry, search engine selection, and navigation buttons
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Create search engine combo box
    GtkComboBoxText* search_engine_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(search_engine_combo, "Google");
    gtk_combo_box_text_append_text(search_engine_combo, "Bing");
    gtk_combo_box_text_append_text(search_engine_combo, "DuckDuckGo");
    gtk_combo_box_text_append_text(search_engine_combo, "Yahoo");
    gtk_combo_box_set_active(GTK_COMBO_BOX(search_engine_combo), 0); // Default to Google
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(search_engine_combo), FALSE, FALSE, 0);

    // Create URL entry
    GtkWidget* url_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), url_entry, TRUE, TRUE, 0);

    // Create Go Back button
    GtkWidget* back_button = gtk_button_new_with_label("Back");
    gtk_box_pack_start(GTK_BOX(hbox), back_button, FALSE, FALSE, 0);

    // Create Go Forward button
    GtkWidget* forward_button = gtk_button_new_with_label("Forward");
    gtk_box_pack_start(GTK_BOX(hbox), forward_button, FALSE, FALSE, 0);

    // Create Home button
    GtkWidget* home_button = gtk_button_new_with_label("Home");
    gtk_box_pack_start(GTK_BOX(hbox), home_button, FALSE, FALSE, 0);

    // Create search button
    GtkWidget* search_button = gtk_button_new_with_label("Search");
    gtk_box_pack_start(GTK_BOX(hbox), search_button, FALSE, FALSE, 0);

    // Create WebView
    WebKitWebView* webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(webview), TRUE, TRUE, 0);

    // Load a default page
    webkit_web_view_load_uri(webview, "https://www.google.com");

    // Store WebView in URL entry data so it's accessible in search button callback
    g_object_set_data(G_OBJECT(url_entry), "webview", webview);
    g_object_set_data(G_OBJECT(url_entry), "search_engine_combo", search_engine_combo);

    // Connect signals
    g_signal_connect(url_entry, "activate", G_CALLBACK(on_url_entry_activate), webview);
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_button_clicked), url_entry);
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), webview);
    g_signal_connect(forward_button, "clicked", G_CALLBACK(on_forward_button_clicked), webview);
    g_signal_connect(home_button, "clicked", G_CALLBACK(on_home_button_clicked), webview);
    g_signal_connect(webview, "load-changed", G_CALLBACK(on_load_changed), NULL);

    // Show everything
    gtk_widget_show_all(window);

    // Start main loop
    gtk_main();

    return 0;
}
