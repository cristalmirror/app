#include <jni.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

static int global_sock = -1;

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapp_MainActivity_connectNative(JNIEnv* env, jobject thiz, jstring ip, jint port) {
    if (global_sock != -1) {
        close(global_sock);
        global_sock = -1;
    }

    const char *ip_str = env->GetStringUTFChars(ip, 0);
    struct sockaddr_in serv_addr;

    if ((global_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        env->ReleaseStringUTFChars(ip, ip_str);
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_str, &serv_addr.sin_addr) <= 0) {
        close(global_sock);
        global_sock = -1;
        env->ReleaseStringUTFChars(ip, ip_str);
        return -2;
    }

    if (connect(global_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(global_sock);
        global_sock = -1;
        env->ReleaseStringUTFChars(ip, ip_str);
        return -3;
    }

    env->ReleaseStringUTFChars(ip, ip_str);
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapp_MainActivity_sendNative(JNIEnv* env, jobject thiz, jstring mensaje) {
    if (global_sock == -1) {
        return -4; // No conectado
    }

    const char *msg = env->GetStringUTFChars(mensaje, 0);
    ssize_t bytes_sent = send(global_sock, msg, strlen(msg), 0);

    env->ReleaseStringUTFChars(mensaje, msg);

    if (bytes_sent < 0) {
        return -5; // Error al enviar
    }

    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_myapp_MainActivity_disconnectNative(JNIEnv* env, jobject thiz) {
    if (global_sock != -1) {
        close(global_sock);
        global_sock = -1;
    }
}
