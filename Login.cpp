#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

// URL decode
string urlDecode(const string& str) {
    string decoded;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '+') decoded += ' ';
        else if (str[i] == '%' && i + 2 < str.length()) {
            string hex = str.substr(i + 1, 2);
            char c = static_cast<char>(stoi(hex, nullptr, 16));
            decoded += c;
            i += 2;
        }
        else decoded += str[i];
    }
    return decoded;
}

// Parse POST data
void parsePostData(string& product, string& quantity) {
    char* contentLengthStr = getenv("CONTENT_LENGTH");
    if (!contentLengthStr) return;

    int contentLength = atoi(contentLengthStr);
    if (contentLength <= 0) return;

    string postData(contentLength, '\0');
    cin.read(&postData[0], contentLength);

    size_t pos;
    pos = postData.find("product=");
    if (pos != string::npos) {
        size_t end = postData.find('&', pos);
        product = urlDecode(postData.substr(pos + 8, end - pos - 8));
    }

    pos = postData.find("quantity=");
    if (pos != string::npos) {
        size_t end = postData.find('&', pos);
        quantity = urlDecode(postData.substr(pos + 9, end - pos - 9));
    }
}

// Append order to order.json
void appendOrderJSON(const string& product, const string& quantity) {
    json orders;
    ifstream inFile("order.json");
    if (inFile.is_open()) {
        inFile >> orders;
        inFile.close();
    }

    json newOrder;
    newOrder["product"] = product;
    newOrder["quantity"] = quantity;

    orders.push_back(newOrder);

    ofstream outFile("order.json");
    if (outFile.is_open()) {
        outFile << orders.dump(4);
        outFile.close();
    }
}

// Load products from product.json
json loadProducts() {
    json products;
    ifstream inFile("product.json");
    if (inFile.is_open()) {
        inFile >> products;
        inFile.close();
    }
    else {
        products = json::array({
            { {"name", "Laptop"} },
            { {"name", "Mouse"} },
            { {"name", "Keyboard"} }
            });
    }
    return products;
}

// Load HTML template
string loadTemplate() {
    ifstream file("order_template.html");
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    cout << "Content-type: text/html\n\n";

    string product, quantity;
    parsePostData(product, quantity);

    if (!product.empty() && !quantity.empty()) {
        appendOrderJSON(product, quantity);
    }

    string html = loadTemplate();

    string options;
    json products = loadProducts();
    for (auto& item : products) {
        options += "<option value='" + string(item["name"]) + "'";
        if (item["name"] == product) options += " selected";
        options += ">" + string(item["name"]) + "</option>\n";
    }

    size_t pos = html.find("{{product_options}}");
    if (pos != string::npos) {
        html.replace(pos, string("{{product_options}}").length(), options);
    }

    cout << html;

    return 0;
}