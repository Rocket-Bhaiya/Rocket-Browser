#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <glib.h>
#include <stdio.h>

// Forward declarations
static void on_url_entry_activate(GtkEntry* entry, gpointer data);
static void on_search_button_clicked(GtkButton* button, gpointer data);
static void on_back_button_clicked(GtkButton* button, gpointer data);
static void on_forward_button_clicked(GtkButton* button, gpointer data);
static void on_home_button_clicked(GtkButton* button, gpointer data);
static void on_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event, gpointer user_data);
static void on_tab_close_clicked(GtkButton* button, GtkNotebook* notebook);

// Structure to hold tab-specific widgets
typedef struct {
    GtkWidget* container;
    GtkWidget* url_entry;
    WebKitWebView* webview;
    GtkComboBoxText* search_engine_combo;
    GtkWidget* title_label;  // Added to store reference to the label
} BrowserTab;

// URL validation functions
static gboolean is_valid_url(const gchar* url) {
    return g_str_has_prefix(url, "http://") || g_str_has_prefix(url, "https://");
}

static gboolean is_localhost(const gchar* input) {
    return g_str_has_prefix(input, "localhost:") || g_str_has_prefix(input, "127.0.0.1:");
}

static gboolean is_domain(const gchar* input) {
    return g_str_has_suffix(input, ".com") || g_str_has_suffix(input, ".org") ||
           g_str_has_suffix(input, ".net") || g_str_has_suffix(input, ".edu") ||
           g_str_has_suffix(input, ".gov") || g_str_has_suffix(input, ".co") ||
           g_str_has_suffix(input, ".io");
}

// Function to update tab title based on webpage title
static void on_title_changed(WebKitWebView* web_view, GParamSpec* pspec, gpointer user_data) {
    BrowserTab* tab = (BrowserTab*)user_data;
    const gchar* title = webkit_web_view_get_title(web_view);
    if (title != NULL && tab->title_label != NULL) {
        gtk_label_set_text(GTK_LABEL(tab->title_label), title);
    }
}

// Function to create a new browser tab
static BrowserTab* create_browser_tab(void) {
    BrowserTab* tab = g_new(BrowserTab, 1);
    
    // Create container for the tab content
    tab->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Create horizontal box for controls
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(tab->container), hbox, FALSE, FALSE, 0);
    
    // Create search engine combo box
    tab->search_engine_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(tab->search_engine_combo, "Google");
    gtk_combo_box_text_append_text(tab->search_engine_combo, "Bing");
    gtk_combo_box_text_append_text(tab->search_engine_combo, "DuckDuckGo");
    gtk_combo_box_text_append_text(tab->search_engine_combo, "Yahoo");
    gtk_combo_box_set_active(GTK_COMBO_BOX(tab->search_engine_combo), 0);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(tab->search_engine_combo), FALSE, FALSE, 0);
    
    // Create URL entry
    tab->url_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), tab->url_entry, TRUE, TRUE, 0);
    
    // Create navigation buttons
    GtkWidget* back_button = gtk_button_new_with_label("Back");
    GtkWidget* forward_button = gtk_button_new_with_label("Forward");
    GtkWidget* home_button = gtk_button_new_with_label("Home");
    GtkWidget* search_button = gtk_button_new_with_label("Search");
    
    gtk_box_pack_start(GTK_BOX(hbox), back_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), forward_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), home_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), search_button, FALSE, FALSE, 0);
    
    // Create WebView
    tab->webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_box_pack_start(GTK_BOX(tab->container), GTK_WIDGET(tab->webview), TRUE, TRUE, 0);
    
    // Store references for callbacks
    g_object_set_data(G_OBJECT(tab->url_entry), "webview", tab->webview);
    g_object_set_data(G_OBJECT(tab->url_entry), "search_engine_combo", tab->search_engine_combo);
    
    // Connect signals
    g_signal_connect(tab->url_entry, "activate", G_CALLBACK(on_url_entry_activate), tab->webview);
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_button_clicked), tab->url_entry);
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), tab->webview);
    g_signal_connect(forward_button, "clicked", G_CALLBACK(on_forward_button_clicked), tab->webview);
    g_signal_connect(home_button, "clicked", G_CALLBACK(on_home_button_clicked), tab->webview);
    g_signal_connect(tab->webview, "load-changed", G_CALLBACK(on_load_changed), NULL);
    
    // Load default page
    webkit_web_view_load_uri(tab->webview, "https://www.google.com");
    
    return tab;
}

// Function to create new tab label with close button
static GtkWidget* create_tab_label(const gchar* text, GtkNotebook* notebook, BrowserTab* tab) {
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    tab->title_label = gtk_label_new(text);
    GtkWidget* close_button = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
    gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);
    
    gtk_box_pack_start(GTK_BOX(hbox), tab->title_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), close_button, FALSE, FALSE, 0);
    
    g_signal_connect(close_button, "clicked", G_CALLBACK(on_tab_close_clicked), notebook);
    g_object_set_data(G_OBJECT(close_button), "tab", tab);
    
    // Connect title changed signal
    g_signal_connect(tab->webview, "notify::title", G_CALLBACK(on_title_changed), tab);
    
    gtk_widget_show_all(hbox);
    return hbox;
}

// Navigation and URL handling callbacks remain the same
static void on_url_entry_activate(GtkEntry* entry, gpointer data) {
    const gchar* text = gtk_entry_get_text(entry);
    WebKitWebView* webview = WEBKIT_WEB_VIEW(data);

    if (is_valid_url(text)) {
        webkit_web_view_load_uri(webview, text);
    } else if (is_localhost(text)) {
        gchar* url_with_prefix = g_strdup_printf("http://%s", text);
        webkit_web_view_load_uri(webview, url_with_prefix);
        g_free(url_with_prefix);
    } else if (is_domain(text)) {
        gchar* url_with_prefix = g_strdup_printf("http://%s", text);
        webkit_web_view_load_uri(webview, url_with_prefix);
        g_free(url_with_prefix);
    } else {
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
    on_url_entry_activate(GTK_ENTRY(entry), webview);
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
    webkit_web_view_load_uri(webview, "https://www.google.com");
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

// Tab management callbacks
static void on_tab_close_clicked(GtkButton* button, GtkNotebook* notebook) {
    BrowserTab* tab = g_object_get_data(G_OBJECT(button), "tab");
    gint page_num = gtk_notebook_page_num(notebook, tab->container);
    
    // Don't close if it's the last tab
    if (gtk_notebook_get_n_pages(notebook) > 1) {
        gtk_notebook_remove_page(notebook, page_num);
        g_free(tab);
    }
}

static void on_new_tab_clicked(GtkButton* button, GtkNotebook* notebook) {
    BrowserTab* tab = create_browser_tab();
    gint page_num = gtk_notebook_append_page(notebook, tab->container,
                                           create_tab_label("New Tab", notebook, tab));
    gtk_widget_show_all(tab->container);
    gtk_notebook_set_current_page(notebook, page_num);
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    
    // Create main window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
    gtk_window_set_title(GTK_WINDOW(window), "Rocket Browser");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "images/favicon.ico", NULL);
    
    // Create main vertical box
    GtkWidget* main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);
    
    // Create notebook for tabs
    GtkWidget* notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);
    
    // Create new tab button
    // GtkWidget* new_tab_button = gtk_button_new_with_label("+");
    // gtk_button_set_relief(GTK_BUTTON(new_tab_button), GTK_RELIEF_NONE);
    // g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_clicked), notebook);
    // Replace the new tab button creation code with this:

// Create new tab button with larger size
    GtkWidget* new_tab_button = gtk_button_new_with_label("+");
    gtk_button_set_relief(GTK_BUTTON(new_tab_button), GTK_RELIEF_NONE);

    // Create a larger label for the button
    GtkWidget* button_label = gtk_bin_get_child(GTK_BIN(new_tab_button));
    PangoAttrList* attr_list = pango_attr_list_new();
    pango_attr_list_insert(attr_list, pango_attr_scale_new(1.5)); // Make text 1.5 times larger
    gtk_label_set_attributes(GTK_LABEL(button_label), attr_list);
    pango_attr_list_unref(attr_list);

    // Set minimum size for the button
    gtk_widget_set_size_request(new_tab_button, 40, 40);

// Add padding around the button content
    GtkCssProvider* provider = gtk_css_provider_new();
    const gchar* css = 
        "button { padding: 5px 10px; margin: 2px; }"
        "button label { font-weight: bold; }";
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(new_tab_button),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);

    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_clicked), notebook);
    // Add new tab button to notebook
    gtk_notebook_set_action_widget(GTK_NOTEBOOK(notebook), new_tab_button, GTK_PACK_END);
    gtk_widget_show(new_tab_button);
    
    // Create initial tab
    BrowserTab* initial_tab = create_browser_tab();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), initial_tab->container,
                           create_tab_label("New Tab", GTK_NOTEBOOK(notebook), initial_tab));
    
    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}