# Rocket Browser

Rocket Browser is a simple web browser built using GTK and WebKit2GTK. It provides basic browsing features like back, forward, search, home, and supports loading local servers running on `localhost` with specified ports.

## Features

- **Navigation**: Supports forward, backward navigation using buttons.
- **Search**: Allows searching via Google, Bing, DuckDuckGo, and Yahoo.
- **Home Button**: Set to load Google as the default homepage.
- **URL/Domain Handling**: If the input is a valid URL, it will load directly. If the input is a domain, it will be prefixed with `http://` and loaded.
- **Localhost Support**: Can access local servers with `localhost` or `127.0.0.1` followed by a port number (e.g., `localhost:8080`).

## Requirements

To compile and run the Rocket Browser, ensure you have the following dependencies installed:

- **GTK 3.0**: For GUI elements.
- **WebKit2GTK**: For rendering web pages.
- **SQLite3**: For storage support (if required).
- **OpenSSL**: For secure HTTP connections.
- **libcurl**: For handling URL fetching.
- **Glib**: For general utility functions.

## Installation

### Prerequisites

Make sure the following libraries are installed on your system:

- **GTK 3.0**: Install via your package manager, e.g., `sudo apt-get install libgtk-3-dev` for Ubuntu.
- **WebKit2GTK**: Install via your package manager, e.g., `sudo apt-get install libwebkit2gtk-4.0-dev` for Ubuntu.
- **SQLite3**: Install via your package manager, e.g., `sudo apt-get install libsqlite3-dev` for Ubuntu.
- **OpenSSL**: Install via your package manager, e.g., `sudo apt-get install libssl-dev` for Ubuntu.
- **libcurl**: Install via your package manager, e.g., `sudo apt-get install libcurl4-openssl-dev` for Ubuntu.

### Building

To build the browser, use the following `gcc` command:

```sh
gcc -o Rocket_Browser Rocket_Browser.c `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -lsqlite3 -lssl -lcrypto -lcurl
./Rocket_Browser
