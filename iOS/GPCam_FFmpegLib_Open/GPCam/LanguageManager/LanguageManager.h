//
//  LanguageManager.h
//  ios_language_manager
//
//  Created by Maxim Bilan on 12/23/14.
//  Copyright (c) 2014 Maxim Bilan. All rights reserved.
//

#import <Foundation/Foundation.h>

//#define ChangeLanguageDynamically
#ifdef ChangeLanguageDynamically
    #define SettingLocalizedString( string , note ) NSLocalizedString( string, note)
#else
    #define SettingLocalizedString( string , note ) string
#endif


@interface LanguageManager : NSObject

+ (void)setupCurrentLanguage;
+ (NSArray *)languageStrings;
+ (NSString *)currentLanguageString;
+ (NSString *)currentLanguageCode;
+ (NSInteger)currentLanguageIndex;
+ (void)saveLanguageByIndex:(NSInteger)index;
+ (void)saveLanguageByString:(NSString *)TargetLanguage;
+ (BOOL)isCurrentLanguageRTL;

@end
