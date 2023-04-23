#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/pmake.h"


/**
 * Returns 1 if there are no rules in the linked structure
 */
 int is_empty(Rule *first_rule) {
     if (first_rule == NULL) {
         return 1;
     }
     return 0;
 }

/**
 * Returns 0 if a rule with the name already exists, 1 otherwise
 */
int target_exists(Rule *first_rule, char name[MAXLINE]) {
    while (first_rule != NULL) {

        // There is a match
        if (strcmp(first_rule->target, name) == 0) {
            return 0;
        }

        first_rule = first_rule->next_rule;
    }
    return 1;
}

Rule *find_rule(Rule *first_rule, char name[MAXLINE]) {

    while (first_rule != NULL) {
        // There is a match
        if (strcmp(first_rule->target, name) == 0) {
            return first_rule;
        }
        first_rule = first_rule->next_rule;
    }
    return NULL;
}

/**
 * Creates and new Dependency and appends it to the end of the rule's linked dependency structure
 */
Dependency *add_dependency(Rule *rule) {

    Dependency *new_dependency = malloc(sizeof (Dependency));
    if (new_dependency == NULL) {
        perror("malloc");
        exit(1);
    }

    new_dependency->next_dep = NULL;

    // There is no previous dependency
    if (rule->dependencies == NULL) {
        rule->dependencies = new_dependency;
        return new_dependency;
    }

    // There is at least one previous dependency
    Dependency *curr_dependency = rule->dependencies;
    while(curr_dependency->next_dep != NULL) {
        curr_dependency = curr_dependency->next_dep;
    }
    curr_dependency->next_dep = new_dependency;
    return new_dependency;
}

/**
 * Sets the rule attribute of the dependency to point at the appropriate rule
 */
void link_dependency(Rule *first_rule, Dependency *dependency, char name[MAXLINE]) {

    Rule *appropriate_rule = find_rule(first_rule, name);
    dependency->rule = appropriate_rule;

}

/**
 * Creates and adds a new action to the end of the linked structure of the rule
 */
void add_action(Rule *rule, char** arguments) {
    // Create the new action
    Action *new_action = malloc(sizeof (Action));
    if (new_action == NULL) {
        perror("malloc");
        exit(1);
    }

    new_action->next_act = NULL;
    new_action->args = arguments;

    // Add the action to the linked structure
    // There is no previous action
    if (rule->actions == NULL) {
        rule->actions = new_action;
        return;
    }

    // There is at least one previous dependency
    Action *curr_action = rule->actions;
    while(curr_action->next_act != NULL) {
        curr_action = curr_action->next_act;
    }
    curr_action->next_act = new_action;

}

/**
 * Creates a new rule and adds it to the linked structure of rules
 * Returns a pointer to the new rule
 */
Rule *create_rule(Rule *first_rule, char name[MAXLINE]) {
    // Creating the Rule node
    Rule *new_rule = malloc(sizeof (Rule));
    if (new_rule == NULL) {
        perror("malloc");
        exit(1);
    }

    new_rule->target = malloc(sizeof (char) * MAXLINE);
    if (new_rule->target == NULL) {
        perror("malloc");
        exit(1);
    }

    strncpy(new_rule->target, name, MAXLINE);
    new_rule->actions = NULL;
    new_rule->dependencies = NULL;
    new_rule->next_rule = NULL;

    if (first_rule == NULL) {
        first_rule = new_rule;
        return first_rule;
    }

    while (first_rule->next_rule != NULL) {
        first_rule = first_rule->next_rule;
    }
    first_rule->next_rule = new_rule;
    return new_rule;

}

char **create_argument_array(char *curr_line) {

    char temp[MAXLINE];

    strncpy(temp, curr_line, strlen(curr_line) + 1);

    // Counting the number of command line arguments
    int count = 0;
    char *token = strtok(temp, " \n\t\r");
    while (token != NULL) {
        token = strtok(NULL, " \n\t\r");
        count++;
    }
    char **actions = malloc(sizeof (char *) * count + 1);
    if (actions == NULL) {
        perror("malloc");
        exit(1);
    }

    token = strtok(curr_line, " \n\t\r");
    for (int i = 0; i < count; i++) {
        actions[i] = malloc(sizeof(char) * strlen(token));
        if (actions[i] == NULL) {
            perror("malloc");
            exit(1);
        }

        strncpy(actions[i], token, strlen(token) + 1);
        token = strtok(NULL, " \n\t\r");
    }
    actions[count] = NULL;
    return actions;
}

/* Return 1 if the line is an action line, as defined on the assignment handout.
   Return 0 otherwise.
 */
int is_action_line(const char *line) {
    if (line[0] == '\t') {
        return 1;
    }
    return 0;
}

/* Read from the open file fp, and create the linked data structure
   that represents the Makefile contained in the file.
   See the top of pmake.h for the specification of Makefile contents.
 */
Rule *parse_file(FILE *fp) {

    Rule *first_rule = NULL;
    Rule *curr_rule = NULL;

    char curr_line[MAXLINE];
    // Keep readings lines
    while (fgets(curr_line, MAXLINE, fp)) {

        // Comment or blank line read in
        if (is_comment_or_empty(curr_line) == 1) {
            continue;
        }

        if (curr_line[0] == '\n' || curr_line[0] == '\r') {
            continue;
        }


        // Action line read in
        else if (is_action_line(curr_line) == 1) {

            char **actions = create_argument_array(curr_line);
            add_action(curr_rule, actions);

        }

        // Target line read in
        else {
            // Fetching the target name
            char *target = strtok(curr_line, " \r");

            // Target doesn't exist
            if (target_exists(first_rule, target) == 1) {

                // Creating the rule node
                Rule *rule = create_rule(first_rule, target);

                if (is_empty(first_rule)) {
                    first_rule = rule;
                }

                // Setting current rule node
                curr_rule = rule;
            }
            // Target already exists
            else {
                curr_rule = find_rule(first_rule, target);
            }

            // Dependencies
            char *dependency = strtok(NULL, " "); // Colon read in, don't need value
            dependency = strtok(NULL, " \n\r");

            while (dependency != NULL) {

                // The rule does not already exist
                if (target_exists(first_rule, dependency) == 1) {
                    // Create the new rule
                    create_rule(first_rule, dependency);
                }
                Dependency *new_dependency = add_dependency(curr_rule);
                link_dependency(first_rule, new_dependency, dependency);

                dependency = strtok(NULL, " \n\r");

            }
        }
    }
    return first_rule;
}


/******************************************************************************
 * These helper functions are provided for you. Do not modify them.
 *****************************************************************************/
/* Print the list of actions */
void print_actions(Action *act) {
    while(act != NULL) {
        if(act->args == NULL) {
            fprintf(stderr, "ERROR: action with NULL args\n");
            act = act->next_act;
            continue;
        }
        printf("\t");

        int i = 0;
        while(act->args[i] != NULL) {
            printf("%s ", act->args[i]) ;
            i++;
        }
        printf("\n");
        act = act->next_act;
    }
}

/* Print the list of rules to stdout in makefile format. If the output
   of print_rules is saved to a file, it should be possible to use it to
   run make correctly.
 */
void print_rules(Rule *rules){
    Rule *cur = rules;
    while (cur != NULL) {

        if (cur->dependencies || cur->actions) {
            // Print target
            printf("%s : ", cur->target);

            // Print dependencies
            Dependency *dep = cur->dependencies;
            while (dep != NULL){
                if(dep->rule->target == NULL) {
                    fprintf(stderr, "ERROR: dependency with NULL rule\n");
                }
                printf("%s ", dep->rule->target);
                dep = dep->next_dep;
            }
            printf("\n");

            // Print actions
            print_actions(cur->actions);
        }
        cur = cur->next_rule;
    }
}


/* Return 1 if the line is a comment line, as defined on the assignment handout.
   Return 0 otherwise.
 */
int is_comment_or_empty(const char *line) {
    for (int i = 0; i < strlen(line); i++){
        if (line[i] == '#') {
            return 1;
        }
        if (line[i] != '\t' && line[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

/* Convert an array of args to a single space-separated string in buffer.
   Returns buffer.  Note that memory for args and buffer should be allocted
   by the caller.
 */
char *args_to_string(char **args, char *buffer, int size) {
    buffer[0] = '\0';
    int i = 0;
    while (args[i] != NULL) {
        strncat(buffer, args[i], size - strlen(buffer));
        strncat(buffer, " ", size - strlen(buffer));
        i++;
    }
    return buffer;
}
