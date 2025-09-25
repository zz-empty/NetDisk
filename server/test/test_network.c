#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/network.h"
#include "../include/config.h"

void test_network_creation() {
    printf("=== Test 1: Network Server Configuration ===\n");
    
    // 创建默认配置
    server_config_t *config = config_create_default();
    if (!config) {
        printf("✗ Failed to create configuration\n");
        return;
    }

    // 创建线程池
    threadpool_t *pool = threadpool_create(2, 10);
    if (!pool) {
        printf("✗ Failed to create thread pool\n");
        config_destroy(config);
        return;
    }

    // 创建网络服务器
    network_server_t *server = network_server_create(config, pool);
    if (server) {
        printf("✓ Network server created successfully\n");
        printf("✓ Port: %d\n", server->port);
        printf("✓ Max clients: %d\n", server->max_clients);
        
        network_server_destroy(server);
    } else {
        printf("✗ Failed to create network server\n");
    }

    threadpool_destroy(pool, 1);
    config_destroy(config);
    printf("\n");
}

void test_client_connection() {
    printf("=== Test 2: Client Connection Management ===\n");

    // 创建测试客户端连接
    struct sockaddr_in test_addr;
    memset(&test_addr, 0, sizeof(test_addr));
    test_addr.sin_family = AF_INET;
    test_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    test_addr.sin_port = htons(8080);

    // 使用一个无效的fd测试
    client_connection_t *client = client_connection_create(-1, test_addr);
    if (client) {
        printf("✓ Client connection created\n");
        printf("✓ Client ID: %d\n", client->client_id);
        printf("✓ Connect time: %ld\n", client->connect_time);

        client_connection_destroy(client);
        printf("✓ Client connection destroyed\n");
    } else {
        printf("✗ Failed to create client connection\n");
    }

    printf("\n");
}

void test_network_communication() {
    printf("=== Test 3: Network Communication (Basic) ===\n");

    // 创建一个简单的服务器和客户端进行通信测试
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("✗ Failed to create server socket\n");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0;       // 让系统分配端口

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        printf("✗ Failed to bind server socket\n");
        close(server_fd);
        return;
    }

    // 获取分配的端口
    socklen_t addr_len = sizeof(server_addr);
    getsockname(server_fd, (struct sockaddr*)&server_addr, &addr_len);

    if (listen(server_fd, 1) < 0) {
        printf("✗ Failed to listen on server socket\n");
        close(server_fd);
        return;
    }

    printf("✓ Test server listening on port %d\n", ntohs(server_addr.sin_port));

    // 在子进程中启动客户端
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程：客户端
        sleep(1);   // 等待服务器准备好

        int client_id = socket(AF_INET, SOCK_STREAM, 0);
        if (client_id == -1) {
            exit(1);
        }

        struct sockaddr_in connect_addr;
        memset(&connect_addr, 0, sizeof(connect_addr));
        connect_addr.sin_family = AF_INET;
        connect_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        connect_addr.sin_port = server_addr.sin_port;

        if (connect(client_id, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) != 0) {
            close(client_id);
            exit(1);
        }

        // 发送测试数据
        const char *test_data = "Hello Server!";
        send(client_id, test_data, strlen(test_data), 0);

        // 接受响应
        char buffer[256];
        ssize_t received = recv(client_id, buffer, sizeof(buffer), 0);

        if (received > 0) {
            buffer[received] = '\0';
            printf("✓ Client received: %s\n", buffer);
        }
        
        close(client_id);
        exit(0);
    } else {
        // 主程序: 服务器
        
        int client_id = accept(server_fd, NULL, NULL);
        if (client_id >= 0) {
            printf("✓ Test client connected\n");

            // 接受数据
            char buffer[256];
            ssize_t received;
            network_error_t result = network_receive_data(client_id, buffer, sizeof(buffer), &received);

            if (result == NETWORK_SUCCESS && received > 0) {
                printf("✓ Server received: %s\n", buffer);

                // 发送响应
                const char *response = "Hello client!";
                result = network_send_data(client_id, response, strlen(response));
                if (result == NETWORK_SUCCESS) {
                    printf("✓ Server response sent\n");
                }
            }

            close(client_id);
        }

        close(server_fd);
        wait(NULL);     // 等待子进程结束
    }
    printf("\n");
}

int main() {
    printf("Staring Network Module Tests...\n");

    test_network_creation();
    test_client_connection();
    test_network_communication();

    printf("All network module tests completed!\n");
    return 0;
}
