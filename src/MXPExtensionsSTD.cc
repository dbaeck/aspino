//
// Created by Daniel Baeck on 30.09.15.
//

#include "MaxSatSolver.h"
#include "utils/algorithm.h"
#include <utils/System.h>
#include <utility>
#include <sstream>

namespace aspino {

    slvec MaxSatSolver::getConflict(slvec b, slvec d, slvec c) {
        slvec out;
        cout << "in get conflict std::vector" << endl;
        if (d.size() != 0 && !isConsistent(b)) {
            // return empty set
            cout << "empty" << endl;
            return out;
        }

        if (c.size() == 1) {

            // return C
            cout << "c size 1";
            return c;
        }

        cout << "rec get conflict" << endl;
        slvec c1, c2, d1, d2, bc1, bd2;

        //Split C into disjoint non-empty sets C1 & C2
        aspino::split(c, c1, c2);

        cout << "split " << c.size() << " into " << c1.size() << " and " << c2.size() << endl;

        if(c1.size() > 0)
        {
            //cout << "gc1" << endl;
            // getConflict (B u C1, C1, C2)
            bc1 = aspino::merge(b, c1);
            d2 = getConflict(bc1, c1, c2);
        }

        if(c2.size() > 0)
        {
            //cout << "gc2" << endl;
            // getConflict (B u D2, D2, C1)
            bd2 = aspino::merge(b, d2);
            d1 = getConflict(bd2, d2, c1);
        }

        //return D1 u D2
        cout << "return merged" << endl;
        return aspino::merge(d1, d2);
    }

    bool MaxSatSolver::isConsistent(slvec toCheck) {

        cout << "consitency check for std::vector" << endl;
        uint64_t budget = conflicts - lastConflict;

        lvec oldAssumptions;
        assumptions.moveTo(oldAssumptions);
        lvec toCheckA;
        aspino::vecCast(toCheck, toCheckA);

        toCheckA.moveTo(assumptions);
//        cout << "to check" << endl;
//        for(int i = 0; i<assumptions.size(); i++)
//            cout << assumptions[i] << " ";
//        cout << endl;

        float sumLBD_ = sumLBD;
        uint64_t conflictsRestarts_ = conflictsRestarts;
        sumLBD = 0;
        conflictsRestarts = 0;
        setConfBudget(budget);
        PseudoBooleanSolver::solve();
        budgetOff();
        sumLBD += sumLBD_;
        conflictsRestarts += conflictsRestarts_;

        oldAssumptions.moveTo(assumptions);

        cout << "checked" << endl;

        if(status == l_True)
            cout << "true" << endl;
        if(status == l_False)
            cout << "false" << endl;
        if(status == l_Undef)
            cout << "undef" << endl;

        return (status == l_True);
    }

    std::vector<slvec > MaxSatSolver::findConflicts(slvec b, slvec c, slvec &cprime, int depth, string parent, int n) {

        std::vector<slvec > conflicts;

        //isConsistent (B u C)?

        slvec bc = aspino::merge(b, c);


        stringstream sstm;
        sstm << "n" << runs << "_" << parent << "_" << depth << "_" << n;
        string node = sstm.str();

        if (isConsistent(bc)) {
            //return < C, EmptySet >
            cprime = bc;
            //cout << "consistent" << endl;
            if(depth > 0)
                cout << "graph " << parent << "->" << node << endl;

            cout << "graph " << node << "[label=\"[";
            for (std::vector<Lit>::const_iterator i = bc.begin(); i != bc.end(); ++i)
                cout << *i << ' ';
            cout << "]\"];" << endl;
            cout << "graph " << node << "[color=green]" << endl;
            return conflicts;
        }

        cout << "graph " << node << "[color=red]" << endl;
        //cout << "!consistent" << endl;

        //|C| = 1
        if (c.size() == 1) {
            // return < EmptySet, { C } >
            cprime.clear();
            conflicts.push_back(c);
//            dbg(c);
            //cout << "|c| == 1" << endl;
            if(depth > 0)
                cout << "graph " << parent << "->" << node << endl;

            cout << "graph " << node << "[label=\"[";
            for (std::vector<Lit>::const_iterator i = bc.begin(); i != bc.end(); ++i)
                cout << *i << ' ';
            cout << "]\"];" << endl;
            cout << "graph " << node << "[color=blue]" << endl;
            return conflicts;
        }

        //cout << "|c| != 1" << endl;

        std::vector<std::vector<Lit> > conflicts1, conflicts2;
        slvec c1, c2, c1prime, c2prime;

        //Split C into disjoint sets
        aspino::split(c, c1, c2);

        //Find Conflicts in subsets
        if(c1.size() > 0)
        {
            //cout << "split" << endl;
            conflicts1 = findConflicts(b, c1, c1prime, depth+1, node, 1);
        }

        if(c2.size() > 0)
        {
            //cout << "split 2" << endl;
            conflicts2 = findConflicts(b, c2, c2prime, depth+1, node, 2);
        }

        slvec merged = aspino::merge(c1prime, c2prime);

        merged = aspino::merge(merged, b);

        cout << "merged at depth " << depth << " " << c1prime.size() <<  " " << c2prime.size() << endl;
        for (int i = 0; i<merged.size(); i++)
            cout << merged[i] << ' ';
        cout << endl;

        cout << "merged" << endl;
        //Line 10
        conflicts = conflicts1;
        conflicts.insert(conflicts.end(), conflicts2.begin(), conflicts2.end());
        cout << "inserted" << endl;
        cout << "c1p " << c1prime.size() << endl;

        //while (C1' u C2' u B) is not consistent
        while (!isConsistent(merged)) {
            cout << "c1p " << c1prime.size() << endl;

            //cout << "not consistent" << endl;
            //X = GetConflict
            slvec bc2 = aspino::merge(b, c2prime);
            cout << "getting conflicts" << endl;
            slvec x = getConflict(bc2, c2prime, c1prime);

            cout << "bc2 " << bc2.size() << endl;
            cout << "c2prime " << c2prime.size() << endl;
            cout << "c1prime " << c1prime.size() << endl;
            cout << "x " << x.size() << endl;

            //CS = X u GetConflict
            slvec bx = aspino::merge(b, x);


            cout << "bx " << bx.size() << endl;
            cout << "x " << x.size() << endl;

            //cout << "getting conflicts 2" << endl;
            slvec cs = getConflict(bx, x, c2prime);
            cs = aspino::merge(cs, x);

            cout << "c1prime " << c1prime.size() << " - c2prime " << c2prime.size() << " - cs " << cs.size() << endl;


            // remove elements of X from C1'
            cout << "remove all" << endl;
            c1prime = aspino::removeAll(c1prime, x);

            cout << "removed " << x.size() << endl;

            //add conflict sets
            //cout << "add conflicts" << endl;
            conflicts.push_back(cs);

            merged = aspino::merge(c1prime, c2prime);
            merged = aspino::merge(merged, b);
        }

        if(depth > 0)
            cout << "graph " << parent << "->" << node << endl;

        cout << "graph " << node << "[label=\"[";
        for (std::vector<Lit>::const_iterator i = bc.begin(); i != bc.end(); ++i)
            cout << *i << ' ';
        cout << "][";
        for (std::vector<Lit>::const_iterator i = merged.begin(); i != merged.end(); ++i)
            cout << *i << ' ';
        cout << "]";
        for (int i = 0; i < conflicts.size(); i++)
        {
            cout << "(";
            for(int j = 0; j< conflicts[i].size(); j++)
                cout << conflicts[i][j] << " ";
            cout << ")";
        }
        cout <<  "\"];" << endl;

        //return <C1' u C2', conflicts >
        cprime = aspino::merge(c1prime, c2prime);
        return conflicts;
    }


    void MaxSatSolver::mergexplainMinimizeStd(int64_t limit) {

        dbg("mergeXplain");

        cout << "conflict size before: " << conflict.size() << endl;
        cout << "conflict before: " << endl;
        for(int j = 0; j< conflict.size(); j++)
            cout << conflict[j] << " ";
        cout << endl;



        //cout << "mxp" << endl;
        assert(decisionLevel() == 0);
        // disabled for now, such that we enter MXP also for the |c| = 1 case
        //if (conflict.size() <= 1) return;

//        double cpuTime = Glucose::cpuTime() - lastCallCpuTime;
//        uint64_t budget = conflicts - lastConflict;
//        uint64_t budgetMin = budget / cpuTime;
//        if (budget > budget * 30 / cpuTime) budget = budget * 30 / cpuTime;
//        trace(maxsat, 10,
//              "Minimize core of size " << conflict.size() << " (" << (conflicts - lastConflict) << " conflicts; " <<
//              cpuTime << " seconds; each check with budget " << budget << ")");
//        trim();

        //disabled for now, |c| = 1 case
        //if (budget == 0) return;

        cout << "conflict size: " << conflict.size() << endl;
        vec<Lit> core;
        conflict.copyTo(core);
        cout << "core size: " << core.size() << endl;
        //slvec cc = stdCast(conflict);

//        isConsistent(core);
//        exit(0);


        vec<Lit> allAssumptions;
        for (int i = 0; i < core.size(); i++) allAssumptions.push(~core[i]);

        //this->dump("MergeXplain Minimize Std");
        //cout << "mxp ass" << endl;
        dbg(core);

        //MergeXPlain
        //If !isConsistent(B) - for the moment, we have no B
        //If isConsistent(B u C) - already handled at this point

        slvec b, cp; //cp is cprime at end, not relevant
        slvec score = aspino::stdCast(allAssumptions);
        //cout << "mxp conf" << endl;
        runs++;
        std::vector<std::vector<Lit> > conflicts = findConflicts(b, score, cp, 0, "", 1);

        //dbg(conflicts.size());
        //cout << conflicts.size() << endl;

        //MXP found conflicts, use the first found minimal core for the moment
        if(conflicts.size() >= 1) {
            cout << "Number of Conflicts " << conflicts.size() << endl;
            std::vector<Lit> cf = conflicts[0];
            lvec cfg;
            vecCast(cf, cfg);
            cout << "before sort: " << endl;
//            for(int j = 0; j< cfg.size(); j++)
//            {
//                cout << cfg[j] << " ";
//            }
//
//            sort(cfg);
//            cout << "after sort: " << cfg.size() << endl;
//            for(int j = 0; j< cfg.size(); j++)
//            {
//                cout << cfg[j] << " ";
//            }

            cout << endl;

            assumptions.clear();

            cancelUntil(0);

            cout << "conflicts after: " << endl;
            for(int j = 0; j< conflicts.size(); j++)
            {
                std::vector<Lit> cc = conflicts[j];
                for(int k = 0; k<cc.size(); k++)
                    cout << cc[k] << " ";
                cout << endl;
            }
            cout << endl;

            //cfg.moveTo(conflict);
            conflict.clear();
            for (int i = 0; i < cfg.size(); i++) conflict.push(~cfg[i]);
        }
        else {
            cout << "Number of Conflicts " << conflicts.size() << endl;
            cout << "fu" << endl;
            cancelUntil(0);
            core.moveTo(conflict);
        }

        cout << "conflict after: " << conflict.size() << endl;
        for(int j = 0; j< conflict.size(); j++)
            cout << conflict[j] << " ";
        cout << endl;

        cout << "assumptions after: " << endl;
        for(int j = 0; j< assumptions.size(); j++)
            cout << assumptions[j] << " ";
        cout << endl;

       // exit(0);
    }

}