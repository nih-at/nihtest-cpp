
/*
  TestParser.cc -- parse test case
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

#include "TestParser.h"

#include <iostream>

#include "Exception.h"

TestParser::TestParser(const std::string &file_name_, Test *test_, const std::vector<Test::Directive> &directives_) : test(test_), file_name(file_name_), line_no(0), ok(true) {
    file = std::ifstream(file_name);
    if (!file) {
        throw Exception("cannot open test case '" + file_name + "'", true);
    }
    for (auto &directive : directives_) {
        directives[directive.name] = &directive;
    }
}


void TestParser::parse() {
    std::string line;
    while (std::getline(file, line)) {
        line_no += 1;
        if (line[0] == '#') {
            continue;
        }
        auto space = line.find(' ');
        auto command = line.substr(0, space);
        auto it = directives.find(command);
        if (it == directives.end()) {
            print_error("unknown directive '" + command + "'");
            continue;
        }
        
        auto directive = it->second;
        
        if (directive->only_once && seen_directives.find(directive) != seen_directives.end()) {
            print_error("directive '" + command + "' only allowed once");
            continue;
        }
        
        seen_directives.insert(directive);

        std::vector<std::string> args;
        
        if (directive->minimum_arguments == -1) {
            if (space != std::string::npos) {
                args.push_back(line.substr(space + 1));
            }
            else {
                args.push_back("");
            }
        }
        else {
            if (space != std::string::npos) {
                tokenize(&args, line, space + 1);
            }
            if (args.size() < directive->minimum_arguments || args.size() > directive->maximum_arguments) {
                
            }
        }
        
        try {
            test->process_directive(directive, args);
        }
        catch (Exception e) {
            print_error(e.what());
        }
    }
    
    for (auto pair : directives) {
        auto directive = pair.second;
        if (directive->required && seen_directives.find(directive) == seen_directives.end()) {
            print_error("directive '" + directive->name + "' is requried");
        }
    }
    
    if (!ok) {
        throw Exception();
    }
}


void TestParser::print_error(const std::string &message) {
    std::cerr << file_name << ":" << line_no << ": " << message << "\n";
    ok = false;
}


void TestParser::tokenize(std::vector<std::string> *args, const std::string &line, std::string::size_type start) {
    auto whitespace = " \t";
    
    start = line.find_first_not_of(whitespace, start);
    auto end = line.size();
        
    while (start < end) {
        if (line[start] == '"') {
            std::string arg;
            start += 1;
            while (start < end && line[start] != '"') {
                auto next = line.find_first_of("\\\"", start);
                if (next == std::string::npos) {
                    start = end;
                    break;
                }
                arg += line.substr(start, next - start);
                if (line[next] == '"') {
                    start = next;
                    break;
                }
                else {
                    if (next == end - 1) {
                        print_error("incomplete backslash esacpe");
                        return;
                    }
                    switch (line[next + 1]) {
                    case '\\':
                    case '"':
                        arg += '\\';
                        break;
                        
                    case 'b':
                        arg += '\b';
                        break;
                        
                    case 'f':
                        arg += '\f';
                        break;
                        
                    case 'n':
                        arg += '\n';
                        break;

                    case 'r':
                        arg += '\r';
                        break;

                    case 't':
                        arg += '\t';
                        break;
                    }
                    start = next + 2;
                }
            }

            if (start == end) {
                print_error("unterminated quoted argument");
                return;
            }
            args->push_back(arg);
            start = line.find_first_not_of(whitespace, start + 1);
        }
        else {
            auto space = line.find(' ', start);
            if (space == std::string::npos) {
                args->push_back(line.substr(start));
                start = end;
            }
            else {
                args->push_back(line.substr(start, space-start));
                start = line.find_first_not_of(whitespace, space + 1);
            }
        }
    }
}
