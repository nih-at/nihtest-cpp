/*
  Config.cc - nihtest configuratin
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

#include "Configuration.h"

#include "Exception.h"
#include "Parser.h"

const std::vector<Parser::Directive> Configuration::directives = {
    Parser::Directive("default-program", "directory", 1, true),
    Parser::Directive("file-compare", "test-extension source-extension command [args ...]", 3, false, false, -1),
    Parser::Directive("keep-sandbox", "when", 1, true),
    Parser::Directive("print-results", "when", 1, true),
    Parser::Directive("sandbox-directory", "directory", 1, true),
    Parser::Directive("source-directory", "directory", 1, true),
    Parser::Directive("top-build-directory", "directory", 1, true)
};

Configuration::Configuration(const std::string &file_name) : keep_sandbox(NEVER), print_results(WHEN_FAILED) {
    try {
        auto parser = Parser(file_name, this, directives);
        try {
            parser.parse();
        }
        catch (Exception e) {
            // parse errors are fatal
            throw;
        }
    }
    catch (Exception e) {
        // config file is optional, so ignore errors opening it
        return;
    }
}


void Configuration::process_directive(const Parser::Directive *directive, const std::vector<std::string> &args) {
    if (directive->name == "default-program") {
        default_program = args[0];
    }
    else if (directive->name == "file-compare") {
        std::string key = args[0] + "." + args[1];
        if (file_compare.find(key) != file_compare.end()) {
            throw Exception("duplicate file-compare for extensions '" + args[0] + "' and '" + args[1] + "'");
        }
        std::vector<std::string> command;
        command.insert(command.begin(), args.begin() + 2, args.end());
        file_compare[key] = command;
    }
    else if (directive->name == "keep-sandbox") {
        keep_sandbox = get_when(args[0]);
    }
    else if (directive->name == "print-results") {
        print_results = get_when(args[0]);
    }
    else if (directive->name == "sandbox-directory") {
        sandbox_directory = args[0];
    }
    else if (directive->name == "source-directory") {
        source_directory = args[0];
    }
    else if (directive->name == "top-build-directory") {
        top_build_directory = args[0];
    }
}


Configuration::When Configuration::get_when(const std::string &arg) {
    if (arg == "never") {
        return NEVER;
    }
    else if (arg == "failed") {
        return WHEN_FAILED;
    }
    else if (arg == "always") {
        return ALWAYS;
    }
    else {
        throw Exception("unknown setting '" + arg + "'");
    }
}
