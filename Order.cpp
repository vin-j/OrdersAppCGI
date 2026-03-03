#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
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

    // Extract product
    pos = postData.find("product=");
    if (pos != string::npos) {
        size_t end = postData.find('&', pos);
        product = urlDecode(postData.substr(pos + 8, end - pos - 8));
    }

    // Extract quantity
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

// Load HTML template from /htdocs/
string loadTemplate() {
    ifstream file("../htdocs/order_template.html");  // adjust path if needed
    string html;
    if (file.is_open()) {
        html.assign((istreambuf_iterator<char>(file)),
            istreambuf_iterator<char>());
        file.close();
    }
    else {
        html = "<html><body><h2>Error: order_template.html not found</h2></body></html>";
    }
    return html;
}

// Replace placeholders and optionally insert confirmation
string fillTemplate(const string& templateHtml,
    const string& product,
    const string& quantity) {
    string html = templateHtml;

    
    // Replace product placeholder
    size_t pos = html.find("{{product}}");
    if (pos != string::npos) {
        html.replace(pos, string("{{product}}").length(), product);
    }

    // Replace quantity placeholder
    pos = html.find("{{quantity}}");
    if (pos != string::npos) {
        html.replace(pos, string("{{quantity}}").length(), quantity);
    }

    return html;
}

int main() {
    cout << "Content-type: text/html\n\n";

    // Read POST data
    string product, quantity;
    parsePostData(product, quantity);

    // Save order if submitted
    if (!product.empty() && !quantity.empty()) {
        appendOrderJSON(product, quantity);
    }

    // Load template
    string html = loadTemplate();

    // Fill template with values and confirmation
    html = fillTemplate(html, product, quantity);

    cout << html;

    return 0;
}