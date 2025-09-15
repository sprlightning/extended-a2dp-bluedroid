/**
 * @file lhdc_v5_dec_workspace.c
 * @brief LHDC V5解码器工作区实现
 */
#include "lhdc_v5_dec_workspace.h"
#include <string.h>

static const char* error_codes[] = {
    "No error",
    "Workspace not initialized",
    "Insufficient workspace memory",
    "Null pointer error"
};

int lhdcv5_dec_workspace_init(lhdcv5_dec_workspace_t *workspace) {
    if (!workspace) {
        return 3; // Null pointer error
    }
    
    memset(workspace, 0, sizeof(lhdcv5_dec_workspace_t));
    workspace->is_initialized = 1;
    workspace->last_error = 0;
    
    return 0;
}

void lhdcv5_dec_workspace_deinit(lhdcv5_dec_workspace_t *workspace) {
    if (workspace && workspace->is_initialized) {
        memset(workspace->main_workspace, 0, LHDCV5_DEC_WORKSPACE_SIZE);
        memset(workspace->frame_info, 0, LHDCV5_FRAME_INFO_SIZE);
        memset(workspace->temp_buffer, 0, LHDCV5_TEMP_BUFFER_SIZE);
        workspace->is_initialized = 0;
        workspace->workspace_used = 0;
        workspace->last_error = 0;
    }
}

int lhdcv5_dec_workspace_check_space(lhdcv5_dec_workspace_t *workspace, uint32_t size) {
    if (!workspace || !workspace->is_initialized) {
        if (workspace) workspace->last_error = 1;
        return 0;
    }
    
    if (workspace->workspace_used + size > LHDCV5_DEC_WORKSPACE_SIZE) {
        workspace->last_error = 2;
        return 0;
    }
    
    return 1;
}

const char* lhdcv5_dec_workspace_get_status(lhdcv5_dec_workspace_t *workspace) {
    if (!workspace) {
        return error_codes[3];
    }
    
    if (workspace->last_error < sizeof(error_codes)/sizeof(error_codes[0])) {
        return error_codes[workspace->last_error];
    }
    
    return "Unknown error";
}