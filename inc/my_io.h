#ifndef __MY_IO_H
#define __MY_IO_H


//#define DEBUGING
#define PRINT 1

#ifndef bool
#define bool char
#endif


#define ture 1
#define false 0
//#define	FILE_MODE	(S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)
					/* default file access permissions for new files */
                    /* user has read and write permission ... */

//#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
					/* default permissions for new directories */
#define	DIR_MODE    00777
#define	FILE_MODE   00777

#define max_string_len 32

struct User{
    char m_name[max_string_len];             //不要更换成员的顺序顺序 @ref read_userdata()
    char m_passwd[max_string_len];
    char m_identification[max_string_len];
    char m_session[max_string_len];
    char m_ip[max_string_len];
    short m_port;
    int m_sockfd;
    struct User *next;
};

extern const char const *serv_public_key;
extern const char const *serv_private_key;
extern const char const *cli_public_key;
extern const char const *cli_private_key;

extern const char const *serv_userDataFile;
extern const char const *cli_userDataFile;
const char const* g_work_path;
extern struct User *g_servUserdata;
extern struct User *g_cliUserdata;


int file_init(const char const *work_path,const char const* public_key,const char const *private_key,
              struct User **g_userdata,const char const* userDataFile);
void Write(int fd, void *ptr, size_t nbytes);
ssize_t Read(int fd, void *ptr, size_t nbytes);
int save_userData(const char *filename,struct User **g_userdata,char *data);
int read_userdata(const char *filename,struct User **p_data);
void print_userData(struct User *g_userdata);
int create_rand_num(int num,char *string);
int create_rand_string(int num,char *string);
int isUserExist(struct User *g_userdata,char *string);
int isIdentificationExist(struct User *g_userdata,char *string);
void save_userDatabylist(const char const *filename,struct User *p);
int print(char *fmt, ...);
int add_friend(const char *filename,struct User **g_userdata,char *data);
void free_list(struct User **p);

#endif
