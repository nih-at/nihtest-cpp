/*
  OS-Unix.h -- Unix specific functions
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

#include "OS.h"

#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "Exception.h"

#define BUFFER_SIZE (1024 * 1024)

static nfds_t pollfds_remove(struct pollfd *fds, nfds_t nfds, nfds_t i) {
    if (i < nfds - 1) {
	memcpy(fds + i, fds + nfds - 1, sizeof(struct pollfd));
    }
    return nfds - 1;
}


class Buffer {
public:
    Buffer(const std::vector<std::string> &lines);
    Buffer(size_t size);
    ~Buffer();

    bool end() { return offset == size; }
    void get_lines(std::vector<std::string> *lines);
    bool write(int fd);
    void read(int fd);

  private:
    char *data;
    size_t size;
    size_t offset;
};


Buffer::Buffer(const std::vector<std::string> &lines) : size(0), offset(0) {
    for (const auto &line : lines) {
	size += line.size() + 1;
    }
    if ((data = (char *)malloc(size)) == NULL) {
	throw Exception("can't allocate buffer of size " + std::to_string(size));
    }

    for (const auto &line : lines) {
	memcpy(data + offset, line.c_str(), line.size());
	offset += line.size();
	data[offset++] = '\n';
    }

    offset = 0;
}


Buffer::Buffer(size_t size_) : size(size_), offset(0) {
    if ((data = (char *)malloc(size)) == NULL) {                                                                                               
        throw Exception("can't allocate buffer of size " + std::to_string(size));                                                              
    }
}


Buffer::~Buffer() {
    free(data);
}


void
Buffer::read(int fd) {
    auto n = ::read(fd, data + offset, size - offset);

    if (n < 0) {
	throw Exception("read error", true);
    }

    offset += static_cast<size_t>(n);

    if (offset == size) {
	// TODO: realloc instead of error?
	throw Exception("buffer full");
    }
}


bool
Buffer::write(int fd) {
    if (offset == size) {
	return true;
    }

    auto n = ::write(fd, data + offset, size - offset);

    if (n < 0) {
	throw Exception("write error", true);
    }

    offset += static_cast<size_t>(n);

    return offset == size;
}


void Buffer::get_lines(std::vector<std::string> *lines) {
    auto stream = std::stringstream(std::string(data, offset));

    std::string line;
    while (std::getline(stream, line)) {
	lines->push_back(line);
    }
}


class Pipe {
public:
    Pipe();
    ~Pipe();

    int read_fd;
    int write_fd;

    void close_read();
    void close_write();
};

Pipe::Pipe() {
    int fd[2];

    if (pipe(fd) < 0) {
	throw Exception("can't create pipe", true);
    }

    read_fd = fd[0];
    write_fd = fd[1];
}

Pipe::~Pipe() {
    close_read();
    close_write();
}


void Pipe::close_read() {
    if (read_fd >= 0) {
	close(read_fd);
	read_fd = -1;
    }
}


void Pipe::close_write() {
    if (write_fd >= 0) {
	close(write_fd);
	write_fd = -1;
    }
}


std::string OS::run_command(const Command *command, std::vector<std::string> *output, std::vector<std::string> *error_output) {
    Pipe pipe_output, pipe_error;
    std::shared_ptr<Pipe> pipe_input;
    int fd_input = -1;
    std::string preload_library;
    std::string program;
    
    if (is_absolute(command->program)) {
        if (file_exists(program)) {
            program = command->program;
        }
    }
    else {
        for (const auto &dir : command->path) {
            auto file = dir + "/" + command->program;
            if (file_exists(file)) {
                program = file;
                break;
            }
        }
    }
    if (program.empty()) {
        throw Exception("can't find program '" + command->program + "'");
    }
    
    if (!command->preload_library.empty()) {
        char *cwd_c = getcwd(NULL, 0);
        if (cwd_c == NULL) {
            throw Exception("can't get current directory", true);
        }
        auto dir = std::string(cwd_c) + "/..";
        free(cwd_c);

        auto preload_directory = OS::dirname(command->preload_library);
        auto preload_name = OS::basename(command->preload_library);
        
        if (preload_directory != ".") {
            dir += "/" + preload_directory;
        }
        preload_library = dir + "/.libs/" + preload_name;
        if (!OS::file_exists(preload_library)) {
            preload_library = dir + "/lib" + preload_name;
            if (!OS::file_exists(preload_library)) {
                throw Exception("preload library '" + command->preload_library + "' doesn't exist");
            }
        }
    }

    if (command->input != NULL) {
        pipe_input = std::make_shared<Pipe>();
    }
    else if (!command->input_file.empty()) {
        if ((fd_input = open(command->input_file.c_str(), O_RDONLY)) < 0) {
            throw Exception("can't open '" + command->input_file + "'", true);
        }
    }
    
    pid_t pid = fork();
    
    switch (pid) {
    case -1:
	throw Exception("can't fork", true);

    case 0: { // child
        if (pipe_input) {
            pipe_input->close_write();
        }
	pipe_output.close_read();
	pipe_error.close_read();

        auto ok = true;
        
        // TODO: handle pipein-file
        
        if (pipe_input) {
            if (dup2(pipe_input->read_fd, 0) < 0) {
                ok = false;
            }
            
        }
        else if (fd_input >= 0) {
            if (dup2(fd_input, 0) < 0) {
                ok = false;
            }
        }
        if (ok) {
            if (dup2(pipe_output.write_fd, 1) < 0 || dup2(pipe_error.write_fd, 2) < 0) {
                ok = false;
            }
        }
        if (!ok) {
	    std::string message = "can't set up standard file descriptors: " + std::string(strerror(errno)) + "\n";
	    write(pipe_error.write_fd, message.c_str(), message.size());
	    exit(17);
	}
        if (pipe_input) {
            pipe_input->close_read();
        }
        else if (fd_input >= 0) {
            close(fd_input);
        }
        pipe_output.close_write();
        pipe_error.close_write();

        if (command->environment != NULL) {
            for (const auto &pair : *command->environment) {
                setenv(pair.first.c_str(), pair.second.c_str(), 1);
            }
        }

	const char * argv[command->arguments.size() + 2];

	size_t index = 0;
	argv[index++] = command->program.c_str();
	for (const auto &arg : command->arguments) {
	    argv[index++] = arg.c_str();
	}
	argv[index++] = NULL;
        
        // TODO: set limits
        
        if (!preload_library.empty()) {
            setenv("LD_PRELOAD", preload_library.c_str(), 1);
        }

        execv(program.c_str(), const_cast<char *const *>(argv));
	std::cerr << "can't start program '" << command->program << "': " << strerror(errno) << "\n";
	exit(17);
    }

    default: { // parent
        if (pipe_input) {
            pipe_input->close_read();
        }
        if (fd_input >= 0) {
            close(fd_input);
        }
	pipe_output.close_write();
	pipe_error.close_write();

        std::shared_ptr<Buffer> buffer_input;
	auto buffer_output = Buffer(BUFFER_SIZE);
	auto buffer_error = Buffer(BUFFER_SIZE);

	struct pollfd fds[] = {
            { pipe_output.read_fd, POLLIN, 0 },
            { pipe_error.read_fd, POLLIN, 0 },
            { -1, POLLOUT, 0 }
	};
	nfds_t nfds = 2;

        if (pipe_input) {
            buffer_input = std::make_shared<Buffer>(*command->input);
            fds[nfds++].fd = pipe_input->write_fd;
        }

	while (nfds > 0) {
	    auto ret = poll(fds, nfds, -1); // TODO: timeout
	    if (ret < 0) {
		throw Exception("poll failed", true);
	    }

	    if (ret == 0) {
		// TODO: test timed out
	    }

	    for (nfds_t i = 0; i < nfds; i++) {
                if (fds[i].revents & POLLIN) {
                    if (fds[i].fd == pipe_output.read_fd) {
                        buffer_output.read(pipe_output.read_fd);
                    }
                    if (fds[i].fd == pipe_error.read_fd) {
                        buffer_error.read(pipe_error.read_fd);
                    }
                }
		if (fds[i].revents & POLLHUP) {
		    if (pipe_input && fds[i].fd == pipe_input->write_fd) {
			if (!buffer_input->end()) {
			    // TODO: stdin closed before we wrote all data
			}
		    }
		    // TODO: handle HUP
		    nfds = pollfds_remove(fds, nfds, i);
		    --i;
		    continue;
		}
		if (fds[i].revents & POLLOUT) {
		    if (pipe_input && fds[i].fd == pipe_input->write_fd) {
			if (buffer_input->write(pipe_input->write_fd)) {
                            pipe_input->close_write();
			    nfds = pollfds_remove(fds, nfds, i);
			    --i;
			    continue;
			}
		    }	
		}
	    }
	}

	buffer_output.get_lines(output);
	buffer_error.get_lines(error_output);

	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status)) {
	    return std::to_string(WEXITSTATUS(status));
	}
	else if (WIFSIGNALED(status)) {
            switch WTERMSIG(status) {
            case SIGABRT:
                return "SIGABRT";
            case SIGALRM:
                return "SIGALRM";
            case SIGBUS:
                return "SIGBUS";
            case SIGFPE:
                return "SIGFPE";
            case SIGHUP:
                return "SIGHUP";
            case SIGILL:
                return "SIGILL";
            case SIGINT:
                return "SIGINT";
            case SIGKILL:
                return "SIGKILL";
            case SIGPIPE:
                return "SIGPIPE";
            case SIGQUIT:
                return "SIGQUIT";
            case SIGSEGV:
                return "SIGSEGV";
            case SIGSYS:
                return "SIGSYS";
            case SIGTERM:
                return "SIGTERM";
            case SIGTRAP:
                return "SIGTRAP";
                
            default:
                return "unknown signal " + std::to_string(WTERMSIG(status));
            }
	}
	else {
	    // can this happen?!
	    return "unknown status " + std::to_string(status);
	}
    }
    }
}
