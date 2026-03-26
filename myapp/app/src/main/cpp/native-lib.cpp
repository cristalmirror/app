#include <jni.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iomanip>
#include "Cipher.hpp"
#include "Decipher.hpp"
#include "colorString.hpp"

#define BLOCK_SIZE 16
using namespace std;

static int global_sock = -1;

// Implementation of the UI status update helper
void updateUIStatus(JNIEnv* env, jobject thiz, const string& message) {
    jclass clazz = env->GetObjectClass(thiz);
    jmethodID methodId = env->GetMethodID(clazz, "updateUIStatus", "(Ljava/lang/String;)V");
    if (methodId != nullptr) {
        jstring jmsg = env->NewStringUTF(message.c_str());
        env->CallVoidMethod(thiz, methodId, jmsg);
        env->DeleteLocalRef(jmsg);
    }
}

void printBlock(const vector<unsigned char>& block, const string& message) {
    cout << message << ": ";
    for (unsigned char b : block) {
        cout << hex << setw(2) << setfill('0') << (int)b << " ";
    }
    cout << dec << endl;
}

class FileHandler {
public:
    static vector<vector<unsigned char>> readFileInBlocksCC20(const string &filename, size_t blockSize);
    static vector<vector<unsigned char>> readFileInBlocks(const string &filename);
    static void writeBlocksToFile(const string& filename, const vector<vector<unsigned char>>& blocks);
};

vector<vector<unsigned char>> FileHandler::readFileInBlocksCC20(const string &filename, size_t blockSize) {
    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        cerr << Color::ROJO <<"Error in open archive"<< Color::RESET <<endl;
        return{};
    }

    vector<vector<unsigned char>> blocks;
    vector<unsigned char> buffer(blockSize);
    while(inFile.read(reinterpret_cast<char*>(buffer.data()),blockSize)) {
        blocks.push_back(buffer);
    }

    size_t bytesRead = inFile.gcount();
    if (bytesRead > 0) {
        vector<unsigned char> lastBlock(blockSize,0);
        for (size_t i = 0; i < bytesRead; i++) {
            lastBlock[i] = buffer[i];
        }
        blocks.push_back(lastBlock);
    }
    inFile.close();
    return blocks;
}

vector<vector<unsigned char>> FileHandler::readFileInBlocks(const string &filename) {
    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        cerr << Color::ROJO <<"Error in open archive"<< Color::RESET <<endl;
        return{};
    }

    vector<vector<unsigned char>> blocks;
    vector<unsigned char> buffer(BLOCK_SIZE,0);
    while(inFile.read(reinterpret_cast<char*>(buffer.data()),BLOCK_SIZE)) {
        blocks.push_back(buffer);
    }

    size_t bytesRead = inFile.gcount();
    if (bytesRead > 0) {
        vector<unsigned char> lastBlock(BLOCK_SIZE,0);
        for (size_t i = 0; i < bytesRead; i++) {
            lastBlock[i] = buffer[i];
        }
        blocks.push_back(lastBlock);
    }
    inFile.close();
    return blocks;
}

void FileHandler::writeBlocksToFile(const string& filename, const vector<vector<unsigned char>>& blocks) {
    ofstream outFile(filename,ios::binary);
    if (!outFile) {
        cerr<< Color::ROJO <<"Error open file to write"<< Color::RESET <<endl;
        return;
    }
    cout <<Color::MAGENTA<<"*<<[writing binary bloks]>>*"<< Color::RESET<<endl;
    for (const auto& block: blocks) {
        outFile.write(reinterpret_cast<const char*>(block.data()), block.size());
    }

    outFile.close();
    cout << Color::NARANJA <<"*** Blocks => ["<< blocks.size() <<"] write in: "<< filename <<"***"<< Color::RESET<<endl;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapp_MainActivity_encryptFile(JNIEnv* env, jobject thiz, jstring filePath) {
    const char *path_str = env->GetStringUTFChars(filePath, 0);
    if (path_str == nullptr) return -1;

    string filename(path_str);
    string filenameOutput = filename + ".enc";

    vector<vector<unsigned char>> blocks = FileHandler::readFileInBlocks(filename);
    if (blocks.empty()) {
        env->ReleaseStringUTFChars(filePath, path_str);
        return -2;
    }

    // Fixed: Passing env and thiz
    vector<unsigned char> key = generateSaveKeyBase(env, thiz, "key.aes");
    if (key.empty()) {
        env->ReleaseStringUTFChars(filePath, path_str);
        return -3;
    }

    // Fixed: Passing env and thiz to constructor
    AESCipher cipher(env, thiz, key);
    vector<vector<unsigned char>> encryptedBlocks;

    for (size_t i = 0; i < blocks.size(); i++) {
        // Fixed: Passing env and thiz to encryptBlock
        vector<unsigned char> encrypted = cipher.encryptBlock(env, thiz, blocks[i]);
        printBlock(blocks[i], Color::NARANJA_NEGRO + "Bloque original " + to_string(i) + Color::RESET);
        printBlock(encrypted, Color::AZUL_FONDO + "Bloque cifrado " + to_string(i) + Color::RESET);
        encryptedBlocks.push_back(encrypted);
        cout << Color::CIAN << "***block " << i << "/" << blocks.size() << " encrypted ***" << Color::RESET << endl;
    }

    FileHandler::writeBlocksToFile(filenameOutput, encryptedBlocks);

    env->ReleaseStringUTFChars(filePath, path_str);
    return 0;
}

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
        return -4;
    }

    const char *msg = env->GetStringUTFChars(mensaje, 0);
    ssize_t bytes_sent = send(global_sock, msg, strlen(msg), 0);

    env->ReleaseStringUTFChars(mensaje, msg);

    if (bytes_sent < 0) {
        return -5;
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
