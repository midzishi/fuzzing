#include <unistd.h>

char *adjust_array (int old, int new, char *p);
int length_word(char * word);
int length_com(char ** cmd);
char *readword(int *msg, int *eol, int *backmode, int *qts);
struct node * add_node_first(struct node *list, char *wrd, int *qts);
struct node * add_node(struct node *list ,int *msg, int *eol, int *backmode, int *qts);
void change_fd (char **dir);
int check_arr(int *right, int *dright, int *left);
char ** make_command(const struct node *pp, char ** dir, int * error, int *no_pipe);
void print_dir(char ** dir);
int check_cd (char **wrd);
int cd_func (char ** first_cmd);
int exec_cmd(char ** cmd, int *backmode, char **dir);
void stack(char *cmd, ssize_t len);
void kill_zombies();
int run_pipe(int *no_pipe, int *backmode, char **cmd, char **dir);
int run_pipe(int *no_pipe, int *backmode, char **cmd, char **dir);
void free_mem(struct node *list, char **cmd, char ** dir, int  *eol);
int modify();
struct inserted_cmd {
  int priority;
  char title[20];
  int par;
};
void print_cmd(const struct inserted_cmd *list);
