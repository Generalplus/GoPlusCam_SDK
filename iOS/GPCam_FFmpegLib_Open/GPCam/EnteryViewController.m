//
//  EnteryViewController.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/28/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "EnteryViewController.h"
#import "ProtocolAgent.h"
#import "LogDataHelper.h"
#import "Reachability.h"
#import "SimplePingHelper.h"

#import "MZFormSheetController.h"
#import "MZCustomTransition.h"
#import "MZFormSheetSegue.h"
#import "AppDelegate.h"
#include <ifaddrs.h>
#include <arpa/inet.h>
#import <net/if.h>

@interface EnteryViewController ()<UIAlertViewDelegate>
{
    int m_iSetTime;
    int m_iSaveLog;
    BOOL m_bSetDiscardcorrupt;
    NSTimer *m_aliveTimer;
    NSString* m_strAliveIP;
}

@property (retain, nonatomic) IBOutlet UILabel      *VersionLabel;
@property (retain, nonatomic) IBOutlet UIButton     *ConnectButton;

@end

@implementation EnteryViewController
//-----------------------------------------------------------------
-(void)ShowAlert:(NSString *)Title
         Message:(NSString *)Message
         handler:(void (^ __nullable)(UIAlertAction *action))Userhandler
{
    UIAlertController *alertController = [UIAlertController
                                          alertControllerWithTitle:Title
                                          message:Message
                                          preferredStyle:UIAlertControllerStyleAlert];
    
    UIAlertAction *okAction = [UIAlertAction
                               actionWithTitle:NSLocalizedString(@"OK", @"OK action")
                               style:UIAlertActionStyleDefault
                               handler:Userhandler];
    
    [alertController addAction:okAction];
    [self presentViewController:alertController animated:YES completion:nil];
}
//-----------------------------------------------------------------
- (BOOL) IsDeviceAlive {
    
    return true;
}
//-----------------------------------------------------------------
- (BOOL) IsConnectToWifi {
    return ([[Reachability reachabilityForLocalWiFi] currentReachabilityStatus] == ReachableViaWiFi);
}
//-----------------------------------------------------------------
- (void)EnableButtons:(bool)bEnable
{
    if(bEnable)
    {
        _ConnectButton.enabled = true;
        _ConnectButton.alpha = 1.0;
    }
    else
    {
        _ConnectButton.enabled = false;
        _ConnectButton.alpha = 0.5;
    }
    
}
//-----------------------------------------------------------------
-(IBAction)prepareForEntryUnwind:(UIStoryboardSegue *)segue
{
    if([[MZFormSheetController formSheetControllersStack] lastObject])
        [[[MZFormSheetController formSheetControllersStack] lastObject] dismissAnimated:YES completionHandler:nil];
    
    NSLog(@"Lost connection with device");
}
//-----------------------------------------------------------------
//get ip address
- (NSString *)getIpAddresses{
    NSString *address = @"error";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    NSLog(@"success = %d",success);
    if (success == 0)
    {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        if (temp_addr == NULL ) {
            NSLog(@"temp_addr = null");
        }
        
        while(temp_addr != NULL)
        {
            NSLog(@"temp_addr->ifa_addr->sa_family = %d",temp_addr->ifa_addr->sa_family);
            if(temp_addr->ifa_addr->sa_family == AF_INET)
            {
                NSLog(@"temp_addr->ifa_name = %@",[NSString stringWithUTF8String:temp_addr->ifa_name]);
                // Check if interface is en0 which is the wifi connection on the iPhone
                if([[NSString stringWithUTF8String:temp_addr->ifa_name] isEqualToString:@"en0"])
                {
                    // Get NSString from C String
                    address = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)];
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    return address;
}

//-----------------------------------------------------------------
-(IBAction)PressConnect:(id)sender
{
    [self EnableButtons:false];

    NSString *ipString = [EnteryViewController getIPAddress:true];//[self getIpAddresses];
    const char * cString = COMMAND_IP;
    NSLog(@"getIpAddresses = %@",ipString);
    NSString *serverAddress = @"";
    
    BOOL b192 = NO;
    if (ipString.length > 3) {
        b192 = [[ipString substringWithRange:NSMakeRange(0, 3)] isEqualToString:@"192"];
    }
    if([ipString isEqualToString:@"error"] || !b192){
        [self pingResult:[NSNumber numberWithBool:NO]];
        return;
    }
    else{
        m_strAliveIP = ipString;
        NSArray *aArray = [ipString componentsSeparatedByString:@"."];
        serverAddress = [NSString stringWithFormat:@"%@.%@.%@.%@", aArray[0], aArray[1],aArray[2],@"1"];
        [ProtocolAgent GetShareAgent].m_strCommandIP = serverAddress;
        
//        [ProtocolAgent GetShareAgent].m_strCommandIP = [NSString stringWithFormat:@"0:0:0:0:0:ffff:%@%@:%@%@"
//        , [NSString stringWithFormat:@"%02x",[aArray[0] intValue]&0xff]
//        , [NSString stringWithFormat:@"%02x",[aArray[1] intValue]&0xff]
//        , [NSString stringWithFormat:@"%02x",[aArray[2] intValue]&0xff]
//        , [NSString stringWithFormat:@"%02x",1&0xff]];
        cString = [serverAddress UTF8String];
    }
    NSLog(@"[ProtocolAgent GetShareAgent].m_strCommandIP = %@",[ProtocolAgent GetShareAgent].m_strCommandIP);
    
    [SimplePingHelper ping:[NSString stringWithCString:cString encoding:NSUTF8StringEncoding]
                    target:self
                       sel:@selector(pingResult:)
        ];
}
//------------------------- ----------------------------------------
- (void)pingResult:(NSNumber*)success
{

    if(![success boolValue])
    {
        m_strAliveIP = @"";
        [self ShowAlert:NSLocalizedString(@"Wifi sport Cam not found","")
                Message:NSLocalizedString(@"Please connect to Wifi sport Cam first.","")
                handler:^(UIAlertAction *action)
        {
            [self openURL];
            }
        ];
        [self EnableButtons:true];
        
            return ;
    }
    
    [[ProtocolAgent GetShareAgent] ConnectToDevice:[[ProtocolAgent GetShareAgent].m_strCommandIP UTF8String]
                                          WithPort:COMMAND_PORT];
    
    [[LogDataHelper GetShareHelper] ResetLog];
        
    [self EnableButtons:true];

}
//------------------------- ----------------------------------------
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    [ProtocolAgent GetShareAgent].ActiveView = self.view;
    [ProtocolAgent GetShareAgent].AlertViewController = self;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(ConnectionChangeNotification:)
                                                 name:Notification_Device_ConnectionChange
                                               object:nil];
    
    if(m_aliveTimer)
    {
        [m_aliveTimer invalidate];
        m_aliveTimer = nil;
    }
    m_strAliveIP = @"";
}
//------------------------- ----------------------------------------
- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:Notification_Device_ConnectionChange
                                                  object:nil];
}
//----------------------------------------------------------------------
-(void) aliveTimerHandler:(NSTimer*) timer
{
    NSString *ipString = [EnteryViewController getIPAddress:true];
    
    if([ipString isEqualToString:@"error"]){
        
        [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_ConnectionChange
                                                            object:@"DisConnected"];
        NSLog(@"error disconnect");
        return;
    }
    if(NO == [ipString isEqualToString:m_strAliveIP]) {
        [[NSNotificationCenter defaultCenter] postNotificationName:Notification_Device_ConnectionChange
                                                            object:@"DisConnected"];
        NSLog(@"ip disconnect");
    }
}
//----------------------------------------------------------------------
- (void) ConnectionChangeNotification:(NSNotification*) notification
{
    NSString *Status = [notification object];
    
    if([Status isEqualToString:@"Connected"])
    {
        dispatch_async(dispatch_get_main_queue(), ^{

            [self performSegueWithIdentifier:@"Show_Main" sender:self];
            
            if(m_aliveTimer)
            {
                [m_aliveTimer invalidate];
                m_aliveTimer = nil;
            }
            
            m_aliveTimer = [NSTimer scheduledTimerWithTimeInterval:0.5
                                                             target:self
                                                           selector:@selector(aliveTimerHandler:)
                                                           userInfo:nil
                                                            repeats:YES];
        });
        
    }
    else {
        [self openURL];
    }
    
}
//-----------------------------------------------------------------
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    _VersionLabel.text = [NSString stringWithFormat:@"V%@ B%@",
                          [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"],
                          [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"]];
    
    UITapGestureRecognizer *tapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(labelTapped)];
    tapGestureRecognizer.numberOfTapsRequired = 1;
    [_VersionLabel addGestureRecognizer:tapGestureRecognizer];
    _VersionLabel.userInteractionEnabled = YES;
    
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    m_iSetTime = (int)[userDefaults integerForKey:NSUserDefaults_SetTime];
    m_bSetDiscardcorrupt = (BOOL)[userDefaults boolForKey:NSUserDefaults_SetDiscardcorrupt];
    m_iSaveLog = 0;
    
}
//-----------------------------------------------------------------
- (void)labelTapped {
    UIAlertView * alert =[[UIAlertView alloc ] initWithTitle:NSLocalizedString(@"Setting","")
                                                     message:nil
                                                    delegate:self
                                           cancelButtonTitle:NSLocalizedString(@"Cancel","")
                                           otherButtonTitles:NSLocalizedString(@"Set Wifi sport Cam Time",""),NSLocalizedString(@"Save Log",""),NSLocalizedString(@"Set Buffering Time","") ,nil];
                          //, NSLocalizedString(@"Discard corrupt","") ,nil];
    alert.tag = 0;
    [alert show];
}
//-----------------------------------------------------------------
- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
    if (0 == alertView.tag) {
        if (buttonIndex == 0)
        {
            NSLog(@"You have clicked Cancel");
        }
        else if(buttonIndex == 1)
        {
            UIAlertController *alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"Set Wifi sport Cam Time","")
                                                                           message:nil
                                                                    preferredStyle:UIAlertControllerStyleAlert];
            
            
            UIAlertActionStyle style1;
            UIAlertActionStyle style2;
            if (0 == m_iSetTime) {
                style1 = UIAlertActionStyleDestructive;
                style2 = UIAlertActionStyleDefault;
            }
            else {
                style1 = UIAlertActionStyleDefault;
                style2 = UIAlertActionStyleDestructive;
            }
            UIAlertAction *Action1 = [UIAlertAction actionWithTitle:NSLocalizedString(@"Disable","")
                                                              style:style1
                                                            handler:^(UIAlertAction *action) {
                                                                m_iSetTime = 0;
                                                                [[NSUserDefaults standardUserDefaults] setInteger:m_iSetTime forKey:NSUserDefaults_SetTime];
                                                                [[NSUserDefaults standardUserDefaults] synchronize];
                                                                
                                                            }];
            [alert addAction:Action1];
            
            UIAlertAction *Action2 = [UIAlertAction actionWithTitle:NSLocalizedString(@"Enable","")
                                                              style:style2
                                                            handler:^(UIAlertAction *action) {
                                                                m_iSetTime = 1;
                                                                [[NSUserDefaults standardUserDefaults] setInteger:m_iSetTime forKey:NSUserDefaults_SetTime];
                                                                [[NSUserDefaults standardUserDefaults] synchronize];
                                                                
                                                            }];
            [alert addAction:Action2];
            
            UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"Cancel","") style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                
            }];
            [alert addAction:cancelAction];
            
            [self presentViewController:alert animated:YES completion:nil];
        }
        else if(buttonIndex == 2)
        {
            UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Save Log"
                                                                           message:nil
                                                                    preferredStyle:UIAlertControllerStyleAlert];
            
            UIAlertActionStyle style1;
            UIAlertActionStyle style2;
            if (0 == m_iSaveLog) {
                style1 = UIAlertActionStyleDestructive;
                style2 = UIAlertActionStyleDefault;
            }
            else {
                style1 = UIAlertActionStyleDefault;
                style2 = UIAlertActionStyleDestructive;
            }
            UIAlertAction *Action1 = [UIAlertAction actionWithTitle:NSLocalizedString(@"Disable","")
                                                              style:style1
                                                            handler:^(UIAlertAction *action) {
                                                                m_iSaveLog = 0;
                                                                AppDelegate* delegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
                                                                [delegate restoreStdErr];
                                                            }];
            [alert addAction:Action1];
            
            UIAlertAction *Action2 = [UIAlertAction actionWithTitle:NSLocalizedString(@"Enable","")
                                                              style:style2
                                                            handler:^(UIAlertAction *action) {
                                                                m_iSaveLog = 1;
                                                                AppDelegate* delegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
                                                                [delegate redirectStdErrToFile];
                                                            }];
            [alert addAction:Action2];
            
            UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"Cancel","") style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                
            }];
            [alert addAction:cancelAction];
            
            [self presentViewController:alert animated:YES completion:nil];
        }
        else if(buttonIndex == 3)
        {
            UIAlertController *alertController = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"Set Buffering Time","")
                                                                                     message:@"\n\n\n"
                                                                              preferredStyle:UIAlertControllerStyleAlert];

            alertController.view.autoresizesSubviews = YES;
            UITextView *textView = [[UITextView alloc] initWithFrame:CGRectZero];
            textView.translatesAutoresizingMaskIntoConstraints = NO;
            textView.editable = YES;
            textView.dataDetectorTypes = UIDataDetectorTypeAll;
            textView.textColor = [UIColor blackColor];
            
            NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
            int myInt = (int)[userDefaults integerForKey:NSUserDefaults_SetBufferingTime];
            textView.text = [[NSString alloc]initWithFormat:@"%i", myInt];
            textView.userInteractionEnabled = YES;
            textView.backgroundColor = [UIColor whiteColor];
            textView.scrollEnabled = YES;
            textView.keyboardType = UIKeyboardTypeNumberPad;
            NSLayoutConstraint *leadConstraint = [NSLayoutConstraint constraintWithItem:alertController.view attribute:NSLayoutAttributeLeading relatedBy:NSLayoutRelationEqual toItem:textView attribute:NSLayoutAttributeLeading multiplier:1.0 constant:-8.0];
            NSLayoutConstraint *trailConstraint = [NSLayoutConstraint constraintWithItem:alertController.view attribute:NSLayoutAttributeTrailing relatedBy:NSLayoutRelationEqual toItem:textView attribute:NSLayoutAttributeTrailing multiplier:1.0 constant:8.0];

            NSLayoutConstraint *topConstraint = [NSLayoutConstraint constraintWithItem:alertController.view attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:textView attribute:NSLayoutAttributeTop multiplier:1.0 constant:-64.0];
            NSLayoutConstraint *bottomConstraint = [NSLayoutConstraint constraintWithItem:alertController.view attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:textView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:64.0];
            [alertController.view addSubview:textView];
            [NSLayoutConstraint activateConstraints:@[leadConstraint, trailConstraint, topConstraint, bottomConstraint]];

            
            UIAlertAction* okay = [UIAlertAction actionWithTitle:NSLocalizedString(@"OK", @"OK action") style:UIAlertActionStyleDefault
                                                       handler:^(UIAlertAction * action) {
                                                           NSLog(@"%@", textView.text);
                
                NSCharacterSet* notDigits = [[NSCharacterSet decimalDigitCharacterSet] invertedSet];

                if ([textView.text rangeOfCharacterFromSet:notDigits].location == NSNotFound)
                {
                    // newString consists only of the digits 0 through 9
                    int myInt = [textView.text intValue];
                    [[NSUserDefaults standardUserDefaults] setInteger:myInt forKey:NSUserDefaults_SetBufferingTime];
                                   [[NSUserDefaults standardUserDefaults] synchronize];
                }
                
               
                
                                                       }];
            UIAlertAction* cancel1 = [UIAlertAction actionWithTitle:NSLocalizedString(@"Cancel","") style:UIAlertActionStyleDefault
                                                           handler:^(UIAlertAction * action) {
                                                               [alertController dismissViewControllerAnimated:YES completion:nil];
                                                           }];
            [alertController addAction:okay];
            [alertController addAction:cancel1];
            [self presentViewController:alertController animated:YES completion:^{

            }];
        }
        else if(buttonIndex == 4)
        {
            UIAlertController *alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"Discard corrupt","")
                                                                           message:nil
                                                                    preferredStyle:UIAlertControllerStyleAlert];
            
            UIAlertActionStyle style1;
            UIAlertActionStyle style2;
            if (NO == m_bSetDiscardcorrupt) {
                style1 = UIAlertActionStyleDestructive;
                style2 = UIAlertActionStyleDefault;
            }
            else {
                style1 = UIAlertActionStyleDefault;
                style2 = UIAlertActionStyleDestructive;
            }
            UIAlertAction *Action1 = [UIAlertAction actionWithTitle:NSLocalizedString(@"Disable","")
                                                              style:style1
                                                            handler:^(UIAlertAction *action) {
                m_bSetDiscardcorrupt = NO;
                [[NSUserDefaults standardUserDefaults] setBool:m_bSetDiscardcorrupt forKey:NSUserDefaults_SetDiscardcorrupt];
                [[NSUserDefaults standardUserDefaults] synchronize];
                AppDelegate* delegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
                [delegate restoreStdErr];
                                                            }];
            [alert addAction:Action1];
            
            UIAlertAction *Action2 = [UIAlertAction actionWithTitle:NSLocalizedString(@"Enable","")
                                                              style:style2
                                                            handler:^(UIAlertAction *action) {
                m_bSetDiscardcorrupt = YES;
                [[NSUserDefaults standardUserDefaults] setBool:m_bSetDiscardcorrupt forKey:NSUserDefaults_SetDiscardcorrupt];
                [[NSUserDefaults standardUserDefaults] synchronize];
                AppDelegate* delegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
                [delegate redirectStdErrToFile];
                                                            }];
            [alert addAction:Action2];
            
            UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"Cancel","") style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                
            }];
            [alert addAction:cancelAction];
            
            [self presentViewController:alert animated:YES completion:nil];
        }
    }
    
    
}
//-----------------------------------------------------------------
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
//-----------------------------------------------------------------
-(UIStatusBarStyle)preferredStatusBarStyle {
    return UIStatusBarStyleLightContent;
}
//-----------------------------------------------------------------
/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

- (void)openURL {
//    // 亲测：iOS 8.1 ~ iOS11.3
//    // 跳转到设置 - 相机 / 该应用的设置界面
//    NSURL *url1 = [NSURL URLWithString:@"App-Prefs:root=WIFI"];
//    // iOS10也可以使用url2访问，不过使用url1更好一些，可具体根据业务需求自行选择
//    NSURL *url2 = [NSURL URLWithString:@"App-Prefs:root"];
//    if (@available(iOS 11.0, *)) {
//        [[UIApplication sharedApplication] openURL:url2 options:@{} completionHandler:nil];
//    } else {
//        if ([[UIApplication sharedApplication] canOpenURL:url1]){
//            if (@available(iOS 10.0, *)) {
//                [[UIApplication sharedApplication] openURL:url1 options:@{} completionHandler:nil];
//            } else {
//                [[UIApplication sharedApplication] openURL:url1];
//            }
//        }
//    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        NSURL *url = [NSURL URLWithString:UIApplicationOpenSettingsURLString];
        if ([[UIApplication sharedApplication] canOpenURL:url]){
            if (@available(iOS 10.0, *)) {
                [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
            } else {
                [[UIApplication sharedApplication] openURL:url];
            }
        }
    });
}

#define IOS_CELLULAR    @"pdp_ip0"
#define IOS_WIFI        @"en0"
#define IOS_VPN         @"utun0"
#define IP_ADDR_IPv4    @"ipv4"
#define IP_ADDR_IPv6    @"ipv6"
+ (NSString *)getIPAddress:(BOOL)preferIPv4
{
    NSArray *searchArray = preferIPv4 ?
    @[ IOS_VPN @"/" IP_ADDR_IPv4, IOS_VPN @"/" IP_ADDR_IPv6, IOS_WIFI @"/" IP_ADDR_IPv4, IOS_WIFI @"/" IP_ADDR_IPv6, IOS_CELLULAR @"/" IP_ADDR_IPv4, IOS_CELLULAR @"/" IP_ADDR_IPv6 ] :
    @[ IOS_VPN @"/" IP_ADDR_IPv6, IOS_VPN @"/" IP_ADDR_IPv4, IOS_WIFI @"/" IP_ADDR_IPv6, IOS_WIFI @"/" IP_ADDR_IPv4, IOS_CELLULAR @"/" IP_ADDR_IPv6, IOS_CELLULAR @"/" IP_ADDR_IPv4 ] ;
    
    NSDictionary *addresses = [self getIPAddressesDic];
    //NSLog(@"addresses: %@", addresses);
    
    __block NSString *address;
    [searchArray enumerateObjectsUsingBlock:^(NSString *key, NSUInteger idx, BOOL *stop)
     {
         address = addresses[key];
         //篩選出IP地址格式
         if([self isValidatIP:address]) *stop = YES;
     } ];
    return address ? address : @"0.0.0.0";
}
+ (BOOL)isValidatIP:(NSString *)ipAddress {
    if (ipAddress.length == 0) {
        return NO;
    }
    NSString *urlRegEx = @"^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
    "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
    "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
    "([01]?\\d\\d?|2[0-4]\\d|25[0-5])$";
    
    NSError *error;
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:urlRegEx options:0 error:&error];
    
    if (regex != nil) {
        NSTextCheckingResult *firstMatch=[regex firstMatchInString:ipAddress options:0 range:NSMakeRange(0, [ipAddress length])];
        
        if (firstMatch) {
            NSRange resultRange = [firstMatch rangeAtIndex:0];
            NSString *result=[ipAddress substringWithRange:resultRange];
            //輸出結果
            NSLog(@"%@",result);
            return YES;
        }
    }
    return NO;
}
+ (NSDictionary *)getIPAddressesDic
{
    NSMutableDictionary *addresses = [NSMutableDictionary dictionaryWithCapacity:8];
    
    // retrieve the current interfaces - returns 0 on success
    struct ifaddrs *interfaces;
    if(!getifaddrs(&interfaces)) {
        // Loop through linked list of interfaces
        struct ifaddrs *interface;
        for(interface=interfaces; interface; interface=interface->ifa_next) {
            if(!(interface->ifa_flags & IFF_UP) /* || (interface->ifa_flags & IFF_LOOPBACK) */ ) {
                continue; // deeply nested code harder to read
            }
            const struct sockaddr_in *addr = (const struct sockaddr_in*)interface->ifa_addr;
            char addrBuf[ MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN) ];
            if(addr && (addr->sin_family==AF_INET || addr->sin_family==AF_INET6)) {
                NSString *name = [NSString stringWithUTF8String:interface->ifa_name];
                NSString *type;
                if(addr->sin_family == AF_INET) {
                    if(inet_ntop(AF_INET, &addr->sin_addr, addrBuf, INET_ADDRSTRLEN)) {
                        type = IP_ADDR_IPv4;
                    }
                } else {
                    const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6*)interface->ifa_addr;
                    if(inet_ntop(AF_INET6, &addr6->sin6_addr, addrBuf, INET6_ADDRSTRLEN)) {
                        type = IP_ADDR_IPv6;
                    }
                }
                if(type) {
                    NSString *key = [NSString stringWithFormat:@"%@/%@", name, type];
                    addresses[key] = [NSString stringWithUTF8String:addrBuf];
                }
            }
        }
        // Free memory
        freeifaddrs(interfaces);
    }
    return [addresses count] ? addresses : nil;
}

@end
