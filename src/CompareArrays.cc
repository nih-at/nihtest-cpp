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

#include <iostream>

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
    
    auto exptected_iter = expected.cbegin();
    auto got_iter = got.cbegin();
    
    // TODO: find more common lines
    
    bool ok = true;
    while (exptected_iter != expected.cend() && got_iter != got.cend()) {
        if (*exptected_iter != *got_iter) {
            ok = false;
            
            print_line('-', *exptected_iter);
            print_line('+', *got_iter);
        }
        
        exptected_iter++;
        got_iter++;
    }

    while (exptected_iter != expected.cend()) {
        print_line('-', *exptected_iter);
        exptected_iter++;
    }
    
    while (got_iter != got.cend()) {
        print_line('+', *got_iter);
        got_iter++;
    }
    
    return ok;
}


void CompareArrays::print_line(char indicator, const std::string &line) {
    if (!printed_header) {
        std::cout << "Unexpected " << what << ":\n";
        printed_header = true;
    }
    
    std::cout << indicator << line << "\n";
}
