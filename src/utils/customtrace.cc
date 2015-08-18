#include "customtrace.h"
#include <iostream>

std::string set_color(Color foreground, Color background) {
    char num_s[3];
    std::string s = "\033[";

    if (!foreground && !background) s += "0"; // reset colors if no params

    if (foreground) {
        snprintf(num_s, 3, "%d", 29 + foreground);
        s += num_s;

        if (background) s += ";";
    }

    if (background) {
        snprintf(num_s, 3, "%d", 39 + background);
        s += num_s;
    }

    return s + "m";
}