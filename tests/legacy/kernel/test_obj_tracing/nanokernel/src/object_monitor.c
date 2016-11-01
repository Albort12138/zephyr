/* object_monitor.c - object monitor */

/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zephyr.h>
#include <tc_util.h>
#include <util_test_common.h>
#include <misc/debug/object_tracing.h>
#include "phil.h"

/**
 *
 * @brief Thread that traverses, counts and reports
 * the kernel objects in the philosophers application.
 *
 */

#define TOTAL_TEST_NUMBER 2

/* 1 IPM console fiber if enabled */
#if defined(CONFIG_IPM_CONSOLE_RECEIVER) && defined(CONFIG_PRINTK)
#define IPM_THREAD 1
#else
#define IPM_THREAD 0
#endif /* CONFIG_IPM_CONSOLE_RECEIVER && CONFIG_PRINTK*/

/* Must account for:
 *	N Philosopher fibers
 *	1 Object monitor fiber
 *	1 main() task
 *	1 IPM console fiber
 */

#define TOTAL_FIBERS (N_PHILOSOPHERS + 1 + IPM_THREAD)
#define TOTAL_TASKS 1
#define TOTAL_THREADS (TOTAL_FIBERS + TOTAL_TASKS)

#define OBJ_LIST_NAME nano_sem
#define OBJ_LIST_TYPE struct nano_sem

static inline int test_thread_monitor(void)
{
	int obj_counter = 0;
	struct tcs *thread_list = NULL;

	thread_list   = (struct tcs *)SYS_THREAD_MONITOR_HEAD;
	while (thread_list != NULL) {
		if (thread_list->prio == -1) {
			TC_PRINT("TASK: %p FLAGS: 0x%x\n",
			thread_list, thread_list->flags);
		} else {
			TC_PRINT("FIBER: %p FLAGS: 0x%x\n",
			thread_list, thread_list->flags);
		}
		thread_list =
			(struct tcs *)SYS_THREAD_MONITOR_NEXT(thread_list);
		obj_counter++;
	}
	TC_PRINT("THREAD QUANTITY: %d\n", obj_counter);

	if (obj_counter == TOTAL_THREADS) {
		TC_END_RESULT(TC_PASS);
		return 1;
	}

	TC_END_RESULT(TC_FAIL);
	return 0;
}

void object_monitor(void)
{
	int obj_counter;
	int test_counter = 0;
	void *obj_list   = NULL;

	TC_START("OBJECT TRACING TEST");

	obj_counter = 0;
	obj_list   = SYS_TRACING_HEAD(OBJ_LIST_TYPE, OBJ_LIST_NAME);
	while (obj_list != NULL) {
		TC_PRINT("SEMAPHORE REF: %p\n", obj_list);
		obj_list = SYS_TRACING_NEXT(OBJ_LIST_TYPE, OBJ_LIST_NAME,
						obj_list);
		obj_counter++;
	}
	TC_PRINT("SEMAPHORE QUANTITY: %d\n", obj_counter);

	if (obj_counter == N_PHILOSOPHERS) {
		TC_END_RESULT(TC_PASS);
		test_counter++;
	} else {
		TC_END_RESULT(TC_FAIL);
	}

	test_counter += test_thread_monitor();

	if (test_counter == TOTAL_TEST_NUMBER) {
		TC_END_REPORT(TC_PASS);
	} else {
		TC_END_REPORT(TC_FAIL);
	}
}
