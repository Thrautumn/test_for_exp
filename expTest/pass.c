#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>

#define DB_PATH "./attendance.db"

void print_html_head()
{
    printf("Content-type:text/html\n\n");

    printf("<html>");
    printf("<head>");
    printf("<meta charset='UTF-8'>");

    printf("<style>");

    printf("body{font-family:Microsoft YaHei;background:#f0f2f5;padding:20px;}");

    printf("table{width:100%%;border-collapse:collapse;background:white;}");

    printf("th{background:#1890ff;color:white;height:40px;}");

    printf("td{text-align:center;height:35px;border-bottom:1px solid #ddd;}");

    printf("a{margin:10px;}");

    printf("</style>");

    printf("</head>");
    printf("<body>");
}

void print_html_tail()
{
    printf("</body>");
    printf("</html>");
}

void get_time(char *buf)
{
    time_t t;
    struct tm *tmp;

    time(&t);
    tmp = localtime(&t);

    strftime(buf,64,"%Y-%m-%d %H:%M:%S",tmp);
}

void add_record(char *uid)
{
    sqlite3 *db;
    char sql[512];
    char timebuf[64];

    sqlite3_open(DB_PATH,&db);

    get_time(timebuf);

    sprintf(sql,
        "INSERT INTO attendance VALUES('%s','手动签到','%s');",
        uid,
        timebuf);

    sqlite3_exec(db,sql,0,0,0);

    sqlite3_close(db);

    print_html_head();

    printf("<h2>手动打卡成功！</h2>");

    printf("<a href='/index.html'>返回首页</a>");

    print_html_tail();
}

int callback(void *NotUsed,int argc,char **argv,char **azColName)
{
    printf("<tr>");

    for(int i=0;i<argc;i++)
    {
        printf("<td>%s</td>",argv[i] ? argv[i] : "");
    }

    printf("</tr>");

    return 0;
}

void list_records()
{
    sqlite3 *db;

    sqlite3_open(DB_PATH,&db);

    print_html_head();

    printf("<h2>全部考勤记录</h2>");

    printf("<table border='1'>");

    printf("<tr>");
    printf("<th>UID</th>");
    printf("<th>状态</th>");
    printf("<th>时间</th>");
    printf("</tr>");

    sqlite3_exec(
        db,
        "SELECT * FROM attendance ORDER BY time DESC LIMIT 10;",
        callback,
        0,
        0
    );

    printf("</table>");

    printf("<br>");

    printf("<a href='/index.html'>返回首页</a>");

    print_html_tail();

    sqlite3_close(db);
}

void search_record(char *uid)
{
    sqlite3 *db;
    char sql[512];

    sqlite3_open(DB_PATH,&db);

    sprintf(sql,
        "SELECT * FROM attendance WHERE id='%s';",
        uid);

    print_html_head();

    printf("<h2>查询结果</h2>");

    printf("<table border='1'>");

    printf("<tr>");
    printf("<th>UID</th>");
    printf("<th>状态</th>");
    printf("<th>时间</th>");
    printf("</tr>");

    sqlite3_exec(db,sql,callback,0,0);

    printf("</table>");

    printf("<br>");

    printf("<a href='/index.html'>返回首页</a>");

    print_html_tail();

    sqlite3_close(db);
}

void clear_db()
{
    sqlite3 *db;

    sqlite3_open(DB_PATH,&db);

    sqlite3_exec(db,"DELETE FROM attendance;",0,0,0);

    sqlite3_close(db);

    print_html_head();

    printf("<h2>数据库已清空！</h2>");

    printf("<a href='/index.html'>返回首页</a>");

    print_html_tail();
}

int main()
{
    char *method;
    char *query;
    char data[1024]={0};

    method=getenv("REQUEST_METHOD");

    if(strcmp(method,"GET")==0)
    {
        query=getenv("QUERY_STRING");

        if(strstr(query,"action=list"))
        {
            list_records();
        }
        else if(strstr(query,"action=search"))
        {
            char uid[128]={0};

            sscanf(query,
                "action=search&uid=%s",
                uid);

            search_record(uid);
        }
    }
    else if(strcmp(method,"POST")==0)
    {
        int len;

        len=atoi(getenv("CONTENT_LENGTH"));

        fread(data,1,len,stdin);

        if(strstr(data,"action=add"))
        {
            char uid[128]={0};

            sscanf(data,
                "action=add&uid=%s",
                uid);

            add_record(uid);
        }
        else if(strstr(data,"action=clear"))
        {
            clear_db();
        }
    }

    return 0;
}