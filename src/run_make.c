#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "../include/pmake.h"

void execute_actions(Action *action) {

    while (action != NULL) {

        int n = fork();
        if (n < 0) {
            perror("fork");
            exit(1);
        }

        // Child
        else if (n == 0) {
            char buffer[MAXLINE];
            printf("%s\n", args_to_string(action->args, buffer, MAXLINE));

            execvp(action->args[0], action->args);
            perror("exec");
            exit(1);
        }

        // Parent
        else {
            int status;
            if (wait(&status) < 0) {
                perror("wait");
                exit(1);
            }
            // Child did not exit normally
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 1) {
                    perror("error executing action");
                    exit(1);
                }
            }
        }

        action = action->next_act;
    }

}

void evaluate_rule_seq(Rule *rule) {

    // Evaluate each dependency
    Dependency *dep = rule->dependencies;
    while (dep != NULL) {
        evaluate_rule_seq(dep->rule);
        dep = dep->next_dep;
    }

    // Compare last modified time for each dependency to the target
    // Execute rule if
    struct stat file_stat;

    if (stat(rule->target, &file_stat) != 0) {
        // 1. Target does not exist

        execute_actions(rule->actions);
        return;
    }

    // 2. At least one of the dependencies is more recent than target
    time_t rule_mtime = file_stat.st_mtime;

    while (dep != NULL) {
        stat(rule->target, &file_stat);
        time_t dep_mtime = file_stat.st_mtime;
        if (dep_mtime > rule_mtime) {

            execute_actions(rule->actions);
            return;
        }
        dep = dep->next_dep;

    }
    return;
}

void evaluate_rule_parr(Rule *rule) {

    // Create child for each dependency
    Dependency *dep = rule->dependencies;
    Dependency *dep2 = rule->dependencies;
    while (dep != NULL) {
        int n = fork();
        if (fork < 0) {
            perror("fork");
            exit(1);
        }

        // Child
        else if (n == 0) {

            // Make recursive call to update each dependency
            evaluate_rule_parr(dep->rule);
            return;
        }

        dep = dep->next_dep;
    }

    // Collecting all the children return values
    while (dep2 != NULL) {
        int status;

        if (wait(&status) < 0) {
            perror("wait");
            exit(1);
        }

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                perror("evaluating child dependency");
                exit(1);
            }

        }
        dep2 = dep2->next_dep;
    }

    // Update current dependency
    // Compare last modified time for each dependency to the target
    // Execute rule if
    struct stat file_stat;

    if (stat(rule->target, &file_stat) != 0) {
        // 1. Target does not exist
        execute_actions(rule->actions);
        return;
    }

    // 2. At least one of the dependencies is more recent than target
    time_t rule_mtime = file_stat.st_mtime;

    while (dep != NULL) {
        stat(rule->target, &file_stat);
        time_t dep_mtime = file_stat.st_mtime;
        if (dep_mtime > rule_mtime) {
            execute_actions(rule->actions);
            return;
        }
        dep = dep->next_dep;
    }


}

Rule *fetch_rule(char *target, Rule *first_rule) {
    while (first_rule != NULL) {
        if (strcmp(first_rule->target, target) == 0) {
            return first_rule;
        }
        first_rule = first_rule->next_rule;
    }
    return NULL;
}

/* Evaluate the rule in rules corresponding to the given target.
   If target is NULL, evaluate the first rule instead.
   If pflag is 0, evaluate each dependency in sequence.
   If pflag is 1, then evaluate each dependency in parallel (by creating one 
   new process per dependency). In this case, the parent process will wait until
   all child processes have terminated before checking dependency modified times
   to decide whether to execute the actions.
 */
void run_make(char *target, Rule *rules, int pflag) {

    // Evaluate dependencies in sequence
    if (pflag == 0) {

        if (target == NULL) {
            evaluate_rule_seq(rules);
            return;
        }

        Rule *rule = fetch_rule(target, rules);
        if (rule == NULL) {
            fprintf(stderr, "Invalid target\n");
            exit(1);
        }
        evaluate_rule_seq(rule);
        return;
    }
    // Evaluate dependencies in parallel
    else if (pflag == 1) {

        if (target == NULL) {
            evaluate_rule_parr(rules);
            return;
        }

        Rule *rule = fetch_rule(target, rules);
        if (rule == NULL) {
            fprintf(stderr, "Invalid target\n");
            exit(1);
        }
        evaluate_rule_parr(rule);
        return;

    }

}


