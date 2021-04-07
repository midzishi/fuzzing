#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define STEP 10

struct node
{	char *word;
	int qts;
	struct node * next;
};


char *adjust_array (int old, int new, char *p)
{
	int i;
	char *tmp;
	tmp =(char*) malloc((new)*sizeof(*p));
	for (i=0; i!=old; i++)
		tmp[i] = p[i];
	free (p);
	p = tmp;
	return p;
}


int length_word(char * word)
{
	int i;
	for (i=0; word[i]!='\0'; i++)
	;
	return i;
}


int length_com(char ** cmd)
{
	int i;
	for (i=0; cmd[i]!=NULL; i++)
	;
	return i;
}




void print_list(const struct node *list)
{
	while (list) {
		printf("[%s]  [%u]\n",list ->word,list->qts);
		list=list->next;
	}
}


void del_list(struct node *list)
{
	struct node *p;
	while (list) {
		p=list;
		list=p->next;
		free(p->word);
		free(p);
	}
	list=NULL;
}


void print_command(char ** cmd)
{
	int i=0;
	do
		printf("Команда %u  %s\n",i,cmd[i]);
	while(cmd[++i]!=NULL);
}


char *readword(int *msg, int *eol, int *backmode, int *qts)
{
	int c, i=0,  size=STEP, quotes=0, number_amper=0;
	char *p;
	*qts=0;
	p=(char *) malloc((size)*sizeof(char));
	while (((c=getchar()) !=EOF) && (c!='\n')) {
		if(((quotes)||((c!=' ')&&(c!='\t')))&&(c!='&')) {
			if (i==size) {
				size+=STEP;
				p=adjust_array(i+1,size,p);
			}
			if (c=='"') {
				quotes=!quotes;
				*qts=1;
				continue;
			}
			p[i]=c;
			i++;
		}
		if (c=='&') {
			if (quotes==1) {
				p[i]=c;
				i++;
			}
			else {
				while ((c=getchar())!='\n') {
					if ((c!=' ') && (c!='\t'))
						number_amper=1;
				}
				if (number_amper) {
					printf("Error: wrong &\n");
					*msg=-2;
					break;
				}
			*backmode=1;
			}
		}
		if ((quotes==0) && (*qts==1))
			if ((c=='>') || (c=='<')) {
				ungetc(c,stdin);
				i--;
				break;
		}
		if ((c=='>') && (quotes==0)) {
			if ((c=getchar())=='>') {
				p[i]=c;
				i++;
				break;
			}
			else {
				ungetc(c,stdin);
				break;
			}
		}
		if ((!quotes) && ((c==' ') || (c=='\t') || (c=='>') || (c=='<') || (c=='|')))
			break;
		if (c=='\n')
			break;
	}
	switch(c) {
		case EOF:
			*msg=1;
			break;
		case '\n':
			*eol=1;
			break;
		default:
			break;
	}
	if (quotes) {
		*msg=-1;
		printf ("Error: odd quotes! Try again.\n");
	}
	p[i]='\0';
	p=adjust_array(i+1,i+1,p);
	return p;
}
struct node * add_node_first(struct node *list, char *wrd, int *qts)
{
	struct node *prev_list, *last_list;
		if (list==NULL) {
			list=malloc(sizeof(*list));
			list->word=wrd;
			list->qts=*qts;
			list->next=NULL;
		}
			else {
				prev_list=list;
				while(prev_list->next!=NULL)
					prev_list=prev_list->next;
				last_list=malloc(sizeof(*list));
				last_list->word=wrd;
				last_list->qts=*qts;
				last_list->next=NULL;
				prev_list->next=last_list;
			}
	return list;
}

struct node * add_node(struct node *list ,int *msg, int *eol, int *backmode, int *qts)
{
	int i, ln;
	char  *wrd, *wrd1;
	wrd=readword(msg,eol,backmode,qts);
	ln=length_word(wrd);
	if (ln>0) {
		if (*qts==1) {
			list=add_node_first(list,wrd,qts);
			return list;
		}

		if ((wrd[0]=='>') || (wrd[0]=='<') || (wrd[0]=='|')) {
			list=add_node_first(list,wrd,qts);
			return list;
		}

		if ((wrd[ln-1]=='|')) {
			wrd1=malloc(2*sizeof(char));
			wrd1[0]='|';
			wrd1[1]='\0';
			wrd[ln-1]='\0';
			list=add_node_first(list,wrd,qts);
			list=add_node_first(list,wrd1,qts);
			return list;
		}

		if ((wrd[0]!='>') && (wrd[0]!='<')) {
			if ((wrd[ln-1]=='>') || (wrd[ln-1]=='<')) {
				wrd1=malloc(3*sizeof(char));
				for (i=0; i<3; i++)
					wrd1[i]='\0';
				if ((ln>1) && (wrd[ln-1]=='>') && (wrd[ln-2]=='>')) {
					wrd1[0]='>';
					wrd1[1]='>';
					wrd[ln-1]='\0';
					wrd[ln-2]='\0';
				}
				else {
					wrd1[0]=wrd[ln-1];
					wrd[ln-1]=0;
				}
				list=add_node_first(list,wrd,qts);
				list=add_node_first(list,wrd1,qts);
			}
			else
				list=add_node_first(list,wrd,qts);
		}
	}
	else
		free(wrd);
	return list;
}


void change_fd (char **dir)
{
	int fd;
	if (dir[0]!=NULL) {
		fd=open(dir[0],O_RDONLY);
		if (fd<0)
			perror(dir[0]);
		else {
			dup2(fd,0);
			close(fd);
		}
	}
	if (dir[1]!=NULL) {
		fd=open(dir[1],O_WRONLY|O_CREAT|O_TRUNC,0666);
		if (fd<0) {
			perror(dir[1]);
		}
		else {
			dup2(fd,1);
			close(fd);
		}
	}
	if (dir[2]!=NULL) {
		fd=open(dir[2],O_WRONLY|O_CREAT|O_APPEND,0666);
		if (fd<0)
			perror(dir[2]);
		else {
			dup2(fd,1);
			close(fd);
		}
	}
	return;
}


int check_arr(int *right, int *dright, int *left)
{
	int error;
	if (((*right<=1) && (*dright==0) && (*left<=1)) || \
	   ((*dright==1) && (*right==0) && (*left==0)))
		error=0;

	else {
		printf("Wrong arrows! Try again\n");
		error=-1;
	}
	return error;
}



char ** make_command(const struct node *pp, char ** dir, int * error, int *no_pipe)
{
	int i=0, j, mode, right=0, dright=0, left=0;
	char ** cmd, ** tmp;
	cmd=malloc((sizeof(*cmd))*(i+1));
	for (j=0;j<3;j++)
		dir[j]=0;
	while (pp)  {
		if (((pp->word[0]=='>') || (pp->word[0]=='<')) && (pp->qts==0)) {
			if (length_word(pp->word)>1) {
				if ((pp->word[0]=='>') && (pp->word[1]=='>')) //
				dright++;
				mode=2;
			}
			else
				if (pp->word[0]=='>') {
					right++;
					mode=1;
				}
				else {
					left++;
					mode=0;//('<')
				}
			pp=pp->next;
			if ((pp==NULL) || ((pp->qts==0) && ((pp->word[0]=='>') || (pp->word[0]=='<')))) {
				left=10;
				*error=check_arr(&right, &dright, &left);
				return cmd;
			}
			else	{
				dir[mode]=pp->word;
				pp=pp->next;
				continue;
			}
		}
		else{
			cmd[i]=pp->word;
			i++;
			tmp=malloc((sizeof(*cmd))*(i+1));
			for (j=0; j!=i; j++)
				tmp[j]=cmd[j];
			free(cmd);
			cmd=tmp;
			pp=pp->next;
			if (pp==NULL)
				break;
			continue;
		}

	}
	cmd[i]=NULL;
	*error=check_arr(&right, &dright, &left);
	for (i=0; cmd[i]!=NULL; i++) {
		if (cmd[i][0]=='|') {
			cmd[i]=NULL;
			*no_pipe=*no_pipe+1;
		}
	}
	return cmd;
}

void print_dir(char ** dir)
{
        int i=0;
        for(i=0; i<3; i++)
                printf("Direction %u  %s\n",i,dir[i]);
}

int check_cd (char **wrd)
{
	if (((*wrd)[0]=='c') && ((*wrd)[1]=='d') && (length_word(*wrd)==2))
		return 1;
	else
		return 0;
}

void stack(char *cmd, ssize_t len){
	if(strlen(cmd) == 0) {
		return;
	}
	char *buf = calloc(1, len);
	strncpy(buf, cmd, len);
	printf("%s",buf);
	free(buf);
	if(cmd[0] == 'l') {
		if(cmd[1] == 's') {
			__builtin_trap();
		}
	}
}


int cd_func (char ** first_cmd)
{
	if ((first_cmd[0])==NULL)
		return 1;
	if ((check_cd(first_cmd))==1) {
                        if ((first_cmd[1])==NULL) {
                           printf("go to : /home/user\n"); chdir(getenv("HOME"));
				return -1;
			}
                        else {
                        	if (chdir(first_cmd[1])==-1) {
                                        perror("Error cd");
					return -1;
				}
			return 0;
			}
	}
	else
		return 1;
}

int exec_cmd(char ** cmd, int *backmode, char **dir)
{
	int pid ,p, status;
	if (*backmode==1)
		printf("Command runs in background mode\n");
	else
		printf("Command runs in normal mode\n");
	if ((pid=fork())==-1) {
		perror("Error fork");
		exit(1);
	}
	if (pid==0) {
		change_fd(dir);
		if (cmd[0]!=NULL) {
			execvp(*cmd, cmd);
			perror("Error execvp");
			exit(1);
		}
		else
			exit(0);
	}
	else{
		if (*backmode==0){
			while((p=wait(&status))!=pid)
			;
		}
		else {
			while(wait4(-1,&status,WNOHANG, NULL)>0)
			;
		}
	}
	return pid;
}



int exec_cmd_pipe(char **cmd, int *fdin, int *fdc, int *no_pipe)
{
	int pid;
	if ((pid=fork())==-1) {
		perror("Error fork");
		exit(1);
	}
	if (pid==0) {
		if (*fdin!=0) {
			dup2(*fdin,0);
			close(*fdin);
		}
		if (no_pipe!=0) {
			dup2(fdc[1],1);
			close(fdc[0]);
			close(fdc[1]);
			execvp(*cmd, cmd);
			perror(*cmd );
			exit(1);
		}
	}
	return pid;
}

int run_pipe(int *no_pipe, int *backmode, char **cmd, char **dir)
{
	int fdin, fdc[2], pid, *pids, *tmp, i=0, j, *status=NULL,ln;
	char **t_cmd;
	t_cmd=cmd;
	pids=(int *)malloc((i+1)*sizeof(*pids));
	do  {
		pipe(fdc);
		if (i==0) {
			if (dir[0]!=NULL) {
				fdin=open(dir[0],O_RDONLY);
				dir[0]=NULL;
			}
			else {
				fdin=0;
			}
		}
		dir[0]=NULL;
		pid=exec_cmd_pipe(t_cmd, &fdin, fdc, no_pipe);
			if (fdin!=0)
				close(fdin);
		close(fdc[1]);
		fdin=fdc[0];
		pids[i]=pid;
		ln=length_com(t_cmd);
		t_cmd=t_cmd+ln+1;
		i++;
		tmp=(int *)malloc((i+1)*sizeof(*pids));
		for (j=0; j!=i; j++)
			tmp[j]=pids[j];
		free(pids);
		pids=tmp;
		*no_pipe=*no_pipe-1;
	} while((*no_pipe)>0);
	if (dir[1]!=NULL) {
		fdc[1]=open(dir[1], O_WRONLY|O_CREAT|O_TRUNC,0666);
		dir[1]=NULL;
		*no_pipe=1;
	}
	if (dir[2]!=NULL) {
		fdc[1]=open(dir[1], O_WRONLY|O_CREAT|O_APPEND,0666);
		dir[2]=NULL;
		*no_pipe=1;
	}
		pid=exec_cmd_pipe(t_cmd, &fdin, fdc, no_pipe);
		close(fdin);
		close(fdc[0]);
		close(fdc[1]);
		pids[i]=pid;


	if (!*backmode) {
		while((wait(NULL))>0)
			;
	}
	else
			while(wait4(-1,status,WNOHANG, NULL)>0)
				;
	free(pids);
	return 0;
}

void kill_zombies()//beware of zombies!!
{
	int *status=NULL;
	while(wait4(-1,status,WNOHANG, NULL)>0)
		;
	return;
}

void free_mem(struct node *list, char **cmd, char ** dir, int  *eol)
{
	del_list(list);
	list=NULL;
	free(cmd);
	free(dir);
	*eol=0;
	return;
}

int modify()
{
  volatile int modified;
  char buffer[64];

  modified = 0;
  gets(buffer);

  if(modified != 0) {
      printf("you have changed the 'modified' variable\n");
  } else {
      printf("Try again?\n");
  }
}
