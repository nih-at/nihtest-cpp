/*
  Test.cc -- main class for a test case
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

#include "Test.h"

#include <fstream>
#include <iostream>
#include <regex>

#include "Exception.h"
#include "OS.h"
#include "TestParser.h"

Test::Directive::Directive(const std::string name_, const std::string usage_, int minimum_arguments_, bool only_once_, bool required_, int maximum_arguments_) : name(name_), usage(usage_), only_once(only_once_), required(required_), minimum_arguments(minimum_arguments_) {
    maximum_arguments = maximum_arguments_ == 0 ? minimum_arguments : maximum_arguments_;
}

const std::vector<Test::Directive> Test::directives = {
    Test::Directive("args", "[arg ...]", 0, true, true, -1),
    Test::Directive("description", "text", -1, true),
    Test::Directive("features", "feature ...", 1, true, false, -1),
    Test::Directive("file", "test in out", 3),
    Test::Directive("file-del", "test in", 2),
    Test::Directive("file-new", "test out", 2),
    Test::Directive("mkdir", "mode name", 2),
    Test::Directive("pipefile", "file", 1, true),
    Test::Directive("precheck", "command [args ...]", 1, false, false, -1),
    Test::Directive("preload", "library", 1, true),
    Test::Directive("program", "name", 1, true),
    Test::Directive("return", "exit-code", 1, true, true),
    Test::Directive("setenv", "variable value", 2),
    Test::Directive("stderr", "text", -1),
    Test::Directive("stderr-replace", "pattern replacement", 2, true),
    Test::Directive("stdin", "text", -1),
    Test::Directive("stdout", "text", -1),
    Test::Directive("touch", "mtime file", 2),
    Test::Directive("ulimit", "limit value", 2)
};


void Test::initialize(const std::string &test_case, const Variables &variables) {
    sandbox_directory = variables.get("SANDBOX_DIRECTORY");
    source_directory = variables.get("SOURCE_DIRECTORY");
    top_build_directory = variables.get("TOP_BUILD_DIRECTORY");
    
    auto file_name = test_case;
    auto dot = test_case.find('.');
    if (dot != std::string::npos) {
        name = test_case.substr(0, dot);
    }
    else {
        name = test_case;
        file_name = test_case + ".test";
    }
    // TODO: strip dir from test name
    
    auto test_file_name = find_file(file_name);
    
    auto parser = TestParser(test_file_name, this, directives);
    
    parser.parse();

    // TODO: read pipefile into input
    
    if (program.empty()) {
        program = variables.get("DEFAULT_PROGRAM");
        if (program.empty()) {
            throw Exception("no program specified");
        }
    }
}


void Test::enter_sandbox() {
    if (in_sandbox) {
	throw Exception("already in sandbox");
    }

    sandbox_name = OS::make_temp_directory(sandbox_directory, "sandbox_" + name);

    OS::change_directory(sandbox_name);
}


std::string Test::find_file(const std::string &name) {
    if (OS::file_exists(name)) {
        return name;
    }
    
    if (!source_directory.empty()) {
        auto source_name = make_filename(source_directory, name);
        if (OS::file_exists(source_name)) {
            return source_name;
        }
    }
    
    throw Exception("can't find input file '" + name + "'");
}


int Test::get_int(const std::string &string) {
    // TODO: error handling
    return std::stoi(string.c_str());
}


bool Test::has_feature(const std::string &name) {
    if (!features) {
        features = read_features();
    }
    
    return features->is_set(name);
}


void Test::leave_sandbox(bool keep) {
    // TODO: implement
    return;
}


std::string Test::make_filename(const std::string &directory, const std::string name) const {
    std::string real_directory;
    if (OS::is_absolute(directory)) {
        real_directory = directory;
    }
    else {
        real_directory = OS::append_path_component(directory, "..");
    }
    return OS::append_path_component(real_directory, name);
}


void Test::process_directive(const Directive *directive, const std::vector<std::string> &args) {
    if (directive->name == "args") {
        arguments = args;
    }
    else if (directive->name == "features") {
        required_features = args;
    }
    else if (directive->name == "file") {
        files.push_back(File(args[0], args[1], args[2]));
    }
    else if (directive->name == "file-del") {
        files.push_back(File(args[0], args[1], ""));
    }
    else if (directive->name == "file-new") {
        files.push_back(File(args[0], "", args[1]));
    }
    else if (directive->name == "mkdir") {
        if (directories.find(args[1]) != directories.end()) {
            throw Exception("duplicate mkdir for '" + args[1], "'");
        }
        directories[args[1]] = get_int(args[2]);
    }
    else if (directive->name == "pipefile") {
        if (!input.empty()) {
            throw Exception("only one of 'pipefile' or 'stdin' allowed");
        }
        pipe_file = args[0];
    }
    else if (directive->name == "precheck") {
        precheck_command = args;
    }
    else if (directive->name == "preload") {
        preload_library = args[0];
    }
    else if (directive->name == "program") {
        program = args[0];
    }
    else if (directive->name == "return") {
        exit_code = get_int(args[0]);
    }
    else if (directive->name == "setenv") {
        if (environment.find(args[0]) != environment.end()) {
            throw Exception("duplicate setenv for '" + args[0], "'");
        }
        environment[args[0]] = args[1];
    }
    else if (directive->name == "stderr") {
        error_output.push_back(args[0]);
    }
    else if (directive->name == "stderr-replace") {
        // TODO: error_output_pattern;
        error_output_replacement = args[1];
    }
    else if (directive->name == "stdin") {
        if (!pipe_file.empty()) {
            throw Exception("only one of 'pipefile' or 'stdin' allowed");
        }
        input.push_back(args[0]);
    }
    else if (directive->name == "stdout") {
        output.push_back(args[0]);
    }
    else if (directive->name == "touch") {
        if (touch_files.find(args[1]) != touch_files.end()) {
            throw Exception("duplicate setenv for '" + args[1], "'");
        }
        touch_files[args[1]] = get_int(args[0]);
    }
    else if (directive->name == "ulimit") {
        if (args[0].size() != 1) {
            throw Exception("invalid limit '" + args[0] + "'");
        }
        auto limit = args[0][0];
        // TODO: check for valid limit
        if (limits.find(limit) != limits.end()) {
            throw Exception("duplicate limit for '" + args[0] + "'");
        }
        // TODO: handle unlimited
        limits[limit] = get_int(args[1]);
    }
}


VariablesPointer Test::read_features() {
    VariablesPointer features(new Variables());
    
    auto config_file_name = make_filename(top_build_directory, "config.h");
    
    auto config_file = std::ifstream(config_file_name);
    if (!config_file) {
        throw Exception("cannot open config header '" + config_file_name + "'", true);
    }
    
    auto define = std::regex("^#define ([_A-Za-z0-9]*)$");
    
    std::string line;
    while (std::getline(config_file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, define)) {
            features->set(match.str(1), "YES");
        }
    }

    return features;
}


Test::Result Test::run(void) {
    if (!required_features.empty()) {
	read_features();
	for (auto feature : required_features) {
	    if (!features->is_set(feature)) {
		return SKIPPED;
	    }
	}
    }

    // TODO: skip if preload &c not supported
    
    enter_sandbox();

    try {
	for (auto file : files) {
	    if (!file.input.empty()) {
		OS::copy_file(find_file(file.input), file.name);
	    }
	}

	// TODO: setenv, limits, &c

	std::vector<std::string> error_output_got;
	std::vector<std::string> output_got;

	auto exit_code_got = OS::run_command(program, arguments, environment, input, &output_got, &error_output_got);

        if (exit_code != exit_code_got) {
            // TODO: output
            failed = true;
        }
        
	// TODO: implement
    }
    catch (Exception e) {
	leave_sandbox(keep_sandbox != NEVER);
        throw;
    }

    leave_sandbox(keep_sandbox == ALWAYS || (keep_sandbox == WHEN_BROKEN && failed));
    return failed ? FAILED : PASSED;
}
