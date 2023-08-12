#include "manager.h"
#include <signal.h>

void handle_pipe(int sig)
{
    //no operator
}

int main(){
    struct sigaction action;
    action.sa_handler = handle_pipe;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, NULL);

    Manager::GetInstance()->init();
    return 0;
}