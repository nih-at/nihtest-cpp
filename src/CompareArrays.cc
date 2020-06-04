/*
  CompareArrays.cc -- compare two string arrays
  Copyright (C) 2020 Dieter Baron and Thomas Klausner

  This file is part of nihtest, regression tests for command line utilities.
  The authors can be contacted at <nihtest@nih.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CompareArrays.h"

#include <array>
#include <cstring>
#include <iostream>
#include <vector>

bool CompareArrays::compare() {
    if (verbose) {
        return compare_verbose();
    }
    else {
        return compare_quiet();
    }
}


bool CompareArrays::compare_quiet() {
    if (expected.size() != got.size()) {
        return false;
    }

    for (size_t i = 0; i < expected.size(); i++) {
        if (expected[i] != got[i]) {
            return false;
        }
    }

    return true;
}

bool CompareArrays::compare_verbose() {
    printed_header = false;

    if (compare_quiet()) {
	return true;
    }

    /* Algorithm from
     * Eugene W. Myers - A O(ND) Difference algorithm and Its Variations
     */
    /* The graph G consists of nodes [x, y] where x is the index into
     * the array of lines "expected" and y is the index into the array
     * of lines "got". G(x,y) always has outgoing edges to G(x+1, y)
     * and G(x, y+1) with weight 1. Such an edge corresponds to
     * accepting the line expected[x] (or got[y] resp.) as different
     * and printing it. Additionally, an edge G(x, y) -> G(x+1, y+1)
     * with weight 0 exists if expected[x] is equal to got[y]. Going
     * along such an edge corresponds to finding a matching line in
     * expected and got.
     *
     * The problem of finding the smallest diff corresponds to finding a
     * directed path from G(0, 0) to G(N, M) with minimal weight. */

    /* did we find a path from G(0, 0) to G(N, M)? */
    bool done = false;
    /* x dimension of G */
    const int N = static_cast<int>(expected.size());
    /* y dimension of G */
    const int M = static_cast<int>(got.size());
    /* upper limit on the number of differences this algorithm accepts
     * before giving up. If set to M+N like here it will never give up
     * and always find a path. */
    const int max_size = M + N;
    /* v[k] = highest x coordinate we can reach after d non-diagonal
     * steps, where y = x - k */
    /* v is updated in-place in each iteration */
    /* The array v in the paper is [-max_size ... +max_size]; w is v
       rebased by max_size, i.e. v[k] = w[max_size + k] */
    std::vector<int> w(2*max_size+1);

    /* keep copies of v/w from previous iterations for retracing our
     * steps and getting the actual diff */
    std::vector<std::vector<int> > path_lengths;
    /* number of non-diagonal steps needed = number of differences
     * between expected and got */
    int d;

    /* initialize w, only needed for debugging */
    for (int i=0; i<w.size(); i++) {
	w[i] = 999999999;
    }

    /* Initialize v[1] for first step of algorithm */
    w[max_size+1] = 0;
    /* loop over all possible differences, stop on the lowest one where we reach G(N, M) */
    for (d=0; d <= max_size; d++) {
	/* update v_d[k] using v_{d-1}[k] - can be done in-place, so only v[k] is used here */
	for (int k=-d; k<=d; k+=2) {
	    int x, y;
	
	    if (k == -d || (k != d && w[max_size+k-1] < w[max_size+k+1])) {
		/* step down, x does not increase */
		x = w[max_size+k+1];
	    } else {
		/* step right, x increases */
		x = w[max_size+k-1]+1;
	    }
	    y = x - k;
	    /* follow free edges (diagonal steps) as far as possible */
	    while (x < N && y < M && expected[x] == got[y]) {
		/* both lines are equal, so weight of path doesn't increase */
		x++;
		y++;
	    }
	    w[max_size+k] = x;
	    /* did we reach G(N, M)? if yes, stop */
	    if (x >= N && y >= M) {
		done = true;
		break;
	    }
	}

        /* save v_d[k] for generating diff */
	std::vector<int> dest;
	//	std::cout << "array of level " << d << ": ";
	//int j = -max_size;
	for (int i: w) {
	    dest.push_back(i);
	    // if (i < 10000) {
	    // std::cout << "[" << i << "/" << (i-j) << "] (" << j << "), ";
	    // }
	    //j++;
	}
	//std::cout << std::endl;
	path_lengths.push_back(dest);
	if (done) {
	    break;
	}
    }
    if (done) {
	output(path_lengths, max_size, d, N, M);
    }
    return false;
}

void CompareArrays::output(std::vector<std::vector<int> > &path_lengths, int max_size, int d, int x, int y) {
    std::vector<std::string> lines;
    if (d == 0) {
	return;
    }

    auto wdprev = path_lengths[d-1];
    auto k = x - y;
    bool change;

    /* walking backwards we cannot follow all diagonals completely,
     * going forward we might have entered in the middle of one */
    do {
	change = false;
	/* did we step right in the previous iteration? */
	if (x == wdprev[max_size + k - 1] + 1) {
	    output(path_lengths, max_size, d-1, x-1, y);
	    print_line('-', expected[x-1]);
	    for (auto el : lines) {
		print_line(' ', el);
	    }
	    return;
	}
	/* did we step down in the previous iteration? */
	if (x == wdprev[max_size + k + 1]) {
	    output(path_lengths, max_size, d-1, x, y-1);
	    print_line('+', got[y-1]);
	    for (auto el : lines) {
		print_line(' ', el);
	    }
	    return;
	}
	/* no match, so try going up diagonally one step and trying again */
	if (x > 0 && y > 0 && expected[x-1] == got[y-1]) {
	    /* unchanged lines, for context */
	    lines.insert(lines.begin(), expected[x-1]);
	    x--;
	    y--;
	    change = true;
	}
    } while (change);

    std::cerr << "internal error creating diff" << std::endl;
}


void CompareArrays::print_line(char indicator, const std::string &line) {
    if (!printed_header) {
        std::cout << "Unexpected " << what << ":\n";
        printed_header = true;
    }

    std::cout << indicator << line << "\n";
}
