#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types/p2p-msg.h>
#include <utils/const-define.h>

#define FILE_P2P_MSG "p2p-msg.c"

/**
 * @brief Return the char associated with the type
 *
 * @param type P2P_msg_type
 * @return char*
 */
char *p2pMsgTypeToChar(P2P_msg_type type) {
    switch (type) {
    case P2P_CONNECTION_SERVER:
        return "P2P_USER_ID";
    case P2P_CONNECTION_OK:
        return "P2P_CONNECTION_OK";
    case P2P_CONNECTION_KO:
        return "P2P_CONNECTION_KO";
    case P2P_ACCEPT:
        return "P2P_ACCEPT";
    case P2P_REJECT:
        return ("P2P_REJECT");
    case P2P_REQUEST_IN:
        return ("P2P_REQUEST_IN");
    case P2P_REQUEST_OUT:
        return ("P2P_REQUEST_OUT");
    case P2P_GET_AVAILABLE:
        return ("P2P_GET_AVAILABLE");
    case P2P_AVAILABLE:
        return ("P2P_AVAILABLE");
    case P2P_CON_FAILURE:
        return ("P2P_CON_FAILURE");
    case P2P_CON_SUCCESS:
        return ("P2P_CON_SUCCESS");
    case P2P_GET_INFOS:
        return ("P2P_GET_INFOS");
    case P2P_INFOS:
        return ("P2P_INFOS");
    case P2P_TRY_CLIENT_MODE:
        return ("P2P_TRY_CLIENT_MODE");
    case P2P_TRY_SERVER_MODE:
        return ("P2P_TRY_SERVER_MODE");
    default:
        return "<unknown>";
    }
}

P2P_msg *initP2PMsg(P2P_msg_type type) {
    P2P_msg *msg = malloc(sizeof(P2P_msg));
    memset(msg, 0, sizeof(P2P_msg));
    msg->type = type;
    msg->nb_user_online = 0;
    msg->private_port = -1;
    msg->public_port = -1;
    msg->try_port = -1;
    return 0;
}

void deinitP2PMsg(P2P_msg **msg) {
    char FUN_NAME[32] = "deinitP2PMsg";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    assertl(*msg, FILE_P2P_MSG, FUN_NAME, -1, "*msg NULL");

    free(*msg);
    *msg = NULL;
}

char *p2pMsgToTXT(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgToTXT";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");

    char *txt = malloc(SIZE_TXT);
    memset(txt, 0, SIZE_TXT);
    switch (msg->type) {
    case P2P_REQUEST_IN:
        snprintf(txt, SIZE_TXT, "REQUEST from <%s>", msg->user_id);
        break;
    case P2P_GET_INFOS:
        snprintf(txt, SIZE_TXT, "Request accepted");
        break;
    case P2P_REJECT:
        snprintf(txt, SIZE_TXT, "Request rejected");
        break;
    case P2P_AVAILABLE:
        for (unsigned i = 0; i < msg->nb_user_online; i++) {
            strncat(txt, msg->list_user_online[i], SIZE_TXT);
        }
        break;
    default:
        snprintf(txt, SIZE_TXT, "P2P_msg : %s", p2pMsgTypeToChar(msg->type));
    }
    return txt;
}

void p2pMsgIntoTXT(P2P_msg *msg, char *txt) {
    char FUN_NAME[32] = "p2pMsgIntoTXT";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    assertl(txt, FILE_P2P_MSG, FUN_NAME, -1, "txt NULL");

    char *TXT = p2pMsgToTXT(msg);
    strncpy(txt, TXT, SIZE_TXT);
    free(TXT);
}

void p2pMsgCopy(P2P_msg *msg_dst, P2P_msg *msg_src) {
    char FUN_NAME[32] = "p2pMsgCopy";
    assertl(msg_src, FILE_P2P_MSG, FUN_NAME, -1, "msg_src NULL");
    assertl(msg_dst, FILE_P2P_MSG, FUN_NAME, -1, "msg_dst NULL");

    memcpy(msg_dst, msg_src, sizeof(P2P_msg));
}

/*
    Getters on P2P_msg
*/

P2P_msg_type p2pMsgGetType(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgGetType";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    return msg->type;
}

char *p2pMsgGetUserId(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgGetUserId";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");

    char *id = malloc(SIZE_NAME);
    strncpy(id, msg->user_id, SIZE_NAME);
    return id;
}

char *p2pMsgGetPassword(P2P_msg *msg);

GenList *p2pMsgGetListUserOnline(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgGetListUserOnline";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");

    GenList *l = initGenList(MAX_ONLINE);
    char *id;
    for (unsigned i = 0; i < msg->nb_user_online; i++) {
        id = malloc(SIZE_NAME);
        strncpy(id, msg->list_user_online[i], SIZE_NAME);
        genListAdd(l, id);
    }
    return l;
}

int p2pMsgGetPublicPort(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgGetPublicPort";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");

    return msg->public_port;
}

int p2pMsgGetPrivatePort(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgGetPrivatePort";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    return msg->private_port;
}

char *p2pMsgGetPrivateIp(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgGetPrivateIp";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    char *ip = malloc(SIZE_IP_CHAR);
    strncpy(ip, msg->private_ip, SIZE_IP_CHAR);
    return ip;
}

char *p2pMsgGetTryIp(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgGetTryIp";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    char *ip = malloc(SIZE_IP_CHAR);
    strncpy(ip, msg->try_ip, SIZE_IP_CHAR);
    return ip;
}

int p2pMsgGetTryPort(P2P_msg *msg) {
    char FUN_NAME[32] = "p2pMsgGetTryPort";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    return msg->try_port;
}

P2P_error p2pMsgGetError(P2P_msg *msg);

/*
    Setteur on P2P_msg*
*/
void p2pMsgSetType(P2P_msg *msg, P2P_msg_type type) {
    char FUN_NAME[32] = "p2pMsgSetType";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    msg->type = type;
}

void p2pMsgSetUserId(P2P_msg *msg, char *user_id) {
    char FUN_NAME[32] = "p2pMsgSetUserId";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    assertl(user_id, FILE_P2P_MSG, FUN_NAME, -1, "usze_id NULL");

    strncpy(msg->user_id, user_id, SIZE_NAME);
}

void p2pMsgSetPassword(P2P_msg *msg, char *password);

void p2pMsgSetListUserOnline(P2P_msg *msg, GenList *list_online) {
    char FUN_NAME[32] = "p2pMsgSetListUserOnline";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    assertl(list_online, FILE_P2P_MSG, FUN_NAME, -1, "list_online NULL");
    char *id;
    msg->nb_user_online = 0;
    for (unsigned i = 0; i < genListSize(list_online) && i < MAX_ONLINE; i++) {
        strncpy(msg->list_user_online[i], genListGet(list_online, i), SIZE_NAME);
        msg->nb_user_online++;
    }
}

void p2pMsgSetPublicPort(P2P_msg *msg, int port) {
    char FUN_NAME[32] = "p2pMsgSetPublicPort";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");

    msg->public_port = port;
}

void p2pMsgSetPrivatePort(P2P_msg *msg, int port) {
    char FUN_NAME[32] = "p2pMsgSetPrivatePort";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");

    msg->private_port = port;
}

void p2pMsgSetPrivateIp(P2P_msg *msg, char *ip) {
    char FUN_NAME[32] = "p2pMsgSetPrivateIp";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    assertl(ip, FILE_P2P_MSG, FUN_NAME, -1, "ip NULL");

    strncpy(msg->private_ip, ip, SIZE_IP_CHAR);
}

void p2pMsgSetTryInfo(P2P_msg *msg, char *ip, int port) {
    char FUN_NAME[32] = "p2pMsgSetTryInfo";
    assertl(msg, FILE_P2P_MSG, FUN_NAME, -1, "msg NULL");
    assertl(ip, FILE_P2P_MSG, FUN_NAME, -1, "ip NULL");
    strncpy(msg->try_ip, ip, SIZE_IP_CHAR);
    msg->try_port = port;
}

void p2pMsgSetError(P2P_msg *msg, P2P_error);

/*
    debug
*/

void p2pMsgPrintl(P2P_msg *msg) {
    printl("\n| type : <%s>", p2pMsgTypeToChar(msg->type));
    printl("| user_id : <%s>", msg->user_id);
    printl("| nb_user_online : <%d>", msg->nb_user_online);
    for (unsigned i = 0; i < msg->nb_user_online; i++) {
        printl("  | <%s>", msg->list_user_online[i]);
    }
    printl("| public_port : <%d>", msg->public_port);
    printl("| private_port : <%d>", msg->private_port);
    printl("| private_ip : <%s>", msg->private_ip);
    printl("| try_port : <%d>", msg->try_port);
    printl("| try_ip : <%s>", msg->try_ip);
}