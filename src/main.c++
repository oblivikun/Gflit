#include <gtk/gtk.h>
#include <curl/curl.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp> // For JSON parsing

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t new_length = size*nmemb;
    size_t old_length = s->size();
    try {
        s->resize(old_length + new_length);
    } catch(std::bad_alloc const &e) {
        // Handle memory problem
    }
    std::copy((char*)contents,(char*)contents+new_length,s->begin()+old_length);
    return size*nmemb;
}
static void fetch_and_display_flits(GtkWidget* button, gpointer user_data) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://tweetor.pythonanywhere.com/api/get_flits?skip=0&limit=20");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Check if the response is not empty
            if(!readBuffer.empty()) {
                try {
                    nlohmann::json j = nlohmann::json::parse(readBuffer);
                    
                    // Initialize an empty string to accumulate flit details
                    std::string accumulatedDetails;

                    // Iterate over the JSON array
                    for(auto& flit : j) {
                        std::ostringstream oss;
                        oss << "Content: " << flit["content"] << "\n";
                        oss << "Hashtag: " << flit["hashtag"] << "\n";
                        oss << "ID: " << flit["id"] << "\n";
                        bool isRefLit = (flit["is_reflit"].is_number() && flit["is_reflit"].get<int>() > 0);
                        oss << "Is Reflit: " << (isRefLit ? "Yes" : "No") << "\n";
                        oss << "Meme Link: " << flit["meme_link"] << "\n";
                        oss << "Original Flit ID: " << flit["original_flit_id"] << "\n";
                        oss << "Timestamp: " << flit["timestamp"] << "\n";
                        oss << "User Handle: " << flit["userHandle"] << "\n";
                        oss << "Username: " << flit["username"] << "\n\n"; // Newline for separation between items
                        
                        // Append the formatted string to the accumulatedDetails
                        accumulatedDetails += oss.str();
                    }

                    // Update the label's text with the accumulated details
                    gtk_label_set_text(GTK_LABEL(user_data), accumulatedDetails.c_str());

                } catch(const nlohmann::json::parse_error& e) {
                    std::cerr << "JSON parse error: " << e.what() << '\n';
                }
            } else {
                std::cerr << "Received empty response.\n";
            }
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}


static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget* window;
    GtkWidget* button;
    GtkWidget* label; // To display the fetched data
    GtkWidget* vbox; // Vertical box to hold both the button and the label

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Flit Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    button = gtk_button_new_with_label("Fetch Flits");
    
    // Create a vertical box and add the button and label to it
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(vbox), button);
    
    label = gtk_label_new(""); // Initialize a label to display the fetched data
    gtk_box_append(GTK_BOX(vbox), label);
    
    // Now connect the button click signal, passing the label as user_data
    g_signal_connect(button, "clicked", G_CALLBACK(fetch_and_display_flits), label);
    
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    gtk_window_present(GTK_WINDOW(window));
}


int main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
