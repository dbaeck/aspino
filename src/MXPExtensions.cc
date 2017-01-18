//
// Created by Daniel Baeck on 30.09.15.
//

#include "MaxSatSolver.h"
#include "utils/algorithm.h"
#include <utils/System.h>
#include <utility>

namespace aspino {

    void MaxSatSolver::getConflict(vec<Lit> &b, vec<Lit> &d, vec<Lit> &c, vec<Lit> &out) {


        out.clear();
        if (d.size() != 0 && !isConsistent(b)) {
            // return empty set
            return;
        }

        if (c.size() == 1) {
            // return C
            c.copyTo(out);
            return;
        }

        lvec c1, c2, d1, d2, bc1, bd2;

        //Split C into disjoint non-empty sets C1 & C2
        aspino::split(c, c1, c2);

        // getConflict (B u C1, C1, C2)
        aspino::merge(b, c1, bc1);
        getConflict(bc1, c1, c2, d2);

        // getConflict (B u D2, D2, C1)
        aspino::merge(b, d2, bd2);
        getConflict(bd2, d2, c1, d1);

        //return D1 u D2
        aspino::merge(d1, d2, out);
    }

    bool MaxSatSolver::isConsistent(vec<Lit> &toCheck) {

        cout << "graph consistency2;" << endl;
        cout << "consitency check for Glucose::vec" << endl;
        lvec oldAssumptions;
        assumptions.moveTo(oldAssumptions);
        toCheck.copyTo(assumptions);

        PseudoBooleanSolver::solve();
        //oldAssumptions.moveTo(assumptions);

        if(status == l_False)
            cout << "false" << endl;
        if(status == l_True)
            cout << "true" << endl;
        if(status == l_Undef)
            cout << "undef" << endl;

        return (status == l_True);
    }

    std::vector<std::vector<Lit> > MaxSatSolver::findConflicts(vec<Lit> &b, vec<Lit> &c, vec<Lit> &cprime) {

        std::vector<std::vector<Lit> > conflicts;

        //isConsistent (B u C)?
        lvec bc;
        aspino::merge(b, c, bc);
        if (isConsistent(bc)) {
            //return < C, EmptySet >
            c.copyTo(cprime);
            return conflicts;
        }

        //|C| = 1
        if (c.size() == 1) {
            // return < EmptySet, { C } >
            cprime.clear();
            conflicts.push_back(stdCast(c));
            dbg(c);
            return conflicts;
        }

        lvec c1, c2, c1prime, c2prime, merged, x, cs, bc2, bx, aux1;
        std::vector<std::vector<Lit> > conflicts1, conflicts2;

        //Split C into disjoint sets
        aspino::split(c, c1, c2);

        //Find Conflicts in subsets
        conflicts1 = findConflicts(b, c1, c1prime);
        conflicts2 = findConflicts(b, c2, c2prime);

        aspino::merge(c1prime, c2prime, merged);
        aspino::merge(merged, b, merged);

        //Line 10
        conflicts = conflicts1;
        conflicts.insert(conflicts.end(), conflicts2.begin(), conflicts2.end());

        //while (C1' u C2' u B) is not consistent
        while (!isConsistent(merged)) {
            //X = GetConflict
            aspino::merge(b, c2prime, bc2);
            getConflict(bc2, c2prime, c1prime, x);

            //CS = X u GetConflict
            aspino::merge(b, x, bx);
            getConflict(bx, x, c2prime, cs);
            aspino::merge(cs, x, cs);

            // remove elements of X from C1'
            aspino::removeAll(c1prime, x, aux1);
            aux1.moveTo(c1prime);

            //add conflict sets
            conflicts.push_back(stdCast(cs));
        }

        //return <C1' u C2', conflicts >
        aspino::merge(c1prime, c2prime, cprime);
        return conflicts;
    }


    void MaxSatSolver::mergexplainMinimize(int64_t limit) {
        dbg("mergeXplain");
        assert(decisionLevel() == 0);
        // disabled for now, such that we enter MXP also for the |c| = 1 case
        //if (conflict.size() <= 1) return;

        double cpuTime = Glucose::cpuTime() - lastCallCpuTime;
        uint64_t budget = conflicts - lastConflict;
        uint64_t budgetMin = budget / cpuTime;
        if (budget > budget * 30 / cpuTime) budget = budget * 30 / cpuTime;
        trace(maxsat, 10,
              "Minimize core of size " << conflict.size() << " (" << (conflicts - lastConflict) << " conflicts; " <<
              cpuTime << " seconds; each check with budget " << budget << ")");
        trim();

        //disabled for now, |c| = 1 case
        //if (budget == 0) return;

        vec<Lit> core;
        conflict.moveTo(core);

        vec<Lit> allAssumptions;
        for (int i = 0; i < core.size(); i++) allAssumptions.push(~core[i]);

        this->dump("MergeXplain Minimize");
        dbg(core);

        //MergeXPlain
        //If !isConsistent(B) - for the moment, we have no B
        //If isConsistent(B u C) - already handled at this point

        lvec b, cp; //cp is cprime at end, not relevant

        std::vector<std::vector<Lit> > conflicts = findConflicts(b, core, cp);

        dbg(conflicts.size());

        //MXP found conflicts, use the first found minimal core for the moment
        if(conflicts.size() >= 1) {
            std::vector<Lit> cf = conflicts[0];
            lvec cfg;
            vecCast(cf, cfg);
            aspino::removeAll(assumptions, cfg, assumptions);
            cancelUntil(0);
            cfg.moveTo(conflict);
        }
        else {
            cancelUntil(0);
            core.moveTo(conflict);
        }
    }

}