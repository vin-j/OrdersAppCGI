#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

std::string getFormValue(const std::string& data, const std::string& key) {
    std::string search = key + "=";
    size_t start = data.find(search);
    if (start == std::string::npos) return "";
    start += search.length();
    size_t end = data.find("&", start);
    std::string value = data.substr(start, end - start);
    // Replace '+' with space
    for (size_t i = 0; i < value.length(); ++i)
        if (value[i] == '+') value[i] = ' ';
    return value;
}

int main() {
    std::cout << "Content-Type: text/html\n\n";

    std::string postData;
    char* len = _dupenv_s(&contentLengthStr, &len, "CONTENT_LENGTH");
    int contentLength = len ? std::stoi(len) : 0;

    for (int i = 0; i < contentLength; ++i) {
        char c = getchar();
        postData += c;
    }

    std::string product = getFormValue(postData, "product");
    std::string quantity = getFormValue(postData, "quantity");

    std::ifstream templateFile("C:/Apache24/htdocs/order_template.html");
    if (!templateFile.is_open()) {
        std::cout << "<h1>Error: Could not open template</h1>";
        return 1;
    }

    std::string line;
    while (std::getline(templateFile, line)) {
        size_t pos;
        while ((pos = line.find("{{product}}")) != std::string::npos)
            line.replace(pos, 11, product);
        while ((pos = line.find("{{quantity}}")) != std::string::npos)
            line.replace(pos, 12, quantity);
        std::cout << line << "\n";
    }

    return 0;
}
