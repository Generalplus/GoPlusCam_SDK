//
//  AppDelegate.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/7/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "AppDelegate.h"
#import "MZFormSheetController.h"
#import "ProtocolAgent.h"
#import "CrashReporter/CrashReporter.h"
#import "LogDataHelper.h"
#import "UIButton+InsensitiveTouch.h"

#import <sys/types.h>
#import <sys/sysctl.h>

@interface AppDelegate ()
{

}

@end

@implementation AppDelegate
@synthesize stdErrRedirected;

static int savedStdErr = 0;

- (void)redirectStdErrToFile
{
    if (!stdErrRedirected)
    {
        stdErrRedirected = YES;
        savedStdErr = dup(STDERR_FILENO);
        
        NSError *error;
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectory = [paths objectAtIndex:0]; // Get documents folder
        NSString *dataPath = [documentsDirectory stringByAppendingPathComponent:@"/LOGCAT"];
        
        if (![[NSFileManager defaultManager] fileExistsAtPath:dataPath])
            [[NSFileManager defaultManager] createDirectoryAtPath:dataPath withIntermediateDirectories:NO attributes:nil error:&error]; //Create folder

        
        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateFormat:@"yyyy_MM_dd_hh_mm_ss"];
        NSString *dateString = [dateFormatter stringFromDate:[NSDate date]];
        
        NSString *logPath = [dataPath stringByAppendingPathComponent:[NSString stringWithFormat:@"LOGCAT_%@_Log.log",dateString]];
        freopen([logPath cStringUsingEncoding:NSASCIIStringEncoding], "a+", stderr);
        freopen([logPath cStringUsingEncoding:NSASCIIStringEncoding], "a+", stdin);
        freopen([logPath cStringUsingEncoding:NSASCIIStringEncoding], "a+", stdout);
    }
}

- (void)restoreStdErr
{
    if (stdErrRedirected)
    {
        stdErrRedirected = NO;
        fflush(stderr);
        
        dup2(savedStdErr, STDERR_FILENO);
        close(savedStdErr);
        savedStdErr = 0;
    }
}

//---------------------------------------------------------------------------------------
/* A custom post-crash callback */
void post_crash_callback (siginfo_t *info, ucontext_t *uap, void *context) {
    // this is not async-safe, but this is a test implementation
    NSLog(@"post crash callback: signo=%d, uap=%p, context=%p", info->si_signo, uap, context);
    
    PLCrashReporter *pShareReport =  [PLCrashReporter sharedReporter];
    save_crash_report(pShareReport);
    [[LogDataHelper GetShareHelper] SaveToFile];
}
//---------------------------------------------------------------------------------------
static bool debugger_should_exit (void) {
#if !TARGET_OS_IPHONE
    return false;
#endif
    
    struct kinfo_proc info;
    size_t info_size = sizeof(info);
    int name[4];
    
    name[0] = CTL_KERN;
    name[1] = KERN_PROC;
    name[2] = KERN_PROC_PID;
    name[3] = getpid();
    
    if (sysctl(name, 4, &info, &info_size, NULL, 0) == -1) {
        NSLog(@"sysctl() failed: %s", strerror(errno));
        return false;
    }
    
    if ((info.kp_proc.p_flag & P_TRACED) != 0)
        return true;
    
    return false;
}
//---------------------------------------------------------------------------------------
/* If a crash report exists, make it accessible via iTunes document sharing. This is a no-op on Mac OS X. */
static void save_crash_report (PLCrashReporter *reporter) {
    if (![reporter hasPendingCrashReport])
        return;
    
#if TARGET_OS_IPHONE
    NSFileManager *fm = [NSFileManager defaultManager];
    NSError *error;
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    if (![fm createDirectoryAtPath: documentsDirectory withIntermediateDirectories: YES attributes:nil error: &error]) {
        NSLog(@"Could not create documents directory: %@", error);
        return;
    }
    
    
    NSData *data = [[PLCrashReporter sharedReporter] loadPendingCrashReportDataAndReturnError: &error];
    if (data == nil) {
        NSLog(@"Failed to load crash report data: %@", error);
        return;
    }
    
    NSString *outputPath = [documentsDirectory stringByAppendingPathComponent: @"GoPlusCam.plcrash"];
    if (![data writeToFile: outputPath atomically: YES]) {
        NSLog(@"Failed to write crash report");
    }
    
    NSLog(@"Saved crash report to: %@", outputPath);
#endif
}

//---------------------------------------------------------------------------------------
//
// Called to handle a pending crash report.
//
- (void) handleCrashReport
{
    NSLog(@"Install crash report.");
    
    NSError *error = nil;
    /* Configure our reporter */
    PLCrashReporterConfig *config = [[PLCrashReporterConfig alloc] initWithSignalHandlerType: PLCrashReporterSignalHandlerTypeMach
                                                                        symbolicationStrategy: PLCrashReporterSymbolicationStrategyAll];
    PLCrashReporter *reporter = [[PLCrashReporter alloc] initWithConfiguration: config];
    
    /* Set up post-crash callbacks */
    PLCrashReporterCallbacks cb = {
        .version = 0,
        .context = (void *) 0xABABABAB,
        .handleSignal = post_crash_callback
    };
    [reporter setCrashCallbacks: &cb];
    
    /* Enable the crash reporter */
    if (![reporter enableCrashReporterAndReturnError: &error]) {
        NSLog(@"Could not enable crash reporter: %@", error);
    }
}
//---------------------------------------------------------------------------------------
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    if(!debugger_should_exit())
        [self handleCrashReport];
    
    [UIButton enableInsensitiveTouch];
    
    NSDictionary *dicDefault = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithInt:40], NSUserDefaults_SetBufferingTime,nil];
    [[NSUserDefaults standardUserDefaults] registerDefaults:dicDefault];
    return YES;
}
//---------------------------------------------------------------------------------------
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 90000
- (UIInterfaceOrientationMask)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window
#else
- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window
#endif
{
    NSUInteger orientations = UIInterfaceOrientationMaskAll;
    
    if ([MZFormSheetController formSheetControllersStack] > 0) {
        MZFormSheetController *viewController = [[MZFormSheetController formSheetControllersStack] lastObject];
        return [viewController.presentedFSViewController supportedInterfaceOrientations];
    }
    
    return orientations;
}
//---------------------------------------------------------------------------------------
- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}
//---------------------------------------------------------------------------------------
- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}
//---------------------------------------------------------------------------------------
- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
    
    [[UIApplication sharedApplication]setIdleTimerDisabled:YES];
}
//---------------------------------------------------------------------------------------
- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}
//---------------------------------------------------------------------------------------
- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}
//---------------------------------------------------------------------------------------
@end
