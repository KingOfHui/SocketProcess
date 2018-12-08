#include <jni.h>
#include <string>
#include "native-lib.h"

const char *PATH = "/data/data/com.dh.socketprocess/my.sock";
const char *userId;
int m_child;
extern "C" {
JNIEXPORT jstring

JNICALL
Java_com_dh_socketprocess_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

void child_do_child() {
    //开启socket   服务端
    if (child_create_channel()) {
        child_listen_msg();
    }
}
/**
 ** 服务端读取信息
 */
void child_listen_msg() {
    fd_set rfds;
    struct timeval timeout = {3, 0};
    while (1) {
        //清空内容
        FD_ZERO(&rfds);
        FD_SET(m_child, &rfds);
        //4个客户端
        int r = select(m_child + 1, &rfds, NULL, NULL, &timeout);
//        LOGE("读取消息前  %d", r);
        if (r > 0) {
            char pkg[256] = {0};
            //保证所读到的信息是指定apk客户端
            if (FD_ISSET(m_child, &rfds)) {
                int result = read(m_child, pkg, sizeof(pkg));
                LOGE("userId   %s",userId);
                //开启服务
                execlp("am", "am", "startservice", "--user", userId,
                       "com.dh.socketprocess/com.dh.socketprocess.ProcessService", (char*) NULL);
                break;
            }
        }
        sleep(1);
    }
}
/**
 * 创建服务端socket
 * 客户端
 * @return
 */
int child_create_channel() {
    //linux
    int listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    //addr 指向内存区域
    struct sockaddr_un addr;
    //清空之前的连接
    unlink(PATH);
    //清空内存
    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, PATH);
    int connfd = 0;
    if (bind(listenfd, (const sockaddr *)(&addr), sizeof(sockaddr_un)) < 0) {
        LOGE("绑定错误");
        return 0;
    }
    listen(listenfd, 5);
//    保证宿主进程连接成功
    while (1) {
        //返回值是  客户端的地   阻塞式函数
        if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("读取错误");
                return 0;
            }
        }
        m_child = connfd;
        LOGE("apk 父进程连接上了  %d", m_child);
        break;
    }
    return 1;
}


JNIEXPORT void JNICALL
Java_com_dh_socketprocess_Watcher_createWatcher(JNIEnv *env, jobject instance, jstring userId_) {
    userId = env->GetStringUTFChars(userId_, 0);

    // 开启双进程
    pid_t pid = fork();
    if (pid < 0) {

    } else if (pid == 0) {
        //子进程 守护进程
        child_do_child();
    } else if (pid > 0) {
        //父进程
    }

    env->ReleaseStringUTFChars(userId_, userId);
}
JNIEXPORT void JNICALL
Java_com_dh_socketprocess_Watcher_connectMonitor(JNIEnv *env, jobject instance) {
    int socked;
    struct sockaddr_un addr;
    while (1) {
        LOGE("11---客户端父进程开始连接");
        socked = socket(AF_LOCAL, SOCK_STREAM, 0);
        if (socked < 0) {
            LOGE("---1111-连接失败");
            return;
        }
        memset(&addr, 0, sizeof(sockaddr_un));
        addr.sun_family=AF_LOCAL;
        strcpy(addr.sun_path, PATH);
        if ((connect(socked, (const sockaddr *)(&addr), sizeof(sockaddr_un)))  < 0){
            LOGE("---2222-连接失败");
            close(socked);
            sleep(1);
            //再来下一次尝试连接
            continue;
        }
        LOGE("连接成功");
        break;
    }


}
}





















