//
// Created by Daniel Baeck on 30.10.15.
//

#ifndef ASPINO_ALGORITHMTESTS_H
#define ASPINO_ALGORITHMTESTS_H

#include <cxxtest/TestSuite.h>
#include <vector>
#include <utils/algorithm.h>
#include <mtl/Vec.h>

class AlgorithmTests: public CxxTest::TestSuite {
public:
    void testAddition(void)
    {
        TS_ASSERT(1 + 1 > 1);
        TS_ASSERT_EQUALS(1 + 1, 2);
    }

    void testVecMerge()
    {
        Glucose::vec<int> a, b, c;
        a.push(1);
        a.push(2);
        b.push(4);
        b.push(3);
        aspino::merge(a, b, c);
        TS_ASSERT_EQUALS(c[0], 1);
        TS_ASSERT_EQUALS(c[1], 2);
        TS_ASSERT_EQUALS(c[2], 4);
        TS_ASSERT_EQUALS(c[3], 3);
    }

    void testStdVecMerge()
    {
        std::vector<int> a, b, c;
        a.push_back(1);
        a.push_back(2);
        b.push_back(4);
        b.push_back(3);
        c = aspino::merge(a, b);
        TS_ASSERT_EQUALS(c[0], 1);
        TS_ASSERT_EQUALS(c[1], 2);
        TS_ASSERT_EQUALS(c[2], 4);
        TS_ASSERT_EQUALS(c[3], 3);
    }

    void testVecSplit()
    {
        Glucose::vec<int> a, b, c;
        a.push(1);
        a.push(2);
        a.push(4);
        a.push(3);
        aspino::split(a, b, c);
        TS_ASSERT_EQUALS(b[0], 1);
        TS_ASSERT_EQUALS(b[1], 2);
        TS_ASSERT_EQUALS(c[0], 4);
        TS_ASSERT_EQUALS(c[1], 3);
    }

    void testStdVecSplit()
    {
        std::vector<int> a, b, c;
        a.push_back(1);
        a.push_back(2);
        a.push_back(4);
        a.push_back(3);
        aspino::split(a, b, c);
        TS_ASSERT_EQUALS(b[0], 1);
        TS_ASSERT_EQUALS(b[1], 2);
        TS_ASSERT_EQUALS(c[0], 4);
        TS_ASSERT_EQUALS(c[1], 3);
    }
};


#endif //ASPINO_ALGORITHMTESTS_H
