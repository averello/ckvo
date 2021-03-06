//
//  testCKVO.c
//  ckvo
//
//  Created by George Boumis on 12/04/19.
//  Copyright (c) 2019 George Boumis <developer.george.boumis@gmail.com>. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory_management/memory_management.h>
#include "ckvo.h"

typedef struct _Object {
    int value;
    const char *header;
    CKVOSupport *kvo;
} Object;

int main(void);
Object *CreateObject(void);
void ObjectDealloc(void *_object);
int *ObjectObtainValue(Object * o);
void ObjectSetValue(Object *o, int value);
const char *ObjectObtainHeader(Object * o);
void ObjectSetHeader(Object *o, const char *const header);
static void ObjectObserveValueForKey(void *const observed,
                                     const void *const oldValue,
                                     const void *const newValue,
                                     const char *const key,
                                     const void *const context);

int main(void) {
    
    CKVOObserver *const valueObserver = CKVOObserverCreate("value", // key
                                                           CKVOObservingOptionsNew, // options
                                                           (void *)&main, // context
                                                           (void *(*)(const void *const ))ObjectObtainValue, // getter
                                                           ObjectObserveValueForKey // callback
                                                           );
    CKVOObserver *const valueObserverOtherContext = CKVOObserverCreate("value", // key
                                                                       CKVOObservingOptionsNew, // options
                                                                       (void *)&CreateObject, // context
                                                                       (void *(*)(const void *const ))ObjectObtainValue, // getter
                                                                       ObjectObserveValueForKey // callback
                                                                       );
    CKVOObserver *const headerObserver = CKVOObserverCreate("header", // key
                                                            CKVOObservingOptionsNew, // options
                                                            NULL, // context
                                                            (void *(*)(const void *const ))ObjectObtainHeader, // getter
                                                            ObjectObserveValueForKey // callback
                                                            );
    
    Object *const o = CreateObject();
    ckvo_add_observer_for_key(valueObserver, o->kvo);
    ckvo_add_observer_for_key(valueObserverOtherContext, o->kvo);
    ckvo_add_observer_for_key(headerObserver, o->kvo);
    ObjectSetValue(o, 2);
    ObjectSetHeader(o, "new_header");
    ckvo_remove_observer_for_key(valueObserver, o->kvo);
    ckvo_remove_observer_for_key(valueObserverOtherContext, o->kvo);
    ckvo_remove_observer_for_key(headerObserver, o->kvo);
    
    release(valueObserver);
    release(valueObserverOtherContext);
    release(headerObserver);
    
    release(o);
    return 0;
}

static void ObjectObserveValueForKey(void *const observed,
                                     const void *const oldValue,
                                     const void *const newValue,
                                     const char *const key,
                                     const void *const context) {
    if (context == (void *)&main) {
        printf("Observed.1.1 %p, old = %d, new = %d, key = \"%s\", context = main(%p)\n",
               observed, (int)oldValue, (int)newValue, key, context);
    }
    else if (strcmp(key, "header") == 0) {
        printf("Observed.2 %p, old = \"%s\", new = \"%s\", key = \"%s\", context = %p\n",
               observed, (const char *)oldValue, (const char *)newValue, key, context);
    }
    else {
        printf("Observed.1.2 %p, old = %d, new = %d, key = \"%s\", context = other(%p)\n",
               observed, (int)oldValue, (int)newValue, key, context);
    }
}

Object *CreateObject(void) {
    Object *const o = MEMORY_MANAGEMENT_ALLOC(sizeof(Object));
    o->value = 0;
    o->header = "default_header";
    o->kvo = CKVOSupportCreate(o);
    MEMORY_MANAGEMENT_ATTRIBUTE_SET_DEALLOC_FUNCTION(o, ObjectDealloc);
    return o;
}

void ObjectDealloc(void *_object) {
    Object *const object = _object;
    release(object->kvo);
}

int *ObjectObtainValue(Object * o) {
    return (int *)((uint64_t)o->value);
}

void ObjectSetValue(Object *o, int value) {
    ckvo_will_change_value_for_key(o->kvo, "value");
    o->value = value;
    ckvo_did_change_value_for_key(o->kvo, "value");
}

const char *ObjectObtainHeader(Object * o) {
    return o->header;
}

void ObjectSetHeader(Object *o, const char *const header) {
    ckvo_will_change_value_for_key(o->kvo, "header");
    o->header = header;
    ckvo_did_change_value_for_key(o->kvo, "header");
}
