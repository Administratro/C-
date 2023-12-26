#include<stdio.h>
#include<stdlib.h>
#include <errno.h>
#include<string.h>

#define MAX_DATA 512
#define MAX_ROWS 100

struct Address
{
    int id;
    int set;
    char name[MAX_DATA];
    char email[MAX_DATA];
};

struct Database
{
	struct Address rows[MAX_ROWS];
};

struct Connection
{
	FILE *file;
	struct Database *db;
};

//注意：函数的申名放到我们所定义的结构体下面，不然出错
struct Connection *Database_open(char *filename,char action);
void Database_load(struct Connection *conn);
void die(const char *message);
void Database_write(struct Connection *conn);
void Database_create(struct Connection *conn);
void Database_close(struct Connection *conn);
void Database_set(struct Connection *conn,int id,char *name,char *email);
void Address_print(struct Address *addr);
void Database_list(struct Connection *conn);
void Database_delete(struct Connection *conn,int id);
void Database_get(struct Connection *conn,int id);

struct Connection *Database_open(char *filename,char action)
{
	struct Connection *conn=malloc(sizeof(struct Connection));
	if(!conn) 
		die("数据库连接变量conn内存申请失败！\n");
	conn->db=malloc(sizeof(struct Database));
	if(conn->db==NULL) 
		die("数据库变量db内存申请失败！\n");
	if(action=='c')
	{
		conn->file=fopen(filename,"w");

	}
	else
	{
		conn->file=fopen(filename,"r+");
		if(conn->file==NULL)
			die("fopen读取数据库文件失败！\n");
		else
		{
			Database_load(conn);
		}
	}
	if(conn->file==NULL)
		die("fopen写文件失败！\n");
	return conn;
}

void Database_load(struct Connection *conn)
{
	//直接读入一整个数据库
	int rc=fread(conn->db,sizeof(struct Database),1,conn->file);
	if(rc!=1)
		die("fread无法读取conn->file变量中的文件!\n");
}

void Database_create(struct Connection *conn)
{
	int i=0;
	for(i=0;i<MAX_ROWS;i++)
	{
		struct Address addr={.id=i,.set=0};
		conn->db->rows[i]=addr;
	}
}

void Database_write(struct Connection *conn)
{
	rewind(conn->file);
	// 直接写入一整个数据库
	int rc=fwrite(conn->db,sizeof(struct Database),1,conn->file);
	if(rc!=1)
		die("在Database_write函数中，数据写入失败！\n");
	rc=fflush(conn->file);
	if(rc==-1)
		die("在Database_write函数中flush失败！\n");
}

void Database_set(struct Connection *conn,int id,char *name,char *email)
{
	struct Address *addr=&conn->db->rows[id];
	if(addr->set !=0)
	{
		printf("id=%d ",id);
		die("此条目已经存在！\n");
	}
	else
		addr->set=1;
	char *res=strncpy(addr->name,name,MAX_DATA);
	if(res==NULL)
		die("在Database_set函数中，设置name失败！\n");
	res=strncpy(addr->email,email,MAX_DATA);
	if(res==NULL)
		die("在Database_set函数中，设置email失败！\n");
}

void Address_print(struct Address *addr)
{
	printf("%d %s %s",addr->id, addr->name, addr->email);
	printf("\n");
}


void Database_list(struct Connection *conn)
{
	int i = 0;
	struct Database *db = conn->db;
	struct Address *cur=NULL;
	for(i = 0; i < MAX_ROWS; i++) 
	{
		cur = &db->rows[i];
		if(cur->set) {
		Address_print(cur);
		}
	}
}


void Database_delete(struct Connection *conn,int id)
{
	struct Address addr={.id=id,.set=0};
	conn->db->rows[id]=addr;
	
}

void Database_get(struct Connection *conn,int id)
{
	struct Address *addr=&conn->db->rows[id];
	if(addr->set !=0) 
	{
		Address_print(addr);
	}
	else
	{
		die("需要获取的数据不存在\n");
	}
}

void Database_close(struct Connection *conn)
{
	if(conn->file)
	{
		fclose(conn->file);
		printf("数据库文件关闭成功！\n");
	if(conn->db)
	{
		free(conn->db);
		printf("数据库变量conn->db，内存释放成功！\n");
	}
	free(conn);
	printf("数据库变量conn，内存释放成功！\n");
	printf("数据库关闭成功！\n");
	}
}

void die(const char *message)
{
    if(errno) 
	{
        perror(message);
	} 
	else 
	{
		printf("ERROR: %s\n", message);
	}

	 exit(1);
}

int main(int argc,char *argv[])
{
	if(argc<3)
	{
		printf("USAGE: ex17 <dbfile> <action> [action params]\n");
		exit(0);
	}
	char action=argv[2][0];
	char *filename=argv[1];
	printf("filename=%s,action=%c\n",filename,action);
	struct Connection *conn = Database_open(filename, action);
	int id=0;
	if(argc>3) 
		//atoi函数把数字字符串转换成int数字
		id=atoi(argv[3]);
	if(id >= MAX_ROWS) 
		die("这个id值大于当前数据库行数，请重新输入！\n");
	printf("filename=%s,action=%c,id=%d\n",filename,action,id);
	switch(action)
	{
		case 'c':
			printf("创建一个数据库！\n");
			Database_create(conn);
			printf("写入并且初始化!\n");
			Database_write(conn);
			break;
		case 's':
			if(argc!=6) 
			{
				printf("需要输入参数id,name,email写入个人信息！\n");
				exit(1);
			}
			printf("正在设置id=%d,name=%s,email=%s\n",id,argv[4],argv[5]);
			Database_set(conn,id,argv[4],argv[5]);
			Database_write(conn);
			printf("设置完成\n");
			break;
		case 'l':
			printf("输出目前数据库中所有的条目！\n");
			Database_list(conn);
			break;
		case 'd':
			printf("删除id=%d的条目\n",id);
			Database_delete(conn,id);
			Database_write(conn);
			printf("id=%d,删除成功\n",id);
			break;
		case 'g':
			if(argc !=4) 
			{
				printf("获取数据方法：<脚本名> <数据文件> g id\n");
				die("获取数据错误\n");
			}
			Database_get(conn,id);
			printf("获取id=%d的数据成功\n",id);
			break;
		default:
			die("用法：c=create, g=get, s=set, d=del, l=list");
			break;
	}
	Database_close(conn);
	return 0;
}
