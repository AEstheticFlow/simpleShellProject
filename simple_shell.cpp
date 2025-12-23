#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <vector>
#include <sstream>
#include <fcntl.h>
#include <algorithm>

using namespace std;

/* =====================================================
   TEAM MEMBER 1
   String Utilities & Parsing Helpers
   - cleanToken
   - split
   - buildArgs
   ===================================================== */

/*
  cleanToken:
  - removes quotes (' and ")
  - trims extra spaces
*/
string cleanToken(string s) {
    s.erase(remove(s.begin(), s.end(), '\"'), s.end());
    s.erase(remove(s.begin(), s.end(), '\''), s.end());

    size_t start = s.find_first_not_of(" \t");
    size_t end   = s.find_last_not_of(" \t");

    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

/*
  split:
  - splits input string by delimiter (;, &&, |)
*/
vector<string> split(const string& s, const string& delim) {
    vector<string> tokens;
    size_t prev = 0, pos;

    while ((pos = s.find(delim, prev)) != string::npos) {
        tokens.push_back(cleanToken(s.substr(prev, pos - prev)));
        prev = pos + delim.length();
    }
    tokens.push_back(cleanToken(s.substr(prev)));
    return tokens;
}

/*
  buildArgs:
  - converts vector<string> to char* array for execvp
*/
vector<char*> buildArgs(vector<string>& args) {
    vector<char*> cargs;
    for (auto& a : args)
        if (!a.empty())
            cargs.push_back(const_cast<char*>(a.c_str()));
    cargs.push_back(nullptr);
    return cargs;
}

/* =====================================================
   TEAM MEMBER 2
   Core Command Execution (No fork)
   - executeBase
   ===================================================== */

/*
  executeBase:
  - parses command
  - handles redirection
  - executes execvp directly
  - used inside pipe
*/
void executeBase(string cmd) {
    stringstream ss(cmd);
    string token;
    vector<string> args;

    int out_fd = -1;
    int err_fd = -1;

    while (ss >> token) {
        if (token == ">" || token == "1>") {
            ss >> token;
            out_fd = open(token.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        else if (token == ">>" || token == "1>>") {
            ss >> token;
            out_fd = open(token.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else if (token == "2>") {
            ss >> token;
            err_fd = open(token.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        else if (token == "2>>") {
            ss >> token;
            err_fd = open(token.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else {
            args.push_back(cleanToken(token));
        }
    }

    if (out_fd != -1) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }

    if (err_fd != -1) {
        dup2(err_fd, STDERR_FILENO);
        close(err_fd);
    }

    auto cargs = buildArgs(args);
    execvp(cargs[0], cargs.data());

    perror("exec failed");
    exit(1);
}

/* =====================================================
   TEAM MEMBER 3
   Single Command Execution
   - execSingle
   ===================================================== */

/*
  execSingle:
  - forks process
  - child executes command
  - parent waits and returns exit status
*/
int execSingle(string cmd) {
    pid_t pid = fork();

    if (pid == 0) {
        executeBase(cmd);
    }

    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

/* =====================================================
   TEAM MEMBER 4
   Pipe Handling
   - execPipe
   ===================================================== */

/*
  execPipe:
  - connects two commands using pipe
  - first writes to pipe
  - second reads from pipe
  - returns exit status of second command
*/
int execPipe(string left, string right) {
    int fd[2];
    pipe(fd);

    pid_t p1 = fork();
    if (p1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        executeBase(left);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        executeBase(right);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(p1, nullptr, 0);
    int status;
    waitpid(p2, &status, 0);

    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

/* =====================================================
   TEAM MEMBER 5
   Prompt Display
   - printPrompt
   ===================================================== */

/*
  printPrompt:
  - displays shell prompt
  - format: [username@hostname:current_directory]$
*/
void printPrompt() {
    char cwd[1024], host[1024];
    getcwd(cwd, sizeof(cwd));
    gethostname(host, sizeof(host));

    passwd* pw = getpwuid(getuid());

    cout << "[" << (pw ? pw->pw_name : "user")
         << "@" << host
         << ":" << cwd
         << "]$ " << flush;
}

/* =====================================================
   TEAM MEMBER 6 & 7
   Main Shell Logic
   - command loop
   - handling ; && |
   ===================================================== */

int main() {
    string input;

    while (true) {
        printPrompt();

        if (!getline(cin, input) || input == "exit")
            break;

        if (input.empty())
            continue;

        // split by ;
        auto commands = split(input, ";");

        for (auto& cmd : commands) {

            // split by &&
            auto andParts = split(cmd, "&&");

            for (auto& part : andParts) {

                int status = 0;

                if (part.find('|') != string::npos) {
                    size_t pos = part.find('|');
                    status = execPipe(
                        part.substr(0, pos),
                        part.substr(pos + 1)
                    );
                }
                else {
                    status = execSingle(part);
                }

                if (status != 0 && andParts.size() > 1)
                    break;
            }
        }
    }
    return 0;
}