/*
  Parser.h -- parse config file or test case
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

#ifndef HAD_PARSER_H
#define HAD_PARSER_H

#include <fstream>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

class ParserConsumer;

class Parser {
public:
    struct Directive {
        std::string name;
        std::string usage;
        bool only_once;
        bool required;
        int minimum_arguments;
        int maximum_arguments;
        
        Directive(const std::string name_, const std::string usage_, int minimum_arguments_, bool required_ = false, bool only_once = false, int maximum_arguments_ = 0);
    };
    
    Parser(const std::string &file_name_, ParserConsumer *consuemr_, const std::vector<Directive> &directives_);
    
    void parse();
    
private:
    void print_error(const std::string &message);
    void tokenize(std::vector<std::string> *args, const std::string &line, std::string::size_type start);
    
    ParserConsumer *consumer;
    std::string file_name;
    size_t line_no;
    std::ifstream file;
    bool ok;
    std::unordered_set<const Directive *> seen_directives;
    
    std::map<std::string, const Directive *> directives;
};

class ParserConsumer {
public:
    virtual void process_directive(const Parser::Directive *directive, const std::vector<std::string> &args) = 0;
};

#endif // HAD_PARSER_H
