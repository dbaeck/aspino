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


#include "AbstractSolver.h"
#include "SatSolver.h"
#include "PseudoBooleanSolver.h"
#include "MaxSatSolver.h"
#include "AspSolver.h"
#include "utils/trace.h"
#include "utils/algorithm.h"

#include <utils/Options.h>

#include <errno.h>
#include <signal.h>
#include <zlib.h>

#include <string>
#include <iostream>
#if defined(__linux__)
#include <fpu_control.h>
#endif

using namespace aspino;
using namespace std;

static Glucose::EnumOption option_mode("MAIN", "mode", "How to interpret input.\n", "asp|sat|maxsat|pbs");
Glucose::IntOption option_n("MAIN", "n", "Number of desired solutions. Non-positive integers are interpreted as unbounded.\n", 1, Glucose::IntRange(0, INT32_MAX));
extern Glucose::EnumOption option_maxsat_strat;
extern Glucose::EnumOption option_core_reduction;

static aspino::AbstractSolver* solver;

static void SIGINT_interrupt(int) { solver->interrupt(); }

#include<sys/time.h>

long currentTime()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}


int main(int argc, char** argv)
{
    long starttime = currentTime();

    signal(SIGINT, SIGINT_interrupt);
    signal(SIGTERM, SIGINT_interrupt);

    Glucose::setUsageHelp(
        "Solve ASP or SAT problems read from STDIN or provided as command-line argument.\n\n"
        "usage: %s [flags] [input-file]\n");

#if defined(__linux__)
    fpu_control_t oldcw, newcw;
    _FPU_GETCW(oldcw); newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE; _FPU_SETCW(newcw);
    //printf("c WARNING: for repeatability, setting FPU to use double precision\n");
#endif

    Glucose::IntOption cpu_lim("MAIN", "cpu-lim","Limit on CPU time allowed in seconds.\n", INT32_MAX, Glucose::IntRange(0, INT32_MAX));
    Glucose::IntOption mem_lim("MAIN", "mem-lim","Limit on memory usage in megabytes.\n", INT32_MAX, Glucose::IntRange(0, INT32_MAX));

    Glucose::parseOptions(argc, argv, true);

    if(strcmp(option_mode, "asp") == 0)
        solver = new AspSolver();
    else if(strcmp(option_mode, "sat") == 0)
        solver = new SatSolver();
    else if(strcmp(option_mode, "maxsat") == 0)
        solver = new MaxSatSolver();
    else if(strcmp(option_mode, "pbs") == 0)
        solver = new PseudoBooleanSolver();
    else
        assert(0);
    
    if(argc > 2) {
        cerr << "Extra argument: " << argv[2] << endl;
        solver->exit(-1);
    }

    gzFile in = argc == 1 ? gzdopen(0, "rb") : gzopen(argv[1], "rb");
    solver->parse(in);
    gzclose(in);


    dbg("parsed");

    MaxSatSolver *sv = (MaxSatSolver *) solver;
    sv->dump();
    //sv->mxp();

    solver->eliminate(true);
    dbg("eliminated");
    if(!solver->okay()) {
        cout << "UNSATISFIABLE" << endl;
        cout << "Total time: " << currentTime() - starttime << endl;
        solver->exit(20);
    }

    cout << "graph digraph{" << endl;
    int cfg[17] = {455, 340, 286, 256, 255, 254, 253, 240, 239, 238, 237, 198, 197, 108, 107, 76, 75 };

//    sv->conflict.clear();
//    for(int i = 0; i < 17; i++)
//        sv->conflict.push(mkLit(cfg[i]-1));
//
//    cout << sv->conflict << endl;
//
//    sv->mergexplainMinimizeStd(9999);
//
//
//    exit(0);
    lbool ret = solver->solve(option_n);
    cout << "graph }" << endl;
    cout << "Total time: " << currentTime() - starttime << "ms" << endl;

#ifndef SAFE_EXIT
    solver->exit(ret == l_True ? 10 : ret == l_False ? 20 : 0);     // (faster than "return", which will invoke the destructor for 'Solver')
#else
    delete solver;
    return (ret == l_True ? 10 : ret == l_False ? 20 : 0);
#endif
}