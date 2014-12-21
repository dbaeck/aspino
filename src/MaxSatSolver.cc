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

#include "MaxSatSolver.h"

#include <mtl/Map.h>
#include <core/Dimacs.h>

#include "utils/algorithm.h"

using Glucose::Map;

bool validate_maxsat_strat(const char* name, const string& value) {
    if(value == "one") return true;
    if(value == "one_neg") return true;
    if(value == "pmres") return true;
    if(value == "pmres_split_conj") return true;
    if(value == "pmres_log") return true;
    cerr << "Invalid value for --" << name << ": " << value << "\n";
    return false;
}
DEFINE_string(maxsat_strat, "one", "Set optimization strategy. Valid values: one, one_neg, pmres, pmres_log, pmres_split_conj.");

bool validate_maxsat_disjcores(const char* name, const string& value) {
    if(value == "no") return true;
    if(value == "pre") return true;
    if(value == "all") return true;
    cerr << "Invalid value for --" << name << ": " << value << "\n";
    return false;
}
DEFINE_string(maxsat_disjcores, "pre", "Set disjunct unsatisfiable cores policy. Valid values: no, pre, all.");

DEFINE_bool(maxsat_saturate, false, "Eliminate all cores of weight W before considering any core of level smaller than W.");
DEFINE_bool(maxsat_printmodel, true, "Print optimal model if found.");

namespace aspino {

template<class B>
static long parseLong(B& in) {
    long    val = 0;
    bool    neg = false;
    skipWhitespace(in);
    if      (*in == '-') neg = true, ++in;
    else if (*in == '+') ++in;
    if (*in < '0' || *in > '9') fprintf(stderr, "PARSE ERROR! Unexpected char: %c\n", *in), exit(3);
    while (*in >= '0' && *in <= '9')
        val = val*10 + (*in - '0'),
        ++in;
    return neg ? -val : val; }

    
MaxSatSolver::MaxSatSolver() : lowerbound(0) {
    if(FLAGS_maxsat_strat == "one") corestrat = &MaxSatSolver::corestrat_one;
    else if(FLAGS_maxsat_strat == "one_neg") corestrat = &MaxSatSolver::corestrat_one_neg;
    else if(FLAGS_maxsat_strat == "pmres") corestrat = &MaxSatSolver::corestrat_pmres;
    else if(FLAGS_maxsat_strat == "pmres_split_conj") corestrat = &MaxSatSolver::corestrat_pmres_split_conj;
    else if(FLAGS_maxsat_strat == "pmres_log") corestrat = &MaxSatSolver::corestrat_pmreslog;
    else assert(0);
    
    if(FLAGS_maxsat_disjcores == "no") disjcores = NO;
    else if(FLAGS_maxsat_disjcores == "pre") disjcores = PRE;
    else if(FLAGS_maxsat_disjcores == "all") disjcores = ALL;
    else assert(0);
    
    saturate = FLAGS_maxsat_saturate;
    setIncrementalMode();
}

MaxSatSolver::~MaxSatSolver() {
}

void MaxSatSolver::sameSoftVar(Lit soft, long weight) {
    assert(weights[var(soft)] != 0);
    assert(decisionLevel() == 0);
    int pos = 0;
    for(int i = 0; i < softLiterals.size(); i++, pos++) if(var(softLiterals[i]) == var(soft)) break;
    
    if(softLiterals[pos] == soft) {
        weights[var(soft)] += weight;
        return;
    }
        
    if(weights[var(soft)] == weight) {
        lowerbound += weight;
        setFrozen(var(soft), false);
        int j = 0;
        for(int i = 0; i < softLiterals.size(); i++) {
            if(softLiterals[i] == ~soft) continue;
            softLiterals[j++] = softLiterals[i];
        }
        assert(softLiterals.size() == j+1);
        softLiterals.shrink_(1);
    }else if(weights[var(soft)] < weight) {
        lowerbound += weights[var(soft)];
        for(int i = 0; i < softLiterals.size(); i++) if(softLiterals[i] == ~soft) { softLiterals[i] = soft; break; }
        weights[var(soft)] = weight - weights[var(soft)];
    }
    else {
        assert(weights[var(soft)] > weight);
        lowerbound += weight;
        weights[var(soft)] -= weight;
    }
}

void MaxSatSolver::addWeightedClause(vec<Lit>& lits, long weight) {
    Lit soft;
    if(lits.size() == 1)
        soft = lits[0];
    else {
        newVar();
        weights.push(0);
        soft = mkLit(nVars()-1);
        lits.push(~soft);
        addClause_(lits);
    }

    assert(weights.size() == nVars());
    if(weights[var(soft)] != 0) {
        sameSoftVar(soft, weight);
        return;
    }
    
    softLiterals.push(soft);
    assert_msg(weights[var(soft)] == 0, lits);
    weights[var(soft)] = weight;
    setFrozen(var(soft), true);
}
    
void MaxSatSolver::parse(gzFile in_) {
    Minisat::StreamBuffer in(in_);

    bool weighted = false;
    long top = -1;
    long weight = 1;
    
    vec<Lit> lits;
    int vars = 0;
    int count = 0;
    for(;;) {
        skipWhitespace(in);
        if(*in == EOF) break;
        if(*in == 'p') {
            ++in;
            if(*in != ' ') cerr << "PARSE ERROR! Unexpected char: " << static_cast<char>(*in) << endl, exit(3);
            ++in;
            if(*in == 'w') { weighted = true; ++in; }
            
            if(eagerMatch(in, "cnf")) {
                vars = parseInt(in);
                inClauses = parseInt(in);
                if(weighted && *in != '\n') top = parseInt(in);
                
                nInVars(vars);
                while(weights.size() < nInVars()) weights.push(0);
                while(nVars() < nInVars()) newVar();
            }
            else {
                cerr << "PARSE ERROR! Unexpected char: " << static_cast<char>(*in) << endl, exit(3);
            }
        }
        else if(*in == 'c')
            skipLine(in);
        else {
            count++;
            if(weighted) weight = parseLong(in);
            readClause(in, *this, lits);
            if(weight == top) addClause_(lits);
            else addWeightedClause(lits, weight);
        }
    }
    if(count != inClauses)
        cerr << "WARNING! DIMACS header mismatch: wrong number of clauses." << endl;
}

long MaxSatSolver::setAssumptions(long limit) {
    assumptions.clear();
    cancelUntil(0);
    long next = limit;
    int j = 0;
    for(int i = 0; i < softLiterals.size(); i++) {
        long w = weights[var(softLiterals[i])];
        if(w == 0) continue;
        if(w + lowerbound > upperbound) {
            addClause(softLiterals[i]);
            trace(maxsat, 10, "Hardening of " << softLiterals[i] << " of weight " << weights[var(softLiterals[i])]);
            weights[var(softLiterals[i])] = 0;
            continue;
        }
        softLiterals[j++] = softLiterals[i];
        if(var(softLiterals[i]) >= lastSoftLiteral) continue;
        if(w >= limit)
            assumptions.push(softLiterals[i]);
        else if(next == limit || w > next)
            next = w;
    }
    softLiterals.shrink_(softLiterals.size()-j);
    return next;
}

void MaxSatSolver::preprocess() {
    assert(clauses.size() < inClauses);
    for(int i = 0; i < inClauses; i++) {
        Clause& clause = ca[clauses[i]];
        long min = LONG_MAX;
        for(int j = 0; j < clause.size(); j++) {
            if(weights[var(clause[j])] == 0) { min = LONG_MAX; break; }
                
            int pos = 0;
            for(int k = 0; k < softLiterals.size(); k++, pos++) if(var(softLiterals[k]) == var(clause[j])) break;
            
            if(softLiterals[pos] == clause[j]) { min = LONG_MAX; break; }
            
            if(weights[var(clause[j])] < min) min = weights[var(clause[j])];
        }
        if(min == LONG_MAX) continue;
        
        conflict.clear();
        for(int j = 0; j < clause.size(); j++) conflict.push(clause[j]);
        trace(maxsat, 4, "Analyze conflict of size " << conflict.size() << " and weight " << min);
        lowerbound += min;
        cout << "o " << lowerbound << endl;
        (this->*corestrat)(min);
    }

}

lbool MaxSatSolver::solve() {
    inClauses = clauses.size();
    upperbound = LONG_MAX;
    cout << "o " << lowerbound << endl;
    detectLevels();

    for(;;) {
        assert(levels.size() > 0);
        lbool ret = solveCurrentLevel();
        if(ret == l_False) {
            cout << "s UNSATISFIABLE" << endl;
            return l_False;
        }
        assert(ret == l_True);
        
        if(lowerbound == upperbound) break;
        
        cancelUntil(0);
        for(int i = 0; i < softLiterals.size(); i++) {
            assert(weights[var(softLiterals[i])] + lowerbound > upperbound);
            addClause(softLiterals[i]);
            trace(maxsat, 10, "Hardening of " << softLiterals[i] << " of weight " << weights[var(softLiterals[i])]);
            weights[var(softLiterals[i])] = 0;
        }
    }
    
    while(levels.size() > 0) { delete levels.last(); levels.pop(); }
    
    cout << "s OPTIMUM FOUND" << endl;
    copyModel();
    if(FLAGS_maxsat_printmodel) printModel();
    return l_True;
}

lbool MaxSatSolver::solveCurrentLevel() {
    trace(maxsat, 1, "Solve level " << levels.size());
    levels.last()->moveTo(softLiterals);
    delete levels.last();
    levels.pop();
    weightOfPreviousLevel.pop();
    
//    preprocess();
    
    numberOfCores = sizeOfCores = 0;

    lastSoftLiteral = disjcores == NO ? INT_MAX : nVars();
    firstLimit = LONG_MAX;
    
    int iteration = 1;

    trace(maxsat, 2, "Iteration " << iteration);
    lbool ret = solve_();
    trace(maxsat, 2, "Bounds after iteration " << iteration << ": [" << lowerbound << ":" << upperbound << "]");

    if(ret == l_False) return l_False;
    assert(ret == l_True);

    while(lastSoftLiteral < nVars()) {
        iteration++;
        lastSoftLiteral = disjcores == ALL ? nVars() : INT_MAX;
        trace(maxsat, 2, "Iteration " << iteration);
        ret = solve_();
        trace(maxsat, 2, "Bounds after iteration " << iteration << ": [" << lowerbound << ":" << upperbound << "]");
        assert(ret == l_True);
    }

    return l_True;
}

lbool MaxSatSolver::solve_() {
    long limit = firstLimit;
    long nextLimit;
    bool foundCore = false;
    
    bool allowToSkip = true;
    
    for(;;) {
        //aspino::shuffle(softLiterals);
        nextLimit = setAssumptions(limit);
        
        trace(maxsat, 2, "Solve with " << assumptions.size() << " assumptions. Current bounds: [" << lowerbound << ":" << upperbound << "]");
        trace(maxsat, 20, "Assumptions: " << assumptions);
        
        if(assumptions.size() == 0 && upperbound != LONG_MAX) status = l_Undef;
        else PseudoBooleanSolver::solve();
        
        if(status != l_False) {
            if(status == l_True) updateUpperBound();
            
            if(saturate && lastSoftLiteral < nVars()) {
                lastSoftLiteral = nVars();
                trace(maxsat, 8, "Continue on limit " << limit << " considering " << (nVars() - lastSoftLiteral) << " more literals");
                continue;
            }
            
            if(nextLimit == limit) {
                trace(maxsat, 4, "SAT! No other limit to try");
                return l_True;
            }
            
            trace(maxsat, 4, "SAT! Decrease limit to " << nextLimit);
            limit = nextLimit;
            if(!foundCore) firstLimit = limit;

            continue;
        }
        
        foundCore = true;
        
        trace(maxsat, 2, "UNSAT! Conflict of size " << conflict.size());
        trace(maxsat, 10, "Conflict: " << conflict);
        
        if(conflict.size() == 0) return l_False;
        
        cancelUntil(0);
//        trim();
        minimize();
        addClause(conflict);

//        numberOfCores++;
//        sizeOfCores += conflict.size();
//        if(allowToSkip && conflict.size() > 3*sizeOfCores/numberOfCores/2) {
//            trace(maxsat, 4, "This is a huge core of size " << conflict.size() << " (previous average is " << sizeOfCores/numberOfCores << ")! Skip it and shuffle soft literals");
//            addClause(conflict);
//            shuffle(softLiterals, random_seed);
//            allowToSkip = false;
//            continue;
//        }
//        allowToSkip = true;
        
        assert(conflict.size() > 0);
        
        trace(maxsat, 4, "Analyze conflict of size " << conflict.size() << " and weight " << limit);
        lowerbound += limit;
        cout << "o " << lowerbound << endl;
        (this->*corestrat)(limit);
    }
}

void MaxSatSolver::updateUpperBound() {
    assert(weightOfPreviousLevel.size() > 0);
    long newupperbound = lowerbound;
    for(int i = 0; i < softLiterals.size(); i++)
        if(value(softLiterals[i]) == l_False) newupperbound += weights[var(softLiterals[i])];
    for(int i = 0; i < levels.size(); i++) {
        vec<Lit>& v = *levels[i];
        for(int j = 0; j < v.size(); j++)
            if(value(v[j]) == l_False) newupperbound += weights[var(v[j])];
    }
    if(newupperbound < upperbound) {
        upperbound = newupperbound;
        cout << "c ub " << upperbound << endl;
    }
}

void MaxSatSolver::trim() {
    assert(decisionLevel() == 0);
    
    if(conflict.size() <= 1) return;

    do{
        addClause(conflict);
        assumptions.clear();
        for(int i = 0; i < conflict.size(); i++) assumptions.push(~conflict[i]);
        PseudoBooleanSolver::solve();
        assert(status == l_False);
        trace(maxsat, 4, "Trim " << assumptions.size() - conflict.size() << " literals from conflict");
        trace(maxsat, 10, "Conflict: " << conflict);
        cancelUntil(0);
        if(conflict.size() <= 1) return;
    }while(assumptions.size() > conflict.size());
    
    assert(conflict.size() > 1);
}

void MaxSatSolver::minimize() {
    assert(decisionLevel() == 0);
    if(conflict.size() <= 1) return;
    
    static int64_t previousConflicts = 0;
    int64_t budget = 100 * (conflicts - previousConflicts) / 100;
    
    static vec<lbool> required;
    required.growTo(nVars(), l_False);
    for(int i = 0; i < conflict.size(); i++) required[var(conflict[i])] = l_False;
    
    vec<Lit> core;
    conflict.moveTo(core);

    int requiredAssumptions = 0;
    
    for(;;) {
        assumptions.shrink_(assumptions.size()-requiredAssumptions);
        int nUnknown = 0;
        for(int i = 0; i < core.size(); i++) {
            if(required[var(core[i])] != l_Undef) continue;
            assumptions.push(~core[i]);
            nUnknown++; 
        }
        Lit last = lit_Undef;
        for(int i = 0; i < core.size(); i++) {
            if(required[var(core[i])] != l_False) continue;
            if(last != lit_Undef) assumptions.push(~last);
            last = core[i];
        }
        if(last == lit_Undef) break;
        
        trace(maxsat, 10, "Try to minimize with " << assumptions.size() << " assumptions (" << requiredAssumptions << " required, ie. " << 100*requiredAssumptions/(assumptions.size()+1) << "% of the core; " << nUnknown << " unknowns, ie. " << 100*nUnknown/(assumptions.size()+1) << "% of the core)");
        setConfBudget(budget);
        PseudoBooleanSolver::solve();
        

        if(status == l_False) {
//            cancelUntil(0);
//            addClause(conflict);
            
            conflict.moveTo(core);
            
            reverse(core);
            
            continue;
        }
        
        required[var(last)] = status;
        if(status == l_True) {
            assumptions[requiredAssumptions++] = ~last;
        }
    }
    
    core.moveTo(conflict);
    cancelUntil(0);
    
    budgetOff();
    previousConflicts = conflicts;
}

void MaxSatSolver::corestrat_one(long limit) {
    trace(maxsat, 10, "Use algorithm one");
    WeightConstraint wc;
    wc.bound = conflict.size() - 1;
    while(conflict.size() > 0) {
        weights[var(conflict.last())] -= limit;
        wc.lits.push(~conflict.last());
        wc.coeffs.push(1);
        conflict.pop();
    }
    assert(conflict.size() == 0);
    for(int i = 0; i < wc.bound; i++) {
        newVar();
        if(i != 0) addClause(~softLiterals.last(), mkLit(nVars()-1));
        setFrozen(nVars()-1, true);
        weights.push(limit);
        softLiterals.push(mkLit(nVars()-1));
        wc.lits.push(~mkLit(nVars()-1));
        wc.coeffs.push(1);
    }
    
    if(wc.size() > 1) addConstraint(wc);
}

void MaxSatSolver::corestrat_one_neg(long limit) {
    trace(maxsat, 10, "Use algorithm one");
    WeightConstraint wc;
    wc.bound = conflict.size() - 1;
    while(conflict.size() > 0) {
        weights[var(conflict.last())] -= limit;
        wc.lits.push(~conflict.last());
        wc.coeffs.push(1);
        conflict.pop();
    }
    assert(conflict.size() == 0);
    for(int i = 0; i < wc.bound; i++) {
        newVar();
        if(i != 0) addClause(~softLiterals.last(), ~mkLit(nVars()-1));
        setFrozen(nVars()-1, true);
        weights.push(limit);
        softLiterals.push(~mkLit(nVars()-1));
        wc.lits.push(mkLit(nVars()-1));
        wc.coeffs.push(1);
    }
    
    if(wc.size() > 1) addConstraint(wc);
}

void MaxSatSolver::corestrat_pmres(long limit) {
    trace(maxsat, 10, "Use algorithm pmres");

    Lit prec = lit_Undef;
    vec<Lit> lits;
    while(conflict.size() > 0) {
        weights[var(conflict.last())] -= limit;
        
        if(prec == lit_Undef) prec = ~conflict.last();
        else {
            // disjunction
            newVar();
            setFrozen(nVars()-1, true);
            weights.push(limit);
            softLiterals.push(mkLit(nVars()-1));
            lits.push(prec);
            lits.push(~conflict.last());
            lits.push(mkLit(nVars()-1, true));
            addClause(lits);
            lits.clear();
            addClause(~prec, mkLit(nVars()-1));
            addClause(conflict.last(), mkLit(nVars()-1));
            
            if(conflict.size() == 0) break;

            // conjunction
            newVar();
            weights.push(0);
            addClause(prec, mkLit(nVars()-1, true));
            addClause(~conflict.last(), mkLit(nVars()-1, true));
            lits.push(~prec);
            lits.push(conflict.last());
            lits.push(mkLit(nVars()-1));
            addClause(lits);
            lits.clear();
            
            prec = mkLit(nVars()-1);
        }
        
        conflict.pop();
    }
    assert(conflict.size() == 0);
}

void MaxSatSolver::corestrat_pmres_split_conj(long limit) {
    trace(maxsat, 10, "Use algorithm pmres_split_conj");

    vec<Lit> lits;
    for(int i = 0; i < conflict.size(); i++) {
        weights[var(conflict[i])] -= limit;
        
        if(i == 0) continue;
        
        newVar();
        setFrozen(nVars()-1, true);
        weights.push(limit);
        softLiterals.push(mkLit(nVars()-1));
        
        for(int j = 0; j < i; j++) {
            lits.push(mkLit(nVars()-1, true));
            lits.push(~conflict[j]);
            lits.push(~conflict[i]);
            addClause_(lits);
            lits.clear();
        }
        
        addClause(mkLit(nVars()-1), conflict[i]);
        lits.push(mkLit(nVars()-1));
        for(int j = 0; j < i; j++) lits.push(conflict[j]);
        addClause_(lits);
        lits.clear();
    }
}

void MaxSatSolver::corestrat_pmreslog(long limit) {
    trace(maxsat, 10, "Use algorithm pmreslog");

    vec<Lit> lits;
    while(conflict.size() > 1) {
        int j = 0;
        for(int i = 0; i < conflict.size(); i += 2) {
            if(i+1 == conflict.size()) { conflict[j++] = conflict[i]; break; }
            
            weights[var(conflict[i])] -= limit;
            assert(weights[var(conflict[i])] >= 0);
            weights[var(conflict[i+1])] -= limit;
            assert(weights[var(conflict[i+1])] >= 0);
            
            // disjunction
            newVar();
            setFrozen(nVars()-1, true);
            weights.push(limit);
            softLiterals.push(mkLit(nVars()-1));
            lits.push(~conflict[i]);
            lits.push(~conflict[i+1]);
            lits.push(mkLit(nVars()-1, true));
            addClause(lits);
            lits.clear();
            addClause(conflict[i], mkLit(nVars()-1));
            addClause(conflict[i+1], mkLit(nVars()-1));
            
            if(conflict.size() <= 2) return;
            
            // conjunction
            newVar();
            weights.push(limit);
            addClause(~conflict[i], mkLit(nVars()-1, true));
            addClause(~conflict[i+1], mkLit(nVars()-1, true));
            lits.push(conflict[i]);
            lits.push(conflict[i+1]);
            lits.push(mkLit(nVars()-1));
            addClause(lits);
            lits.clear();
            
            conflict[j++] = mkLit(nVars()-1, true);
        }
        conflict.shrink_(conflict.size()-j);
    }
    assert(conflict.size() == 1);
    weights[var(conflict[0])] -= limit;
}

void MaxSatSolver::detectLevels() {
    vec<long> allWeights;
    Map<long, vec<Lit>*> wMap;
    for(int i = 0; i < softLiterals.size(); i++) {
        Lit lit = softLiterals[i];
        long w = weights[var(lit)];
        if(!wMap.has(w)) { wMap.insert(w, new vec<Lit>()); allWeights.push(w); }
        wMap[w]->push(lit);
    }
    
    int n = allWeights.size();
    while(n > 0) {
        int newn = 0;
        for(int i = 1; i < n; i++) {
            if(allWeights[i-1] > allWeights[i]) {
                long tmp = allWeights[i-1];
                allWeights[i-1] = allWeights[i];
                allWeights[i] = tmp;
                newn = i;
            }
        }
        n = newn;
    }
    
    assert(levels.size() == 0);
    
    long cumulative = 0;
    for(int i = 0; i < allWeights.size(); i++) {
        long w = allWeights[i];
        if(w > cumulative) { levels.push(new vec<Lit>()); weightOfPreviousLevel.push(cumulative); }
        vec<Lit>& v = *wMap[w];
        for(int j = 0; j < v.size(); j++) levels.last()->push(v[j]);
        cumulative += w * v.size();
        delete wMap[w];
    }
    weightOfPreviousLevel.push(cumulative);
    
    trace(maxsat, 1, "Detected " << levels.size() << " levels");
    if(levels.size() == 0) {
        trace(maxsat, 2, "Add fake, empty level");
        levels.push(new vec<Lit>());
        weightOfPreviousLevel.push(cumulative);
    }
    assert(weightOfPreviousLevel.size() == levels.size() + 1);
}

} // namespace aspino
