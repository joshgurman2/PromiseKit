#import <dispatch/object.h>
#import <dispatch/queue.h>
#import <Foundation/NSObject.h>
#import <PromiseKit/Swift.h>

typedef void (^PMKResolver)(id);

typedef NS_ENUM(NSInteger, PMKCatchPolicy) {
    PMKCatchPolicyAllErrors,
    PMKCatchPolicyAllErrorsExceptCancellation
};


/**
 @see AnyPromise.swift
*/
@interface AnyPromise (objc)

/**
 The provided block is executed when its receiver is resolved.

 If you provide a block that takes a parameter, the value of the receiver will be passed as that parameter.

 @param block The block that is executed when the receiver is resolved.

    [NSURLConnection GET:url].then(^(NSData *data){
        // do something with data
    });

 @return A new promise that is resolved with the value returned from the provided block. For example:

    [NSURLConnection GET:url].then(^(NSData *data){
        return data.length;
    }).then(^(NSNumber *number){
        //…
    });

 @warning *Important* The block passed to `then` may take zero, one, two or three arguments, and return an object or return nothing. This flexibility is why the method signature for then is `id`, which means you will not get completion for the block parameter, and must type it yourself. It is safe to type any block syntax here, so to start with try just: `^{}`.

 @warning *Important* If an exception is thrown inside your block, or you return an `NSError` object the next `Promise` will be rejected. See `catch` for documentation on error handling.

 @warning *Important* `then` is always executed on the main queue.

 @see thenOn
 @see thenInBackground
*/
- (AnyPromise *(^)(id))then;


/**
 The provided block is executed on the default queue when the receiver is fulfilled.

 This method is provided as a convenience for `thenOn`.

 @see then
 @see thenOn
*/
- (AnyPromise *(^)(id))thenInBackground;

/**
 The provided block is executed on the dispatch queue of your choice when the receiver is fulfilled.

 @see then
 @see thenInBackground
*/
- (AnyPromise *(^)(dispatch_queue_t, id))thenOn;

#ifndef __cplusplus
/**
 The provided block is executed when the receiver is rejected.

 Provide a block of form `^(NSError *){}` or simply `^{}`. The parameter has type `id` to give you the freedom to choose either.

 The provided block always runs on the main queue.
 
 @warning *Note* Cancellation errors are not caught.
 
 Note, since catch is a c++ keyword, this method is not availble in Objective-C++ files. Instead use catchWithPolicy.

 @see catchWithPolicy
*/
- (AnyPromise *(^)(id))catch;
#endif

/**
 The provided block is executed when the receiver is rejected with the specified policy.

 @param policy The policy with which to catch. Either for all errors, or all errors *except* cancellation errors.

 @see catch
*/
- (AnyPromise *(^)(PMKCatchPolicy, id))catchWithPolicy;

/**
 The provided block is executed when the receiver is resolved.

 The provided block always runs on the main queue.

 @see finallyOn
*/
- (AnyPromise *(^)(dispatch_block_t))finally;

/**
 The provided block is executed on the dispatch queue of your choice when the receiver is resolved.

 @see finally
 */
- (AnyPromise *(^)(dispatch_queue_t, dispatch_block_t))finallyOn;

/**
 Creates a resolved promise.

 When developing your own promise systems, it is ocassionally useful to be able to return an already resolved promise.

 @param value The value with which to resolve this promise. Passing an `NSError` will cause the promise to be rejected, otherwise the promise will be fulfilled.

 @return A resolved promise.
*/
+ (instancetype)promiseWithValue:(id)value;

/**
 Create a new promise with an associated resolver.

 Use this method when wrapping asynchronous code that does *not* use
 promises so that this code can be used in promise chains. Generally,
 prefer resolverWithBlock: as the resulting code is more elegant.

    PMKResolver resolve;
    AnyPromise *promise = [AnyPromise promiseWithResolver:&resolve];

    // later
    resolve(@"foo");

 @param resolver A reference to a block pointer of PMKResolver type.
 You can then call your resolver to resolve this promise.

 @return A new promise.

 @see promiseWithResolverBlock:
*/
+ (instancetype)promiseWithResolver:(PMKResolver __strong *)resolver;

/**
 Create a new promise that resolves with the provided block.

 Use this method when wrapping asynchronous code that does *not* use
 promises so that this code can be used in promise chains.
 
 If `resolve` is called with an `NSError` object, the promise is
 rejected, otherwise the promise is fulfilled.

 Don’t use this method if you already have promises! Instead, just
 return your promise.

 Should you need to fulfill a promise but have no sensical value to use:
 your promise is a `void` promise: fulfill with `nil`.

 The block you pass is executed immediately on the calling thread.

 @param block The provided block is immediately executed, inside the block
 call `resolve` to resolve this promise and cause any attached handlers to
 execute. If you are wrapping a delegate-based system, we recommend
 instead to use: promiseWithResolver:

 @return A new promise.

 @see http://promisekit.org/sealing-your-own-promises/
 @see http://promisekit.org/wrapping-delegation/
*/
+ (instancetype)promiseWithResolverBlock:(void (^)(PMKResolver resolve))resolverBlock;


/**
 The value of the asynchronous task this promise represents.

 A promise has `nil` value until the asynchronous task it represents completes.

 @return If `fulfilled` the object that was used to resolve this promise;
 if pending, nil; if rejected, nil.
 
 @warning *Important* The value implementation for PromiseKit 1.x incorrectly returned the rejected error if the promise was rejected. If you require this behavior still for porting a library @see PMKPromise.
*/
@property (nonatomic, readonly) id value;

@end



typedef void (^PMKAdapter)(id, NSError *);
typedef void (^PMKIntegerAdapter)(NSInteger, NSError *);
typedef void (^PMKBooleanAdapter)(BOOL, NSError *);

@interface AnyPromise (Adapters)

/**
 Create a new promise by adapting an existing asynchronous system.

 The pattern of a completion block that passes two parameters, the first
 the result and the second an `NSError` object is so common that we
 provide this convenience adapter to make wrapping such systems more
 elegant.

    return [PMKPromise promiseWithAdapter:^(PMKAdapter adapter){
        PFQuery *query = [PFQuery …];
        [query findObjectsInBackgroundWithBlock:adapter];
    }];

 @warning *Important* If both parameters are nil, the promise fulfills,
 if both are non-nil the promise rejects. This is per the convention.

 @see http://promisekit.org/sealing-your-own-promises/
 */
+ (instancetype)promiseWithAdapterBlock:(void (^)(PMKAdapter adapter))block;

/**
 Create a new promise by adapting an existing asynchronous system.

 Adapts asynchronous systems that complete with `^(NSInteger, NSError *)`.
 NSInteger will cast to enums provided the enum has been wrapped with
 `NS_ENUM`. All of Apple’s enums are, so if you find one that hasn’t you
 may need to make a pull-request.

 @see promiseWithAdapter
 */
+ (instancetype)promiseWithIntegerAdapterBlock:(void (^)(PMKIntegerAdapter adapter))block;

/**
 Create a new promise by adapting an existing asynchronous system.

 Adapts asynchronous systems that complete with `^(BOOL, NSError *)`.

 @see promiseWithAdapter
 */
+ (instancetype)promiseWithBooleanAdapterBlock:(void (^)(PMKBooleanAdapter adapter))block;

@end



@interface AnyPromise (when)
/**
 `when` is a mechanism for waiting more than one asynchronous task and responding when they are all complete.

 `when` accepts varied input. If an array is passed then when those promises fulfill, when’s promise fulfills with an array of fulfillment values. If a dictionary is passed then the same occurs, but when’s promise fulfills with a dictionary of fulfillments keyed as per the input.
 
 Interestingly, if a single promise is passed then when waits on that single promise, and if a single non-promise object is passed then when fulfills immediately with that object. If the array or dictionary that is passed contains objects that are not promises, then these objects are considered fulfilled promises. The reason we do this is to allow a pattern know as "abstracting away asynchronicity".

 If *any* of the provided promises reject, the returned promise is immediately rejected with that promise’s rejection error.

 @param input The input upon which to wait before resolving this promise.

 @return A promise that is resolved with either:

  1. An array of values from the provided array of promises.
  2. The value from the provided promise.
  3. The provided non-promise object.
*/
+ (instancetype)when:(id)input;
@end

@interface AnyPromise (join)
/**
 Creates a new promise that resolves only when all provided promises have resolved.

 Typically, you should use `+when:`.

 This promise is not rejectable.

 @param promises An array of promises.

 @return A promise that thens three parameters:

  1) An array of mixed values and errors from the resolved input.
  2) An array of values from the promises that fulfilled.
  3) An array of errors from the promises that rejected or nil if all promises fulfilled.

 @see when
*/
+ (instancetype)join:(NSArray *)promises;
@end

@interface AnyPromise (hang)
/**
 Literally hangs this thread until the promise has resolved.
 
 Do not use hang… unless you are testing, playing or debugging.
 
 If you use it in production code I will literally and honestly cry like a child.
 
 @warning *Important* t safe. It is not safe. It is not safe. It is not safe. It is no
 */
+ (instancetype)hang:(AnyPromise *)promise;
@end



/**
 Whenever resolving a promise you may resolve with a tuple, eg.
 returning from a `then` or `catch` handler or resolving a new promise.

 Consumers of your Promise are not compelled to consume any arguments and
 in fact will often only consume the first parameter. Thus ensure the
 order of parameters is: from most-important to least-important.

 Currently PromiseKit limits you to THREE parameters to the manifold.
*/
#define PMKManifold(...) __PMKManifold(__VA_ARGS__, 3, 2, 1)
#define __PMKManifold(_1, _2, _3, N, ...) __PMKArrayWithCount(N, _1, _2, _3)
extern id __PMKArrayWithCount(NSUInteger, ...);


#ifndef PMKNoBackCompat
#import <PromiseKit/PMKPromise.h>
#endif
