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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>

#include "CompareArrays.h"
#include "CompareFiles.h"
#include "Exception.h"
#include "OS.h"
#include "Parser.h"

const std::vector<Parser::Directive> Test::directives = {
    Parser::Directive("args", "[arg ...]", 0, true, false, -1),
    Parser::Directive("description", "text", -1, true),
    Parser::Directive("features", "feature ...", 1, true, false, -1),
    Parser::Directive("file", "test in [out]", 2, false, false, 3),
    Parser::Directive("file-del", "test in", 2),
    Parser::Directive("file-new", "test out", 2),
    Parser::Directive("mkdir", "mode name", 2),
    Parser::Directive("precheck", "command [args ...]", 1, false, false, -1),
    Parser::Directive("preload", "library", 1, true),
    Parser::Directive("program", "name", 1, true),
    Parser::Directive("return", "exit-code", 1, true, true),
    Parser::Directive("setenv", "variable value", 2),
    Parser::Directive("stderr", "text", -1),
    Parser::Directive("stderr-replace", "pattern replacement", 2),
    Parser::Directive("stdin", "text", -1),
    Parser::Directive("stdin-file", "file", 1, true),
    Parser::Directive("stdout", "text", -1),
    Parser::Directive("touch", "mtime file", 2),
    Parser::Directive("ulimit", "limit value", 2)
};


Test::Test(const std::string &test_case, Configuration configuration_) : configuration(configuration_), run_test(true), in_sandbox(false), features_read(false) {
    auto file_name = test_case;
    name = OS::basename(test_case);
    auto dot = name.find('.');
    if (dot != std::string::npos) {
        name = name.substr(0, dot);
    }
    else {
        file_name += ".test";
    }
        
    auto test_file_name = find_file(file_name);
    
    auto parser = Parser(test_file_name, this, directives);
    
    parser.parse();
    
    if (program.empty()) {
        program = configuration.default_program;
        if (program.empty()) {
            throw Exception("no program specified");
        }
    }

    if (!directories.empty()) {
        throw Exception("mkdir not implemented yet");
    }
    if (!limits.empty()) {
        throw Exception("ulimit not implemented yet");
    }
    if (!touch_files.empty()) {
        throw Exception("touch not implemented yet");
    }

    std::sort(files.begin(), files.end());
    
    rewrite_lines(error_output_replace, &error_output);
}

void Test::compare_arrays(const std::vector<std::string> &expected, const std::vector<std::string> &got, const std::string &what) {
    auto compare = CompareArrays(expected, got, what, configuration.print_results != Configuration::NEVER);
    if (!compare.compare()) {
        failed.push_back(what);
    }
}


void Test::compare_files() {
    std::vector<std::string> files_got = OS::list_files(".");
    
    auto compare = CompareFiles(files, files_got, this, configuration.print_results != Configuration::NEVER);
    if (!compare.compare()) {
        failed.push_back("files");
    }
}


void Test::enter_sandbox() {
    if (in_sandbox) {
	throw Exception("already in sandbox");
    }

    sandbox_name = OS::make_temp_directory(configuration.sandbox_directory, "sandbox_" + name);

    OS::change_directory(sandbox_name);
    in_sandbox = true;
}


Test::Result Test::execute_test() {
    auto operating_system = OS::operating_system();
    if (!preload_library.empty()) {
        if (operating_system == "Darwin" || operating_system == "Windows") {
            return SKIPPED;
        }
    }
    
    // TODO: skip if limits &c not supported
    
    if (!required_features.empty()) {
        for (const auto &feature : required_features) {
            if (!has_feature(feature)) {
                return SKIPPED;
            }
        }
    }
    
    if (!precheck_command.empty()) {
        OS::Command command;
        command.program = find_file(precheck_command[0]);
        command.arguments.insert(command.arguments.begin(), precheck_command.begin() + 1, precheck_command.end());
        command.path.push_back(".");

        std::vector<std::string> output;
        std::vector<std::string> error_output;
        
        auto result = OS::run_command(&command, &output, &error_output);
        if (result != "0") {
            // TODO: print output if verbose?
            return SKIPPED;
        }
    }
    
    enter_sandbox();
    
    try {
        for (const auto &file : files) {
            if (!file.input.empty()) {
                OS::copy_file(find_file(file.input), file.name);
            }
        }
        
        std::vector<std::string> error_output_got;
        std::vector<std::string> output_got;
        
        OS::Command command;
        command.arguments = arguments;
        command.environments.push_back(&OS::standard_environment);
        if (!environment.empty()) {
            command.environments.push_back(&environment);
        }
        if (!input.empty()) {
            command.input = &input;
        }
        command.input_file = input_file;
        if (!limits.empty()) {
            command.limits = &limits;
        }
        command.path.push_back("..");
        command.path.push_back(OS::append_path_component(configuration.source_directory, ".."));
        command.preload_library = preload_library;
        command.program = program;

        auto exit_code_got = OS::run_command(&command, &output_got, &error_output_got);
        
        if (exit_code != exit_code_got) {
            failed.push_back("exit status");
            if (configuration.print_results != Configuration::NEVER) {
                std::cout << "Unexpected exit status:\n";
                std::cout << "-" << exit_code << "\n";
                std::cout << "+" << exit_code_got << "\n";
            }
        }
        
        std::vector<Replace> replacements;
        
        replacements.push_back(Replace(std::regex("^[^: ]*" + OS::basename(program) + ": "), ""));
        replacements.insert(replacements.end(), error_output_replace.begin(), error_output_replace.end());
        
        rewrite_lines(replacements, &error_output_got);
        
        compare_arrays(output, output_got, "output");
        compare_arrays(error_output, error_output_got, "error output");
        
        compare_files();
    }
    catch (Exception e) {
        leave_sandbox(configuration.keep_sandbox != Configuration::NEVER);
        throw;
    }
    
    leave_sandbox(configuration.keep_sandbox == Configuration::ALWAYS || (configuration.keep_sandbox == Configuration::WHEN_FAILED && !failed.empty()));
    return failed.empty() ? PASSED : FAILED;
}


std::string Test::find_file(const std::string &name) const {
    if (OS::is_absolute(name)) {
        return name;
    }
    
    std::string build_name;
    if (in_sandbox) {
        build_name = OS::append_path_component("..", name);
    }
    else {
        build_name = name;
    }
    
    if (OS::file_exists(build_name)) {
        return build_name;
    }
    
    if (!configuration.source_directory.empty()) {
        auto source_name = make_filename(configuration.source_directory, name);
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
    if (!features_read) {
        read_features();
    }
    
    return features.find(name) != features.end();
}


void Test::leave_sandbox(bool keep) {
    OS::change_directory("..");
    if (!keep) {
        OS::remove_directory(sandbox_name);
    }
    return;
}


std::string Test::make_filename(const std::string &directory, const std::string name) const {
    std::string real_directory;
    if (!in_sandbox || OS::is_absolute(directory)) {
        real_directory = directory;
    }
    else {
        real_directory = OS::append_path_component("..", directory);
    }
    return OS::append_path_component(real_directory, name);
}


void Test::process_directive(const Parser::Directive *directive, const std::vector<std::string> &args) {
    if (directive->name == "args") {
        arguments = args;
    }
    else if (directive->name == "features") {
        required_features = args;
    }
    else if (directive->name == "file") {
        if (args.size() == 2) {
            files.push_back(File(args[0], args[1], args[1]));
        }
        else {
            files.push_back(File(args[0], args[1], args[2]));
        }
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
    else if (directive->name == "stdin-file") {
        if (!input.empty()) {
            throw Exception("only one of 'stdin-file' or 'stdin' allowed");
        }
        input_file = args[0];
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
        exit_code = args[0];
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
        error_output_replace.push_back(Replace(std::regex(args[0]), args[1]));
    }
    else if (directive->name == "stdin") {
        if (!input_file.empty()) {
            throw Exception("only one of 'stdin-file' or 'stdin' allowed");
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


void Test::print_result(Result result) const {
    switch (result) {
        case PASSED:
        case SKIPPED:
        if (configuration.print_results != Configuration::ALWAYS) {
                return;
            }
        case FAILED:
        case ERROR:
        if (configuration.print_results == Configuration::NEVER) {
                return;
            }
    }

    std::cout << name << " -- ";
    switch (result) {
        case PASSED:
            std::cout << "PASS";
            break;
            
        case SKIPPED:
            std::cout << "SKIP";
            break;
            
        case FAILED: {
            std::cout << "FAIL: ";
            auto first = true;
            for (const auto &type : failed) {
                if (first) {
                    first = false;
                }
                else {
                    std::cout << ", ";
                }
                std::cout << type;
            }
            break;
        }
            
        case ERROR:
            std::cout << "ERROR";
    }
    std::cout << "\n";
}


void Test::read_features() {
    auto config_file_name = make_filename(configuration.top_build_directory, "config.h");
    
    auto config_file = std::ifstream(config_file_name);
    if (!config_file) {
        throw Exception("cannot open config header '" + config_file_name + "'", true);
    }
    
    auto define = std::regex("^#define HAVE_([_A-Za-z0-9]*)$");
    
    std::string line;
    while (std::getline(config_file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, define)) {
            features.insert(match.str(1));
        }
    }
}


void Test::rewrite_lines(const std::vector<Replace> &replacements, std::vector<std::string> *lines) {
    for (auto &line : *lines) {
        for (const auto &replace : replacements) {
            line = std::regex_replace(line, replace.pattern, replace.replacement);
        }
    }
}


Test::Result Test::run() {
    auto result = execute_test();
    print_result(result);
    return result;
}
