@import MessageUI;
@import UIKit;
#import "UIViewController+AnyPromise.h"
@import XCTest;

@interface TestUIViewControllerM: XCTestCase @end @implementation TestUIViewControllerM

- (void)setUp {
    [UIApplication sharedApplication].keyWindow.rootViewController = [UIViewController new];
}

- (void)tearDown {
    [UIApplication sharedApplication].keyWindow.rootViewController = nil;
}

- (void)testCanCancelMailComposeViewController {
    id rootvc = [UIApplication sharedApplication].keyWindow.rootViewController;
    id ex = [self expectationWithDescription:@""];

    MFMailComposeViewController *mf = [MFMailComposeViewController new];

    [mf setToRecipients:@[@"mxcl@me.com"]];

    [rootvc promiseViewController:mf animated:NO completion:^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.05 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            UIBarButtonItem *button = [mf.viewControllers[0] navigationItem].leftBarButtonItem;

            ((void (*)(id, SEL))[button.target methodForSelector:button.action])(button.target, button.action);
        });
    }].catchWithPolicy(PMKCatchPolicyAllErrors, ^{
        [ex fulfill];
    });

    [self waitForExpectationsWithTimeout:1 handler:nil];
}

@end
