#include <client/tui.h>
#include <network/manager.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <types/command.h>
#include <types/message.h>
#include <types/p2p-msg.h>
#include <types/packet.h>
#include <utils/logger.h>

#define FILE_TUI "tui.c"

bool isValidCharacter(unsigned char character) {
    return character >= 'A' && character <= 'Z' || character >= 'a' && character <= 'z' ||
           character >= '0' && character <= '9' || character == '_';
}

bool isValidUserId(char *user_id) {
    for (int i = 0; user_id[i] == ' '; i++) {
        if (!isValidCharacter(user_id[i]))
            return false;
    }
    return true;
}

TUI_error stdinGetUserInput(char *buffer) {
    char FUN_NAME[32] = "stdinGetUserInput";
    buffer = calloc(SIZE_INPUT, sizeof(char));
    if (buffer == NULL) {
        warnl(FILE_TUI, FUN_NAME, "fail calloc buffer char[SIZE_DATA_PACKET]");
        return TUI_MEMORY_ALLOCATION_ERROR;
    }
    size_t size_allocated = SIZE_INPUT;
    if (getline(&buffer, &size_allocated, stdin) == -1) {
        warnl(FILE_TUI, FUN_NAME, "fail getline buffer");
        free(buffer);
        return TUI_INPUT_ERROR;
    }
    return TUI_SUCCESS;
}

void *stdinHandler(void *arg) {
    char FUN_NAME[32] = "stdinHandler";
    Manager *manager = (Manager *)arg;
    char *buffer, *token;
    TUI_error error;
    Command *command;
    Msg *msg;
    Packet *packet;
    char *user_id = managerGetUser(manager);
    bool exited = false;
    while (!exited) {
        switch (error = stdinGetUserInput(buffer)) {
        case TUI_SUCCESS:
            if (*buffer == '/') {
                command = initCommand(buffer);
                switch (command->cmd) {
                case CMD_LIST:
                    commandList(command, manager);
                    break;
                case CMD_REQUEST:
                    commandRequest(command, manager);
                    break;
                case CMD_ACCEPT:
                case CMD_REJECT:
                    commandAnswer(command, manager);
                    break;
                case CMD_CLOSE:
                    commandClose(command, manager);
                    break;
                case CMD_QUIT:
                    commandQuit(command, manager);
                    exited = true;
                    break;
                case CMD_UNKNOWN:
                    commandUnknown(command, manager);
                case CMD_HELP:
                    commandHelp(command, manager);
                    break;
                }
                deinitCommand(&command);
            } else {
                msg = initMsg(user_id, buffer);
                packet = initPacketMsg(msg);
                // TODO: Chat To be Implemented
                deinitMsg(&msg);
                deinitPacket(&packet);
            }
        case TUI_INPUT_ERROR:
        case TUI_MEMORY_ALLOCATION_ERROR:
            exit(error);
        default:
            warnl(FILE_TUI, FUN_NAME, "Unreachable !!!");
            exit(error);
        }
    }
    free(user_id);
    return NULL;
}

TUI_error stdoutDisplayPacket(Packet *packet) {
    char FUN_NAME[32] = "stdoutDisplayPacket";
    char *output;
    switch (packet->type) {
    case PACKET_TXT:
        printf("%s\n", packet->txt);
        break;
    case PACKET_MSG:
        if ((output = msgToTXT(&packet->msg)) == NULL) {
            warnl(FILE_TUI, FUN_NAME, "failed to format Msg to TXT");
            return TUI_OUTPUT_FORMATTING_ERROR;
        }
        printf("%s\n", output);
        free(output);
        break;
    case PACKET_P2P_MSG:
        if ((output = p2pMsgToTXT(&packet->p2p)) == NULL) {
            warnl(FILE_TUI, FUN_NAME, "failed to format p2pMsg to TXT");
            return TUI_OUTPUT_FORMATTING_ERROR;
        }
        printf("%s\n", output);
        free(output);
        break;
    }
    return TUI_SUCCESS;
}

void *stdoutHandler(void *arg) {
    char FUN_NAME[32] = "stdoutHandler";
    Manager *manager = (Manager *)arg;
    bool exited = false;
    Packet *packet;
    char *buffer;
    while (!exited) {
        switch (managerReceiveBlocking(manager, MANAGER_MOD_OUTPUT, &packet)) {
        case MANAGER_ERR_SUCCESS:
            if (stdoutDisplayPacket(packet) == TUI_OUTPUT_FORMATTING_ERROR) {
                buffer = packetTypeToString(&packet->type);
                printf("Couldn't display the recieved packet of type : %s\n", buffer);
                free(buffer);
            }
            break;
        case MANAGER_ERR_ERROR:
            warnl(FILE_TUI, FUN_NAME, "manager has encountered an error");
            break;
        case MANAGER_ERR_RETRY:
            break;
        case MANAGER_ERR_CLOSED:
            warnl(FILE_TUI, FUN_NAME, "manager was closed unexpectedly");
            exited = true;
            break;
        }
        deinitPacket(&packet);
    }
    return NULL;
}
