/*
 *  Copyright (C) 2014  Mario Alviano (mario@alviano.net)
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef __customtrace_h__
#define __customtrace_h__

#define COLOR_TRACE

#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>

#include <utils/Options.h>

extern Glucose::IntOption option_trace_sat;
extern Glucose::IntOption option_trace_pbs;
extern Glucose::IntOption option_trace_maxsat;
extern Glucose::IntOption option_trace_asp;
extern Glucose::IntOption option_trace_asp_pre;


enum Color {
    NONE = 0,
    BLACK, RED, GREEN,
    YELLOW, BLUE, MAGENTA,
    CYAN, WHITE
};

std::string set_color(Color foreground = NONE, Color background = NONE);

#define trace(type, level, msg) \
    if(200 >= level) {\
        std::cout << set_color(GREEN) << "[" << #type << "]" << std::string(level, ' ') << msg << set_color() << std::endl;\
    }

#define dbg(msg) \
    cout << set_color(RED) << "[DEBUG] " << msg << set_color() << std::endl;
#else

#endif
