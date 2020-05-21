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
#include <regex>

#include "Exception.h"
#include "OS.h"

void Test::initialize(const std::string &test_case, const Variables &variables) {
    top_build_directory = variables.get("TOP_BUILD_DIRECTORY");
    source_directory = variables.get("SOURCE_DIRECTORY");
    
    auto file_name = test_case;
    auto dot = test_case.find('.');
    if (dot != std::string::npos) {
        name = test_case.substr(0, dot);
    }
    else {
        name = test_case;
        file_name = test_case + ".test";
    }
    
    auto test_file_name = find_file(name);
    
    auto test_file = std::ifstream(test_file_name);
    if (!test_file) {
        throw Exception("cannot open test case '" + test_file_name + "'", true);
    }
    
    std::string line;
    
    while (std::getline(test_file, line)) {
        process_line(line);
    }
    
    // TODO: validate test case
}


bool Test::has_feature(const std::string &name) {
    if (!features) {
        features = read_features();
    }
    
    return features->is_set(name);
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

void Test::process_line(const std::string &line) {
    // TODO: implement
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


int Test::run(void) {
    // TODO: implement
    return 0;
}
