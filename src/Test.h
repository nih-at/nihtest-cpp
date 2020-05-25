/*
  Test.h -- main class for a test case
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

#ifndef HAD_TEST_H
#define HAD_TEST_H

#include <string>
#include <regex>
#include <unordered_map>
#include <vector>

#include "Variables.h"

class Test {
public:
    enum When {
        NEVER,
        WHEN_BROKEN,
        ALWAYS
    };

    enum Result {
	PASSED,
	FAILED,
	SKIPPED
    };
    
    struct Directive {
        std::string name;
        std::string usage;
        bool only_once;
        bool required;
        int minimum_arguments;
        int maximum_arguments;
        
        Directive(const std::string name_, const std::string usage_, int minimum_arguments_, bool required_ = false, bool only_once = false, int maximum_arguments_ = 0);
    };

    Test() : keep_sandbox(NEVER), print_results(WHEN_BROKEN), run_test(true), in_sandbox(false) { }
    
    void initialize(const std::string &name, const Variables &variables);
    Result run(void);
    
    void process_directive(const Directive *directive, const std::vector<std::string> &args);
    
    std::string name;
    When keep_sandbox;
    When print_results;
    bool run_test;
    std::string sandbox_directory;
    std::string source_directory;
    std::string top_build_directory;
    VariablesPointer features;
    
private:
    struct File {
        std::string name;
        std::string input;
        std::string output;
        
        File(const std::string &name_, const std::string &input_, const std::string &output_) : name(name_), input(input_), output(output_) { }
    };
    
    static const std::vector<Directive> directives;
    
    void enter_sandbox();
    std::string find_file(const std::string &name);
    int get_int(const std::string &string);
    bool has_feature(const std::string &name);
    void leave_sandbox(bool keep);
    std::string make_filename(const std::string &directory, const std::string name) const;
    VariablesPointer read_features();
    
    bool in_sandbox;
    std::string sandbox_name;
    bool failed;

    std::vector<std::string> arguments;
    std::unordered_map<std::string, int> directories;
    std::unordered_map<std::string, std::string> environment;
    std::vector<std::string> error_output;
    std::regex error_output_pattern;
    std::string error_output_replacement;
    int exit_code;
    std::vector<File> files;
    std::vector<std::string> input;
    std::unordered_map<char, int> limits;
    std::vector<std::string> output;
    std::string pipe_file;
    std::vector<std::string> precheck_command;
    std::string preload_library;
    std::string program;
    std::vector<std::string> required_features;
    std::unordered_map<std::string, time_t> touch_files;
};

#endif // HAD_TEST_H
