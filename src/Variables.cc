/*
  Variables.cc -- collection of variables with fallback to environment
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

#include "Variables.h"

#ifdef _MSC_VER
// We're okay with using the incredibly insecure function getenv().
// (Yes, it's not thread save, and we're not multi-threaded.)
#define _CRT_SECURE_NO_WARNINGS
#endif

void Variables::add(const std::string &assignment) {
    auto pos = assignment.find('=');
    if (pos == std::string::npos) {
        // TOOD: handle error
        return;
    }

    set(assignment.substr(0, pos), assignment.substr(pos + 1, std::string::npos));
}


std::string Variables::get(const std::string &name) const {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }

    if (use_environment) {
        auto env = getenv(name.c_str());
        if (env != NULL) {
            return env;
        }
    }
    
    return "";
}


const bool Variables::is_set(const std::string &name) const {
    return variables.find(name) != variables.end() || (use_environment && getenv(name.c_str()) != NULL);
}


void Variables::set(const std::string &name, const std::string &value) {
    variables[name] = value;
}
