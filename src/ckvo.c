/*!
 *  @file ckvo.c
 *
 *  Created by @author George Boumis
 *  @date 2013/12/11.
 *	@version 1.1
 *  @copyright Copyright (c) 2013 George Boumis <developer.george.boumis@gmail.com>. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <errno.h>

#include <memory_management/memory_management.h>
#include "ckvo.h"

#ifndef TAILQ_FOREACH_SAFE
#define TAILQ_FOREACH_SAFE(var, head, field, tvar)          \
    for ((var) = TAILQ_FIRST((head));               \
        (var) && ((tvar) = TAILQ_NEXT((var), field), 1);        \
        (var) = (tvar))
#endif

struct ckvo_observer_t {
    // how to get the observed value
    CKVOGetter obtain;
    
    // callback function upon change
    CKVOCallback callback;
    
    // the key to observe
    const char *const key;
    // the options (old | new)
    CKVOObservingOptions options;
    // the observer's context
    const void *const context;
    
    // used for tracking values
    const void *oldValue;
    const void *newValue;
};


struct _list_observer_item {
    CKVOObserver *observer;
    TAILQ_ENTRY(_list_observer_item) items;
};

struct ckvo_support_t {
    // associated observers
    TAILQ_HEAD(ListHead, _list_observer_item) observers;
    
    // the pointer to the object that supports KVO
    const void *observable;
};

void CKVOSupportDealloc(void *const _support);

CKVOSupport *CKVOSupportCreate(const void *const context) {
    CKVOSupport *const support = MEMORY_MANAGEMENT_ALLOC(sizeof(CKVOSupport));
    TAILQ_INIT(&(support->observers));
    support->observable = context;
    MEMORY_MANAGEMENT_ATTRIBUTE_SET_DEALLOC_FUNCTION(support, CKVOSupportDealloc);
    return support;
}

void CKVOSupportRelease(CKVOSupport *const support) {
    release(support);
}

void CKVOSupportDealloc(void *const _support) {
    CKVOSupport *const support = _support;
    struct _list_observer_item *item, *tmp;
    TAILQ_FOREACH_SAFE(item, &(support->observers), items, tmp) {
        TAILQ_REMOVE(&(support->observers), item, items);
        release(item->observer);
        release(item);
    }
}

CKVOObserver *CKVOObserverCreate(const char *const key,
                                 CKVOObservingOptions options,
                                 const void *const context,
                                 CKVOGetter obtain,
                                 CKVOCallback callback) {
    CKVOObserver *const observer = MEMORY_MANAGEMENT_ALLOC(sizeof(CKVOObserver));
    const char **setKey = (const char **)&(observer->key);
    const void **setContext = (const void **)&(observer->context);
    *setKey = key;
    observer->options = options;
    *setContext = context;
    observer->obtain = obtain;
    observer->callback = callback;
    return observer;
}

void CKVOObserverRelease(CKVOObserver *const observer) {
    release(observer);
}


void ckvo_will_change_value_for_key(CKVOSupport *support, const char *const key) {
    struct _list_observer_item *item;
    TAILQ_FOREACH(item, &(support->observers), items) {
        CKVOObserver *const observer = item->observer;
        if (strcmp(observer->key, key) != 0) {
            continue;
        }
        if (observer->options & CKVOObservingOptionsOld) {
            observer->oldValue = observer->obtain(support->observable);
        }
    }
}

void ckvo_did_change_value_for_key(CKVOSupport *support, const char *const key) {
    struct _list_observer_item *item;
    TAILQ_FOREACH(item, &(support->observers), items) {
        CKVOObserver *const observer = item->observer;
        if (strcmp(observer->key, key) != 0) {
            continue;
        }
        if (observer->options & CKVOObservingOptionsNew) {
            observer->newValue = observer->obtain(support->observable);
        }
        observer->callback((void *)support->observable,
                           (observer->options & CKVOObservingOptionsOld) ? observer->oldValue : NULL,
                           (observer->options & CKVOObservingOptionsNew) ? observer->newValue : NULL,
                           observer->key,
                           observer->context);
    }
}

void ckvo_add_observer_for_key(CKVOObserver *observer, CKVOSupport *support) {
    struct _list_observer_item *item = MEMORY_MANAGEMENT_ALLOC(sizeof(struct _list_observer_item));
    item->observer = retain(observer);
    TAILQ_INSERT_TAIL(&(support->observers), item, items);
}

void ckvo_remove_observer_for_key(CKVOObserver *observer, CKVOSupport *support) {
    struct _list_observer_item *item, *tmp;
    TAILQ_FOREACH_SAFE(item, &(support->observers), items, tmp) {
        if (observer == item->observer) {
            TAILQ_REMOVE(&(support->observers), item, items);
            release(item->observer);
            release(item);
        }
    }
}
