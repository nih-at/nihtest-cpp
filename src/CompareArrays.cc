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
    bool done = false;
    const auto N = expected.size();
    const auto M = got.size();
    /* maximum size of differences accepted (could be set lower than M + N) */
    const auto max_size = M + N;
    int w[2*max_size+1];
    /* for retracing our steps and getting the actual diff */
    std::vector<std::vector<int> > path_lengths;
    /* re-index w so that v[-max_size .. max_size] are valid */
    /* v[k] = highest x we can reach after d non-diagonal steps, where y = x - k */
    auto v = w+max_size;
    int d;

    /* initialize variable, only needed for debugging */
    for (int i=0; i<2*max_size+1; i++) {
	w[i] = 999999999;
    }

    /* no differences before start of file */
    v[1] = 0;
    /* loop over all possible diagonals */
    for (d=0; d <= max_size; d++) {
	/* update path lengths */
	for (int k=-d; k<=d; k+=2) {
	    /* find best candidate for shortest path */
            /*
             * x -- offset into first array (expected), <= N
             * y -- offset into second array (got), <= M
	     */
	    int x, y;
	    
	    if (k == -d || (k != d && v[k-1] < v[k+1])) {
		/* step down, x does not increase */
		x = v[k+1];
	    } else {
		/* step right, x increases */
		x = v[k-1]+1;
	    }
	    y = x - k;
	    /* go diagonally as far as possible */
	    while (x < N && y < M && expected[x] == got[y]) {
		/* both lines are equal, so shortest path doesn't grow */
		x++;
		y++;
	    }
	    v[k] = x;
	    if (x >= N && y >= M) {
		done = true;
		break;
	    }
	}
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
	std::cout << std::endl;
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
	if (x == wdprev[max_size + k - 1] + 1) {
	    output(path_lengths, max_size, d-1, x-1, y);
	    print_line('-', expected[x-1]);
	    for (auto el : lines) {
		print_line(' ', el);
	    }
	    return;
	}
	if (x == wdprev[max_size + k + 1]) {
	    output(path_lengths, max_size, d-1, x, y-1);
	    print_line('+', got[y-1]);
	    for (auto el : lines) {
		print_line(' ', el);
	    }
	    return;
	}
	/* if possible, walk one step backward on the diagonal and try again */
	if (x > 0 && y > 0 && expected[x-1] == got[y-1]) {
	    /* for context */
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
