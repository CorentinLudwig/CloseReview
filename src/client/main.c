
#include <network/manager.h>
#include <network/tls-com.h>
#include <pthread.h>
#include <stdio.h>
#include <types/message.h>
#include <types/p2p-msg.h>
#include <types/packet.h>
#include <unistd.h>
#include <utils/logger.h>
#include <utils/project_constants.h>

#define BLUE "\033[38;5;45m"
#define GREEN "\033[38;5;82m"
#define RED "\033[38;5;196m"
#define RESET "\033[0m"

#define FILE_MAIN "main.c"

int port_server;
char ip_server[SIZE_IP_CHAR];
char user_id[SIZE_NAME];
char password[SIZE_PASSWORD];
TLS_infos *tls;
Manager *manager;

void getArgs(int argc, char *argv[]);
void *funInput(void *arg);
void *funOutput(void *arg);
void *funPeer(void *arg);

void tryClient(char *act);
void okClient(void);
void koClient(void);

int main(int argc, char *argv[]) {

    init_logger(NULL);
    tryClient("getArgs");
    getArgs(argc, argv);
    okClient();

    tryClient("initTLSInfos");
    tls = initTLSInfos(ip_server, port_server, TLS_CLIENT, NULL, NULL);
    okClient();

    tryClient("tlsOpenCom");
    tlsOpenCom(tls, NULL);
    okClient();

    tryClient("Connect");
    P2P_msg *msg = initP2PMsg(P2P_CONNECTION_SERVER);
    p2pMsgSetUserId(msg, user_id);
    p2pMsgSetPassword(msg, password);
    Packet *packet = initPacketP2PMsg(msg);
    tlsSend(tls, packet);
    okClient();

    tryClient("answer");
    if (tlsReceiveBlocking(tls, &packet) == TLS_SUCCESS) {
        printf(" answer : %s \n", p2pMsgToTXT(&(packet->p2p)));
    }
    okClient();

    char buff[32];
    printf("waiting .... \n");
    scanf("%s", buff);

    tryClient("get available");
    msg = initP2PMsg(P2P_GET_AVAILABLE);
    packet = initPacketP2PMsg(msg);
    tlsSend(tls, packet);
    okClient();

    tryClient("answer");
    if (tlsReceiveBlocking(tls, &packet) == TLS_SUCCESS) {
        printf(" answer : %s \n", p2pMsgToTXT(&(packet->p2p)));
    }
    okClient();

    tryClient("close");
    tlsCloseCom(tls, NULL);
    okClient();

    close_logger();
    return 0;
}

void *funInput(void *arg) {
    (void)arg;
    char FUN_NAME[32] = "funInput";
    Packet *p;
    Msg *msg;
    size_t size = SIZE_MSG_DATA;
    char *buff = malloc(size);
    while (1) {
        sleep(1);
        /* read next message */
        printf("\n > ");
        getline(&buff, &size, stdin);
        printf("\033[2K\r");

        /* send message */
        msg = initMsg(user_id, buff);
        p = initPacketMsg(msg);
        deinitMsg(&msg);
        if (managerSend(manager, MANAGER_MOD_PEER, p) != MANAGER_ERR_SUCCESS) {
            warnl(FILE_MAIN, FUN_NAME, "fail to send message <%s>", p->msg.buffer);
            continue;
        }

        /* display in output */
        if (managerSend(manager, MANAGER_MOD_OUTPUT, p) != MANAGER_ERR_SUCCESS) {
            warnl(FILE_MAIN, FUN_NAME, "fail to display message in output <%s>", p->msg.buffer);
            continue;
        }
        deinitPacket(&p);
    }
}

void *funOutput(void *arg) {
    (void)arg;
    char FUN_NAME[32] = "funOutput";
    Packet *p;
    Manager_error error;
    char *buff;
    while (1) {
        error = managerReceiveBlocking(manager, MANAGER_MOD_OUTPUT, &p);
        if (error != MANAGER_ERR_SUCCESS) {
            warnl(FILE_MAIN, FUN_NAME, "failed to read packet from manager");
            managerSetState(manager, MANAGER_MOD_OUTPUT, MANAGER_STATE_CLOSED);
            exit(-1);
        }
        if (p->type != PACKET_MSG) {
            warnl(FILE_MAIN, FUN_NAME, "unexpected type");
            deinitPacket(&p);
            continue;
        }
        buff = msgToTXT(&(p->msg));
        printf("\n <+>------------------------ \n\n %s", buff);
        free(buff);
        deinitPacket(&p);
    }
}

TLS_error nextPacket(Manager *manager, Manager_module module, Packet **p) {
    char FUN_NAME[32] = "nextPacket";
    Manager_error error;
    error = managerReceiveNonBlocking(manager, module, p);
    switch (error) {
    case MANAGER_ERR_SUCCESS:
        return TLS_SUCCESS;
        break;
    case MANAGER_ERR_RETRY:
        *p = NULL;
        return TLS_RETRY;
        break;
    case MANAGER_ERR_CLOSED:
        warnl(FILE_MAIN, FUN_NAME, "manager closed");
        *p = NULL;
        return TLS_CLOSE;
        break;
    case MANAGER_ERR_ERROR:
        warnl(FILE_MAIN, FUN_NAME, "failed to read packet from manager");
        *p = NULL;
        return TLS_ERROR;
        break;
    }
}

void MSGmanager(Manager *manager, Manager_module module, Packet *p) {
    char FUN_NAME[32] = "MSGmanager";
    Manager_error error;
    (void)module;
    error = managerSend(manager, MANAGER_MOD_OUTPUT, p);
    if (error != MANAGER_ERR_SUCCESS) {
        warnl(FILE_MAIN, FUN_NAME, "error sending packet");
    }
}

void P2Pmanager(Manager *manager, Manager_module module, Packet *p) {
    char FUN_NAME[32] = "P2Pmanager";
    warnl(FILE_MAIN, FUN_NAME, "packet P2P received ????");
    (void)p;
    (void)manager;
    (void)module;
}

void *funPeer(void *arg) {
    (void)arg;
    tlsStartListenning(tls, manager, MANAGER_MOD_PEER, nextPacket, MSGmanager, P2Pmanager);
    deinitTLSInfos(&tls);
    managerSetState(manager, MANAGER_MOD_PEER, MANAGER_STATE_CLOSED);
    return NULL;
}

void tryClient(char *act) {
    printf("\n %s[CLIENT] > %s %s\n", BLUE, act, RESET);
    fflush(stdout);
}

void okClient(void) {
    printf("\t\t %s <+>---------- OK %s\n", GREEN, RESET);
    fflush(stdout);
}

void koClient(void) {
    printf("\t\t %s <+>---------- KO %s\n", RED, RESET);
    fflush(stdout);
}

void displayUsage(char *bin) {
    printf("usage : %s <ip_server> <port_server> <id> <password>\n", bin);
}

void getArgs(int argc, char *argv[]) {
    if (argc != 5) {
        displayUsage(argv[0]);
        exit(1);
    }
    strncpy(ip_server, argv[1], SIZE_IP_CHAR);
    port_server = atoi(argv[2]);
    if (port_server < 1024) {
        displayUsage(argv[0]);
        exit(1);
    }
    strncpy(user_id, argv[3], SIZE_NAME);
    strncpy(password, argv[4], SIZE_PASSWORD);
}
