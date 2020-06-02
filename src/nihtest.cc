/*
  nihtest.cc -- main program
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

#ifdef _MSC_VER
// We're okay with using the incredibly insecure function getenv().
// (Yes, it's not thread save, and we're not multi-threaded.)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "nihtest.h"

#include <iostream>
#include <string>

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
#include "getopt_long.h"
#endif

#include "Configuration.h"
#include "Exception.h"
#include "Test.h"

static const std::string usage_tail = " [-hqVv] [-C config] [--keep-broken] [--no-cleanup] [--setup-only] [VARIABLE=VALUE ...] testcase\n";

static const std::string help_head = PACKAGE " by Dieter Baron and Thomas Klausner\n\n";

static const std::string version_string = PACKAGE " " VERSION "\n"
    "Copyright (C) 2020 Dieter Baron and Thomas Klausner\n"
    PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n";

static const std::string help_tail = "\n"
    "  -C, --config-file  Use the argument as config file\n"
    "  -h, --help         display this help message and exit\n"
    "      --keep-broken  keep sandbox if test fails\n"
    "      --no-cleanup   keep sandbox\n"
    "  -q, --quiet        don't print test results\n"
    "      --setup-only   set up sandbox, but don't run test\n"
    "  -v, --verbose      print detailed test results\n"
    "  -V, --version      display version number and exit\n";

enum {
    OPT_KEEP_BROKEN = 256,
    OPT_NO_CLEANUP,
    OPT_SETUP_ONLY
};

#define OPTIONS "ChVv"

struct option options[] = {
    { "help", 0, 0, 'h' },
    { "version", 0, 0, 'V' },
    
    { "config-file", 0, 0, 'C' },
    { "keep-broken", 0, 0, OPT_KEEP_BROKEN },
    { "no-cleanup", 0, 0, OPT_NO_CLEANUP },
    { "quiet", 0, 0, 'q' },
    { "setup-only", 0, 0, OPT_SETUP_ONLY },
    { "verbose", 0, 0, 'v' },
    { NULL, 0, 0, 0 }
};
    

static void print_usage(std::ostream &stream);

int main(int argc, char *argv[]) {
    int c;
    auto keep_sandbox = Configuration::NEVER;
    auto keep_sandbox_set = false;
    auto print_results = Configuration::NEVER;
    auto print_results_set = false;
    std::string configuration_file = "nihtest.conf";
    bool run_test = true;
    
    setprogname(argv[0]);
    
    opterr = 0;
    while ((c = getopt_long(argc, argv, OPTIONS, options, 0)) != EOF) {
        switch (c) {
        case 'h':
            std::cout << help_head;
            print_usage(std::cout);
            std::cout << help_tail;
            exit(0);
            
        case 'V':
            std::cout << version_string;
            exit(0);
            
        case 'C': // config-file
            configuration_file = argv[optind];
            break;

        case 'q': // quiet
            print_results = Configuration::NEVER;
            print_results_set = true;
            break;
            
        case 'v': // verbose
            print_results = Configuration::ALWAYS;
            print_results_set = true;
            break;
            
        case OPT_KEEP_BROKEN:
            keep_sandbox = Configuration::WHEN_FAILED;
            keep_sandbox_set = true;
            break;
            
        case OPT_NO_CLEANUP:
            keep_sandbox = Configuration::ALWAYS;
            keep_sandbox_set = true;
            break;
            
        case OPT_SETUP_ONLY:
            keep_sandbox = Configuration::ALWAYS;
            keep_sandbox_set = true;
            run_test = false;
            break;
            
        default:
            print_usage(std::cerr);
            exit(1);
        }
    }
    
    if (optind != argc - 1) {
        print_usage(std::cerr);
        exit(1);
    }
    
    if (!print_results_set) {
        auto value = getenv("VERBOSE");
        if (value != NULL) {
            print_results = Configuration::ALWAYS;
            print_results_set = true;
        }
    }

    if (!keep_sandbox_set) {
        auto value = getenv("KEEP_BROKEN");
        if (value != NULL) {
            keep_sandbox = Configuration::WHEN_FAILED;
            keep_sandbox_set = true;
        }
    }
    
    try {
        auto configuration = Configuration(configuration_file);
        
        if (keep_sandbox_set) {
            configuration.keep_sandbox = keep_sandbox;
        }
        if (print_results_set) {
            configuration.print_results = print_results;
        }
        
        auto test = Test(argv[optind], configuration);
        
        test.run_test = run_test;
        exit(test.run());
    }
    catch (Exception e) {
        if (e.print_message) {
            std::cerr << getprogname() << ": " << e.what() << "\n";
        }
        exit(Test::ERROR);
    }
}

static void print_usage(std::ostream &stream) {
    stream << "Usage: " << getprogname() << usage_tail;
}
