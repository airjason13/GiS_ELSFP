#ifndef APP_TASK_IDS_H
#define APP_TASK_IDS_H
/* Index into shared.task[]. Each Task writes ONLY its own slot. */
typedef enum {
    APP_TASK_HOST=0, APP_TASK_MSM, APP_TASK_LASER, APP_TASK_TEC,
    APP_TASK_DDM, APP_TASK_SAFETY, APP_TASK_STORAGE, APP_TASK_DEBUG,
    APP_TASK_COUNT
} AppTaskId_t;
#endif
