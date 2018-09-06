#ifndef __MY_IO_H
#define __MY_IO_H




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

extern const char const *public_key;
extern const char const *private_key;
extern const char const *userDataFile;
extern struct User *g_userdata;


int file_init(void);
void Write(int fd, void *ptr, size_t nbytes);
ssize_t Read(int fd, void *ptr, size_t nbytes);
int save_userData(const char *filename,char *data);
int read_userdata(const char *filename);
void print_userData();
int create_rand_num(int num,char *string);
int create_rand_string(int num,char *string);
int isUserExist(char *string);
int isIdentificationExist(char *string);
void save_userDatabylist(const char const *filename,struct User *p);

#endif
