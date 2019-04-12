/*!
 *  @file ckvo.h
 *  @brief C Key-Value Observing Module
 *  @details This module used to observer key-value changes.
 *
 *  Created by @author George Boumis
 *  @date 2019/04/12.
 *	@version 1.0
 *  @copyright Copyright (c) 2019 George Boumis <developer.george.boumis@gmail.com>. All rights reserved.
 *
 *  @defgroup ckvo Key-Value Observing Module
 *
 *	@mainpage Key-Value Observing Documentation
 *	@section intro_sec Introduction
 *	A little library in `C` for observing key-value changes. See @ref ckvo for documentation of this library.
 *	Please refer to [ckvo](https://github.com/averello/ckvo)
 */

#ifndef ckvo_h
#define ckvo_h

//#ifdef _cplusplus
//extern "C" {
//#endif

#define CKVO_NONNULL __attribute__ ((nonnull (1)))

typedef enum _CKVOObservingOptions {
    CKVOObservingOptionsNew = 1 << 0,
    CKVOObservingOptionsOld = 1 << 1
} CKVOObservingOptions;

typedef void (*CKVOCallback)(void *const observed,
                             const void *const old,
                             const void *const new,
                             const char *const key,
                             const void *const context);

typedef void *(*CKVOGetter)(const void *const observed);
typedef void (*CKVOSetter)(void *const observed, const void *const value);

/*!
 *  @typedef struct ckvo_observer_t CKVOObserver
 *  @brief An key-value observer.
 *  @ingroup ckvo
 *
 *  @discussion
 *  @a CKVOObserver is a structure used to hold the required information
 *  for observing a key-value pairs. Usually you create an observer using
 *  `CKVOObserverCreate` and you register it with `ckvo_add_observer_for_key`.
 *
 *  To observer a key-value pair change one should provide a bunch of
 *  information:
 *
 *  * The key of the property to observe. The key value must no be @a nil.
 *  * The options, which are a combination of the CKVOObservingOptions that
 *  specifies what is included in observation callbacks.
 *  * The obtain function, which obtains the value from the observable object
 *  * The callback function, which is the function to be called upon any
 *  key-value observation change.
 *  * The context, which is arbitrary data tha is passed to observer. Can be
 *  @a NULL.
 */
typedef struct ckvo_observer_t CKVOObserver;

/*!
 *  @typedef struct ckvo_support_t CKVOSupport
 *  @brief The structure you use to support key-value pair observation.
 *  @ingroup ckvo
 *
 *  @discussion
 *  @a CKVOSupport is a structure used to hold the required information
 *  for tracking observers. Usually you create a CKVOSupport by using
 *  `CKVOSupportCreate` passing the observable instance.
 */
typedef struct ckvo_support_t CKVOSupport;

CKVOObserver *CKVOObserverCreate(const char *const key,
                                 CKVOObservingOptions options,
                                 const void *const context,
                                 CKVOGetter obtain,
                                 CKVOCallback callback);

void CKVOObserverRelease(CKVOObserver *const observer);

CKVOSupport *CKVOSupportCreate(const void *const context);

void CKVOSupportRelease(CKVOSupport *const context);



void ckvo_add_observer_for_key(CKVOObserver *observer, CKVOSupport *support);
void ckvo_remove_observer_for_key(CKVOObserver *observer, CKVOSupport *support);

void ckvo_will_change_value_for_key(CKVOSupport *support, const char *const key);
void ckvo_did_change_value_for_key(CKVOSupport *support, const char *const key);

//#ifdef _cplusplus
//}
//#endif


#endif
