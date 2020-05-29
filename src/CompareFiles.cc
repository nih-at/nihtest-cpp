/*
  CompareFiles.cc -- compare files
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

#include "CompareFiles.h"

#include <iostream>

#include "OS.h"

bool CompareFiles::compare() {
    auto iter_expected = expected.cbegin();
    auto iter_got = got.cbegin();
    
    while (iter_expected != expected.cend() && iter_got != got.cend()) {
        if (iter_expected->output.empty()) {
            iter_expected++;
            continue;
        }
        
        if (iter_expected->name == *iter_got) {
            std::string key = OS::extension(iter_expected->name) + "." + OS::extension(iter_expected->output);
            
            auto compare = comparators->find(key);
            
            if (compare != comparators->end()) {
                compare_files(compare->second, iter_expected->name, iter_expected->output);
            }
            else {
                // TODO: compare binary files
            }

            iter_expected++;
            iter_got++;
        }
        else if (iter_expected->name < *iter_got) {
            print_line('-', iter_expected->name);
            iter_expected++;
        }
        else {
            print_line('+', *iter_got);
            iter_got++;
        }
    }
    
    while (iter_expected != expected.cend()) {
        if (!iter_expected->output.empty()) {
            print_line('-', iter_expected->name);
        }
        iter_expected++;
    }
    
    while (iter_got != got.cend()) {
        print_line('+', *iter_got);
        iter_got++;
    }
 
    return ok;
}


void CompareFiles::compare_files(const std::vector<std::string> &argv, const std::string &got, const std::string &expected) {
    OS::Command command;
    command.program = argv[0];
    command.arguments.insert(command.arguments.begin(), argv.begin() + 1, argv.end());
    command.arguments.push_back(got);
    command.arguments.push_back(test->find_file(expected));
    command.path.push_back("..");
    
    std::vector<std::string> output;
    std::vector<std::string> error_output;
    
    auto result = OS::run_command(&command, &output, &error_output);
    
    if (result != "0") {
        print_line('!', expected);
        if (verbose) {
            for (const auto &line : output) {
                std::cout << line << "\n";
            }
            for (const auto &line : error_output) {
                std::cout << line << "\n";
            }
        }
    }
}


void CompareFiles::print_line(char indicator, const std::string &line) {
    if (verbose) {
        if (ok) {
            std::cout << "Unexpected files:\n";
        }
    
        std::cout << indicator << line << "\n";
    }
    
    ok = false;
}
